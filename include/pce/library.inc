;
; LIBRARY.INC  -  MagicKit include-interface to LIBRARY.ASM
;

;
; These MACROs are just simple one-line setups to the
; back-end library subroutines.  They simply setup
; calling parameters, and call the subroutine
;


; set video memory read and write address:
;
; setvraddr(addr)
; ----
; addr, new address in VRAM to read at
;
.macro setvraddr
	stw	#\1,<_di
	jsr	set_read
.endm

; setvwaddr(addr)
; ----
; addr, new address in VRAM to write at
;
.macro setvwaddr
	stw	#\1,<_di
	jsr	set_write
.endm

; set_bgpal(index, data [, nb])
; ----
; index,	sub-palette index (0-15)
; data,		color data memory address
; nb,		number of sub-palette to load (optional, one by default)

.macro set_bgpal
	lda	\1
	sta	<_al
.if (\?2 = ARG_LABEL)
	stb	#BANK(\2),<_bl
.else
	stb	#$FE,<_bl
.endif
	stw	#\2,<_si
.if \?3
	lda	\3
.else
	lda	#1
.endif
	sta	<_cl
	jsr	load_palette
.endm

; set_sprpal(index, data [, nb])
; ----
; index,	sub-palette index (0-15)
; data,		palette data memory address
; nb,		number of sub-palette to update (optional, one by default)

.macro set_sprpal
	lda	\1
	add	#16
	sta	<_al
.if (\?2 = ARG_LABEL)
	stb	#BANK(\2),<_bl
.else
	stb	#$FE,<_bl
.endif
	stw	#\2,<_si
.if \?3
	lda	\3
.else
	lda	#1
.endif
	sta	<_cl
	jsr	load_palette
.endm

; setcolor(index, r, g, b)
; ----
; index, color index
; r,	red	(0-7)
; g,	green	(0-7)
; b,	blue	(0-7)

.macro setcolor
	stw	\1,color_reg
	stw	#(((\3) << 6) + ((\2) << 3) + (\4)),color_data
.endm

; vload([vram,] data, size)
; ----
; vram, VRAM base address
; data, video data memory address
; size, number of words to copy

.macro vload
.if (\# = 3)
	stw	#\1,<_di
.if (\?2 = ARG_LABEL)
	stb	#BANK(\2),<_bl
.else
	stb	#$FE,<_bl
.endif
	stw	#\2,<_si
	stw	\3,<_cx
.else
	stw	#VRAM(\1),<_di
	stb	#BANK(\1),<_bl
	stw	#\1,<_si
	stw	#\2,<_cx
.endif
	jsr	load_vram
.endm

; batcpy(vram, bat, w, h)
; ----
; vram,	VRAM base address
; bat,	BAT address in memory
; w,	nb of column to copy
; h,	nb of row

.macro batcpy
.if (\# = 5)
.fail "batcpy syntax has changed, update your code!"
.endif
	stw	\1,<_di
.if (\?2 = ARG_LABEL)
	stb	#BANK(\2),<_bl
.else
	stb	#$FE,<_bl
.endif
	stw	#\2,<_si
	lda	\3
	sta	<_cl
	lda	\4
	sta	<_ch
	jsr	load_bat
.endm

; setmap(map, tile, ctable, w ,h)
; ----
; map,		map base address
; tile,		tile vram address
; ctable,	tile palette table address
; w,		map width
; h,		map height

.macro setmap
	stw	#BANK(\1),mapbank
	stw	#\1,mapaddr
	stw	#(\2)>>4,maptilebase
	stw	#\3,mapctable
	stb	#BANK(\3),mapctablebank
	stw	\4,mapwidth
	stw	\5,mapheight
.endm

; mapcpy(vram, x, y, w, h)
; ----
; vram, VRAM base address
; x,	x start coordinate in the map
; y,	y start coordinate
; w,	nb of column to copy
; h,	nb of row

.macro mapcpy
	stw	#\1,<_di
	lda	\2
	sta	<_cl
	lda	\3
	sta	<_ch
	lda	\4
	sta	<_dl
	lda	\5
	sta	<_dh
	jsr	load_map
.endm

; fntcpy(vram, font, color, bg, nb)
; ----
; vram,		VRAM base address
; font,		font address in memory
; color, 	character color
; bg,		bg color
; nb,		nb of char. to copy

.macro fntcpy
	stw	#\1,<_di
	stb	#BANK(\2),<_bl
	stw	#\2,<_si
	lda	\3
	sta	<_al
	lda	\4
	sta	<_ah
	lda	\4
	sta	<_cl
	jsr	load_font
.endm

; scroll(num, x, y, top, bottom, cr)
; ----
; num,		scroll number (0-3)
; x,		BAT x position
; y,		BAT y position
; top,		screen window top
; bottom,	screen window bottom
; cr,		screen window display control (bg/sp)

.macro scroll
	lda	\1
	tax
	lda	LOW_BYTE  \2
	sta	scroll_xl,X
	lda	HIGH_BYTE \2
	sta	scroll_xh,X
	lda	LOW_BYTE  \3
	sta	scroll_yl,X
	lda	HIGH_BYTE \3
	sta	scroll_yh,X
	lda	\4
	sta	scroll_top,X
	lda	\5
	sta	scroll_bottom,X
	lda	\6
	ora	#$01
	sta	scroll_cr,X
.endm

; scroll_disable(num)
; ----
; num,	scroll number (0-3)

.macro scroll_disable
	lda	\1
	tax
	lda	scroll_cr,X
	and	#$fe
	sta	scroll_cr,X
.endm

; setxres (xres)
; ----
; xres,	horizontal display resolution

.macro setxres
	stw	#\1,<_ax
	jsr	set_xres
.endm

