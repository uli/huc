;
; SPRITES.INC  -  MagicKit sprite MACRO definitions
;


; load_sprites(vram_addr, spr_bank, #nb_group)
; ----
; vram_addr,	destination address in VRAM
; spr_bank,	sprite bank address
; nb_group,	number of 32x64 patterns to copy

.macro load_sprites
	; put the VRAM address in di
	stw	#\1,<di

	; put the sprite bank address in si/bl
	stw	#\2,<si
	stb	#BANK(\2),<bl

	; get the number of patterns to copy, multiply it by $200 -
	; the size in words of a 32x64 pattern (remember that
	; 'load_vram' need a size in words), and put it in cx
	lda	\3
	asl a
	stz	<cx
	sta	<cx+1

	; call the 'load_vram' function
	jsr load_vram
.endm


; spr_set(#sprite, satb)
; ----
; sprite,	the sprite number (0-63)
; satb,		the address of the SATB in RAM

.macro spr_set
	; multiply the sprite number by 8 (the size of a SATB entry)
	; and put the result in si
	stz	<si+1
	lda	\1
	asl a
	asl a
	asl a
	rol	<si+1
	sta	<si

	; add the satb address to si
	addw	#\2,<si
.endm


; spr_x(#x [,offset])
; ----
; x, the new x coordinate

.macro spr_x
	ldy	#2
.if (\# = 2)
	lda	LOW_BYTE \1
	clc
	adc	LOW_BYTE \2
	sta	[si],Y
	lda	HIGH_BYTE \1
	adc	HIGH_BYTE \2
.else
	lda	LOW_BYTE \1
	sta	[si],Y
	lda	HIGH_BYTE \1
.endif
	iny
	sta	[si],Y
.endm


; spr_y(#y [,offset])
; ----
; y, the new y coordinate

.macro spr_y
.if (\# = 2)
	lda	LOW_BYTE \1
	clc
	adc	LOW_BYTE \2
	sta	[si]
	lda	HIGH_BYTE \1
	adc	HIGH_BYTE \2
.else
	lda	LOW_BYTE \1
	sta	[si]
	lda	HIGH_BYTE \1
.endif
	ldy	#1
	sta	[si],Y
.endm


; spr_pattern(#addr)
; ----
; addr, address of the sprite pattern in VRAM

.macro spr_pattern
	ldy	#4
.if (\?1 = ARG_IMMED)
	lda	#LOW((\1) >> 5)
	sta	[si],Y
	lda	#HIGH((\1) >> 5)
.else
	lda	\1
	sta	[si],Y
	lda	\1+1
.endif
	iny
	sta	[si],Y
.endm


; spr_ctrl(#mask, #flag)
; ----
; mask, mask of the bits to change
; flag, new bit value

.macro spr_ctrl
	ldy	#7
	lda	\1
	eor	#$FF
	and	[si],Y
	ora	\2
	sta	[si],Y
.endm


; spr_pri(#flag)
; ----
; flag, new priority (1 = in foreground, 0 = in background)

.macro spr_pri
	ldy	#6
	lda	[si],Y
	and	#$7F
	ldx	\1
	beq	.x\@
	ora	#$80
.x\@:
	sta [si],Y
.endm


; spr_pal(#index)
; ----
; index, palette index (0-15)

.macro spr_pal
	ldy	#6
	lda	[si],Y
	and	#$F0
	ora	\1
	sta	[si],Y
.endm


; init_satb(addr)
; ----
; addr, the address of the local RAM SATB

.macro init_satb
	stz	\1
	tii	\1,\1+1,511
.endm


; update_satb(satb[, addr] [,nb_sprite])
; ----
; satb,		the address of the local RAM SATB
; addr,		the address where to copy the SATB in VRAM ($7F00 by default)
; nb_sprite,	number of sprite entry to copy (64 by default)

.macro update_satb
	stw	#\1,<si
	stb	#BANK(\1),<bl

.if (\?2)
	stw	#\2,<di
.else
	stw	#$7F00,<di
.endif

.if (\?3)
	stw	#(\3)*4,<cx
.else
	stw	#$100,<cx
.endif

	jsr	load_vram
.endm


; ----
FLIP_X_MASK	.equ $08
FLIP_Y_MASK	.equ $80
FLIP_MASK	.equ $88
SIZE_MASK	.equ $31

NO_FLIP		.equ 0
NO_FLIP_X	.equ 0
NO_FLIP_Y	.equ 0
FLIP_X		.equ $08
FLIP_Y		.equ $80
SIZE_16x16	.equ 0
SIZE_16x32	.equ $10
SIZE_16x64	.equ $30
SIZE_32x16	.equ $01
SIZE_32x32	.equ $11
SIZE_32x64	.equ $31

