;
; SGX_GFX.ASM - HuC SGX graphic library
;
; Tomaitheous '07


; ----
; local variables

	.zp
__sgx_detect	.ds 1
__sgx_spr_ptr	.ds 2
__sgx_spr_max	.ds 1
__sgx_spr_flag	.ds 1
_sgx_scroll_y	.ds 2
_sgx_scroll_x	.ds 2
sgx_vdc_sts	.ds 1
sgx_vdc_reg	.ds 1
sgx_vdc_cr	.ds 2

	.bss
_sgx_vdc		.ds 20*2
__sgx_init_detect	.ds 1
_sgx_satb		.ds 512

vpc_win_a_size	.ds 2
vpc_win_b_size	.ds 2
vpc_win_blk_1	.ds 1
vpc_win_blk_2	.ds 1

sgx_mapbank		.ds 1
sgx_mapaddr		.ds 2
sgx_mapwidth		.ds 2
sgx_mapheight		.ds 2
sgx_maptiletype		.ds 1
sgx_maptilebank		.ds 1
sgx_maptileaddr		.ds 2
sgx_maptilebase		.ds 2
sgx_mapnbtile		.ds 2
sgx_mapctablebank	.ds 1
sgx_mapctable		.ds 2
sgx_mapwrap		.ds 1
sgx_mapbat_ptr		.ds 2
sgx_mapbat_top_base	.ds 2
sgx_mapbat_top		.ds 1
sgx_mapbat_bottom	.ds 1
sgx_mapbat_x		.ds 2

sgx_bat_width	.ds 2
sgx_bat_height	.ds 1
sgx_bat_hmask	.ds 1
sgx_bat_vmask	.ds 1
sgx_scr_width	.ds 1
sgx_scr_height	.ds 1


;
; SGX_VREG - set up video register to be read/written
;

.macro sgx_vreg
	lda	\1
	sta	<sgx_vdc_reg
.if (\?1 = ARG_IMMED)
	sta	$0010
.else
	sta	sgx_video_reg
.endif
.endm


; ----
; library code

	.code

; [library 3 code]

.include "lib3_versions.asm"


; [SPRITE CODE]

; sgx_spr_set(char num)
; ----
; load SI with the offset of the sprite to change
; SI = satb + 8 * sprite_number
; ----

_sgx_spr_set.1:
	maplibfunc	lib2_sgx_spr_set
	rts

	.bank	LIB2_BANK
lib2_sgx_spr_set:
	cpx	#64
	bhs	.l2
	txa
	inx
	cpx	<__sgx_spr_max
	blo	.l1
	stx	<__sgx_spr_max
	; --
.l1:	stz	<__sgx_spr_ptr+1
	asl a
	asl a
	asl a
	rol	<__sgx_spr_ptr+1
	adc	#low(_sgx_satb)
	sta	<__sgx_spr_ptr
	lda	<__sgx_spr_ptr+1
	adc	#high(_sgx_satb)
	sta	<__sgx_spr_ptr+1
.l2:	rts
	.bank	LIB1_BANK


; spr_hide(char num)
; ----
_sgx_spr_hide:
	cly
	bra	sgx_spr_hide
_sgx_spr_hide.1:
	ldy	#$01
sgx_spr_hide:
	maplibfunc_y	lib2_sgx_spr_hide
	rts

	.bank	LIB2_BANK
lib2_sgx_spr_hide
	cpy	#0
	beq	.l2
	; -- hide sprite number #
	cpx	#64
	bhs	.l1
	jsr	_sgx_spr_hide.sub
	lda	[__ptr],Y
	ora	#$02
	sta	[__ptr],Y
.l1:	rts
	; -- hide current sprite
.l2:	ldy	#1
	lda	[__sgx_spr_ptr],Y
	ora	#$02
	sta	[__sgx_spr_ptr],Y
	rts
	; -- calc satb ptr
_sgx_spr_hide.sub:
	txa
	stz	<__ptr+1
	asl a
	asl a
	asl a
	rol	<__ptr+1
	adc	#low(_sgx_satb)
	sta	<__ptr
	lda	<__ptr+1
	adc	#high(_sgx_satb)
	sta	<__ptr+1
	ldy	#1
	rts

	.bank	LIB1_BANK


; spr_show(char num)
; ----
_sgx_spr_show:
	cly
	bra	sgx_spr_show
_sgx_spr_show.1:
	ldy	#$01
sgx_spr_show:
	maplibfunc_y	lib2_sgx_spr_show
	rts

	.bank	LIB2_BANK
lib2_sgx_spr_show
	cpy	#0
	beq	.l2
	; -- hide sprite number #
	cpx	#64
	bhs	.l1
	jsr	_sgx_spr_hide.sub
	lda	[__ptr],Y
	and	#$01
	sta	[__ptr],Y
.l1:	rts
	; -- hide current sprite
.l2:	ldy	#1
	lda	[__sgx_spr_ptr],Y
	and	#$01
	sta	[__sgx_spr_ptr],Y
	rts
	.bank	LIB1_BANK


; sgx_satb_update()
; ----

_sgx_satb_update.1:
	ldy	#1
	bra	sgx_satb_update
_sgx_satb_update:
	cly
sgx_satb_update:
	maplibfunc_y	lib2_sgx_satb_update
	rts

	.bank	LIB2_BANK
lib2_sgx_satb_update:
	lda	<__sgx_spr_flag
	beq	.l1
	stz	<__sgx_spr_flag
	ldx	#64
	bra	.l3
	; --
.l1:	cpy	#1
	beq	.l2
	ldx	<__sgx_spr_max
.l2:	cpx	#0
	beq	.l4
	; --
.l3:	stx	<_al	; number of sprites
	txa
	dec a		; round up to the next group of 4 sprites
	lsr a
	lsr a
	inc a
	sta	<_cl

; Use TIA, but BLiT 16 words at a time (32 bytes)
; Because interrupt must not deferred too much
;
	stw	#32, _ram_hdwr_tia_size
	stw	#sgx_video_data, _ram_hdwr_tia_dest
	stw	#_sgx_satb, <_si

	stw	#$7F00, <_di
	jsr	sgx_set_write

.l3a:	stw	<_si, _ram_hdwr_tia_src
	jsr	_ram_hdwr_tia
	addw	#32,<_si
	dec	<_cl
	bne	.l3a

;.l3:	stx	<_al
;	stw	#_sgx_satb,<_si
;	stb	#BANK(_sgx_satb),<_bl
;	stw	#$7F00,<_di
;	txa
;	stz	<_ch
;	asl a
;	asl a
;	rol	<_ch
;	sta	<_cl
;	jsr	sgx_load_vram

	; --
	ldx	<_al
.l4:	cla
	rts
	.bank	LIB1_BANK


; init_satb()
; reset_satb()
; ----

_sgx_reset_satb:
_sgx_init_satb:
	clx
	cla
.l1:	stz	_sgx_satb,X
	stz	_sgx_satb+256,X
	inx
	bne	.l1
	; --
	ldy	#1
	sty	<__sgx_spr_flag
	stz	<__sgx_spr_max
	rts

_sgx_spr_x:
	cly
	bra	sgx_lib2_group_1
_sgx_spr_y:
	ldy	#1
	bra	sgx_lib2_group_1
_sgx_spr_get_x:
	ldy	#2
	bra	sgx_lib2_group_1
_sgx_spr_get_y:
	ldy	#3
sgx_lib2_group_1:
	maplibfunc_y	lib2_group_1
	rts

	.bank	LIB2_BANK
lib2_group_1:
	cpy	#$01
	beq	lib2_sgx_spr_y
	bcc	lib2_sgx_spr_x
	cpy	#$03
	bcc	lib2_sgx_spr_get_x
	bcs	lib2_sgx_spr_get_y


; sgx_spr_x(int value)
; ----

lib2_sgx_spr_x:
	ldy	#2
	sax
	add	#32
	sta	[__sgx_spr_ptr],Y
	sax
	adc	#0
	iny
	sta	[__sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_x:
	ldy	#2
	lda	[__sgx_spr_ptr],Y
	sub	#32
	tax
	iny
	lda	[__sgx_spr_ptr],Y
	sbc	#0
	rts


; sgx_spr_y(int value)
; ----

lib2_sgx_spr_y:
	sax
	add	#64
	sta	[__sgx_spr_ptr]
	sax
	adc	#0
	and	#$01
	ldy	#1
	sta	[__sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_y:
	lda	[__sgx_spr_ptr]
	sub	#64
	tax
	ldy	#1
	lda	[__sgx_spr_ptr],Y
	sbc	#0
	rts
	.bank	LIB1_BANK


_sgx_spr_pattern:
	cly
	bra	sgx_lib2_group_2
_sgx_spr_get_pattern:
	ldy	#1
	bra	sgx_lib2_group_2
_sgx_spr_ctrl.2:
	ldy	#2
	bra	sgx_lib2_group_2
_sgx_spr_pal:
	ldy	#3
	bra	sgx_lib2_group_2
_sgx_spr_get_pal:
	ldy	#4
	bra	sgx_lib2_group_2
_sgx_spr_pri:
	ldy	#5
sgx_lib2_group_2:
	maplibfunc_y	lib2_group_2
	rts

	.bank	LIB2_BANK
lib2_group_2:
	cpy	#01
	beq	lib2_sgx_spr_get_pattern
	bcc	lib2_sgx_spr_pattern
	cpy	#03
	bcc	lib2_sgx_spr_ctrl.2
	beq	lib2_sgx_spr_pal
	cpy	#05
	bcc	lib2_sgx_spr_get_pal
	bra	lib2_sgx_spr_pri


; sgx_spr_pattern(int vaddr)
; ----

lib2_sgx_spr_pattern:
	sta	<__temp
	txa
	asl a
	rol	<__temp
	rol a
	rol	<__temp
	rol a
	rol	<__temp
	rol a
	and	#$7
	ldy	#5
	sta	[__sgx_spr_ptr],Y
	lda	<__temp
	dey
	sta	[__sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_pattern:
	ldy	#4
	lda	[__sgx_spr_ptr],Y
	sta	<__temp
	iny
	lda	[__sgx_spr_ptr],Y
	lsr a
	ror	<__temp
	ror a
	ror	<__temp
	ror a
	ror	<__temp
	ror a
	and	#$E0
	tax
	lda	<__temp
	rts


; sgx_spr_ctrl(char mask [al], char value)
; ----

lib2_sgx_spr_ctrl.2:
	txa
	and	<_al
	sta	<__temp
	lda	<_al
	eor	#$FF
	ldy	#7
	and	[__sgx_spr_ptr],Y
	ora	<__temp
	sta	[__sgx_spr_ptr],Y
	rts


; sgx_spr_pal(char pal)
; ----

lib2_sgx_spr_pal:
	txa
	and	#$0F
	sta	<__temp
	ldy	#6
	lda	[__sgx_spr_ptr],Y
	and	#$F0
	ora	<__temp
	sta	[__sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_pal:
	ldy	#6
	lda	[__sgx_spr_ptr],Y
	and	#$0F
	tax
	cla
	rts


; sgx_spr_pri(char pri)
; ----

lib2_sgx_spr_pri:
	ldy	#6
	lda	[__sgx_spr_ptr],Y
	and	#$7F
	cpx	#$00
	beq	.l1
	ora	#$80
.l1:
	sta	[__sgx_spr_ptr],Y
	rts
	.bank	LIB1_BANK


; [VDC CODE]

; sgx_detect()
; ----
; returns 1 if true and 0 if false
; ----
_sgx_detect:
	ldx	__sgx_init_detect
	lda	__sgx_init_detect
	rts


; sgx_vreg(char reg)
; ----
; sgx_vreg(char reg, int data)
; ----

_sgx_vreg.1:
	stx	<sgx_vdc_reg
	stx sgx_video_reg
	rts

_sgx_vreg.2:
	ldx	<_al
	stx	<sgx_vdc_reg
	stx sgx_video_reg

	lda	<_cl
	sta sgx_video_data
	lda	<_ch
	sta sgx_video_data+1
	rts


; ----
; sgx_set_write
; ----
; set the SGX VDC VRAM write pointer
; ----
; IN :	_DI = VRAM location
; ----

sgx_set_write:
	lda	#$00
	sta sgx_video_reg
	lda	<_di
	sta	sgx_video_data_l
.ifdef HUC
	sta	_sgx_vdc
.endif
	lda	<_di+1
	sta	sgx_video_data_h
.ifdef HUC
	sta	_sgx_vdc+1
.endif
	lda	#$02
	sta sgx_video_reg
	rts


; sgx_load_vram
; ----
; copy a block of memory to SGX VRAM
; ----
; IN :	_DI = VRAM location
;	_BL = data bank
;	_SI = data memory location
;	_CX = number of words to copy
; ----

sgx_load_vram:
_sgx_load_vram.3:
	maplibfunc_y	lib2_sgx_load_vram.3
	rts

	.bank	LIB2_BANK
; sgx_load_vram
; ----
lib2_sgx_load_vram.3:
	; ----
	; map data
	;
	jsr	map_data

	; ----
	; setup call to TIA operation (fastest transfer)
	;
	; (instruction setup done during bootup...)

	stw	#sgx_video_data, _ram_hdwr_tia_dest
;	stw	<_si, _ram_hdwr_tia_src
;
;	asl	<_cl	; change from words to bytes (# to xfer)
;	rol	<_ch

	; ----
	; set vram address
	;
	jsr	sgx_set_write

	; ----
	; copy data
	;
	cly
	ldx	<_cl
	beq	.l3
	; --
.l1:	lda	[_si],Y
	sta	sgx_video_data_l
	iny
	lda	[_si],Y
	sta	sgx_video_data_h
	iny
	bne	.l2
	inc	<_si+1
	; --
.l2:	dex
	bne	.l1
	; --
	jsr	remap_data
	; --
.l3:	dec	<_ch
	bpl	.l1

;.l1:	lda	<_ch	; if zero-transfer, exit
;	ora	<_cl
;	beq	.out
;
;	lda	<_ch
;	cmp	#$20	; if more than $2000, repeat xfers of $2000
;	blo	.l2	; while adjusting banks
;	sub	#$20	; reduce remaining transfer amount
;	sta	<_ch
;
;	stw	#$2000, _ram_hdwr_tia_size
;	jsr	_ram_hdwr_tia
;
;	lda	<_si+1	; force bank adjust
;	add	#$20	; and next move starts at same location
;	sta	<_si+1
;
;	jsr	remap_data	; adjust banks
;	bra	.l1
;
;.l2:	sta	HIGH_BYTE _ram_hdwr_tia_size	; 'remainder' transfer of < $2000
;	lda	<_cl
;	sta	LOW_BYTE  _ram_hdwr_tia_size
;	jsr	_ram_hdwr_tia

	; ----
	; unmap data
	;
.out:
	; restore PCE VDC address
	stw	#video_data, _ram_hdwr_tia_dest
	jmp	unmap_data

	.bank	LIB1_BANK


; init_sgx_vdc()
; ----
; same as init_vdc except it initializes SGX VDC.
; ----

init_sgx_vdc:
	maplibfunc	lib2_init_sgx_vdc
	rts

	.bank	LIB2_BANK
; init_sgx_vdc()
; ----

lib2_init_sgx_vdc:
	stw	#_sgx_init_point,<_si	; register table address in '_si'
	cly

__sgx_init_LL1:
	lda	[_si],y
	sta sgx_video_reg
	iny
	lda	[_si],y
	sta sgx_video_data
	iny
	lda	[_si],y
	sta sgx_video_data+1
	iny
	cpy	#$24
	bne	__sgx_init_LL1

	; ----
	; clear the video RAM
	;
	stz	$0010
	stz	$0012
	stz	$0013
	lda	#2
	sta	$0010

	ldx	#128
.l2:	cly
.l3:	stz	$0012
	stz	$0013
	dey
	bne	.l3
	dex
	bne	.l2

	stz	$0010	; Write SGX2 to VDC 2 vram
	stz	$0012
	stz	$0013
	lda	#2
	sta	$0010
	lda	#$53
	sta	$0012
	lda	#$47
	sta	$0013
	lda	#$58
	sta	$0012
	lda	#$32
	sta	$0013

	stz vdc_reg	; write PCE1 to VDC 1 vram
	st0	#0
	st1	#0
	st2	#0
	lda	#2
	sta vdc_reg
	st0	#2
	st1	#$50
	st2	#$43
	st1	#$45
	st2	#$31

	lda	#1
	sta	$0010
	stz	$0012
	stz	$0013
	lda	#2
	sta	$0010

	lda	$0012
	cmp	#$53
	bne	.no_sgx
	lda	$0013
	cmp	#$47
	bne	.no_sgx
	lda	$0012
	cmp	#$58
	bne	.no_sgx
	lda	$0013
	cmp	#$32
	bne	.no_sgx
	bra	.yes_sgx
.no_sgx:
	stz	__sgx_init_detect
	rts
.yes_sgx:
	lda	#1
	sta	__sgx_init_detect

	;disable display and interrupts and update offline status
	stz	<sgx_vdc_cr
	stz	<sgx_vdc_cr+1
	lda	#$05
	sta	<sgx_vdc_sts
	sta sgx_videoport
	stz sgx_video_data
	stz sgx_video_data+1

	;reset scroll registers
	stz	<_al
	stz	<_ah
	stz	<_bl
	stz	<_bh
	jsr	_sgx_scroll.2

	rts

	.bank	LIB1_BANK

_sgx_disp_on:
	cly
	bra	lib2_group_4
_sgx_disp_off:
	ldy	#1
	bra	lib2_group_4
_sgx_spr_on:
	ldy	#2
	bra	lib2_group_4
_sgx_spr_off:
	ldy	#3
	bra	lib2_group_4
_sgx_bg_on:
	ldy	#4
	bra	lib2_group_4
_sgx_bg_off:
	ldy	#5

lib2_group_4:
	maplibfunc_y	lib2_group_4_case
	rts

	.bank	LIB2_BANK
lib2_group_4_case:
	cpy	#1
	bcc	lib2_group_4_lbl0
	beq	lib2_group_4_lbl1
	cpy	#3
	bcc	lib2_group_4_lbl2
	beq	lib2_group_4_lbl3
	cpy	#5
	bcc	lib2_group_4_lbl4
	bcs	lib2_group_4_lbl5

lib2_group_4_lbl0:
	jmp lib2_sgx_disp_on
lib2_group_4_lbl1:
	jmp lib2_sgx_disp_off
lib2_group_4_lbl2:
	jmp lib2_sgx_spr_on
lib2_group_4_lbl3:
	jmp lib2_sgx_spr_off
lib2_group_4_lbl4:
	jmp lib2_sgx_bg_on
lib2_group_4_lbl5:
	jmp lib2_sgx_bg_off


; sgx_disp_on()
; ----
; Turns on SGX BG and SPR. Interrupts are disabled.
; ----

lib2_sgx_disp_on:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	ora	#$C0
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_disp_off()
; ----
; Turns off SGX BG and SPR. Interrupts are disabled.
; ----

lib2_sgx_disp_off:
	lda	#$05
	sta sgx_video_reg
	cla
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_spr_on()
; ----
; Turns on SGX SPR. Interrupts are disabled.
; ----

lib2_sgx_spr_on:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	ora	#$40
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_spr_off()
; ----
; Turns off SGX SPR. Interrupts are disabled.
; ----

lib2_sgx_spr_off:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	and	#$BF
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_bg_on()
; ----
; Turns on SGX BG. Interrupts are disabled.
; ----

lib2_sgx_bg_on:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	ora	#$80
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_bg_off()
; ----
; Turns off SGX BG. Interrupts are disabled.
; ----

lib2_sgx_bg_off:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	and	#$7F
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts

	.bank	LIB1_BANK

; [VPC CODE]

; vpc_win_size( char window_num (& 0x01) , int size );
; ----
;	Change the VPC window size
; ----
;
_vpc_win_size.2:
	lda	<_al
	and	#$01
	asl a
	tay
	lda	<_bl
	sta	$000A, y
	lda	<_bh
	sta	$000A+1, y
	rts


; vpc_win_reg( char window_num (& 0x01) , char var );
; ----
;	Change the VPC window control settings: Sprite priorities and enable/disable VDCs
;	based on windows configuration.
; ----
;
_vpc_win_reg.2:
	maplibfunc_y	lib2_vpc_win_reg.2
	rts

	.bank	LIB2_BANK
; ----
lib2_vpc_win_reg.2:
	lda	<_al
	beq	.win_a
	cmp	#$01
	beq	.win_b
	cmp	#$02
	beq	.win_ab
	;else

.win_none:
	lda	<_bl
	asl a
	asl a
	asl a
	asl a
	sta	<_bl

	lda vpc_win_blk_2
	and	#0x0F
	clc
	adc	<_bl
	sta vpc_win_blk_2
	sta vpc_ctrl_2
	rts

.win_a:
	lda	<_bl
	and	#$0F
	sta	<_bl

	lda vpc_win_blk_2
	and	#0xF0
	clc
	adc	<_bl
	sta vpc_win_blk_2
	sta vpc_ctrl_2
	rts

.win_b:
	lda	<_bl
	asl a
	asl a
	asl a
	asl a
	sta	<_bl

	lda vpc_win_blk_1
	and	#0x0F
	clc
	adc	<_bl
	sta vpc_win_blk_1
	sta vpc_ctrl_1
	rts

.win_ab:
	lda	<_bl
	and	#$0F
	sta	<_bl

	lda vpc_win_blk_1
	and	#0xF0
	clc
	adc	<_bl
	sta vpc_win_blk_1
	sta vpc_ctrl_1
	rts

	rts
	.bank	LIB1_BANK


; [BACKGROUND CODE]

; sgx_load_map(char x [al], char y [ah], int mx, int my, char w [dl], char h [dh])
; ----

_sgx_load_map.6:
	maplibfunc_y	lib2_sgx_load_map.6
	rts

	.bank	LIB2_BANK
lib2_sgx_load_map.6
	tstw	sgx_mapwidth
	beq	.l6
	tstw	sgx_mapheight
	beq	.l6

	; ----
	; adjust map y coordinate
	;
	lda	<_bh
	bmi	.l2
.l1:	cmpw	sgx_mapheight,<_bx
	blo	.l3
	subw	sgx_mapheight,<_bx
	bra	.l1
	; --
.l2:	lda	<_bh
	bpl	.l3
	addw	sgx_mapheight,<_bx
	bra	.l2

	; ----
	; adjust map x coordinate
	;
.l3:	stb	<_bl,<_ch
	lda	<_di+1
	bmi	.l5
.l4:	cmpw	sgx_mapwidth,<_di
	blo	.l7
	subw	sgx_mapwidth,<_di
	bra	.l4
	; --
.l5:	lda	<_di+1
	bpl	.l7
	addw	sgx_mapwidth,<_di
	bra	.l5

	; ----
	; exit
	;
.l6:	rts

	; ----
	; ok
	;
.l7:	stb	<_di,<_cl
	jmp	sgx_load_map

.include "sgx_load_map.asm"

	.bank	LIB1_BANK


;	sgx_scroll( int X [ax], int Y [bx])
; ----
;	updates the SGX background scroll registers
; ----
;
_sgx_scroll.2:
	maplibfunc_y	lib2_sgx_scroll.2
	rts

	.bank	LIB2_BANK
; ----
lib2_sgx_scroll.2:
	lda	#$07
	sta	<sgx_vdc_sts
	sta sgx_video_reg
	lda	<_al
	sta sgx_video_data_l
	sta	<_sgx_scroll_x
	lda	<_ah
	sta sgx_video_data_h
	sta	<_sgx_scroll_x+1

	lda	#$08
	sta	<sgx_vdc_sts
	sta sgx_video_reg
	lda	<_bl
	sta sgx_video_data_l
	sta	<_sgx_scroll_y
	lda	<_bh
	sta sgx_video_data_h
	sta	<_sgx_scroll_y+1

	rts
	.bank	LIB1_BANK


; sgx_load_bat(int vaddr, int *bat_data, char w, char h)
; ----
; load_bat equiv for SGX
; ----
_sgx_load_bat.4:
	maplibfunc_y	lib2_sgx_load_bat
	rts

	.bank	LIB2_BANK

; sgx_load_bat(int vaddr, int *bat_data, char w, char h)
; ----

lib2_sgx_load_bat:
	; ----
	; map data
	;
	jsr	map_data

	; ----
	; copy BAT
	;
	cly
	; --
.l1:	jsr	sgx_set_write
	ldx	<_cl
	; --
.l2:	lda	[_si],Y
	sta	sgx_video_data_l
	iny
	lda	[_si],Y
	sta	sgx_video_data_h
	iny
	bne	.l3
	inc	<_si+1
.l3:	dex
	bne	.l2
	; --
	jsr	remap_data
	; --
	addw	sgx_bat_width,<_di
	dec	<_ch
	bne	.l1

	; ----
	; unmap data
	;
	jmp	unmap_data
	.bank	LIB1_BANK


; sgx_set_bat_size ( char size )
; ----
sgx_set_bat_size:
	maplibfunc_y	lib2_sgx_set_bat_size
	rts

	.bank	LIB2_BANK

; ----
; sgx_set_bat_size
; ----
; set bg map virtual size for SGX
; ----
; IN : A = new size (0-7)
; ----

lib2_sgx_set_bat_size:
	and	#$07
	pha
	; --
	lda	#09
	sta sgx_video_reg
	pla
	tax
	asl a
	asl a
	asl a
	asl a
.ifdef HUC
	sta	_sgx_vdc+18
.endif
	sta	sgx_video_data_l
	; --
	lda	__sgx_width,X
	sta	sgx_bat_width
	stz	sgx_bat_width+1
	dec a
	sta	sgx_bat_hmask
	; --
	lda	__sgx_height,X
	sta	sgx_bat_height
	sta	sgx_mapbat_bottom
	stz	sgx_mapbat_top
	stz	sgx_mapbat_top_base
	stz	sgx_mapbat_top_base+1
	dec a
	sta	sgx_bat_vmask
	rts

__sgx_width:	.db $20,$40,$80,$80,$20,$40,$80,$80
__sgx_height:	.db $20,$20,$20,$20,$40,$40,$40,$40
	.bank	LIB1_BANK


; sgx_set_screen_size(char size)
; ----
; sgx_set screen virtual size
; ----

_sgx_set_screen_size.1:
	txa
	jmp	sgx_set_bat_size


; sgx_set_map_data(int *ptr)
; sgx_set_map_data(char *map [bl:si], int w [ax], int h)
; sgx_set_map_data(char *map [bl:si], int w [ax], int h [dx], char wrap)
; ----
; map,	map base address
; w,	map width
; h,	map height
; wrap, wrap flag (1 = wrap, 0 = do not wrap)
; ----

_sgx_set_map_data.1:
	cly
	bra	lib3_group_1
_sgx_set_map_data.3:
	ldy	#1
	bra	lib3_group_1
_sgx_set_map_data.4:
	ldy	#2
	bra	lib3_group_1
_sgx_set_tile_data.1:
	ldy	#3
	bra	lib3_group_1
_sgx_set_tile_data.3:
	ldy	#4
	bra	lib3_group_1
_sgx_load_tile:
	ldy	#5

lib3_group_1:
	maplibfunc_y	lib3_group_select_1
	rts

	.bank	LIB2_BANK
lib3_group_select_1:
	cpy	#1
	bcc	lib3_group_case_1 ;lib3_sgx_set_map_data.1
	beq	lib3_group_case_2 ;lib3_sgx_set_map_data.3
	cpy	#3
	bcc	lib3_group_case_3 ;lib3_sgx_set_map_data.4
	beq	lib3_group_case_4 ;lib3_sgx_set_tile_data.1
	cpy	#5
	bcc	lib3_group_case_5 ;lib3_sgx_set_tile_data.3
	bcs	lib3_group_case_6 ;lib3_sgx_load_tile

lib3_group_case_6:
	jmp lib3_sgx_load_tile
lib3_group_case_5:
	jmp lib3_sgx_set_tile_data.3
lib3_group_case_4:
	jmp lib3_sgx_set_tile_data.1
lib3_group_case_3:
	jmp lib3_sgx_set_map_data.4
lib3_group_case_2:
	jmp lib3_sgx_set_map_data.3
lib3_group_case_1:

lib3_sgx_set_map_data.1:
	__stw	<_si
	ora	<_si
	beq	.l1
	; -- calculate width
	lda	[_si].4
	sub	[_si]
	sta	sgx_mapwidth
	lda	[_si].5
	sbc	[_si].1
	sta	sgx_mapwidth+1
	incw	sgx_mapwidth
	; -- calculate height
	lda	[_si].6
	sub	[_si].2
	sta	sgx_mapheight
	lda	[_si].7
	sbc	[_si].3
	sta	sgx_mapheight+1
	incw	sgx_mapheight
	; -- get map bank
	lda	[_si].8
	sta	sgx_mapbank
	; -- get map addr
	lda	[_si].10
	sta	sgx_mapaddr
	iny
	lda	[_si]
	sta	sgx_mapaddr+1
	; -- no wrap
	stz	sgx_mapwrap
	rts
	; -- null pointer
.l1:	stwz	sgx_mapwidth
	stwz	sgx_mapheight
	stz	sgx_mapbank
	stwz	sgx_mapaddr
	stz	sgx_mapwrap
	rts
lib3_sgx_set_map_data.4:
	stx	sgx_mapwrap
	__ldw	<_dx
	bra	sgx_set_map_data.main
lib3_sgx_set_map_data.3:
	stz	sgx_mapwrap
	inc	sgx_mapwrap
sgx_set_map_data.main:
	__stw	sgx_mapheight
	stw	<_ax,sgx_mapwidth
	stb	<_bl,sgx_mapbank
	stw	<_si,sgx_mapaddr
	rts
	.bank	LIB1_BANK


; sgx_set_tile_data(char *tile_ex [di])
; sgx_set_tile_data(char *tile [bl:si], int nb_tile [cx], char *ptable [al:dx])
; ----
; tile,	tile base index
; nb_tile, number of tile
; ptable,	tile palette table address
; ----

	.bank	LIB2_BANK
lib3_sgx_set_tile_data.1:
	cly
	lda	[_di],Y++
	sta	sgx_mapnbtile
	lda	[_di],Y++
	sta	sgx_mapnbtile+1
	lda	[_di],Y++
	sta	sgx_maptiletype
	iny
	lda	[_di],Y++
	sta	sgx_maptilebank
	iny
	lda	[_di],Y++
	sta	sgx_maptileaddr
	lda	[_di],Y++
	sta	sgx_maptileaddr+1
	lda	#(CONST_BANK+_bank_base)
	sta	sgx_mapctablebank
	lda	[_di],Y++
	sta	sgx_mapctable
	lda	[_di],Y
	sta	sgx_mapctable+1
	rts
lib3_sgx_set_tile_data.3:
	stb	<_bl,sgx_maptilebank
	stw	<_si,sgx_maptileaddr
	stw	<_cx,sgx_mapnbtile
	stb	<_al,sgx_mapctablebank
	stw	<_dx,sgx_mapctable
	; --
	ldy	<_bl	; get tile format (8x8 or 16x16)
	lda	<_si+1
	and	#$1F
	tax
	lda	<_si
	bne	.l2
	cpx	#$0
	bne	.l1
	dey
	ldx	#$20
.l1:	dex
.l2:	dec a
	txa
	ora	#$60
	sta	<_si+1
	tya
	tam	#3
	lda	[_si]
	sta	sgx_maptiletype
	rts
	.bank	LIB1_BANK


	.bank	LIB2_BANK
; sgx_load_tile(int addr)
; ----

lib3_sgx_load_tile:
	__stw	<_di
	stx	<_al
	lsr a
	ror	<_al
	lsr a
	ror	<_al
	lsr a
	ror	<_al
	lsr a
	ror	<_al
	sta	sgx_maptilebase+1
	stb	<_al,sgx_maptilebase
	; --
	stw	sgx_mapnbtile,<_cx
	ldx	#4
	lda	sgx_maptiletype
	cmp	#8
	beq	.l1
	ldx	#6
.l1:	asl	<_cl
	rol	<_ch
	dex
	bne	.l1
	; --
	stb	sgx_maptilebank,<_bl
	stw	sgx_maptileaddr,<_si
	jmp	lib3_sgx_load_vram

	.bank	LIB1_BANK
