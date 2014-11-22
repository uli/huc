;
; LIBRARY.ASM  -  MagicKit Standard Library
;
;

; IMPORTANT NOTE:
; ----
; almost all the library functions have been changed to automatically
; handle bank mapping (you don't have to map data yourself anymore),
; the change will be transparent to you if you were using only library
; macros to call functions, but you will have to update your code
; in case you were directly calling those functions!


; ----
; map_data
; ----
; map data in page 3-4 ($6000-$9FFF)
; ----
; IN :	_BL = data bank
;	_SI = data address
; ----
; OUT:	_BX = old banks
;	_SI = remapped data address
; ----

map_data:
	ldx	<bl

	; ----
	; save current bank mapping
	;
	tma	#3
	sta	<bl
	tma	#4
	sta	<bh
	; --
	cpx	#$FE
	bne	.l1
	; --
	stx	<bp
	rts

	; ----
	; map new bank
	;
.l1:	stz	<bp
	; --
	txa
	tam	#3
	inc	A
	tam	#4

	; ----
	; remap data address to page 3
	;
	lda	<si+1
	and	#$1F
	ora	#$60
	sta	<si+1
	rts


; ----
; unmap_data
; ----
; IN :	_BX = old banks
; ----

unmap_data:

	lda	<bl
	tam	#3
	lda	<bh
	tam	#4
	rts

; ----
; remap_data
; ----

remap_data:
	lda	<bp
	bne	.l1
	lda	<si+1
	bpl	.l1
	sub	#$20
	sta	<si+1
	tma	#4
	tam	#3
	inc	A
	tam	#4
.l1:
	rts


; ----
; load_palette
; ----
; initialize one or more sub-palette
; ----
; IN :	_AL = index of the first sub-palette (0-31)
;	_BL = data bank
;	_SI = address of data
;	_CL = number of sub-palette to copy
; ----

.ifdef HUC
_load_palette.3:
.endif
load_palette:
	maplibfunc	lib2_load_palette
	rts

	.bank	LIB2_BANK
lib2_load_palette:

	; ----
	; map data
	;
	jsr	map_data

	; ----
	; multiply the sub-palette index by 16
	; and set the VCE color index register
	;
	lda	<al
	stz	<ah
	asl	A
	asl	A
	asl	A
	asl	A
	rol	<ah
	sta	color_reg_l
	lda	<ah
	sta	color_reg_h

	; ----
	; load new colors
	;

; Use TIA, but BLiT 16 words at a time (32 bytes)
; Because interrupt must not be deferred too much
;
	stw	#32, _ram_hdwr_tia_size
	stw	#color_data, _ram_hdwr_tia_dest

.loop_a:
	stw	<si, _ram_hdwr_tia_src
	jsr	_ram_hdwr_tia
	addw	#32, <si
	dec	<cl
	bne	.loop_a

	; ----
	; unmap data
	;
	jmp	unmap_data

	.bank	LIB1_BANK


; ----
; load_bat
; ----
; transfer a BAT in VRAM
; ----
; IN :	_DI = VRAM base address
;	_BL = BAT bank
;	_SI = BAT memory location
;	_CL = nb of column to copy
;	_CH = nb of row
; ----

.ifdef HUC
_load_bat.4:
.endif
load_bat:
	maplibfunc	lib2_load_bat
	rts

	.bank	LIB2_BANK

lib2_load_bat:

	; ----
	; map data
	;
	jsr	map_data

	; ----
	; copy BAT
	;
	cly
	; --
.l1:	jsr	set_write
	ldx	<cl
	; --
.l2:	lda	[si],Y
	sta	video_data_l
	iny
	lda	[si],Y
	sta	video_data_h
	iny
	bne	.l3
	inc	<si+1
.l3:	dex
	bne	.l2
	; --
	jsr	remap_data
	; --
	addw	bat_width,<di
	dec	<ch
	bne	.l1

	; ----
	; unmap data
	;
	jmp	unmap_data

	.bank	LIB1_BANK

; ----
; load_map_8/16
; ----
; transfer a tiled map in VRAM
; ----
; IN :	_AL = x screen coordinate (tile unit)
;	_AH = y screen coordinate
;	_CL = x start coordinate in the map
;	_CH = y start coordinate
;	_DL = nb of column to copy
;	_DH = nb of row
; ----

	.bss
mapbank		.ds 1
mapaddr		.ds 2
mapwidth	.ds 2
mapheight	.ds 2
maptiletype	.ds 1
maptilebank	.ds 1
maptileaddr	.ds 2
maptilebase	.ds 2
mapnbtile	.ds 2
mapctablebank	.ds 1
mapctable	.ds 2
mapwrap		.ds 1
mapbat_ptr	.ds 2
mapbat_top_base .ds 2
mapbat_top	.ds 1
mapbat_bottom	.ds 1
mapbat_x	.ds 2
	.code
load_map:
	lda	maptiletype
	cmp	#8
	beq	.l1
	; --
	maplibfunc	lib2_load_map_16
	rts
.l1:	maplibfunc	lib2_load_map_8
	rts

	.bank	LIB2_BANK

lib2_load_map_16:

	; ----
	; save bank mapping
	;
	tma	#2
	pha
	tma	#3
	pha
	tma	#4
	pha

	; ----
	; init
	;
	jsr	load_map_init
	lda	<ch
	sta	mapbat_x+1

	; ----
	; vertical loop
	;
.l1:	ldy	<ah
	lda	<dl
	sta	<al
	lda	mapbat_x+1
	sta	mapbat_x
	bra	.l5

	; ----
	; horizontal loop
	;
.l2:	lda	mapbat_x	; bat wrapping
	add	#2
	and	bat_hmask
	sta	mapbat_x
	bne	.l3
	; --
	lda	bat_hmask
	eor	#$ff
	and	<di
	sta	<di
	bra	.l4
.l3:
	incw	<di
	incw	<di
.l4:
	iny
	; --
	cpy	mapwidth		; horizontal map wrapping
	bne	.l5
	cly
	ldx	mapwrap
	bne	.l5
	ldy	mapwidth
	lda	maptilebase
	sta	<cl
	lda	maptilebase+1
	ora	[bp]
	sta	<ch
	dey
	bra	.l6
.l5:
	lda	[si],Y		; get tile index
	tax			; calculate BAT value (tile + palette)
	sxy
	stz	<ch
	asl	A
	rol	<ch
	asl	A
	rol	<ch
	add	maptilebase
	sta	<cl
	lda	<ch
	adc	maptilebase+1
	adc	[bp],Y
	sta	<ch
	sxy
.l6:
	vreg	#0		; copy tile
	stw	<di,video_data
	vreg	#2
	stw	<cx,video_data
	incw	<cx
	stw	<cx,video_data
	incw	<cx
	vreg	#0
	addw	bat_width,<di,video_data
	vreg	#2
	stw	<cx,video_data
	incw	<cx
	stw	<cx,video_data

	dec	<al		; next tile
	lbne	.l2

	; ----
	; next line
	;
	ldx	#2
	jsr	load_map_next_line
	dec	<dh
	lbne	.l1

	; ----
	; restore bank mapping
	;
	jmp	load_map_exit

lib2_load_map_8:

	; ----
	; save bank mapping
	;
	tma	#2
	pha
	tma	#3
	pha
	tma	#4
	pha

	; ----
	; init
	;
	jsr	load_map_init
	bra	.l2

	; ----
	; vertical loop
	;
.l1:	ldx	#1
	jsr	load_map_next_line
	; --
.l2:	ldy	<ah
	lda	<dl
	sta	<al
	lda	<ch
	sta	<cl
	vreg	#0		; set vram write ptr
	stw	<di,video_data
	vreg	#2
	bra	.l5

	; ----
	; horizontal loop
	;
.l3:	lda	<cl		; bat wrapping
	inc	A
	and	bat_hmask
	sta	<cl
	bne	.l4
	; --
	vreg	#0
	lda	bat_hmask
	eor	#$ff
	and	<di
	sta	video_data_l
	lda	<di+1
	sta	video_data_h
	vreg	#2
.l4:
	iny			; next tile
	; --
	cpy	mapwidth		; map wrapping
	bne	.l5
	; --
	cly
	lda	mapwrap
	bne	.l5
	ldy	mapwidth
	dey
	cla
	bra	.l6
.l5:
	lda	[si],Y		; get tile index
.l6:	tax			; calculate BAT value (tile + palette)
	sxy
	add	maptilebase
	sta	video_data_l
	lda	maptilebase+1
	adc	[bp],Y
	sta	video_data_h
	sxy

	dec	<al
	bne	.l3

	; ----
	; next line
	;
	dec	<dh
	bne	.l1

	; ----
	; restore bank mapping
	;
load_map_exit:
	pla
	tam	#4
	pla
	tam	#3
	pla
	tam	#2
	rts


; ----
; load_map_init
; ----
; load_map sub routine
; ----
; OUT:	_DI = BAT address
;	_SI = map address
;	_BP = palette index table ptr
;	_AH = map X pos
;	_BH = map Y pos
;	_CH = BAT X pos
;	_BL = BAT Y pos
; ----

load_map_init:

	; ----
	; calculate vram address
	;
	ldx	<al
	lda	<ah
	ldy	maptiletype
	cpy	#8
	beq	.l1
	asl	A
	sax
	asl	A
	sax
.l1:	phx
	pha
	jsr	calc_vram_addr
	stw	<di,mapbat_ptr

	; ----
	; calculate map address
	;
	stb	mapaddr,<si
	lda	mapaddr+1
	and	#$1F
	sta	<si+1
	; --
	ldx	<cl
	stx	<ah
	ldy	<ch
	sty	<bh
	; --
	lda	mapwidth+1
	beq	.l2
	tya
	add	<si+1
	sta	<si+1
	bra	.l3
	; --
.l2:	sty	<al
	lda	mapwidth
	sta	<bl
	jsr	mulu8
	addw	<cx,<si

	; ----
	; calculate map bank
	;
.l3:	rol	A
	rol	A
	rol	A
	rol	A
	and	#$0F
	add	mapbank

	; ----
	; map data
	;
	tam	#3
	inc	A
	tam	#4
	lda	mapctablebank
	tam	#2

	; ----
	; adjust data addresses
	;
	lda	<si+1		; tile ptr
	and	#$1F
	ora	#$60
	sta	<si+1
	; --
	stb	mapctable,<bp	; color table ptr
	lda	mapctable+1
	and	#$1F
	ora	#$40
	sta	<bp+1

	; ----
	; bat pos
	;
	pla
	and	bat_vmask
	sta	<bl
	pla
	and	bat_hmask
	sta	<ch
	rts


; ----
; load_map_next_line
; ----
; load_map sub routine
; ----
; IN :	X = BAT line inc value (1-2)
; ----
; OUT:	_DI = BAT address
;	_SI = map address
; ----
; USE:	_BL = BAT Y pos
;	_BH = map Y pos
;	_SI = map address
; ----

load_map_next_line:

	; ----
	; incremente vram address
	;
	txa
	add	<bl
	cmp	mapbat_bottom
	blo	.l1
	; --
	sub	mapbat_bottom	; 1/ vram wrapping
	tax
	inx
	add	mapbat_top
	sta	<bl
	lda	mapbat_ptr
	and	bat_hmask
	add	mapbat_top_base
	sta	mapbat_ptr
	cla
	adc	mapbat_top_base+1
	sta	mapbat_ptr+1
	bra	.l3
	; -- 
.l1:	sta	<bl		; 2/ vram inc
.l2:	lda	bat_width
	add	mapbat_ptr
	sta	mapbat_ptr
	cla
	adc	mapbat_ptr+1
	sta	mapbat_ptr+1
	; --
.l3:	dex
	bne	.l2
	; --
	stw	mapbat_ptr,<di

	; ----
	; increment map address
	;
	inc	<bh
	lda	<bh
	cmp	mapheight
	bne	.l4
	; --
	lda	mapbank		; 1/ map wrapping
	tam	#3
	inc	A
	tam	#4
	stb	mapaddr,<si
	lda	mapaddr+1
	and	#$1F
	ora	#$60
	sta	<si+1
	stz	<bh
	bra	.l5
	; --
.l4:	addw	mapwidth,<si	; 2/ map inc
	cmp	#$80
	blo	.l5
	sub	#$20
	sta	<si+1
	tma	#4
	tam	#3
	inc	A
	tam	#4
.l5:
	rts


	.bank	LIB1_BANK

; ----
; load_font
; ----
; transfer a 8x8 monochrome font into VRAM, slow but can be useful
; ----
; IN :	_DI = VRAM base address
;	_BL = font bank
;	_SI = font memory location
;	_AL = font color (0-15)
;	_AH = bg color (0-15)
;	_CL = number of characters to copy
; ----

load_font:
	maplibfunc	lib2_load_font
	rts

	.bank	LIB2_BANK

	; ----
	; map data
	;
lib2_load_font:
	jsr	map_data
	jsr	set_write

	; ----
	; init bg color
	;
	lda	<cl
	pha
	; --
	ldx	#3
.l1:	cla
	lsr	<ah
	bcc	.l2
	lda	#$FF
.l2:	sta	<cl,X
	dex
	bpl	.l1

	; ----
	; character loop
	;
	plx
.copy:

	; ----
	; plane 1
	;
	cly
.p1:	bbs0	<al,.p2
	lda	[si],Y
	eor	#$FF
	and	<dh
	bra	.p3
	; --
.p2:	lda	<dh
	ora	[si],Y
.p3:	sta	video_data_l

	; ----
	; plane 2
	;
	bbs1	<al,.p4
	lda	[si],Y
	eor	#$FF
	and	<dl
	bra	.p5
	; --
.p4:	lda	<dl
	ora	[si],Y
.p5:	sta	video_data_h
	; --
	iny
	cpy	#8
	bne	.p1

	; ----
	; plane 3
	;
	cly
.t1:	bbs2	<al,.t2
	lda	[si],Y
	eor	#$FF
	and	<ch
	bra	.t3
	; --
.t2:	lda	<ch
	ora	[si],Y
.t3:	sta	video_data_l

	; ----
	; plane 4
	;
	bbs3	<al,.t4
	lda	[si],Y
	eor	#$FF
	and	<cl
	bra	.t5
	; --
.t4:	lda	<cl
	ora	[si],Y
.t5:	sta	video_data_h
	; --
	iny
	cpy	#8
	bne	.t1

	; ----
	; next character
	;
	addw	#8,<si
	; --
	dex
	bne	.copy

	; ----
	; unmap data
	;
	jmp	unmap_data

	; ----
	; restore bank mapping
	;
	.bank	LIB1_BANK


; ----
; load_vram
; ----
; copy a block of memory to VRAM
; ----
; IN :	_DI = VRAM location
;	_BL = data bank
;	_SI = data memory location
;	_CX = number of words to copy
; ----
	.bss

; This actually places a 'TIA' command
; into RAM, from which to execute
; for BLiT to VRAM for SATB transfer
; and other VRAM load functions

_ram_hdwr_tia		.ds	1
_ram_hdwr_tia_src	.ds	2
_ram_hdwr_tia_dest	.ds	2
_ram_hdwr_tia_size	.ds	2
_ram_hdwr_tia_rts	.ds	1

	.code

.ifdef HUC
_load_vram.3:
.endif
load_vram:

	; ----
	; map data
	;
	jsr	map_data

;	; ----
;	; setup call to TIA operation (fastest transfer)
;	;
;	; (instruction setup done during bootup...)
;
;	stw	#video_data, _ram_hdwr_tia_dest
;	stw	<si, _ram_hdwr_tia_src
;
;	asl	<cl		; change from words to bytes (# to xfer)
;	rol	<ch

	; ----
	; set vram address
	;
	jsr	set_write

	; ----
	; copy data
	;
	cly
	ldx	<cl
	beq	.l3
	; --
.l1:	lda	[si],Y
	sta	video_data_l
	iny
	lda	[si],Y
	sta	video_data_h
	iny
	bne	.l2
	inc	<si+1
	; --
.l2:	dex
	bne	.l1
	; --
	jsr	remap_data
	; --
.l3:	dec	<ch
	bpl	.l1

;.l1:	lda	<ch		; if zero-transfer, exit
;	ora	<cl
;	beq	.out
;
;	lda	<ch
;	cmp	#$20		; if more than $2000, repeat xfers of $2000
;	blo	.l2		; while adjusting banks
;	sub	#$20		; reduce remaining transfer amount
;	sta	<ch
;
;	stw	#$2000, _ram_hdwr_tia_size
;	jsr	_ram_hdwr_tia
;
;	lda	<si+1		; force bank adjust
;	add	#$20		; and next move starts at same location
;	sta	<si+1
;
;	jsr	remap_data	; adjust banks
;	bra	.l1
;
;.l2:	sta	HIGH_BYTE _ram_hdwr_tia_size	; 'remainder' transfer of < $2000
;	lda	<cl
;	sta	LOW_BYTE	_ram_hdwr_tia_size
;	jsr	_ram_hdwr_tia

	; ----
	; unmap data
	;

.out:	jmp	unmap_data



; ----
; set_read
; ----
; set the VDC VRAM read pointer
; ----
; IN :	_DI = VRAM location
; ----

set_read:
	vreg	#$01
	lda	<di 
	sta	video_data_l
.ifdef HUC
	sta	_vdc+2
.endif
	lda	<di+1
	sta	video_data_h
.ifdef HUC
	sta	_vdc+3
.endif
	vreg	#$02
	rts 


; ----
; set_write
; ----
; set the VDC VRAM write pointer
; ----
; IN :	_DI = VRAM location
; ----

set_write:
	vreg	#$00
	lda	<di 
	sta	video_data_l
.ifdef HUC
	sta	_vdc
.endif
	lda	<di+1
	sta	video_data_h
.ifdef HUC
	sta	_vdc+1
.endif
	vreg	#$02
	rts 


; ----
; calc_vram_addr
; ----
; calculate VRAM address
; ----
; IN :	X = x coordinates
;	A = y	"
; ----
; OUT:	_DI = VRAM location
; ----

calc_vram_addr:
	phx
	and	bat_vmask
	stz	<di
	ldx	bat_width
	cpx	#64
	beq	.s64
	cpx	#128
	beq	.s128
	; --
.s32:	lsr	A
	ror	<di
	; --
.s64:	lsr	A
	ror	<di
	; --
.s128:	lsr	A
	ror	<di
	sta	<di+1
	; --
	pla
	and	bat_hmask
	ora	<di
	sta	<di
	rts

; ----
; HSR(xres)
; ----
; macros to calculate the value of the HSR VDC register
; ----
; IN :	xres, horizontal screen resolution
; ----

HSR	.macro
.if (\1 < 268)
	; low res
	.db $02
	.db (18 - (\1 / 16))
.else
.if (\1 < 356)
	; high res
	.db $03
	.db (25 - (\1 / 16))
.else
	; very high res
	.db $05
	.db (42 - (\1 / 16))
.endif
.endif
.endm


; ----
; HDR(xres)
; ----
; macros to calculate the value of the HDR VDC register
; ----
; IN :	xres, horizontal screen resolution
; ----

HDR	.macro
	.db ((\1 / 8) - 1)
.if (\1 < 268)
	; low res
	.db (38 - ((18 - (\1 / 16)) + (\1 / 8)))
.else
.if (\1 < 356)
	; high res
	.db (51 - ((25 - (\1 / 16)) + (\1 / 8)))
.else
	; high res
	.db (82 - ((42 - (\1 / 16)) + (\1 / 8)))
.endif
.endif
.endm


; ----
; init_vdc
; ----
; initialize the video controller
;  - 256x224 screen mode
;  - 64x32 virtual bgmap size
;  - display and sprites off
;  - interrupts disabled
;  - SATB at $7F00
;  - VRAM cleared
; ----

	.bss
bat_width	.ds 2
bat_height	.ds 1
bat_hmask	.ds 1
bat_vmask	.ds 1
scr_width	.ds 1
scr_height	.ds 1

	.code

init_vdc:
	maplibfunc lib2_init_vdc
	rts

	.bank	LIB2_BANK
lib2_init_vdc:
; ----
; default screen resolution
;
.ifndef _xres
_xres	.equ 256
.endif

	; ----
	; initialize the VDC registers
	;
	stw	#.table,<si 	; register table address in 'si'
	cly 
.l1:	lda	[si],Y		; select the VDC register
	iny
	sta	<vdc_reg
	sta	video_reg
.ifdef HUC
	asl	A
	tax
.endif
	lda	[si],Y		; send the 16-bit data
	iny 
	sta	video_data_l
.ifdef HUC
	sta	_vdc,X
.endif
	lda	[si],Y
	iny 
	sta	video_data_h
.ifdef HUC
	sta	_vdc+1,X
.endif
	cpy	#36		; loop if not at the end of the
	bne	.l1		; table

	; ----
	; set the screen mode
	;
.if (_xres < 268)
	lda	#(0 | XRES_SOFT)
.else
.if (_xres < 356)
	lda	#(1 | XRES_SOFT)
.else
	lda	#(2 | XRES_SOFT)
.endif
.endif

; This stuff alters display position from HuCard position

;	.if (CDROM)
;
;	ldx	#_xres/8
;	ldy	#30
;	jsr	ex_scrmod
;	lda	#$01
;	jsr	ex_scrsiz
;	lda	#0
;	jsr	ex_imode
;
;	.else

	; pixel clock frequency
	sta	color_ctrl
;
;	.endif


	; ----
	; set the background & border colors to black
	;
	stw	#256,color_reg
	stwz	color_data
	stwz	color_reg
	stwz	color_data

.if (CDROM)

	; ----
	; reset scrolling position (0,0)
	;
	vreg	#7
	stwz	video_data
	vreg	#8
	stwz	video_data
	stwz	bg_x1
	stwz	bg_y1

	; ----
	; set SATB address
	;
	stw	#$7F00,satb_addr
	jsr	ex_sprdma
	lda	#$10
	jsr	ex_dmamod

.endif	; (CDROM)

	; ----
	; clear the video RAM
	;
	st0	#0
	st1	#0
	st2	#0
	st0	#2

	ldx	#128
.l2:	cly
.l3:	st1	#0
	st2	#0
	dey
	bne	.l3
	dex
	bne	.l2

	; ----
	; save screen infos
	;
	stw	#_xres,scr_width	; resolution
	stw	#224,scr_height
	; --
	lda	#BGMAP_SIZE_64x32	; virtual size
	jmp	set_bat_size

	; ----
	; VDC register table
	;
.table:
.ifdef _SGX
_sgx_init_point:
.endif
	.db $05,$00,$00		; CR	control register
	.db $06,$00,$00		; RCR	scanline interrupt counter
	.db $07,$00,$00		; BXR	background horizontal scroll offset
	.db $08,$00,$00		; BYR        "     vertical     "      "
	.db $09,$10,$00		; MWR	size of the virtual screen
	.db $0A			; HSR +
	HSR _xres		;     |			[$02,$02]
	.db $0B			; HDR | display size
	HDR _xres		;     | and synchro	[$1F,$04]
	.db $0C,$02,$17		; VPR |
	.db $0D,$DF,$00		; VDW |
	.db $0E,$0C,$00		; VCR +
	.db $0F,$10,$00		; DCR	DMA control register
	.db $13,$00,$7F		; SATB	address of the SATB
	.bank	LIB1_BANK


; ----
; set_xres
; ----
; set horizontal display resolution
; ----
; IN :	_AX = new x resolution (ie. 320)
;	_CL = 'blur bit' for color register
; USES: _BX
; ----

set_xres:
	maplibfunc	lib2_set_xres
	rts


	.bank	LIB2_BANK

	.bss
vdc_blur	.ds 1	; blur bit
hsw		.ds 1	; temporary parameters for calculating video registers
hds		.ds 1
hdw		.ds 1
hde		.ds 1

	.code

lib2_set_xres:
	lda	#$20		; reset resource-usage flag
	tsb	<irq_m		; to skip joystick read portion of vsync
				; (temporarily disable VSYNC processing)
	lda	<ah
	sta	<bh
	lda	<al
	sta	<bl		; bx now has x-res

	lsr	<bh
	ror	<bl
	lsr	<bh
	ror	<bl
	lsr	<bl		; bl now has x/8

	cly			; offset into numeric tables
				; 0=low-res, 1=mid-res, 2=high-res

	lda	<ah
	beq	.xres_calc	; < 256
	cmp	#3
	bhs	.xres_calc

	cmpw	#$10C,<ax
	blo	.xres_calc	; < 268

	iny
	cmpw	#$164,<ax
	blo	.xres_calc	; < 356

	iny			; 356 < x < 512

.xres_calc:
	lda	.vce_tab,Y
	ora	<cl
	sta	color_ctrl	; dot-clock (x-resolution)

	lda	.hsw_tab,Y	; example calc's (using "low-res" numbers)
	sta	hsw		; hsw = $2
	lda	<bl
	sta	hds		; hds = (x/8) temporarily
	dec	A
	sta	hdw		; hdw = (x/8)-1
	lsr	hds		; hds = (x/16) temporarily

	lda	.hds_tab,Y
	sub	hds
	sta	hds		; hds = 18 - (x/16)

	lda	.hde_tab,Y
	sub	hds
	sub	<bl		; hde = (38 - ( (18-(x/16)) + (x/8) ))
	sta	hde

.xres_putit:
	lda	#$0a
	sta	<vdc_reg
	sta	video_reg
.ifdef HUC
	asl	A
	sax
.endif
	lda	hsw
	sta	video_data_l
.ifdef HUC
	sta	_vdc,X
.endif
	lda	hds
	sta	video_data_h
.ifdef HUC
	sta	_vdc+1,X
.endif

	lda	#$0b
	sta	<vdc_reg
	sta	video_reg
.ifdef HUC
	asl	A
	sax
.endif
	lda	hdw
	sta	video_data_l
.ifdef HUC
	sta	_vdc,X
.endif
	lda	hde
	sta	video_data_h
.ifdef HUC
	sta	_vdc+1,X
.endif

.xres_err:
	lda	#$20
	trb	<irq_m		; re-enable VSYNC processing
	rts

.vce_tab:	.db	0, 1, 2
.hsw_tab:	.db	2, 3, 5
.hds_tab:	.db	18,25,42
.hde_tab:	.db	38,51,82

	.bank	LIB1_BANK	; restore bank context

; ----
; set_bat_size
; ----
; set bg map virtual size
; ----
; IN : A = new size (0-7)
; ----

set_bat_size:
	and	#$07
	pha
	; --
.if (CDROM)
	jsr	ex_scrsiz
	plx
.else
	vreg	#9
	pla
	tax
	asl	A
	asl	A
	asl	A
	asl	A
.ifdef HUC
	sta	_vdc+18
.endif
	sta	video_data_l
.endif
	; --
	lda	.width,X
	sta	bat_width
	stz	bat_width+1
	dec	A
	sta	bat_hmask
	; --
	lda	.height,X
	sta	bat_height
	sta	mapbat_bottom
	stz	mapbat_top
	stz	mapbat_top_base
	stz	mapbat_top_base+1
	dec	A
	sta	bat_vmask
	rts

.width:	.db $20,$40,$80,$80,$20,$40,$80,$80
.height: .db $20,$20,$20,$20,$40,$40,$40,$40


; ----
; init_psg
; ----
; initialize the sound generator.
; ----

init_psg:
	maplibfunc lib2_init_psg
	rts

	.bank	LIB2_BANK
lib2_init_psg:
	stz	psg_mainvol	; main volume to zero
	stz	psg_lfoctrl	; disable the LFO
	
	lda	#5		; set volume to zero for each channel
.clear:	sta	psg_ch		; and disable them
	stz	psg_ctrl
	stz	psg_pan
	dec	A
	bpl	.clear

	lda	#4		; disable noise for channel 5 & 6
	sta	psg_ch
	stz	psg_noise
	lda	#5
	sta	psg_ch
	stz	psg_noise
	rts
	.bank	LIB1_BANK


; ----------------------------------
; Some simple copy/compare functions
; ----------------------------------

; ----
; _strcpy(char *dest [di], char *src [si])
; _strcat(char *dest [di], char *src [si])
; ----
; Copy/Concatenate a string to another string
; ----
;
_strcat.2:
.endlp:	lda	[di]		; same as strcpy, but find end
	beq	_strcpy.2	; of dest string first
	incw	<di
	bra	.endlp

_strcpy.2:
.cpylp:	lda	[si]
	sta	[di]
	beq	.out
	incw	<di
	incw	<si
	bra	.cpylp
.out:	rts


; ----
; _strncpy(char *dest [di], char *src [si], int count [acc])
; _strncat(char *dest [di], char *src [si], int count [acc])
; ----
; Copy/Concatenate a string to another string
; ----
;

_strncat.3:
	__stw	<ax
	__ldw	<di
	__stw	<bx
.endlp:	lda	[di]
	beq	.cpylp
	incw	<di
	bra	.endlp
.cpylp:
	jsr	strncpy_loop
	cla
	sta	[di]
	__ldw	<bx
.out:	rts


_strncpy.3:
	__stw	<ax
	__ldw	<di
	__stw	<bx
	jsr	strncpy_loop
	__ldw	<bx
	rts

strncpy_loop:
	lda	[si]
	sta	[di]
	beq	.out
	incw	<di
	incw	<si
	decw	<ax
	tstw	<ax
	bne	strncpy_loop
.out:	rts

; ----
; _memcpy(char *dest [di], char *src [si], int count [acc])
; ----
; Copy memory
; ----
;
_memcpy.3:
	__stw	<ax
	ora	<al
	beq	.done
	__ldw	<di
	__stw	<bx
.cpylp:	lda	[si]
	sta	[di]
	incw	<si
	incw	<di
	decw	<ax
	tstw	<ax
	bne	.cpylp
	__ldw	<bx
.done:	rts

; same as memcpy, but returns end of dest
_mempcpy.3:
	__stw	<cx
	jsr _memcpy.3
	__addw	<cx
	rts

; _memset(char *s [di], int c [bx], int n [acc])
_memset.3:
	__stw	<ax
	ora	<al
	beq	.done
.setlp:	lda	<bx
	sta	[di]
	incw	<di
	decw	<ax
	tstw	<ax
	bne	.setlp
.done:	rts

; ----
; _memcmp(char *dest [di], char *src [si], int count [acc])
; ----
; Compare memory
; ----
;
_memcmp.3:
	__stw	<ax
	ora	<al
	beq	.done
.cmplp:	lda	[di]
	sub	[si]
	bmi	.minus
	beq	.cont
.plus:	tax
	cla
	rts
.minus:	tax
	lda	#$ff
	rts
.cont:	incw	<di
	incw	<si
	decw	<ax
	tstw	<ax
	bne	.cmplp
	clx
.done:	rts

; ----
; _strcmp(char *dest [di], char *src [si])
; ----
; Compare strings
; ----
;
_strcmp.2:
.cmplp:	lda	[di]
	sub	[si]
	bmi	.minus
	beq	.cont
.plus:	tax
	cla
	rts
.minus:	tax
	lda	#$ff
	rts
.cont:	lda	[di]
	beq	.out
	incw	<di
	incw	<si
	bra	.cmplp
.out:	clx
	rts

; ----
; _strncmp(char *dest [di], char *src [si], int count [acc])
; ----
; Compare strings
; ----
;
_strncmp.3:
	__stw	<ax
.cmplp:	lda	[di]
	sub	[si]
	bmi	.minus
	beq	.cont
.plus:	tax
	cla
	rts
.minus:	tax
	lda	#$ff
	rts
.cont:	lda	[di]
	beq	.out
	incw	<di
	incw	<si
	decw	<ax
	tstw	<ax
	bne	.cmplp
.out	clx
	cla
	rts

_strlen.1:
	cly
.loop:	lda	[si],y
	beq	.out
	iny
	bra .loop
.out:	tya
	tax
	cla
	rts

___builtin_ffs.1:
	maplibfunc lib2____builtin_ffs.1
	rts

	.bank LIB2_BANK
lib2____builtin_ffs.1:
	sax
	ldy #-8
.search_lo:	ror a
	bcs .found_in_lo
	iny
	bne .search_lo
	txa
	ldy #-8
.search_hi:	ror a
	bcs .found_in_hi
	iny
	bne .search_hi
	clx		; no bits set, return 0 (A is already 0)
	rts
.found_in_hi:tya		; found bit in the high byte
	adc #16		; carry is set
	tax
	cla		; return 17 + y
	rts
.found_in_lo:tya		; found bit in the low byte
	adc #8		; carry is set
	tax
	cla		; return 9 + y
	rts
	.bank LIB1_BANK

_mem_mapdatabanks:
	tay		; y = new upper bank
	tma #DATA_BANK+1; a = old upper bank
	say		; y = old upper bank, a = new upper bank
	tam #DATA_BANK+1
do_mapdatabank:
	tma #DATA_BANK	; a = old lower bank
	sax		; x = old lower bank, a = new lower bank
	tam #DATA_BANK
	tya		; a = old upper bank
	rts
_mem_mapdatabank:
	cly
	bra do_mapdatabank


.ifdef HAVE_IRQ
_irq_add_vsync_handler:
	stx	<__temp
	clx
.loop	ldy	user_vsync_hooks+1, x
	beq	.found
	inx
	inx
	cpx	#8
	bne	.loop
	ldx	#1
	cla
	rts
.found	ldy	<__temp
	sei
	sta	user_vsync_hooks+1, x
	tya
	sta	user_vsync_hooks, x
	cli
	clx
	cla
	rts

_irq_enable_user:
	txa
	tsb	<huc_irq_enable
	rts

_irq_disable_user:
	txa
	trb	<huc_irq_enable
	rts
.endif ; HAVE_IRQ

_timer_set:
	stx	timer_cnt
	rts
_timer_start:
	lda	#1
	sta	timer_ctrl
	rts
_timer_stop:
	stz	timer_ctrl
	rts
_timer_get:
	lda	timer_cnt
	and	#$7f
	tax
	cla
	rts
_irq_enable:
	txa
	sei
	ora	irq_disable
	sta	irq_disable
	cli
	rts
_irq_disable:
	txa
	eor	#$ff
	sei
	and	irq_disable
	sta	irq_disable
	cli
	rts

_abort:
	.db 0xe2

_exit:
	.db 0x63

_dump_screen:
	.db 0x33
