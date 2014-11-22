;
; HUC_MATH.ASM  -  HuC Math Library
;

; abs(int val)
; ---

_abs:
	tay
	bpl	.l1
	sax
	eor	#$FF
	add	#1
	sax
	eor	#$FF
	adc	#0
.l1:
	rts

; mov32(void *dst [di], void *src)
; ----

_mov32.2:
	__stw	<si
_mov32.sub:
	ldy	#3
.l1:	lda	[si],Y
	sta	[di],Y
	dey
	bpl	.l1
	rts

; add32(void *dst [di], void *src) /* ax|bx */
; ----

_add32.2:
	__stw	<si
	clc
	cly
	ldx	#4
.l1:	lda	[di],Y
	adc	[si],Y
	sta	[di],Y
	iny
	dex
	bne	.l1
	rts

; sub32(void *dst [di], void *src)
; ----

_sub32.2:
	__stw	<si
	sec
	cly
	ldx	#4
.l1:	lda	[di],Y
	sbc	[si],Y
	sta	[di],Y
	iny
	dex
	bne	.l1
	rts

; mul32(void *dst [bp], void *src)
; ----

_mul32.2:
	__stw	<si
	stw	#ax,<di
	jsr	_mov32.sub
	stw	<bp,<si
	stw	#cx,<di
	jsr	_mov32.sub
	jsr	mulu32
	stw	<bp,<di
	stw	#cx,<si
	jmp	_mov32.sub

; div32(void *dst [di], void *src)
; ----

_div32.2:
	rts

; com32(void *dst)
; ----

_com32.1:
	__stw	<di
	ldy	#3
.l1:	lda	[di],Y
	eor	#$FF
	sta	[di],Y
	dey
	bpl	.l1
	rts
	
; cmp32(void *dst [di], void *src)
; ----

_cmp32.2:
	__stw	<si
	ldy	#3
.l1:	lda	[di],Y
	cmp	[si],Y
	bne	.l2
	dey
	bpl	.l1
	; --
	clx
	cla
	rts
	; --
.l2:	blo	.l3
	ldx	#$01
	cla
	rts
	; --
.l3:	ldx	#$FF
	txa
	rts


.ifdef BCD

; bcd_init(char *dst [bx], char digits)
; ----

_bcd_init.2:
	; -- check digit number (max. 16)
	txa
	cmp	#16
	blo	.l1
	lda	#16
.l1:	inc	A
	lsr	A
	ora	#$80
	sta	[bx]
_bcd_init.clear:
	; -- clear bcd number
	lda	[bx]
	and	#$1F
	tay
	cla
.l2:	sta	[bx],Y
	dey
	bne	.l2
	rts

; bcd_set(char *dst [bx], char *src)
; bcd_mov(char *dst [bx], char *src)
; ----

_bcd_set.2:
_bcd_mov.2:
	__stw	<si
	ora	<si
	beq	_bcd_init.clear
	; -- check dst
	lda	[bx]
	bpl	.x1
	and	#$1F
	beq	.x1
	tax
	; -- check src type
	lda	[si]
	bpl	_bcd_set.ascii
	bra	_bcd_set.bcd
.x1:	rts
	; ----
	; ... from an ascii string (ie. "100")
	;
_bcd_set.ascii:
	; -- get string length
	cly
.l1:	lda	[si],Y
	cmp	#48
	blo	.l2
	cmp	#58
	bhs	.l2
	iny
	bra	.l1
	; -- check if the string is empty
.l2:	tya
	beq	_bcd_init.clear
	; -- copy number
.l3:	cla
	dey
	bmi	.l4
	lda	[si],Y
	sub	#48
	sta	<dl
	dey
	bmi	.l4
	lda	[si],Y
	sub	#48
	asl	A
	asl	A
	asl	A
	asl	A
	ora	<dl
.l4:	sxy
	sta	[bx],Y
	sxy
	dex
	bne	.l3
	rts

	; ----
	; ... from another bcd number
	;
_bcd_set.bcd:
	; -- get src size
	lda	[si]
	bpl	.x1
	and	#$1F
	beq	.x1
	tay
	; -- copy number
.l1:	lda	[si],Y
	sxy
	sta	[bx],Y
	dey
	beq	.x1
	sxy
	dey
	bne	.l1
	; -- adjust number
	sxy
	cla
.l2:	sta	[bx],Y
	dey
	bne	.l2
.x1:	rts

; bcd_add(char *dst [di], char *src)
; ----

_bcd_add.2:
	__stw	<si
	ora	<si
	beq	.x1
	; -- check dst
	lda	[di]
	bpl	.x1
	and	#$1F
	beq	.x1
	tax
	stx	<cl
	; -- check src
	lda	[si]
	bmi	.l1
	; -- convert ascii string
	stw	#__temp,<bx
	jsr	_bcd_set.ascii
	stw	#__temp,<si
	ldx	<cl
	ldy	<cl
	bra	.l2
	; -- get src size
.l1:	and	#$1F
	beq	.x1
	tay
	; -- add numbers
	clc
	sed
.l2:	lda	[di],Y
	sxy
	adc	[di],Y
	sta	[di],Y
	dex
	beq	.l4
	sxy
	dex
	bne	.l2
	; --
.x1:	cld
	rts
	; -- carry
.l3:	lda	[di],Y
	adc	#0
	sta	[di],Y
.l4:	bcc	.x1
	dey
	bne	.l3
	cld
	rts

.endif ; BCD
