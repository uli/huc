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
	__stw	<_si
_mov32.sub:
	  ldy	#3
.l1:	  lda	[_si],Y
	  sta	[_di],Y
	  dey
	  bpl	.l1
	  rts

; add32(void *dst [di], void *src) /* ax|bx */
; ----

_add32.2:
	__stw	<_si
	  clc
	  cly
	  ldx	#4
.l1:	  lda	[_di],Y
	  adc	[_si],Y
	  sta	[_di],Y
	  iny
	  dex
	  bne	.l1
	  rts

; sub32(void *dst [di], void *src)
; ----

_sub32.2:
	__stw	<_si
	  sec
	  cly
	  ldx	#4
.l1:	  lda	[_di],Y
	  sbc	[_si],Y
	  sta	[_di],Y
	  iny
	  dex
	  bne	.l1
	  rts

; mul32(void *dst [bp], void *src)
; ----

_mul32.2:
	__stw	<_si
	  stw	#_ax,<_di
	  jsr	_mov32.sub
	  stw	<_bp,<_si
	  stw	#_cx,<_di
	  jsr	_mov32.sub
	  jsr	 mulu32
	  stw	<_bp,<_di
	  stw	#_cx,<_si
	  jmp	_mov32.sub

; div32(void *dst [di], void *src)
; ----

_div32.2:
	  rts

; com32(void *dst)
; ----

_com32.1:
	__stw	<_di
	  ldy	#3
.l1:	  lda	[_di],Y
	  eor	#$FF
	  sta	[_di],Y
	  dey
	  bpl	.l1
	  rts
	
; cmp32(void *dst [di], void *src)
; ----

_cmp32.2:
	__stw	<_si
	  ldy	#3
.l1:	  lda	[_di],Y
	  cmp	[_si],Y
	  bne	.l2
	  dey
	  bpl	.l1
	  ; --
	  clx
	  cla
	  rts
	  ; --
.l2:	  blo	.l3
	  ldx	#$01
	  cla
	  rts
	  ; --
.l3:	  ldx	#$FF
	  txa
	  rts

; bcd_init(char *dst [bx], char digits)
; ----

_bcd_init.2:
	  ; -- check digit number (max. 16)
	  txa
	  cmp	#16
	  blo	.l1
	  lda	#16
.l1:	  inc	A
	  lsr	A
	  ora	#$80
	  sta	[_bx]
_bcd_init.clear:
	  ; -- clear bcd number
	  lda	[_bx]
	  and	#$1F
	  tay
	  cla
.l2:	  sta	[_bx],Y
	  dey
	  bne	.l2
	  rts

; bcd_set(char *dst [bx], char *src)
; bcd_mov(char *dst [bx], char *src)
; ----

_bcd_set.2:
_bcd_mov.2:
	__stw	<_si
	  ora	<_si
	  beq	_bcd_init.clear
	  ; -- check dst
	  lda	[_bx]
	  bpl	.x1
	  and	#$1F
	  beq	.x1
	  tax
	  ; -- check src type
	  lda	[_si]
	  bpl	_bcd_set.ascii
	  bra	_bcd_set.bcd
.x1:	  rts
    ; ----
    ; ... from an ascii string (ie. "100")
    ;
_bcd_set.ascii:
	  ; -- get string length
	  cly
.l1:	  lda	[_si],Y
	  cmp	#48
	  blo	.l2
	  cmp	#58
	  bhs	.l2
	  iny
	  bra	.l1
	  ; -- check if the string is empty 
.l2:	  tya
	  beq	_bcd_init.clear
	  ; -- copy number
.l3:	  cla
	  dey
	  bmi	.l4
	  lda	[_si],Y
	  sub	#48
	  sta	<_dl
	  dey
	  bmi	.l4
	  lda	[_si],Y
	  sub	#48
	  asl	A
	  asl	A
	  asl	A
	  asl	A
	  ora	<_dl
.l4:	  sxy
	  sta	[_bx],Y
	  sxy
	  dex
	  bne	.l3
	  rts

    ; ----
    ; ... from another bcd number
    ;
_bcd_set.bcd:
	  ; -- get src size
	  lda	[_si]
	  bpl	.x1
	  and	#$1F
	  beq	.x1
	  tay
	  ; -- copy number
.l1:	  lda	[_si],Y
	  sxy
	  sta	[_bx],Y
	  dey
	  beq	.x1
	  sxy
	  dey
	  bne	.l1
	  ; -- adjust number
	  sxy
	  cla
.l2:	  sta	[_bx],Y
	  dey
	  bne	.l2
.x1:	  rts

; bcd_add(char *dst [di], char *src)
; ----

_bcd_add.2:
	__stw	<_si
	  ora	<_si
	  beq	.x1
	  ; -- check dst
	  lda	[_di]
	  bpl	.x1
	  and	#$1F
	  beq	.x1
	  tax
	  stx	<_cl
	  ; -- check src
	  lda	[_si]
	  bmi	.l1
	  ; -- convert ascii string
	  stw	#__temp,<_bx
	  jsr	_bcd_set.ascii
	  stw	#__temp,<_si
	  ldx	<_cl
	  ldy	<_cl
	  bra	.l2
	  ; -- get src size
.l1:	  and	#$1F
	  beq	.x1
	  tay
	  ; -- add numbers
	  clc
	  sed
.l2:	  lda	[_di],Y
	  sxy
	  adc	[_di],Y
	  sta	[_di],Y
	  dex
	  beq	.l4
	  sxy
	  dex
	  bne	.l2
	  ; --
.x1:	  cld
	  rts
	  ; -- carry
.l3:	  lda	[_di],Y
	  adc	#0
	  sta	[_di],Y
.l4:	  bcc	.x1
	  dey
	  bne	.l3
	  cld
	  rts

