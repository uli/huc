;
; MATH.ASM  -  MagicKit Standard Math Routines
;
;


; ----
; divu8
; ----
; 8-bit unsigned division
; ----
; OUT : _CL = _AL / _BL
;	_DL = _AL % _BL
; ----

divu8:
	lda	<al
	asl a
	sta	<cl
	cla
	ldy	#8
.l1:
	rol a
	cmp	<bl
	bcc	.l2
	sbc	<bl
.l2:
	rol	<cl
	dey
	bne	.l1

	sta	<dl
	rts


; ----
; divu10
; ----
; 16-bit unsigned division by 10
; ----
; OUT : _DX = _DX / 10
;	A = _DX % 10
; ----

divu10:
	ldy	#16
	cla
	asl	<dl
	rol	<dh
.l1:	rol	a
	cmp	#10
	blo	.l2
	sbc	#10
.l2:	rol	<dl
	rol	<dh
	dey
	bne	.l1
	rts


.if (!CDROM)

; ----
; mulu8
; ----
; 8-bit unsigned multiplication
; ----
; OUT : _CX = _AL * _BL
; ----

mulu8:
	lda	<bl
	sta	<ch

	cla
	ldy	#8
.l1:
	asl a
	rol	<ch
	bcc	.next
	add	<al
	bcc	.next
	inc	<ch
.next:
	dey
	bne	.l1

	sta	<cl
	rts


; ----
; mulu16
; ----
; 16-bit unsigned multiplication
; ----
; OUT : _DX/CX = _AX * _BX
; ----

mulu16:
	lda	<ah
	ora	<bh
	bne	.l1

	stwz	<dx		; 8-bit multiplication
	jmp	mulu8

.l1:	stw	<bx,<dx	; 16-bit multiplication
	stwz	<cx
	ldy	#16

.l2:	aslw	<cx
	rolw	<dx
	bcc	.l3

	addw	<ax,<cx
	bcc	.l3
	incw	<dx

.l3:	dey
	bne	.l2
	rts

.endif ; (!CDROM)


; ----
; mulu32
; ----
; 32-bit unsigned multiplication
; ----
; OUT : _DX/CX = _BX/AX * _DX/CX
; ----

mulu32:
	stw	<cx,<si
	stw	<dx,<di
	stwz	<cx
	stwz	<dx
	ldy	#32
.loop:
	aslw	<cx
	rolw	<dx
	rolw	<si
	rolw	<di
	bcc	.next

	addw	<ax,<cx
	adcw	<bx,<dx
.next:
	dey
	bne	.loop
	rts


; ----
; srand
; ----
; set random seed
; ----
; IN : _DX/CX = 32-bit seed
; ----

	.bss
rndptr		.ds 2
rndseed2	.ds 2
rndn1		.ds 1
rndn2		.ds 1

	.code
srand:
	stw	<cx,rndptr
	stw	<dx,rndn1
	lda	rndptr+1
	ora	#$e0
	sta	rndptr+1
	cmp	#$f4
	blo	.exit
	lda	#$e0
	sta	rndptr+1
.exit:
	rts


; ----
; rand
; ----
; return 16-bit random number
; ----
; OUT: _DX
; ----

	.zp
rndzp	.ds	2

	.code
rand:	jsr	randomize
	stw	rndn1,<dx
	rts

randomize:
	stw	rndptr,<rndzp

	lda	rndn1	; rotate 3 bits right
	ldx	rndn2
	ror a
	sax
	ror a
	sax
	ror a
	sax
	ror a
	sax
	ror a
	sax
	ror a
	stx	rndn1
	sta	rndn2

	addw	#$05A2,rndn1 ; add #$05A2 to number

	incw	<rndzp	; eor with next 2 bytes of ROM
	lda	rndn2
	eor	[rndzp]
	and	#$7f
	sta	rndn2

	incw	<rndzp
	lda	rndn1
	eor	[rndzp]
	sta	rndn1

	incw	<rndzp		; don't use every consecutive byte

	lda	<rndzp+1	; reset pointer to $e000 if > $f400
	cmp	#$f4
	blo	.l1
	lda	#$e0
	sta	<rndzp+1
.l1:
	stw	<rndzp,rndptr
	rts


; ----
; random
; ----
; return a random number in the interval 0 <= x < A
; ----
; IN :	A = range (1 - 128)
; ----
; OUT : A = random number
; ----
;

random:
	pha
	jsr	rand
	pla
	; ----
	cmp	#128
	blo	.l1

	lda	<dh
	and	#$7f
	rts

.l1:	; asl a
	sta	<al
	lda	<dl
	sta	<bl
	jsr	mulu8

	lda	<ch
	rts
