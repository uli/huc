;
; SCROLL.ASM  -  MagicKit Scrolling Library
;
;

; [ 28] user scrolling vars
	.bss
scroll_xl:	.ds 4	; x       |
scroll_xh:	.ds 4	;         |
scroll_yl:	.ds 4	; y       |
scroll_yh:	.ds 4	;         | scrolling table
scroll_top:	.ds 4	; top     |
scroll_bottom:	.ds 4	; bottom  |
scroll_cr:	.ds 4	; control |

; [ 69] display list
	.bss
s_idx		.ds 1
s_xl		.ds 8
s_xh		.ds 8
s_yl		.ds 8
s_yh		.ds 8
s_cr		.ds 8
s_top		.ds 9
s_bottom	.ds 8
s_list		.ds 8
s_work		.ds 3

	.code
	.bank	LIB2_BANK
; ----
; build_display_list
; ----

build_disp_list:
	; ----
	; quick test
	;
	lda	scroll_cr
	ora	scroll_cr+1
	ora	scroll_cr+2
	ora	scroll_cr+3
	and	#$01
	bne	.l0
	; --
	clc
	rts

	; ----
	; parse user scroll list
	;
.l0:	clx
	cly
	; --
.l1:	lda	scroll_cr,Y
	and	#$01
	beq	.l2
	lda	scroll_top,Y
	cmp	scr_height
	bhs	.l2
	dec a
	jsr	.check_list
	bcs	.l2
	; -- copy scanline
	sta	s_top,X
	lda	scroll_bottom,Y
	sta	s_bottom,X
	; -- copy display control bits
	lda	scroll_cr,Y
	and	#$C0
	sta	s_cr,X
	; -- copy bat coordinates
	lda	scroll_xl,Y
	sta	s_xl,X
	lda	scroll_xh,Y
	sta	s_xh,X
	lda	scroll_yl,Y
	sta	s_yl,X
	lda	scroll_yh,Y
	sta	s_yh,X
	inx
.l2:
	iny
	cpy	#4
	blo	.l1

	; ----
	; init display list
	;
	lda	#$F0
	sta	s_top,X
	sta	s_bottom,X
	inx
	stx	s_idx
	; --
	cly
	cla
.l3:	sta	s_list,Y
	inc a
	iny
	dex
	bne	.l3

	; ----
	; sort display list
	;
	lda	s_idx
	sta	s_work
	bra	.t4
.t1:
	stz	s_work+1
	ldy	#1
.t2:
	ldx	s_list-1,Y
	lda	s_top,X
	inc a
	sta	s_work+2
	ldx	s_list,Y
	lda	s_top,X
	inc a
	cmp	s_work+2
	bhs	.t3
	; --
	lda	s_list-1,Y
	sta	s_list,Y
	txa
	sta	s_list-1,Y
	inc	s_work+1
.t3:
	iny
	cpy	s_work
	blo	.t2
	lda	s_work+1
	beq	.t5
	dec	s_work
	lda	s_work
.t4:	cmp	#2
	bhs	.t1
.t5:
	; ----
	; return
	;
	lda	s_idx
	add	#$FE
	rts

	; ----
	; scan display list
	;
.check_list:
	phx
.x1:	dex
	bmi	.x2
	cmp	s_top,X
	bne	.x1
	plx
	sec
	rts
	; --
.x2:	plx
	clc
	rts

	.bank	LIB1_BANK	; restore context
