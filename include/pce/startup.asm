;
; STARTUP.ASM  -  MagicKit standard startup code
;

; first, set MOUSE to default on:
;
SUPPORT_MOUSE	.equ	1

		.list

		.ifdef HUC
		 .include "huc.inc"      ; HUC
		 .include "huc_opt.inc"
		.endif	; HUC

		.include  "standard.inc" ; HUCARD

; ----
; setup flexible boundaries for startup code
; and user program's "main".
;
START_BANK	.equ	0
LIB1_BANK	.equ	START_BANK
LIB2_BANK	.equ	START_BANK+1
	       .ifdef HUC
FONT_BANK	.equ	START_BANK+1

CONST_BANK	 .equ	START_BANK+2
DATA_BANK	 .equ	START_BANK+3

	       .else

; HuC (because of .proc/.endp) does not use MAIN_BANK
MAIN_BANK	.equ	START_BANK+2
	       .endif	; HUC


; ----
; if FONT_VADDR is not specified, then specify it
; (VRAM address to load font into)
;
	.ifndef	FONT_VADDR
FONT_VADDR	.equ	$0800
	.endif

; ----
; system variables
;
		.zp
zp_ptr1:	.ds 2


		.bss

	.if  (CDROM)	; CDROM def's in system.inc

		.include  "system.inc"

	.else  		; ie HuCard

		.org	$2200
user_jmptbl:		; user interrupt vectors
irq2_jmp:	.ds 2	; IRQ2 (BRK instruction and external IRQ)
irq1_jmp:	.ds 2	; IRQ1 (VDC interrupt)
timer_jmp:	.ds 2	; TIMER
nmi_jmp:	.ds 2	; NMI (unused)
vsync_hook:	.ds 2	; VDC vertical sync routine
hsync_hook:	.ds 2	; VDC horizontal sync rountine

bg_x1:		.ds 2
bg_x2:		.ds 2
bg_y1:		.ds 2
bg_y2:		.ds 2


		.org	$2227
joyena:		.ds 1	; soft reset enable (bit 0/pad 1, bit 1/pad2, etc.)
joy:		.ds 5	; 'current' pad values (pad #1-5)
joytrg:		.ds 5	; 'delta' pad values (new keys hit)
joyold:		.ds 5	; 'previous' pad values

		.org	$2241
irq_cnt:	.ds 1	; VDC interrupt counter; increased 60 times per second
			; reset to zero when vsync() function called
vdc_mwr:	.ds 1
vdc_dcr:	.ds 1

	.endif

		.org	$2244
scr_mode:	.ds 1	; screen mode and dimensions - set by <ex_scrmod>
scr_w:		.ds 1
scr_h:		.ds 1

		.org	$2284
soft_reset:	.ds 2	; soft reset jump loc (run+select)

		.org	$2680
vsync_cnt:	.ds 1	; counter for 'wait_vsync' routine

joybuf:		.ds 5	; 'delta' pad values collector
joyhook:	.ds 2	; 'read_joypad' routine hook
joycallback:	.ds 6	; joypad enhanced callback support
disp_cr:	.ds 1   ; display control (1 = on, 0 = off)
clock_hh	.ds 1	; system clock, hours since startup (0-255)
clock_mm	.ds 1	; system clock, minutes since startup (0-59)
clock_ss	.ds 1	; system clock, seconds since startup (0-59)
clock_tt	.ds 1	; system clock, ticks (1/60th sec) since startup (0-59)

joy6:		.ds 5	; second byte for 6-button joysticks
joytrg6:	.ds 5
joyold6:	.ds 5
joybuf6:	.ds 5

joytmp:		.ds 5
joytmp6:	.ds 5

	.if (CDROM)
ovl_running	.ds   1 ; overlay # that is currently running
cd_super	.ds   1 ; Major CDROM version #
irq_storea	.ds   1	; CDROM IRQ-specific handling stuff
irq_storeb	.ds   1
ram_vsync_hndl	.ds   25
ram_hsync_hndl	.ds   25
	.endif	; (CDROM)

	.ifdef HUC
user_vsync_hook	.ds	2
user_hsync_hook	.ds	2
user_sp_save	.ds	2
		.ds	32
user_irq_stack:
	.zp
user_irq_enable	.ds	1
	.endif

; [ STARTUP CODE ]

; Let's prepare this secondary libray bank first, for use later.
; The reason, as you will see, is because code for a given function
; which sits together in a file, may have things in zero-page,
; bss, LIB1_BANK (ie. START_BANK), and LIB2_BANK.
;
; The assembler must know beforehand what address etc. to use as a basis.
;
	.data
	.bank LIB2_BANK,"Base Library 2/Font"
	.org  $6000
	 .include "font.inc"
	.code
	.bank LIB2_BANK
	.org  $A600


	.data
	.bank CONST_BANK,"Constants"

	.data
	.bank DATA_BANK,"User Program"
	.org  $6000
;
; place overlay array here
; 50 entries, each containing
; 2 bytes for start sector,
; 2 bytes for # sectors
;
ovlarray:	.ds	200


	.code
	.bank START_BANK,"Base Library 1"

; A little introduction to the boot sequence:
;
; A HuCard has its origin at bank 0, and is mapped at $E000
; It needs to grab the interrupt vectors at $FFF6 and implement
; implement handlers for them
;
; A CDROM will load at bank $80 ($68 for SCD), and the initial
; loader will be mapped at $4000.  The current MagicKit sequence
; also maps $C000 to this same bank.  However, the initial boot
; sequence will execute at $4070, proceeding to load additional
; code and data, and then jump to a post-boot section called
; 'init_go'.  This is the point at which the loader explicitly
; relinquishes the $4000 segment.  It should be noted that there
; are library subroutines loaded as part of this initial segment,
; and those routines are located in the $C000 range as well.
;
; Sectors are loaded, up to and including the first "DATA_BANK",
; where the overlay array is stored - so that the CDROM error
; overlay can be located and executed in the event of a CDROM
; system version mismatch (ie. playing SCD games on CDROM)
;
; A second entry point is defined for overlays that are not
; being booted (ie. they are loaded and executed from another
; overlay).  This entry point is at $4000, after the segments
; have all found their natural loading spots (ie. segment $68
; for Super CDROMs).  This entry point maps the necessary
; segments and resets the stack, without clearing memory or
; performing other setup chores, and then maps and executes
; _main() to run the module.  The user has no choice regarding
; this function, although he can pass values through the global
; variables which main() can use to decide what to do next.
;
; An additional "Hook" area has now been defined at $4028,
; which is used at initial load time, in case a SCD overlay
; program is run on plain CDROM hardware, and the author
; wishes to override the default text error message by
; loading and executing a plain CDROM program instead
;
    ; ----
    ; interrupt vectors

       .if !(CDROM)
	.org  $FFF6

	.dw _irq2
	.dw _irq1
	.dw _timer
	.dw _nmi
	.dw _reset
       .endif	; !(CDROM)

    ; ----
    ; develo startup code

       .if (DEVELO)
	.org $6000

	sei
	map  _reset
	jmp  _reset
_restart:
	cla		; map the CD-ROM system bank
	tam   #7
	jmp   $4000	; back to the Develo shell
       .endif	; (DEVELO)


; ----
; reset
; ----
; things start here
; ----

    ; ----
    ; CDROM re-map library bank
    ;
;
; overlay entry point
;
; assume MMR0, MMR1, MMR6, MMR7 are set.
; set others & reset stack pointer
;
       .if (CDROM)
	.org $C000

; current overlay number that is running
; this is overwritten by the isolink prgram; the load
; statement must be the first in the block
;
ovlentry:
	lda  #1
	sta  ovl_running

	lda  #CONST_BANK+_bank_base
	tam  #2
	lda  #DATA_BANK+_bank_base
	tam  #3
	lda  #_call_bank
	tam  #4

	.ifndef SMALL
	stw  #$4000,<__sp
	.else
	stw  #$3f00,<__sp
	.endif
	ldx  #$ff
	txs

 .ifdef LINK_malloc
	__ldwi	__heap_start
	__pushw
	__ldwi	1024
	call	___malloc_init
 .endif

	map  _main
	jsr  _main
	bra  *

;
; CDROM error message alternate load entry point
;
	.org  $4028

cderr_override:		.db	0
cderr_overlay_num:	.db	0

cdrom_err_load:

	; since CDROM program will load into same area in RAM,
	; this load routine must be executed from scratch RAM
	; re-use the ram interrupt handler areas (not yet initialized)

	tii	.load_cd_ovl, ram_vsync_hndl, 64
	jmp	ram_vsync_hndl

.load_cd_ovl:
	lda	cderr_overlay_num
	asl	A
	asl	A
	tay
	lda	#DATA_BANK+$80
	tam	#3
	ldx	ovlarray,Y++
	lda	ovlarray,Y++
	stz	<_cl		; sector (offset from base of track)
	sta	<_ch
	stx	<_dl
	lda	ovlarray,Y
	sta	<_al		; # sectors
	lda	#$80
	sta	<_bl		; bank #
	lda	#3
	sta	<_dh		; MPR #
	jsr	cd_read
	cmp	#0
	bne	.error
	lda	#$80
	tam	#2
	jmp	_boot

.error:	jmp	cd_boot		; Can't load - reboot CDROM system card
	

;
; Proper Boot-time entry point
;
	.org  $4070
_boot:
	stz   $2680		; clear program RAM
	tii   $2680,$2681,$197F

;
; Note: All CDROM boot loaders will load into MMR $80 region
;       regardless of whether they are CD or SCD.
;       Here, we will move the information to occupy
;       base memory at MMR $68 if appropriate
;
	.if (CDROM = SUPER_CD)
	 jsr   ex_getver	; check if SCD program running
	 stx   cd_super		; on SCD hardware
	 cpx   #3		; don't copy to _bank_base if
	 bne   .nocopy		; memory doesn't exist there

	 lda   #_bank_base+1	; copy bank 2 to proper location
	 tam   #6
	 tii   $6000,$C000,$2000
	 tam   #3		; FONT_BANK now lives in SCD area ($69 exactly)
	 lda   #_bank_base	; then copy bank 1
	 tam   #6
	 tii   $4000,$C000,$2000 ; then load rest of program
.nocopy:
	.endif	; (CDROM = SUPER_CD)

       .else		; (ie. if HuCard...)

	.org  $E010
_reset:
	sei			; disable interrupts 
	csh			; select the 7.16 MHz clock
	cld			; clear the decimal flag 
	ldx   #$FF		; initialize the stack pointer
	txs 
	lda   #$FF		; map the I/O bank in the first page
	tam   #0
	lda   #$F8		; and the RAM bank in the second page
	tam   #1
	stz   $2000		; clear all the RAM
	tii   $2000,$2001,$1FFF

       .endif	; (CDROM)

    ; ----
    ; initialize the hardware

_init:
       .if (CDROM)
	jsr   ex_dspoff
	jsr   ex_rcroff
	jsr   ex_irqoff
	jsr   ad_reset
       .else
	stz   timer_ctrl	; init timer
       .endif	; (CDROM)

	jsr   init_psg		; init sound
	jsr   init_vdc		; init video
	lda   #$1F		; init joypad
	sta   joyena

    ; ----
    ; initialize interrupt vectors

       .if  (CDROM)
	jsr   ex_dspon
	jsr   ex_rcron
	jsr   ex_irqon
       .else

	ldx   #4		; user vector table
	cly
.l2:	lda   #LOW(_rti)
	sta   user_jmptbl,Y
	iny
	lda   #HIGH(_rti)
	sta   user_jmptbl,Y
	iny
	dex
	bne   .l2

	stw   #_reset,soft_reset ; soft reset
	stw   #_rts,vsync_hook   ; user vsync routine
	stw   #_rts,hsync_hook   ; user hsync routine

	lda   #$01		 ; enable interrupts
	sta   irq_disable
	stz   irq_status
	cli

    ; ----
    ; enable display and VSYNC interrupt

	vreg  #5
	lda   #$C8
	sta  <vdc_crl
	sta   video_data_l
	st2   #$00
	lda   #$01
	sta   disp_cr

       .endif	; (CDROM)

    ; ----
    ; init TIA instruction in RAM (fast BLiTter to hardware)

	lda   #$E3		; TIA instruction opcode
	sta   _ram_hdwr_tia
	lda   #$60		; RTS instruction opcode
	sta   _ram_hdwr_tia_rts

    ; ----
    ; init random number generator

	lda   #1
	jsr   wait_vsync	; wait for one frame & randomize _rndseed
	stw   #$03E7,<_cx	; set random seed
	stw   _rndseed,<_dx
	jsr   srand

       .if (CDROM)
	.if (CDROM = SUPER_CD)
	 lda   cd_super		; don't load the program if SCD
	 cmp   #3		; program not running on SCD hrdware
	 beq   loadprog
	 lda   cderr_override
	 lbeq  dontloadprog
	 jmp   cdrom_err_load
	.endif	; (SUPER_CD)

    ; ----
    ; load program
    ; ----
    ; CL/CH/DL = sector address
    ; DH = load mode - bank mode ($6000-$7FFF)
    ; BL = bank index
    ; AL = number of sectors
    ;
loadprog:
	lda   ovlentry+1	; current overlay (as written by ISOLINK)
	cmp   #1		; is it initial overlay ?
	lbne  _init_go		; if not initial overlay, somebody else already
				; loaded us completely; do not try to load remainder
				; (ie. executing CDROM error overlay)

	stz   <_cl		; initial boot doesn't load complete program;
	stz   <_ch		; prepare to load remainder
	lda   #10		; 10th sector (0-1 are boot;
				; 2-9 are this library...)
	sta   <_dl
	lda   #3		; load mode (consecutive banks; use MPR 3)
	sta   <_dh
	stw   #(_bank_base+2),<_bx	; 2 banks are boot/base library
	stw   #(_nb_bank-2)*4,<_ax
	jsr   cd_read
	cmp   #$00
	lbeq  _init_go

	; ----
	jmp   cd_boot		; reset


; This is the point in the CDROM loader where the code no longer
; executes in the $4000 segment, in favour of using the $C000
; segment (also used for the library subroutines)

       .org   $C130


; These routines will be run from RAM @ $2000 so we
; need to count bytes to determine how much to xfer
; (The total is 24 bytes, but we copy 25)

	.bank	LIB2_BANK

vsync_irq_ramhndlr:
	php			; 1 byte
	pha			; 1
	tma   #6		; 2
	sta   irq_storea	; 3
	lda   #BANK(_vsync_hndl) ;  2
	tam   #6		; 2
	pla			; 1
	pha			; 1
	jsr   _vsync_hndl	; 3
	lda   irq_storea	; 3
	tam   #6		; 2
	pla			; 1
	plp			; 1
	rts			; 1 = 24 bytes

hsync_irq_ramhndlr:
	php			; 1 byte
	pha			; 1
	tma   #6		; 2
	sta   irq_storeb	; 3
	lda   #BANK(_hsync_hndl) ;  2
	tam   #6		; 2
	pla			; 1
	pha			; 1
	jsr   _hsync_hndl	; 3
	lda   irq_storeb	; 3
	tam   #6		; 2
	pla			; 1
	plp			; 1
	rts			; 1 = 24 bytes

	.bank	LIB1_BANK

_init_go:
	.if (CDROM = SUPER_CD)
dontloadprog:
	.endif

       .endif	; (CDROM)

    ; ----
    ; jump to main routine

    ; ----
    ; load font

       .ifdef HUC
	.ifndef SMALL
	stw   #$4000,<__sp	; init stack ptr first
	.else
	stw   #$3f00,<__sp
	.endif

	stw   #FONT_VADDR,<_di	; Load Font @ VRAM addr

	;
	; this section of font loading was stolen
	; from _load_default_font because the default
	; FONT segment number is not yet guaranteed
	; if the SCD is being run on a plain CDROM system
	; so we need to trick the segment pointer
	; with a reliable one
	;
      __ldw   <_di	; stolen from _load_default_font
			; because segment# default not reliable

	jsr   _set_font_addr		; set VRAM

       .if  (CDROM)
	stb   #FONT_BANK+$80,<_bl	; guarantee FONT_BANK even if
					; SCD on regular CDROM system
       .else
	stb   #FONT_BANK+_bank_base,<_bl
       .endif

	stb   #96,<_cl
	stb   _font_color+1,<_ah
	lda   _font_color
	bne   .fntld
	inc   A
.fntld:	sta   <_al
	clx
	lda   font_table,X
	sta   <_si
	inx
	lda   font_table,X
	sta   <_si+1


	; Now, load the font
	;
	;   Note for CDROM/Super CDROM:
	;
	; The 'REAL' mapping for the lib2_load_font function
	; maybe doesn't exist yet (we are executing from bank $80,
	; not from $68 if it's a Super CDROM)
	;
	; So we must map the version at bank ($80 + LIB2_BANK)
	; before executing it.  We remap the bank after completion,
	; 'just in case'

       .if  (CDROM)
	tma   #page(lib2_load_font)
	pha
	lda   #LIB2_BANK+$80
	tam   #page(lib2_load_font)
	jsr   lib2_load_font
	pla
	tam   #page(lib2_load_font)
       .else
	jsr   load_font
       .endif

	;
	; END stolen font-load
	;

	jsr   _cls

	stz  color_reg	; set color #0 = 0/0/0 rgb
	stz  color_reg+1
	stz  color_data
	stz  color_data+1

	lda  #1		; set color #1 = 7/7/7 rgb
	sta  color_reg
	stz  color_reg+1
	ldx  #$ff
	stx  color_data
	sta  color_data+1

    ; ----
    ; Super CDROM error message
    ; ----

	.if (CDROM)
	 .if (CDROM = SUPER_CD)
	  lda  cd_super
	  cmp  #3
	  lbeq  .ok	; SCD running on Super system

	  lda	  #FONT_BANK+$80	; guarantee FONT_BANK even if
					; SCD on regular CDROM system
	  tam	  #PAGE(scdmsg1)

	  __stwi  <_si, scdmsg1
	  __ldwi  $0180
	  call    _put_string.2

	  __stwi  <_si, scdmsg2
	  __ldwi  $0200
	  call    _put_string.2

	  __stwi  <_si, scdmsg3
	  __ldwi  $0383
	  call    _put_string.2

	  __stwi  <_si, scdmsg4
	  __ldwi  $0403
	  call    _put_string.2

	  bra  *		; otherwise loop on blank screen

	  .bank	LIB2_BANK

scdmsg1:  .db  "This game was written for the"
	  .db  0
scdmsg2:  .db  "PC Engine Super CDROM System"
	  .db  0
scdmsg3:  .db  "Please use a PC Engine"
	  .db  0
scdmsg4:  .db  "Super CDROM System card"
	  .db  0

	  .bank LIB1_BANK

.ok:
	 .endif		; (CDROM = SUPER_CD)

	.endif		; (CDROM)
       .endif		; (HUC)


       .ifdef SUPPORT_MOUSE
	jsr  mousinit		; check existence of mouse
       .endif	; SUPPORT_MOUSE

       .if  (CDROM)

; Now, install the RAM-based version of the
; interrupt-handlers and activate them

	tma   #page(vsync_irq_ramhndlr)
	pha
	lda   #bank(vsync_irq_ramhndlr)
	tam   #page(vsync_irq_ramhndlr)
	tii   vsync_irq_ramhndlr,ram_vsync_hndl,25
	tii   hsync_irq_ramhndlr,ram_hsync_hndl,25
	pla
	tam   #page(vsync_irq_ramhndlr)

	stw   #ram_vsync_hndl,vsync_hook	; set VSYNC handler
	smb   #4,<irq_m		; enable new code
	smb   #5,<irq_m		; disable system card code

	stw   #ram_hsync_hndl,hsync_hook	; set HSYNC handler
	smb   #6,<irq_m		; enable new code
	smb   #7,<irq_m		; disable system card code

       .endif	; (CDROM)

       .ifdef HUC
    ; ----
    ; Map the final stuff before executing main()
    ; ----

	lda   #CONST_BANK+_bank_base	; map string constants bank
	tam   #2		; (ie. $4000-$5FFF)
	lda   #_call_bank	; map call bank
	tam   #4		; (ie. $8000-$9FFF)
	; ---
	.if   (CDROM)
	lda   #1		; first overlay to run at boot time
	sta   ovl_running	; store for later use
	.endif
	; ---
	stz   clock_hh		; clear clock
	stz   clock_mm
	stz   clock_ss
	stz   clock_tt

 .ifdef LINK_malloc
	__ldwi	__heap_start
	__pushw
	__ldwi	1024
	call	___malloc_init
 .endif

	map   _main
	jsr   _main 		; go!
	bra   *
       .else
	map   main
	jmp   main
       .endif	; HUC


; ----
; system
; ----
; give back control to the Develo system
; ----

       .if (DEVELO)
_system:
	sei
	csh
	cld
	ldx   #$FF	; stack
	txs 
	lda   #$FF	; I/O bank
	tam   #0
	lda   #$F8	; RAM bank
	tam   #1
	lda   #$80	; Develo Bank
	tam   #2
	tma   #7	; startup code bank
	tam   #3

    ; ----
    ; re-initialize the machine
    ;
	stz   $2000		; clear RAM
	tii   $2000,$2001,$1FFF
	stz   timer_ctrl	; init timer
	jsr   init_psg		; init sound
	jsr   init_vdc		; init video
	lda   #$1F		; init joypad
	sta   joyena
	lda   #$07		; set interrupt mask
	sta   irq_disable
	stz   irq_status		; reset timer interrupt
	lda   #$80		; disable sound driver
	sta   <$20E7
	st0   #5		; enable display and vsync interrupt
	lda   #$C8
	sta  <vdc_crl
	sta   video_data_l
	jmp   _restart		; restart
       .endif	; (DEVELO)

;±±±[ INTERRUPT CODE ]±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

_rts:
	rts
_rti:
	rti

; ----
; irq2
; ----
; IRQ2 interrupt handler
; ----

       .if !(CDROM)
_irq2:
	bbs0 <irq_m,.user
	rti
.user:
	jmp   [irq2_jmp]
       .endif	; !(CDROM)

; ----
; irq1
; ----
; VDC interrupt handler
; ----

       .if !(CDROM)

_irq1:
	bbs1 <irq_m,user_irq1	; jump to the user irq1 vector if bit set
	; --
	pha			; save registers
	phx
	phy
	; --
	lda   video_reg		; get VDC status register
	sta  <vdc_sr		; save a copy

    ; ----
    ; vsync interrupt
    ;
.vsync:
	bbr5 <vdc_sr,.hsync
	; --
	inc   irq_cnt		; increment IRQ counter
	; --
	st0   #5		; update display control (bg/sp)
	lda  <vdc_crl
	sta   video_data_l
	; --
	bbs5 <irq_m,.hsync
	; --
	jsr  _vsync_hndl
	; --
    ; ----
    ; hsync interrupt
    ;
.hsync:
	bbr2 <vdc_sr,.exit
	bbs7 <irq_m,.exit
	; --
	jsr  _hsync_hndl

    ; ----
    ; exit interrupt
    ;
.exit:
	lda  <vdc_reg		; restore VDC register index
	sta   video_reg
	; --
	ply
	plx
	pla
	rti

       .endif	; !(CDROM)
    ; ----
    ; user routine hooks
    ;
user_irq1:
	jmp   [irq1_jmp]
user_hsync:
	jmp   [user_hsync_hook]
user_vsync:
	jmp   [user_vsync_hook]



; ----
; _vsync_hndl
; ----
; Handle VSYNC interrupts
; ----
_vsync_hndl:
       .ifdef HUC
	bbr0 <user_irq_enable,.l4
	__ldw <__sp
	__stw user_sp_save
	stw   #user_irq_stack, <__sp
	jsr  user_vsync		; call user vsync routine
	__ldw user_sp_save
	__stw <__sp
.l4:
       .endif
       .if  !(CDROM)
	ldx   disp_cr		; check display state (on/off)
	bne  .l1
	and   #$3F		; disable display
	st0   #5		; update display control (bg/sp)
	sta   video_data_l
	bra  .l2
	; --	
       .endif
.l1:	jsr   rcr_init		; init display list

.l2:	st0   #7		; scrolling
	stw   bg_x1,video_data
	st0   #8
	stw   bg_y1,video_data

	; --
	lda   clock_tt		; keep track of time
	inc   A
	cmp   #60
	bne   .lcltt
	lda   clock_ss
	inc   A
	cmp   #60
	bne   .lclss
	lda   clock_mm
	inc   A
	cmp   #60
	bne   .lclmm
	inc   clock_hh
	cla
.lclmm:	sta   clock_mm
	cla
.lclss:	sta   clock_ss
	cla
.lcltt:	sta   clock_tt
	; --

       .if  (CDROM)
	jsr   ex_colorcmd
	inc   rndseed
	jsr   randomize
       .endif

       .ifdef SUPPORT_MOUSE
	lda   msflag		; if mouse supported, and exists
	beq  .l3		; then read mouse instead of pad
	jsr   mousread
	bra  .out
       .endif	; SUPPORT_MOUSE

.l3:	jsr   read_joypad	; else read joypad
.out:	rts


; ----
; _hsync_hndl
; ----
; Handle HSYNC interrupts
; ----
    ; ----
    ; hsync scrolling handler
    ;
_hsync_hndl:
	bbr1 <user_irq_enable,.l1
	jsr  user_hsync		; call user handler
.l1:	ldy   s_idx
	bpl  .r1
	; --
	lda  <vdc_crl
	and   #$3F
	sta  <vdc_crl
	stz   s_idx
	ldx   s_list
	lda   s_top,X
	jsr   rcr5
	rts
	; --
.r1:	ldx   s_list,Y
	lda  <vdc_crl
	and   #$3F
	ora   s_cr,X
	sta  <vdc_crl
	; --
	jsr   rcr_set
	; --
	lda   s_top,X
	cmp   #$FF
	beq  .out
	; --
	st0   #7
	lda   s_xl,X
	ldy   s_xh,X
	sta   video_data_l
	sty   video_data_h
	st0   #8
	lda   s_yl,X
	ldy   s_yh,X
	sub   #1
	bcs  .r2
	dey
.r2:	sta   video_data_l
	sty   video_data_h
.out:	rts

    ; ----
    ; init display list
    ;
rcr_init:
	maplibfunc   build_disp_list
	bcs  .r3
	rts
	; --
.r3:	smb   #7,<vdc_crl
	lda   #$FF
	sta   s_idx
	ldx   s_list
	ldy   s_top,X
	cpy   #$FF
	bne   rcr5
	; --
	ldy   s_xl,X
	sty   bg_x1
	ldy   s_xh,X
	sty   bg_x1+1
	ldy   s_yl,X
	sty   bg_y1
	ldy   s_yh,X
	sty   bg_y1+1
	stz   s_idx
	bra   rcr5

    ; ----
    ; program scanline interrupt
    ;
rcr_set:
	iny
	sty   s_idx
	lda   s_list,Y
	tay
	lda   s_top,Y
	cmp   scr_height
	bhs   rcr6
	cmp   s_bottom,X
	blo   rcr5
	; --
	lda   s_bottom,X
rcr4:	dec   A
	pha
	lda   #$F0
	sta   s_bottom,X
	stz   s_cr,X
	dec   s_idx
	pla
	; --
rcr5:	st0   #6		; set scanline counter
	add   #64
	sta   video_data_l
	cla
	adc   #0
	sta   video_data_h
	bra   __rcr_on
	;--
rcr6:	lda   s_bottom,X
	cmp   scr_height
	blo   rcr4
	bra   __rcr_off

; ----
; rcr_on
; ----
; enable scanline interrupt
; ----

  rcr_on:
 _rcr_on:
 	lda   #5
	sta  <vdc_reg
__rcr_on:
	st0   #5
	lda  <vdc_crl
	ora   #$04
	sta  <vdc_crl
	sta   video_data_l
	rts

; ----
; rcr_off
; ----
; disable scanline interrupt
; ----

  rcr_off:
 _rcr_off:
	lda   #5
	sta  <vdc_reg
__rcr_off:
	st0   #5
	lda  <vdc_crl
	and   #$FB
	sta  <vdc_crl
	sta   video_data_l
	rts



; ----
; timer
; ----
; timer interrupt handler
; ----

       .if  !(CDROM)
_timer_user:
	jmp   [timer_jmp]
_timer:
	bbs2 <irq_m,_timer_user
	pha
	phx
	phy

	sta   irq_status	; acknowledge interrupt

.exit:	ply
	plx
	pla
	rti

       .endif	; !(CDROM)

; ----
; nmi
; ----
; NMI interrupt handler
; ----

       .if  !(CDROM)
_nmi:
	bbs3 <irq_m,.user
	rti
.user:
	jmp   [nmi_jmp]

       .endif	; !(CDROM)


;±±[ DATA ]±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

; ----
; font
; ----

	.ifdef HUC
font_table:
	 .dw font_1
	 .dw font_2
	 .dw font_1
	 .dw font_1

	.endif	; HUC


;±±[ LIBRARY ]±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

; ----
; standard library
; ----

	.include "library.asm"
	.include "scroll.asm"
	.include "math.asm"

       .ifdef HUC
	.include "huc.asm"
	.include "huc_gfx.asm"
	.include "huc_math.asm"
	.include "huc_bram.asm"
	.include "huc_misc.asm"
       .endif	; HUC

       .ifdef SUPPORT_MOUSE
        .include "mouse.asm"
       .endif	; SUPPORT_MOUSE

       .if (CDROM)
	.include "cdrom.asm"
       .else
       .endif   ; CDROM

; ----
; disp_on
; ----
; enable display
; ----

       .ifdef HUC
_disp_on:
	ldx   disp_cr
	lda   #1
	sta   disp_cr
	cla
	rts
       .else
 disp_on:
	lda   #1
	sta   disp_cr
	rts
       .endif	; HUC

; ----
; disp_off
; ----
; disable display
; ----

       .ifdef HUC
_disp_off:
	ldx   disp_cr
	stz   disp_cr
	cla
	rts
       .else
 disp_off:
	stz   disp_cr
	rts
       .endif	; HUC

; ----
; set_intvec
; ----
; set interrupt vector
; ----
; IN : A = vector number
;           0 IRQ2
;           1 IRQ1 (VDC)
;           2 TIMER
;           3 NMI
;           4 VSYNC
;           5 HSYNC
;           6 SOFT RESET (RUN + SELECT)
;      X = vector address low byte
;      Y =   "      "    high byte
; ----

       .if  !(CDROM)

set_intvec:
	php
	sei
	cmp   #6
	blo  .vector
	bne  .exit
.reset:
	stx   soft_reset
	sty   soft_reset+1
	bra  .exit	
.vector:
	pha
	asl   A
	sax
	sta   user_jmptbl,X
	inx
	tya
	sta   user_jmptbl,X
	pla
.exit:
	plp
	rts
       .endif	; !(CDROM)

; ----
; wait_vsync
; ----
; wait the next vsync
; ----
; IN :  A = number of frames to be sync'ed on
; ----
; OUT:  A = number of elapsed frames since last call
; ----

wait_vsync:
	bbr1 <irq_m,.l1
	cla			; return immediately if IRQ1 is redirected
       .ifdef HUC
	clx
       .endif
	rts

.l1:	sei			; disable interrupts
	cmp   irq_cnt		; calculate how many frames to wait
	beq  .l2
	bhs  .l3
	lda   irq_cnt
.l2:	inc   A
.l3:	sub   irq_cnt
	sta   vsync_cnt
	cli			; re-enable interrupts

.l4:	lda   irq_cnt		; wait loop
.l5:	incw  _rndseed
	cmp   irq_cnt
	beq  .l5
	dec   vsync_cnt
	bne  .l4

	stz   irq_cnt		; reset system interrupt counter
	inc   A			; return number of elapsed frames

       .ifndef HUC
        rts
       .else

    ; ----
    ; callback support

	pha
	lda   joycallback	; callback valid?
	bpl  .t3
	bit   #$01
	bne  .t3

	lda   joycallback+1	; get events for all the 
	beq  .t3		; selected joypads
	sta  <_al
	cly
	cla
.t1:    lsr  <_al
	bcc  .t2
	ora   joybuf,Y
.t2:	iny
	cpy   #5
	blo  .t1

	and   joycallback+2	; compare with requested state
	beq  .t3

	inc   joycallback	; lock callback feature
	tax			; call user routine
	tma   #5
	pha
	lda   joycallback+3
	tam   #5
	cla
	jsr  .callback
	pla
	tam   #5
	dec   joycallback	; unlock
	; --
.t3:	plx
	cla
	rts

    ; ----
    ; user routine callback
    ;
.callback:
	jmp   [joycallback+4]
       .endif	; ndef HUC

       .include  "joypad.asm"	; read joypad values


;±±[ USER PROGRAM ]±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;	.nomlist
;	.list

	;...

;	.endif

