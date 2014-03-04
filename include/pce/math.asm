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
	lda   <_al
	asl   A
	sta   <_cl
	cla
	ldy   #8
.l1:
	rol   A
	cmp   <_bl
	bcc   .l2
	sbc   <_bl
.l2:
	rol   <_cl
	dey
	bne   .l1

	sta   <_dl
	rts


; ----
; divu10
; ----
; 16-bit unsigned division by 10
; ----
; OUT : _DX = _DX / 10
;	  A = _DX % 10
; ----

divu10:
	  ldy	#16
  	  cla
	  asl	<_dl
  	  rol	<_dh
.l1:  	  rol	a
  	  cmp	#10
  	  blo	.l2
  	  sbc	#10
.l2:	  rol	<_dl
  	  rol	<_dh
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
	lda   <_bl
	sta   <_ch

	cla
	ldy   #8
.l1:
	asl   A
	rol   <_ch
	bcc   .next
	add   <_al
	bcc   .next
	inc   <_ch
.next:
	dey
	bne   .l1

	sta   <_cl
	rts


; ----
; mulu16
; ----
; 16-bit unsigned multiplication
; ----
; OUT : _DX/CX = _AX * _BX
; ----

mulu16:
	lda   <_ah
	ora   <_bh
	bne   .l1

	stwz  <_dx		;  8-bit multiplication
	jmp   mulu8

.l1:	stw   <_bx,<_dx		; 16-bit multiplication
	stwz  <_cx
	ldy   #16

.l2:	aslw  <_cx
	rolw  <_dx
	bcc   .l3

	addw  <_ax,<_cx
	bcc   .l3
	incw  <_dx

.l3:	dey
	bne   .l2
	rts
	.endif

; ----
; mulu32
; ----
; 32-bit unsigned multiplication
; ----
; OUT : _DX/CX = _BX/AX * _DX/CX
; ----

mulu32:
	stw   <_cx,<_si
	stw   <_dx,<_di
	stwz  <_cx
	stwz  <_dx
	ldy   #32
.loop:
	aslw  <_cx
	rolw  <_dx
	rolw  <_si
	rolw  <_di
	bcc   .next

	addw  <_ax,<_cx
	adcw  <_bx,<_dx
.next:
	dey
	bne   .loop
	rts


; ----
; srand
; ----
; set random seed
; ----
; IN : _DX/CX = 32-bit seed
; ----

	.bss
_rndptr		.ds 2
_rndseed	.ds 2
_rndn1		.ds 1
_rndn2		.ds 1

	.code
srand:
	stw	<_cx,_rndptr
	stw	<_dx,_rndn1
	lda	_rndptr+1
	ora	#$e0
	sta	_rndptr+1
	cmp	#$f4
	blo	.exit
	lda	#$e0
	sta	_rndptr+1
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
_rndzp	.ds	2

	.code
rand:	jsr	randomize
	stw	_rndn1,<_dx
	rts

randomize:
	stw	_rndptr,<_rndzp

	lda	_rndn1	; rotate 3 bits right
	ldx	_rndn2
	ror	A
	sax
	ror	A
	sax
	ror	A
	sax
	ror	A
	sax
	ror	A
	sax
	ror	A
	stx	_rndn1
	sta	_rndn2

	addw	#$05A2,_rndn1 ; add #$05A2 to number

	incw	<_rndzp	; eor with next 2 bytes of ROM
	lda	_rndn2
	eor	[_rndzp]
	sta	_rndn2

	incw	<_rndzp
	lda	_rndn1
	eor	[_rndzp]
	sta	_rndn1

	incw	<_rndzp	; don't use every consecutive byte

	lda	<_rndzp+1	; reset pointer to $e000 if > $f400
	cmp	#$f4
	blo	.l1
	lda	#$e0
	sta	<_rndzp+1
.l1:
	stw	<_rndzp,_rndptr
	rts


; ----
; random
; ----
; return a random number in the interval 0 <= x < A
; ----
; IN  : A = range (1 - 128)
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

	lda	<_dh
	and	#$7f
	rts

.l1:	; asl	A
	sta	<_al
	lda	<_dl
	sta	<_bl
	jsr	mulu8

	lda	<_ch
	rts

