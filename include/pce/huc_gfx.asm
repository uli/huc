;
; HUC_GFX.ASM  -  HuC Graphic Library
;

; ----
; local variables

            .zp
__spr_ptr   .ds 2
__spr_max   .ds 1
__spr_flag  .ds 1

            .bss
_font_base  .ds 2
_font_color .ds 2
_satb       .ds 512	; the local SATB

_gfx_pal    .ds 1

_line_currx	.ds	2
_line_curry	.ds	2
_line_deltax	.ds	2
_line_deltay	.ds	2
_line_error	.ds	2
_line_adjust	.ds	2
_line_xdir	.ds	1
_line_color	.ds	1


; ----
; library code

	 .code

; cls(int val [dx])
; ----

_cls:
	  stw	_font_base,<_dx
_cls.1:
	  setvwaddr $0
	  ; --
	  ldy	bat_height
.l2:	  ldx	bat_width
	  ; --
.l3:	  stw	<_dx,video_data
	  dex
	  bne	.l3
	  dey
	  bne	.l2
	  rts

; set_font_pal(int pal)
; ----

_set_font_pal:
	  txa
	  asl	A
	  asl	A
	  asl	A
	  asl	A
	  sta	<__temp
	  lda	_font_base+1
	  and	#$0F
	  ora	<__temp
	  sta	_font_base+1
	  rts

; set_font_color(char color, char bg)
; ----

_set_font_color.2:
	  txa
	  and	#$F
	  sta	_font_color+1
	  lda	<_al
	  and	#$F
	  sta	_font_color
	  rts

; set_font_addr(int addr)
; ----

_set_font_addr:
	  ; --
	  stx	_font_base
	  lsr	A
	  ror	_font_base
	  lsr	A
	  ror	_font_base
	  lsr	A
	  ror	_font_base
	  lsr	A
	  ror	_font_base
	  sta	<_al
	  ; --
	  lda	_font_base+1
	  and	#$F0
	  ora	<_al
	  sta	_font_base+1
	  rts

; get_font_pal()
; ----

_get_font_pal:
	  lda	_font_base+1
	  lsr	A
	  lsr	A
	  lsr	A
	  lsr	A
	  clx
	  sax
	  rts

; get_font_addr()
; ----

_get_font_addr:
	  ; --
	  lda	_font_base+1
	  sta	<_al
	  lda	_font_base
	  asl	A
	  rol	<_al
	  asl	A
	  rol	<_al
	  asl	A
	  rol	<_al
	  asl	A
	  rol	<_al
	  ldx	<_al
	  sax
	  rts

; load_default_font(char num [dl], int addr [di])
; ----

_load_default_font:
	  ; --
	  stz	<_dl

_load_default_font.1:
	  ; --
	  ldx	#$FF
	  lda	#$FF
	  jsr	calc_vram_addr
	  incw	<_di

_load_default_font.2:
	  ; --
	  lda	<_di
	  ora	<_di+1
	  bne	.l1
	  jsr	_get_font_addr
	__stw	<_di
	  bra	.l2
	  ; --
.l1:	__ldw	<_di
	  jsr	_set_font_addr
	  ; --
.l2:	  stb	#FONT_BANK+_bank_base,<_bl
	  stb	#96,<_cl
	  stb	_font_color+1,<_ah
	  lda	_font_color
	  bne	.l3
	  inc	A
.l3:	  sta	<_al
	  lda	<_dl
	  and	#$03
	  asl	A
	  tax
	  lda	font_table,X
	  sta	<_si
	  inx
	  lda	font_table,X
	  sta	<_si+1
	  jmp	load_font

; load_font(farptr font [bl:si], char nb [cl], int addr [di])
; ----

_load_font.2:
	  ; --
	  ldx	#$FF
	  lda	#$FF
	  jsr	calc_vram_addr
	  incw	<_di

_load_font.3:
	  ; --
	  lda	<_di
	  ora	<_di+1
	  bne	.l1
	  jsr	_get_font_addr
	__stw	<_di
	  bra	.l2
	  ; --
.l1:	__ldw	<_di
	  jsr	_set_font_addr
	  ; --
.l2:	  lda	<_cl
	  stz	<_ch
	  asl	A
	  rol	<_ch
	  asl	A
	  rol	<_ch
	  asl	A
	  rol	<_ch
	  asl	A
	  rol	<_ch
	  sta	<_cl
	  jmp	load_vram

; put_digit(char digit, int offset)
; put_digit(char digit, char x, char y)
; ----

_put_digit.3:
	  ldy	<__arg_idx
	__plb
	  jsr	_put.xy
	  bra	_put_digit.main
_put_digit.2:
	  ldy	<__arg_idx
	  jsr	_put.vram
_put_digit.main:
	__plb
	  sty	<__arg_idx
_put_digit.sub:
	  cmp	#10
	  blo	.l1
	  add	#$07
.l1:	  adc	#$10
	  adc	_font_base
	  sta	video_data_l
	  cla
	  adc	_font_base+1
	  sta	video_data_h
	  rts
_put.xy:
	  sax
	  jsr	calc_vram_addr
	  jmp	set_write
_put.vram:
	  stz   <vdc_reg
	  stz   video_reg
	  stx	video_data_l
	  sta	video_data_h
	  vreg	#$02
	  rts

; put_char(char character, int offset)
; put_char(char character, char x, char y)
; ----

_put_char.3:
	  ldy	<__arg_idx
	__plb
	  jsr	_put.xy
	  bra	_put_char.main
_put_char.2:
	  ldy	<__arg_idx
	  jsr	_put.vram
_put_char.main:
	__plb
	  sty	<__arg_idx
	  ; --
	  cmp	#32
	  bhs	.l1
	  lda	#32
	  sec
.l1:	  sbc	#32
	  add	_font_base
	  sta	video_data_l
	  cla
	  adc	_font_base+1
	  sta	video_data_h
	  rts

; put_raw(int character, int offset)
; put_raw(int character, char x, char y)
; ----

_put_raw.3:
	  ldy	<__arg_idx
	__plb
	  jsr	_put.xy
	  bra	_put_raw.main
_put_raw.2:
	  ldy	<__arg_idx
	  jsr	_put.vram
_put_raw.main:
	__plb
	  sta	video_data_l
	__plb
	  sta	video_data_h
	  sty	<__arg_idx
	  rts

; put_number(int number, char n, int offset)
; put_number(int number, char n, char x, char y)
; ----


_put_number.3:	maplibfunc	lib2_put_number.3
		rts

_put_number.4:	maplibfunc	lib2_put_number.4
		rts


; put_hex(int number, char n, int offset)
; put_hex(int number, char n, char x, char y)
; ----

_put_hex.4:
	  ldy	<__arg_idx
	__plb
	  jsr	_put.xy
	  bra	_put_hex.main
_put_hex.3:
	  ldy	<__arg_idx
	  jsr	_put.vram
_put_hex.main:
	__plb	<_cl,1
	__plw	<_dx
	  sty	<__arg_idx
	  ; --
	  txa
	  beq	.l3
.l1:	  cpx	#5
	  blo	.l2
	  cla
	  jsr	_put_digit.sub
	  dex
	  bra	.l1
	  ; --
.l2:	  txa
	  dec	A
	  asl	A
	  tax
	  jmp	[.tbl,X]
.l3:	  rts
	  ; --
.tbl:	 .dw	.h1,.h2,.h3,.h4
	  ; --
.h4:	  lda	<_dh
	  lsr	A
	  lsr	A
	  lsr	A
	  lsr	A
	  jsr	_put_digit.sub
	  ; --
.h3:	  lda	<_dh
	  and	#$0F
	  jsr	_put_digit.sub
	  ; --
.h2:	  lda	<_dl
	  lsr	A
	  lsr	A
	  lsr	A
	  lsr	A
	  jsr	_put_digit.sub
	  ; --
.h1:	  lda	<_dl
	  and	#$0F
	  jmp	_put_digit.sub

; put_string(char *string, int offset)
; put_string(char *string, char x, char y)
; ----

_put_string.3:
	  ldy	<__arg_idx
	__plb
	  jsr	_put.xy
	  bra	_put_string.main
_put_string.2:
	  ldy	<__arg_idx
	  jsr	_put.vram
_put_string.main:
	__plw	<_si
	  sty	<__arg_idx
	  bra	.l3
	  ; --
.l1:	  cmp	#32
	  bhs	.l2
	  lda	#32
	  sec
.l2:	  sbc	#32
	  add	_font_base
	  sta	video_data_l
	  cla
	  adc	_font_base+1
	  sta	video_data_h
	  incw	<_si
.l3:	  lda	[_si]
	  bne	.l1
	  rts

; vsync(char nb_frame)
; ----

_vsync:
	  txa
	  cpy	#0
	  bne	.l1
	  cla
.l1:	  jmp	wait_vsync

; vreg(char reg)
; ----

_vreg:
	  cpy	#2
	  bne	.l1
	  jmp	setvdc
.l1:
	  stx	<vdc_reg
	  stx	video_reg
	  rts

; vram_addr(char x [al], char y)
; ----

_vram_addr.2:
	  lda	<_al
	  sax
	  jsr	calc_vram_addr
	__ldw	<_di
	  rts

; scan_map_table(int *tbl [si], int *x [ax], int *y [cx])
; ----
; tbl,
; x,
; y,
; ----

_scan_map_table.3:

	  ldy	#1
	  lda	[_ax]
	  sta	<_bl
	  lda	[_ax],Y
	  sta	<_bh
	  lda	[_cx]
	  sta	<_dl
	  lda	[_cx],Y
	  sta	<_dh
	  ; --
	  addw	#4,<_si

    ; ----
    ; check bounds
    ;
	  ; -- bottom
.l1:	  ldy	#7
	  lda	[_si],Y
	  cmp	<_dh
	  blo	.x1
	  bne	.l2
	  dey
	  lda	[_si],Y
	  cmp	<_dl
	  blo	.x1
	  ; -- top
.l2:	  ldy	#3
	  lda	<_dh
	  cmp	[_si],Y
	  blo	.x1
	  bne	.l3
	  dey
	  lda	<_dl
	  cmp	[_si],Y
	  blo	.x1
	  ; -- right
.l3:	  ldy	#5
	  lda	[_si],Y
	  cmp	<_bh
	  blo	.x1
	  bne	.l4
	  dey
	  lda	[_si],Y
	  cmp	<_bl
	  blo	.x1
	  ; -- left
.l4:	  ldy	#1
	  lda	<_bh
	  cmp	[_si],Y
	  blo	.x1
	  bne	.x2
	  lda	<_bl
	  cmp	[_si]
	  bhs	.x2

    ; ----
    ; next
    ;
.x1:	  addw	#12,<_si
	  ldy	#1
	  lda	[_si]
	  and	[_si],Y
	  cmp	#$FF
	  bne	.l1

    ; ----
    ; didn't find map...
    ;
    	  clx
	  cla
	  rts

    ; ----
    ; found map!
    ;
.x2:	  ldy	#1
	  lda	<_bl
	  sub	[_si]
	  sta	[_ax]
	  lda	<_bh
	  sbc	[_si],Y
	  sta	[_ax],Y
	  ; --
	  iny
	  lda	<_dl
	  sub	[_si],Y
	  sta	[_cx]
	  iny
	  lda	<_dh
	  sbc	[_si],Y
	  ldy	#1
	  sta	[_cx],Y
	  ; --
	__ldw	<_si
	  rts

; set_map_data(int *ptr)
; set_map_data(char *map [bl:si], int w [ax], int h)
; set_map_data(char *map [bl:si], int w [ax], int h [dx], char wrap)
; ----
; map,  map base address
; w,    map width
; h,    map height
; wrap, wrap flag (1 = wrap, 0 = do not wrap)
; ----

_set_map_data.1:
	__stw	<_si
	  ora	<_si
	  beq	.l1
	  ; -- calculate width
	  lda	[_si].4
	  sub	[_si]
	  sta	mapwidth
	  lda	[_si].5
	  sbc	[_si].1
	  sta	mapwidth+1
	  incw	mapwidth
	  ; -- calculate height
	  lda	[_si].6
	  sub	[_si].2
	  sta	mapheight
	  lda	[_si].7
	  sbc	[_si].3
	  sta	mapheight+1
	  incw	mapheight
	  ; -- get map bank
	  lda	[_si].8
	  sta	mapbank
	  ; -- get map addr
	  lda	[_si].10
	  sta	mapaddr
	  iny
	  lda	[_si]
	  sta	mapaddr+1
	  ; -- no wrap
	  stz	mapwrap
	  rts
	  ; -- null pointer
.l1:	  stwz	mapwidth
	  stwz	mapheight
	  stz	mapbank
	  stwz	mapaddr
	  stz	mapwrap
	  rts
_set_map_data.4:
	  stx	mapwrap
	__ldw	<_dx
	  bra	_set_map_data.main
_set_map_data.3:
	  stz	mapwrap
	  inc	mapwrap
_set_map_data.main:
	__stw	mapheight
	  stw	<_ax,mapwidth
	  stb	<_bl,mapbank
	  stw	<_si,mapaddr
	  rts

; get_map_width()
; ----

_get_map_width:
	__ldw	mapwidth
	  rts

; get_map_height()
; ----

_get_map_height:
	__ldw	mapheight
	  rts

; set_tile_data(char *tile_ex [di])
; set_tile_data(char *tile [bl:si], int nb_tile [cx], char *ptable [al:dx])
; ----
; tile,    tile base index
; nb_tile, number of tile
; ptable,  tile palette table address
; ----

_set_tile_data.1:
	  cly
	  lda	[_di],Y++
	  sta	mapnbtile
	  lda	[_di],Y++
	  sta	mapnbtile+1
	  lda	[_di],Y++
	  sta	maptiletype
	  iny
	  lda	[_di],Y++
	  sta	maptilebank
	  iny
	  lda	[_di],Y++
	  sta	maptileaddr
	  lda	[_di],Y++
	  sta	maptileaddr+1
	  lda	#(CONST_BANK+_bank_base)
	  sta	mapctablebank
	  lda	[_di],Y++
	  sta	mapctable
	  lda	[_di],Y
	  sta	mapctable+1
	  rts
_set_tile_data.3:
	  stb	<_bl,maptilebank
	  stw	<_si,maptileaddr
	  stw	<_cx,mapnbtile
	  stb	<_al,mapctablebank
	  stw	<_dx,mapctable
	  ; --
	  ldy	<_bl		; get tile format (8x8 or 16x16)
	  lda   <_si+1
	  and	#$1F
	  tax
	  lda	<_si
	  bne	.l2
	  cpx	#$0
	  bne	.l1
	  dey
	  ldx	#$20
.l1:	  dex
.l2:	  dec	A
	  txa
	  ora	#$60
	  sta	<_si+1
	  tya
	  tam	#3
	  lda	[_si]
	  sta	maptiletype
	  rts

; load_tile(int addr)
; ----

_load_tile:
	__stw	<_di
	  stx	<_al
	  lsr	A
	  ror	<_al
	  lsr	A
	  ror	<_al
	  lsr	A
	  ror	<_al
	  lsr	A
	  ror	<_al
	  sta	     maptilebase+1
	  stb	<_al,maptilebase
	  ; --
	  stw	mapnbtile,<_cx
	  ldx	#4
	  lda	maptiletype
	  cmp	#8
	  beq	.l1
	  ldx	#6
.l1:	  asl	<_cl
	  rol	<_ch
	  dex
	  bne	.l1
	  ; --
	  stb	maptilebank,<_bl
	  stw	maptileaddr,<_si
	  jmp	load_vram

; load_map(char x [al], char y [ah], int mx, int my, char w [dl], char h [dh])
; ----

_load_map.6:

	  tstw	mapwidth
	  lbeq	.l6
	  tstw	mapheight
	  lbeq	.l6

    ; ----
    ; adjust map y coordinate
    ;
	  ldy	<__arg_idx
	__plw	<_bx
	  bmi	.l2
.l1:	  cmpw	mapheight,<_bx
	  blo	.l3
	  subw	mapheight,<_bx
	  bra	.l1
	  ; --
.l2:	  lda	<_bh
	  bpl	.l3
	  addw	mapheight,<_bx
	  bra	.l2
	  
    ; ----
    ; adjust map x coordinate
    ;
.l3:	  stb	<_bl,<_ch
	__plw	<_bx
	  sty	<__arg_idx
	  bmi	.l5
.l4:	  cmpw	mapwidth,<_bx
	  blo	.l7
	  subw	mapwidth,<_bx
	  bra	.l4
	  ; --
.l5:	  lda	<_bh
	  bpl	.l7
	  addw	mapwidth,<_bx
	  bra	.l5

    ; ----
    ; exit
    ;
.l6:	  sub	#4,<__arg_idx
	  rts

    ; ----
    ; ok
    ;
.l7:	  stb	<_bl,<_cl
	  jmp	load_map

; spr_set(char num)
; ----
; load SI with the offset of the sprite to change
; SI = satb + 8 * sprite_number
; ----

_spr_set:
	  cpx	#64
	  bhs	.l2
	  txa
	  inx
	  cpx	<__spr_max
	  blo	.l1
	  stx	<__spr_max
	  ; --
.l1:	  stz	<__spr_ptr+1
	  asl	A
	  asl	A
	  asl	A
	  rol	<__spr_ptr+1
	  adc	#low(_satb)
	  sta	<__spr_ptr
	  lda	<__spr_ptr+1
	  adc	#high(_satb)
	  sta	<__spr_ptr+1
.l2:	  rts

; spr_hide(char num)
; ----

_spr_hide:
	  cpy	#0
	  beq	.l2
	  ; -- hide sprite number #
	  cpx	#64
	  bhs	.l1
	  jsr	_spr_hide.sub
	  lda	[__ptr],Y
	  ora	#$02
	  sta	[__ptr],Y
.l1:	  rts
	  ; -- hide current sprite
.l2:	  ldy	#1
	  lda	[__spr_ptr],Y
	  ora	#$02
	  sta	[__spr_ptr],Y
	  rts
	  ; -- calc satb ptr
_spr_hide.sub:
	  txa
	  stz	<__ptr+1
	  asl	A
	  asl	A
	  asl	A
	  rol	<__ptr+1
	  adc	#low(_satb)
	  sta	<__ptr
	  lda	<__ptr+1
	  adc	#high(_satb)
	  sta	<__ptr+1
	  ldy	#1
	  rts

; spr_show(char num)
; ----

_spr_show:
	  cpy	#0
	  beq	.l2
	  ; -- hide sprite number #
	  cpx	#64
	  bhs	.l1
	  jsr	_spr_hide.sub
	  lda	[__ptr],Y
	  and	#$01
	  sta	[__ptr],Y
.l1:	  rts
	  ; -- hide current sprite
.l2:	  ldy	#1
	  lda	[__spr_ptr],Y
	  and	#$01
	  sta	[__spr_ptr],Y
	  rts

; spr_x(int value)
; ----

_spr_x:
	  ldy	#2
	  sax
	  add	#32
	  sta	[__spr_ptr],Y
	  sax
	  adc	#0
	  iny
	  sta	[__spr_ptr],Y
	  rts

_spr_get_x:
	  ldy	#2
	  lda	[__spr_ptr],Y
	  sub	#32
	  tax
	  iny
	  lda	[__spr_ptr],Y
	  sbc	#0
	  rts

; spr_y(int value)
; ----

_spr_y:
	  sax
	  add	#64
	  sta	[__spr_ptr]
	  sax
	  adc	#0
	  and	#$01
	  ldy	#1
	  sta	[__spr_ptr],Y
	  rts

_spr_get_y:
	  lda	[__spr_ptr]
	  sub	#64
	  tax
	  ldy	#1
	  lda	[__spr_ptr],Y
	  sbc	#0
	  rts

; spr_pattern(int vaddr)
; ----

_spr_pattern:
	  sta	<__temp
	  txa
	  asl	A
	  rol	<__temp
	  rol	A
	  rol	<__temp
	  rol	A
	  rol	<__temp
	  rol	A
	  and	#$7
	  ldy	#5
	  sta	[__spr_ptr],Y
	  lda	<__temp
	  dey
	  sta	[__spr_ptr],Y
	  rts

_spr_get_pattern:
	  ldy	#4
	  lda	[__spr_ptr],Y
	  sta	<__temp
	  iny
	  lda	[__spr_ptr],Y
	  lsr   A
	  ror	<__temp
	  ror   A
	  ror	<__temp
	  ror   A
	  ror	<__temp
	  ror	A
	  and	#$E0
	  tax
	  lda	<__temp
	  rts

; spr_ctrl(char mask [al], char value)
; ----

_spr_ctrl.2:
	  txa
	  and	<_al
	  sta	<__temp
	  lda	<_al
	  eor	#$FF
	  ldy	#7
	  and	[__spr_ptr],Y
	  ora	<__temp
	  sta	[__spr_ptr],Y
	  rts

; spr_pal(char pal)
; ----

_spr_pal:
	  txa
	  and	#$0F
	  sta	<__temp
	  ldy	#6
	  lda	[__spr_ptr],Y
	  and	#$F0
	  ora	<__temp
	  sta	[__spr_ptr],Y
	  rts

_spr_get_pal:
	  ldy	#6
	  lda	[__spr_ptr],Y
	  and	#$0F
	  tax
	  cla
	  rts

; spr_pri(char pri)
; ----

_spr_pri:
	  ldy	#6
	  lda	[__spr_ptr],Y
	  and	#$7F
	  cpx	#$00
	  beq	.l1
	  ora	#$80
.l1:
	  sta	[__spr_ptr],Y
	  rts

; satb_update()
; ----

_satb_update:
	  lda	<__spr_flag
	  beq	.l1
	  stz	<__spr_flag
	  ldx	#64
	  bra	.l3
	  ; --
.l1:	  cpy	#1
	  beq	.l2
	  ldx	<__spr_max
.l2:	  cpx	#0
	  beq	.l4
	  ; --
.l3:	  stx	<_al	; number of sprites
	  txa
	  dec	A	; round up to the next group of 4 sprites
	  lsr	A
	  lsr	A
	  inc	A
	  sta	<_cl

; Use TIA, but BLiT 16 words at a time (32 bytes)
; Because interrupt must not deferred too much
;
	  stw	#32, _ram_hdwr_tia_size
	  stw	#video_data, _ram_hdwr_tia_dest
	  stw	#_satb, <_si

	  stw	#$7F00, <_di
	  jsr	set_write

.l3a:	  stw	<_si, _ram_hdwr_tia_src
	  jsr	_ram_hdwr_tia
	  addw	#32,<_si
	  dec	<_cl
	  bne	.l3a
	  
;.l3:	  stx	<_al
;	  stw	#_satb,<_si
;	  stb	#BANK(_satb),<_bl
;	  stw	#$7F00,<_di
;	  txa
;	  stz	<_ch
;	  asl	A
;	  asl	A
;	  rol	<_ch
;	  sta	<_cl
;	  jsr	load_vram

	  ; --
	  ldx	<_al
.l4:	  cla
	  rts


; init_satb()
; reset_satb()
; ----

_reset_satb:
_init_satb:
	  clx
	  cla
.l1:	  stz	_satb,X
	  stz	_satb+256,X
	  inx
	  bne	.l1
	  ; --
	  ldy	#1
	  sty	<__spr_flag
	  stz	<__spr_max
	  rts

; get_color(int index [color_reg])
; ----
; index: index in the palette (0-511)
; ----

_get_color.1:
	  ldx	color_data_l
	  lda	color_data_h
	  and	#$01
	  rts

; set_color(int index [color_reg], int color [color_data])
; ----
; set one palette entry to the specified color
; ----
; index: index in the palette (0-511)
; color: color value,  GREEN: bit 6-8
;                      RED:   bit 3-5
;                      BLUE:  bit 0-2
; ----
; NOTE : inlined
; ----

; fade_color(int color [ax], char level)
; fade_color(int index [color_reg], int color [ax], char level)
; ----
; set one palette entry to the specified color
; ----
; index: index in the palette (0-511)
; color: color value,  GREEN: bit 6-8
;                      RED:   bit 3-5
;                      BLUE:  bit 0-2
; level: level of fading (0 = black, 8 = full)
; ----

_fade_color.2:
_fade_color.3:
	  cpx	#0
	  beq	.l4
	  cpx	#8
	  bhs	.l5
	  ; -- fading
	  ldy	#3
	  stx	<_bl
	  stwz	<_dx
.l1:	  lsr	<_bl
	  bcc	.l2
	  addw	<_ax,<_dx
.l2:	  aslw	<_ax
	  dey
	  bne	.l1
	  lda	<_dh
	  lsr	A
	  ror	<_dl
	  lsr	A
	  ror	<_dl
	  lsr	A
	  ror	<_dl
	  ; -- set color
	  ldx	<_dl
.l3:	  stx	color_data_l
	  sta	color_data_h
	  rts
	  ; -- black
.l4:	  cla
	  bra	.l3
	  ; -- full
.l5:	  ldx	<_al
	  lda	<_ah
	  bra	.l3

; set_color_rgb(int index [color_reg], char r [al], char g [ah], char b)
; ----
; set one palette entry to the specified color
; ----
; index: index in the palette (0-511)
; r:     red              RED:   bit 3-5
; g:     green            GREEN: bit 6-8
; b:     blue             BLUE:  bit 0-2
; ----

_set_color_rgb.4:
	  txa
	  and	#$7
	  sta	<__temp
	  lda	<_al
	  asl	A
	  asl	A
	  asl	A
	  ora	<__temp
	  asl	A
	  asl	A
	  sta	<__temp
	  lda	<_ah
	  lsr	A
	  ror	<__temp
	  lsr	A
	  ror	<__temp
	  ldx	<__temp
	  stx	color_data_l
	  sta	color_data_h
	  rts

; put_tile(int tile_num [dx], int position)
; put_tile(int tile_num [dx], char x [al], char y)
; ----
; draw a single 8x8 or 16x16 tile at a given position
; ----
; pattern:  vram address of the tile pattern
; position: position on screen where to put the tile
; ----

_put_tile.3:
	  lda	<_al
	  ldy	maptiletype
	  cpy	#8
	  beq	.l1
	  ; --
	  asl	A
	  sax
	  asl	A
	  jsr	calc_vram_addr
	  bra	_put_tile_16
	  ; --
.l1:	  sax
	  jsr	calc_vram_addr
	  bra	_put_tile_8
_put_tile.2:
	__stw	<_di
	  ldy	maptiletype
	  cpy	#8
	  bne	_put_tile_16
_put_tile_8:
	  jsr	set_write
	  ; -- calculate tile vram address
	  stw	mapctable,<_bx
	  lda	<_dl
	  tay
	  add	maptilebase
	  tax
	  cla
	  adc	maptilebase+1
	  adc	[_bx],Y
	  ; -- copy tile
	  stx	video_data_l
	  sta	video_data_h
	  rts
_put_tile_16:
	  jsr	set_write
	  ; -- calculate tile vram address
	  stw	mapctable,<_bx
	  stz	<_dh
	  lda	<_dl
	  tay
	  asl   A
	  rol   <_dh
	  asl   A
	  rol   <_dh
	  add   maptilebase
	  sta   <_dl
	  lda   <_dh
	  adc   maptilebase+1
	  adc   [_bx],Y
	  sta   <_dh
	  ; -- copy tile
	  stw   <_dx,video_data
	  incw  <_dx
	  stw   <_dx,video_data
	  incw  <_dx
	  vreg  #0
	  addw  bat_width,<_di,video_data
	  vreg  #2
	  stw   <_dx,video_data
	  incw  <_dx
	  stw   <_dx,video_data
	  rts

; map_get_tile(char x [dl], char y)
; map_put_tile(char x [dl], char y [dh], char tile)
; ----

_map_get_tile.2:
	  stx	<_dh
	  jsr	_map_calc_tile_addr
	  ; --
	  lda	[_cx]
	  tax
	  cla
	  rts

_map_put_tile.3:
	  phx
	  jsr	_map_calc_tile_addr
	  pla
	  sta	[_cx]
	  rts

; map_calc_tile_addr(char x [dl], char y [dh])
; ----
_map_calc_tile_addr:
	  ldx	<_dh
	  lda	mapwidth+1
	  beq	.l1
	  stx	<_ch
	  lda	<_dl
	  sta	<_cl
	  bra	.l2
	  ; --
.l1:	  stx	<_al
	  lda   mapwidth
	  sta	<_bl
	  jsr	mulu8
	  ; --
	  lda	<_cl
	  add	<_dl
	  bcc	.l2
	  inc	<_ch
	  ; --
.l2:	  add	mapaddr
	  sta	<_cl
	  lda	mapaddr+1
	  and	#$1F
	  adc	<_ch
	  tax
	  ; --
;	  rol	A
;	  rol	A
;	  rol	A
;	  rol	A
	  lsr	A
	  lsr	A
	  lsr	A
	  lsr	A
	  lsr	A
	  and	#$0F
	  add	mapbank
	  tam	#3
	  ; --
	  txa
	  and	#$1F
	  ora	#$60
	  sta	<_ch
	  ldx	<_cl
	  rts

; scroll(char num, int x, int y, char top, char bottom, char disp)
; ----
; set screen scrolling
; ----

_scroll:
	  ldy	#8
	  lda   [__sp],Y
	  and   #$03
	  ; --
	  sax
	  and   #$C0
	  ora   #$01
	  sta   scroll_cr,X
	  lda   [__sp]
	  inc   A
	  sta   scroll_bottom,X
	  ldy   #2
	  lda   [__sp],Y
	  sta   scroll_top,X
	  ldy   #4
	  lda   [__sp],Y
	  sta   scroll_yl,X
	  iny
	  lda   [__sp],Y
	  sta   scroll_yh,X
	  iny
	  lda   [__sp],Y
	  sta   scroll_xl,X
	  iny
	  lda   [__sp],Y
	  sta   scroll_xh,X
	  addw	#10,<__sp
	  rts

; scroll_disable(char num)
; ----
; disable screen scrolling for a scroll region
; ----

_scroll_disable:
	  lda	scroll_cr,X
	  and	#$fe
	  sta	scroll_cr,X
	  rts

; set_screen_size(char size)
; ----
; set screen virtual size
; ----

_set_screen_size:
	  txa
	  jmp	set_bat_size

; set_xres(int xres)
; ----
; set horizontal display resolution
; ----

_set_xres.1:
	  lda	#XRES_SOFT
	  sta	<_cl
_set_xres.2:
	  jsr	set_xres
	  ldx	<_al
	  lda	<_ah
	  rts


; ------------------------
; Graphics functions
; ------------------------

; readvram
; ----
; leftover from asm library
; needed for 'a = vram[n]'
; semantic
; ----
readvram:
	  ldy	#1
	  sty	<vdc_reg
	  sty	video_reg
	  stx	video_data_l
	  sta	video_data_h
	  vreg	#$02
	  ldx	video_data_l
	  lda	video_data_h
	  rts


; writevram
; ----
; leftover from asm library
; needed for 'vram[n] = a'
; semantic
; ----
writevram:
	  tay
	  stz	<vdc_reg
	  stz	video_reg
	  lda	[__sp]
	  sta	video_data_l
	  incw	<__sp
	  lda	[__sp]
	  sta	video_data_h
	  incw	<__sp
	  vreg	#2
	  stx	video_data_l
	  sty	video_data_h
	  rts


; gfx_setbgpal(char pal)
; ----
; set default major palette for gfx_* func's
; ----

_gfx_setbgpal:
	  txa
	  asl	A
	  asl	A
	  asl	A
	  asl	A
	  sta	_gfx_pal
	  rts


; gfx_init(int start_vram_addr)
; ----
; initialize graphics mode
;  - points graphics map to tiles at start_vram_addr
; ----

_gfx_init:
	maplibfunc lib2_gfx_init
	rts

	.bank LIB2_BANK
lib2_gfx_init:
	__stw	<_dx	; vram addr

	  lsrw	<_dx	; shift address to make char pattern
	  lsrw	<_dx
	  lsrw	<_dx
	  lsrw	<_dx
	  lda	<_dx+1
	  and	#$0f
	  ora	_gfx_pal	; and add major palette info
	  sta	<_dx+1

	  setvwaddr $0
	  ; --
	  ldy	bat_height
.l2:	  ldx	bat_width
	  ; --
.l3:	  stw	<_dx,video_data
	  incw	<_dx
	  dex
	  bne	.l3
	  dey
	  bne	.l2
	  rts
	.bank LIB1_BANK

; gfx_clear(int start_vram_addr)
; ----
; Clear the values in the graphics tiles
; - places zeroes in graphics tiles at start_vram_addr
; ----

_gfx_clear:
	__stw	<_di		; start_vram_addr
	  jsr	set_write	; setup VRAM addr for writing

	  lda	bat_height
	  sta	<_bl		; loop for all lines
.l2:	  ldx	bat_width	; loop for all characters
.l3:	  ldy	#8		; loop for 16 words
.l4:	  stw	#0,video_data	; unrolled a bit (8 iterations
	  stw	#0,video_data	; @ 2 words each iteration)
	  dey
	  bne	.l4 
	  dex
	  bne	.l3
	  dec	<_bl
	  bne	.l2
	  rts


; gfx_plot(int x [bx] int y [cx] char color [reg acc])
; ----
; Plot a point at location (x,y) in color
; ----

_gfx_plot.3:
	  maplibfunc	lib2_gfx_plot.3
	  rts


; gfx_point(int x [bx], int y [cx])
; ----
; Returns color of point at location (x,y)
; ----

_gfx_point.2:
	  maplibfunc	lib2_gfx_point.2
	  rts


; gfx_line(int x1 [bx], int y1 [cx], int x2 [si], int y2 [bp], char color [reg acc])
; ----
; Plot a line from location (x1,y1) to location (x2,y2) in color
; ----

_gfx_line.5:
	  maplibfunc	lib2_gfx_line.5
	  rts

;---------------------------------

;
; Change to context LIB2_BANK for these functions
; because they are larger than LIB1_BANK functions
; should be
;

	 .bank	LIB2_BANK

; put_number(int number, char n, int offset)
; put_number(int number, char n, char x, char y)
; ----

lib2_put_number.4:
	  ldy	<__arg_idx
	__plb
	  jsr	_put.xy
	  bra	putnum.main
lib2_put_number.3:
	  ldy	<__arg_idx
	  jsr	_put.vram
putnum.main:
	__plb	<_cl,1
	__plw	<_dx
	  sty	<__arg_idx
	  ; --
	  stz	<_al ; sign flag
	  dex
	  cpx	#16
	  bhs	.l5
	  ; --
	  lda	<_dh ; check sign
	  bpl	.l1
	  negw	<_dx ; negate
	  lda	#1
	  sta	<_al
	  ; --
.l1:	  jsr	divu10
	  ora	#$10
	  pha
	  dex
	  bmi	.l3
	  tstw	<_dx
	  bne	.l1
    	  ; --
	  lda	<_al
	  beq	.l2
	  lda	#$0D
	  pha
	  dex
	  bmi	.l3
    	  ; --
	  cla
.l2:	  pha
	  dex
	  bpl	.l2
	  ; --
.l3:	  ldx	<_cl
.l4:	  pla
	  add	_font_base
	  sta	video_data_l
	  cla
	  adc	_font_base+1
	  sta	video_data_h
	  dex
	  bne	.l4
.l5:	  rts


; gfx_line(int x1 [bx], int y1 [cx], int x2 [si], int y2 [bp], char color [reg acc])
; ----
; Plot a line from location (x1,y1) to locations (x2,y2) in color
; ----
lib2_gfx_line.5:		; Bresenham line drawing algorithm
	  stx	_line_color

	  cmpw	<_cx,<_bp	; make y always ascending by swapping
	  bhs	.l1		; co-ordinates
				; jump over swap if _bp > _cx

	  stw	<_bp,_line_curry	; swap coordinates
	  stw	<_cx,<_bp
	  stw	<_si,_line_currx
	  stw	<_bx,<_si

	  bra	.l2

.l1:	  stw	<_bx,_line_currx
	  stw	<_cx,_line_curry

; now:
;  line_currx and line_curry are start point
;  <_si and <_bp are end point
;  <_bx and <_cx are 'dont care'

.l2:
	  lda	LOW_BYTE  <_bp
	  sub	LOW_BYTE  _line_curry
	  sta	LOW_BYTE  _line_deltay
	  lda	HIGH_BYTE <_bp
	  sbc	HIGH_BYTE _line_curry
	  sta	HIGH_BYTE _line_deltay

	  lda	LOW_BYTE  <_si
	  sub	LOW_BYTE  _line_currx
	  sta	LOW_BYTE  _line_deltax
	  lda	HIGH_BYTE <_si
	  sbc	HIGH_BYTE _line_currx
	  sta	HIGH_BYTE _line_deltax

	  stz	_line_xdir	; 0 = positive

	  lda	HIGH_BYTE _line_deltax
	  bpl	.l3

	  lda	#1
	  sta	_line_xdir	; 1 = negative
	  negw	_line_deltax

; now:
;  line_deltay is difference from end to start (positive)
;  line_deltax is difference from end to start (positive)
;  line_xdir shows whether to apply deltax positive or negative


.l3:
	  cmpw	_line_deltax,_line_deltay
	  lbhs	.ybiglp		; jump if deltay > |deltax|

.xbiglp:
	__ldw	_line_deltay
	__aslw
	__stw	_line_adjust
	__stw	_line_error

	  subw	_line_deltax,_line_adjust
	  subw	_line_deltax,_line_adjust

	  subw	_line_deltax,_line_error

	  incw	_line_deltax		; used as counter - get both endpoints

.xlp1:
	  stw	_line_currx,<_bx	; draw pixel
	  stw	_line_curry,<_cx
	  ldx	_line_color
	  cla
	  jsr	lib2_gfx_plot.3

	  decw	_line_deltax		; dec counter
	  tstw	_line_deltax
	  lbeq	.out

	  lda	_line_xdir		; adjust currx
	  beq	.xlppos

	  decw	_line_currx
	  bra	.xlp2

.xlppos:  incw	_line_currx

.xlp2:
	  lda	HIGH_BYTE _line_error
	  bmi	.xlp3

	  addw	_line_adjust,_line_error
	  incw	_line_curry
	  bra	.xlp1
.xlp3:
	  addw	_line_deltay,_line_error
	  addw	_line_deltay,_line_error
	  jmp	.xlp1

.ybiglp:
	__ldw	_line_deltax
	__aslw
	__stw	_line_adjust
	__stw	_line_error

	  subw	_line_deltay,_line_adjust
	  subw	_line_deltay,_line_adjust
	
	  subw	_line_deltay,_line_error

	  incw	_line_deltay		; used as counter - get both endpoints

.ylp1:
	  stw	_line_currx,<_bx	; draw pixel
	  stw	_line_curry,<_cx
	  ldx	_line_color
	  cla
	  jsr	lib2_gfx_plot.3

	  decw	_line_deltay		; dec counter
	  tstw	_line_deltay
	  beq	.out

	  incw	_line_curry

	  lda	HIGH_BYTE _line_error
	  bmi	.ylp2

	  addw	_line_adjust,_line_error
	  lda	_line_xdir
	  beq	.ylppos

	  decw	_line_currx
	  bra	.ylp1

.ylppos:  incw	_line_currx
	  bra	.ylp1

.ylp2:
	  addw	_line_deltax,_line_error
	  addw	_line_deltax,_line_error
	  jmp	.ylp1

.out:
	  rts


; gfx_plot(int x [bx], int y [cx], char color [reg acc])
; ----
; Plot a point at location (x,y) in color
; ----

lib2_gfx_plot.3:
	  stx	<_dl		; color
	  jsr	gfx_getaddr

	; same as vm_rawread - save 21 cycles by inlining
	;
	  vreg	#1		; video read register
	  stw	<_cx,video_data	; VRAM address
	  vreg	#2		; set R/W memory mode
	__ldw	video_data
	;
	; end inline

	  ldy	<_al		; bit offset
	  bbr1	<_dl,.l1
	  ora	gfx_bittbl,Y	; set bit
	  bra	.l1a
.l1:	  and	gfx_bittbl2,Y	; else mask bit
.l1a:
	  sax
	  bbr0	<_dl,.l2
	  ora	gfx_bittbl,Y	; set bit
	  bra	.l2a
.l2:	  and	gfx_bittbl2,Y	; else mask bit
.l2a:
	; same as vm_rawwrite - save >14 cycles by inlining
	;
	  phx
	  tax
	  vreg	#0		; video write register
	  stw	<_cx,video_data	; VRAM address
	  vreg	#2		; set R/W memory mode
	  pla
	__stw	video_data	; write
	;
	; end inline

	  addw	#8,<_cx		; other half of pixel

	; same as vm_rawread - save 21 cycles by inlining
	;
	  vreg	#1		; video read register
	  stw	<_cx,video_data	; VRAM address
	  vreg	#2		; set R/W memory mode
	__ldw	video_data
	;
	; end inline

	  ldy	<_al		; bit offset
	  bbr3	<_dl,.l3
	  ora	gfx_bittbl,Y	; set bit
	  bra	.l3a
.l3:	  and	gfx_bittbl2,Y	; else mask bit
.l3a:
	  sax
	  bbr2	<_dl,.l4
	  ora	gfx_bittbl,Y	; set bit
	  bra	.l4a
.l4:	  and	gfx_bittbl2,Y	; mask bit
.l4a:
	; same as vm_rawwrite - save >14 cycles by inlining
	;
	  phx
	  tax
	  vreg	#0		; video write register
	  stw	<_cx,video_data	; VRAM address
	  vreg	#2		; set R/W memory mode
	  pla
	__stw	video_data	; write
	;
	; end inline

	  rts


; gfx_point(int x [bx], int y [cx])
; ----
; Returns color of point at location (x,y)
; ----

lib2_gfx_point.2:
	  jsr	gfx_getaddr
	  stz	<_ah		; will be color
	__ldw	<_cx		; VRAM address
	  jsr	readvram

	  ldy	<_al		; bit offset
	  and	gfx_bittbl,Y
	  beq	.l1
	  smb1	<_ah
.l1:	  txa
	  and	gfx_bittbl,Y
	  beq	.l2
	  smb0	<_ah
.l2:
	  addw	#8,<_cx
	__ldw	<_cx		; VRAM address part 2
	  jsr	readvram

	  ldy	<_al
	  and	gfx_bittbl,Y
	  beq	.l3
	  smb3	<_ah
.l3:	  txa
	  and	gfx_bittbl,Y
	  beq	.l4
	  smb2	<_ah
.l4:
	  ldx	<_ah
	  cla
	  rts


gfx_bittbl:
	  .db	$80,$40,$20,$10,$08,$04,$02,$01
gfx_bittbl2:
	  .db	$7f,$bf,$df,$ef,$f7,$fb,$fd,$fe


; gfx_getaddr
; ----
; Utility routine to switch x/y pixel
; co-ordinates into VRAM addr and bit #
; ----

gfx_getaddr:
	  lda	<_cl
	  and	#7
	  sta	<_al	; al = lines from tile base

	  lda	<_bl
	  and	#7
	  pha		; = bit offset

	__ldw	<_bx
	__lsrw		; should be only 2 bits in MSB are possible
	__lsrw		; but we'll shift 3 times anyway
	__lsrw
	  phx		; X = character column

	__ldw	<_cx
	__lsrw		; should be only 2 bits in MSB are possible
	__lsrw		; but we'll shift 3 times anyway
	__lsrw
	  txa		; A = character row

	  plx
	  jsr	calc_vram_addr

	__ldw	<_di		; to get BAT addr
	  jsr	readvram	; read BAT value
	__aslw			; change into VRAM tile addr
	__aslw
	__aslw
	__aslw			; cx = VRAM addr start of tile

	  sax
	  clc			; add row within tile
	  adc	<_al
	  sax
	  adc	#0
	__stw	<_cx

	  pla
	  sta	<_al		; al = bit offset

	  rts

; Change back to original LIB1_BANK context

	 .bank	LIB1_BANK



