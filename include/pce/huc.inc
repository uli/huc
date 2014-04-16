;
; Hu-C internal include file
;


; variables
; ----

           .zp
__sp
__stack    .ds 2
__temp     .ds 8
__ptr      .ds 2
__fbank    .ds 1
__fptr     .ds 2
__expr     .ds 2


; macros
; ----

	.macro __farptr
	 ldy	#bank(\1)
	 sty	\2
	 ldy	#low(\1)
	 sty	\3
	 ldy	#high(\1)
	 sty	\3+1
	.endm

	.macro __farptr_i
	 sax
	 add	#low(\1)
	.if (\# = 3)
	 sta	<\3
	.else
	 sta	<__fptr
	.endif
	 sax
	 adc	#high(\1) & $1F
	.if (\# = 3)
	 sta	<\3+1
	.else
	 tax
	.endif
	 rol	A
	 rol	A
	 rol	A
	 rol	A
	 and	#$F
	 add	#bank(\1)
	.if (\# = 3)
	 sta	<\2
	.endif
	.endm

	.macro __farptr_get
	 stx	<\1
	 ldy	#2
	 lda	[__ptr],Y
	 sta	<\2
	 iny
	 lda	[__ptr],Y
	 sta	<\2+1
	.endm

	.macro __fgetb
	 tam	#3
	 txa
	 and	#$1F
	 ora	#$60
	 sta	<__fptr+1
	 lda	[__fptr]
	 tax
	 bpl	.x_\@
	 dec	A
.x_\@:
	.endm

	.macro __fgetub
	 tam	#3
	 txa
	 and	#$1F
	 ora	#$60
	 sta	<__fptr+1
	 lda	[__fptr]
	 tax
	 cla
	.endm

__phax	.macro
	pha
	phx
	.endm
__plax	.macro
	plx
	pla
	.endm
__plxa	.macro
	pla
	plx
	.endm

__ldb	.macro
	.if (\# = 2)
	 clx
	 lda	\1
	 clc
	 adc	#low(\2)
	 sax
	 adc	#high(\2)
	.else
	 ldx	\1
       __extw
       	.endif
	.endm

__ldby:	.macro
	 ldx \1, y
	 __extw
	.endm

__ldub	.macro
	.if (\# = 2)
	 clx
	 lda	\1
	 clc
	 adc	#low(\2)
	 sax
	.else
	 ldx	\1
	 cla
       	.endif
	.endm

__ldbp	.macro
	 lda	[\1]
	 tax
       __extw
	.endm

__ldubp	.macro
	 lda	[\1]
	 tax
	 cla
	.endm

__stbps	.macro
       __phax
       __ldwp	 __stack
       __stw	<__ptr
       __plxa
	 sta	[__ptr]
	 sax
       __addmi	2,__stack
	.endm

__ldw	.macro
	.if (\# = 2)
	 lda	\1
	 clc
	 adc	#low(\2)
	 tax
	 lda	\1+1
	 adc	#high(\2)
	.else
	 ldx	\1
	 lda	\1+1
	.endif
	.endm

__ldwp	.macro
	 lda	[\1]
	 tax
	 ldy	#1
	 lda	[\1],Y
	.endm

__ldwi	.macro
	 ldx	#low(\1)
	 lda	#high(\1)
	.endm

__stw	.macro
	 stx	\1
	 sta	\1+1
	.endm

__stwi	.macro
	 ldx	#low(\2)
	 stx	\1
	 lda	#high(\2)
	 sta	\1 + 1
	.endm

__stbi	.macro
	 ldy	#low(\2)
	 sty	\1
	.endm

__stwz	.macro
	 stz	\1
	 stz	\1 + 1
	.endm

__stwp	.macro
	 sax
	 sta	[\1]
	 sax
	 ldy	#1
	 sta	[\1],Y
	.endm

__stbp	.macro
	 sax
	 sta	[\1]
	 sax
	.endm

__stwps	.macro
       __phax
       __ldwp	 __stack
       __stw	<__ptr
       __plax
       __stwp	 __ptr
       __addmi	2,__stack
	.endm

__stwip .macro
	 __stw	__ptr
	 lda	#low(\1)
	 tax
	 sta	[__ptr]
	 lda	#high(\1)
	 ldy	#1
	 sta	[__ptr],Y
	.endm

__stbip .macro
	 __stw	__ptr
	 lda	#low(\1)
	 sta	[__ptr]
	 tax
	 cla
	.endm

__pushw	.macro
	.ifndef SMALL
	 ldy	<__stack
	 bne	.x_\@
	 dec	<__stack + 1
	.endif
.x_\@:	 dec	<__stack
	 sta	[__stack]
	.ifndef SMALL
	 bne	.y_\@
	 dec	<__stack + 1
	.endif
.y_\@:	 dec	<__stack
         sax
	 sta	[__stack]
	 sax
	.endm

__popw	.macro
	 lda	[__stack]
	 tax
	 ldy	#1
	 lda	[__stack],Y
       __addmi	2,__stack
	.endm

__calls	.macro
	 pha
	 ldy	#\1
	 lda	[__stack],Y
	 sta	<__ptr
	.ifndef SMALL
	 incw	<__stack
	.else
	 inc	<__stack
	.endif
	 lda	[__stack],Y
	 sta	<__ptr+1
	.ifndef SMALL
	 incw	<__stack
	.else
	 inc	<__stack
	.endif
	 pla
	 jsr	hook
	.endm

__swapw	.macro
	 pha
	 ldy	#1
	 lda	[__stack],Y
	 sta	<__temp
	 lda	[__stack]
	 sax
	 sta	[__stack]
	 pla
	 sta	[__stack],Y
	 lda	<__temp
	.endm


; TODO : check if we need pha/pla or not
__tstw	.macro
         ; pha
	 stx	<__temp
	 ora	<__temp
	 cla
	 clx
	 beq	.x_\@
	 inx
.x_\@:
	 ; pla
	.endm

__lsrw	.macro
	lsr	A
	sax
	ror	A
	sax
	.endm

__aslw	.macro
	 sax
	 asl	A
	 sax
	 rol	A
	.endm

__aslwi	.macro
	.if (\1 = 1)
	 __aslw
	.else
	.if (\1 = 2)
	 __aslw
	 __aslw
	.else
	.if (\1 = 8)
	 txa
	 clx
	.else
	 ldy #\1
	 jsr aslzp
	.endif
	.endif
	.endif
	.endm

__aslws	.macro
	 lda	[__stack]
	 asl	a
	 sta	[__stack]
	 ldy	#1
	 lda	[__stack],Y
	 rol	a
	 sta	[__stack],Y
	.endm

__asrw	.macro
	 cmp	#$80
	 ror	A
	 sax
	 ror	A
	 sax
	.endm

__asrwi	.macro
	.if (\1 = 1)
	 __asrw
	.else
	.if (\1 = 2)
	 __asrw
	 __asrw
	.else
	.if (\1 = 8)
	 tax
	 cmp #$80
	 cla
	 bcc .x_\@
	 dec a
.x_\@:
	.else
	 ldy #\1
	 jsr asrzp
	.endif
	.endif
	.endif
	.endm

__lsrwi	.macro
	.if (\1 = 1)
	 __lsrw
	.else
	.if (\1 = 2)
	 __lsrw
	 __lsrw
	.else
	.if (\1 = 8)
	 tax
	 cla
	.else
	 ldy #\1
	 jsr lsrzp
	.endif
	.endif
	.endif
	.endm

__extw	.macro
	 txa	; signed
	 cla
	 bpl	.x_\@
	 dec	A
.x_\@:
	.endm

__comw	.macro
	 sax
	 eor	#$FF
	 sax
	 eor	#$FF
	.endm

__negw	.macro
	 sax
	 eor	#$FF
	 clc
	 adc	#1
	 sax
	 eor	#$FF
	 adc	#0
	.endm

__boolw	.macro
	 stx	<__temp
	 ora	<__temp
	 clx
	 beq	.x\@
	 inx
.x\@	 cla
	.endm

__notw	.macro
	 stx	<__temp
	 ora	<__temp
	 clx
	 bne	.x\@
	 inx
.x\@	 cla
	.endm

__addw	.macro
	 clc
	 sax
	 adc	\1
	 sax
	 adc	\1+1
	.endm	

__addb	.macro
	 clc
	 sax
	 adc	\1
	 sax
	 ldy	\1
	 bpl	.x\@
	 dec a
.x\@:	 adc	#0
	.endm

__addub	.macro
	 clc
	 sax
	 adc	\1
	 sax
	 adc	#0
	.endm

__addwi	.macro
	.if (\1 = 1)
	 cpx #$ff
	 inx
	 adc #0
	.else
	.if (\1 = 2)
	 cpx #$fe
	 inx
	 inx
	 adc #0
	.else
	 clc
	 sax
	 adc	#low(\1)
	 sax
	 adc	#high(\1)
	.endif
	.endif
	.endm	

; pceas workaround; the regular __addwi doesn't work if the argument is
; symbolic because the code size changes as it is resolved.
__addwi_sym	.macro
	 clc
	 sax
	 adc	#low(\1)
	 sax
	 adc	#high(\1)
	.endm

__mulwi	.macro
	.if (\1 = 2)
	 __aslw
	.else
	.if (\1 = 3)
	 __stw <__temp
	 __aslw
	 __addw <__temp
	.else
	.if (\1 = 4)
	 __aslw
	 __aslw
	.else
	.if (\1 = 5)
	 __stw <__temp
	 __aslw
	 __aslw
	 __addw <__temp
	.else
	.if (\1 = 6)
	 __aslw
	 __stw <__temp
	 __aslw
	 __addw <__temp
	.else
	.if (\1 = 7)
	 __stw <__temp
	 __aslw
	 __aslw
	 __aslw
	 __subw <__temp
	.else
	.if (\1 = 8)
	 __aslw
	 __aslw
	 __aslw
	.else
	.if (\1 = 9)
	 __stw <__temp
	 __aslw
	 __aslw
	 __aslw
	 __addw <__temp
	.else
	.if (\1 = 10)
	 __aslw
	 __stw <__temp
	 __aslw
	 __aslw
	 __addw <__temp
	.else
	 __pushw
	 __ldwi \1
	 jsr umul
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endm

__addbi	.macro
	.if (\1 = 1)
	 inx
	.else
	.if (\1 = 2)
	 inx
	 inx
	.else
	.if (\1 = 3)
	 inx
	 inx
	 inx
	.else
	 clc
	 txa
	 adc	#low(\1)
	 tax
	.endif
	.endif
	.endif
	 cla
	.endm	

 .ifdef SMALL
__addmi	.macro
	.if (\1 = 1)
	 inc	<\2		; 6
	.else
	.if (\1 = -1)
	 dec	<\2		; 6
	.else
	.if (\1 = -2)
	 dec	<\2
	 dec	<\2		; => 12
	.else
	.if (\1 = 2)
	 inc	<\2
	 inc	<\2		; => 12
	.else
	 tay			; 2
	 clc			; 2
	 lda	<\2		; 4
	 adc	#low(\1)	; 2
	 sta	<\2		; 4
	 tya			; 2 => 16
	.endif
	.endif
	.endif
	.endif
	.endm	

 .else ; SMALL

__addmi1 .macro		; 10(14)
	 inc	<\1	; 6
	 bne	.x\@	; 4(2)
	 inc	<\1+1	; 0(6)
.x\@:
	.endm

__submi1 .macro			; 14(18)
	 ldy <\1		; 4
	 bne .x\@		; 4(2)
	 dec <\1+1		; 0(6)
.x\@
	 dec <\1		; 6
	.endm

__addmi	.macro
	.if (\1 = 1)
	 __addmi1 \2
	.else
	.if (\1 = -1)
	 __submi1 \2
	.else
	 tay			; 2
	 clc			; 2
	 lda	<\2		; 4
	 adc	#low(\1)	; 2
	 sta	<\2		; 4
	.if (\1 < 256) & (\1 >= 0)
	 bcc	.x\@		; 4(2)
	 inc	<\2+1		; 0(6)
.x\@:
	.else
	.if (\1 >= -256) & (\1 < 0)
	 bcs	.y\@		; 4(2)
	 dec	<\2+1		; 0(6)
.y\@:
	.else
	 lda	<\2+1		; 4
	 adc	#high(\1)	; 2
	 sta	<\2+1		; 4
	.endif
	.endif
	 tya			; 2 => 20(24) / 26
	.endif
	.endif
	.endm	
 .endif ; SMALL

__subw	.macro
	 sec
	 sax
	 sbc	\1
	 sax
	 sbc	\1+1
	.endm	

__subwi	.macro
	.if (\1 = 1)
	 cpx #1
	 dex
	 sbc #0
	.else
	.if (\1 = 2)
	 cpx #2
	 dex
	 dex
	 sbc #0
	.else
	 sec
	 sax
	 sbc	#low(\1)
	 sax
	 sbc	#high(\1)
	.endif
	.endif
	.endm	

__submi	.macro
	 pha
	 sec
	 lda	<\2
	 sbc	#low(\1)
	 sta	<\2
	 lda	<\2+1
	 sbc	#high(\1)
	 sta	<\2+1
	 pla
	.endm	

__addws	.macro
	 sax
	 clc
	 adc	[__stack]
	 sax
	 ldy	#1
	 adc	[__stack],Y
       __addmi 2, __stack
	.endm

__addbs	.macro
	 txa
	 clc
	 adc	[__stack]
	 tax
	 cla
       __addmi 2, __stack
	.endm

__subws	.macro
	stx	<__temp
	sta	<__temp+1
	sec
	lda	[__stack]
	sbc	<__temp
	tax
	ldy	#1
	lda	[__stack],Y
	sbc	<__temp+1
	__addmi	2, __stack
	.endm

__orws	.macro
	 sax
	 ora	[__stack]
	 sax
	 ldy	#1
	 ora	[__stack],Y
       __addmi	2,__stack
	.endm

__orwi	.macro
	 sax
	 ora	#low(\1)
	 sax
	 ora	#high(\1)
	.endm	

__orw	.macro
	 sax
	 ora	\1
	 sax
	 ora	\1+1
	.endm	

__andws	.macro
	 sax
	 and	[__stack]
	 sax
	 ldy	#1
	 and	[__stack],Y
       __addmi	2,__stack
	.endm

__andwi	.macro
	 sax
	 and	#low(\1)
	 sax
	 and	#high(\1)
	.endm	

__andw	.macro
	 sax
	 and	\1
	 sax
	 and	\1+1
	.endm	

__eorwi	.macro
	 sax
	 eor	#low(\1)
	 sax
	 eor	#high(\1)
	.endm

__eorws	.macro
	 sax
	 eor	[__stack]
	 sax
	 ldy	#1
	 eor	[__stack],Y
       __addmi	2,__stack
	.endm

__lbra	.macro
	 jmp	\1
	.endm

__lbran	.macro
	 bra	\1
	.endm

__lbeq	.macro
	 cpx #0
	 bne	.x\@
	 jmp	\1
.x\@
	.endm

__lbeqn	.macro
	 cpx #0
	 beq	\1
	.endm

__lbne	.macro
	 cpx #0
	 beq	.x\@
	 jmp	\1
.x\@
	.endm

__lbnen	.macro
	 cpx #0
	 bne	\1
	.endm

__cmpwi_eq .macro
	cpx	#low(\1)
	clx
	bne	.f\@
	cmp	#high(\1)
	bne	.f\@
	ldx	#1
.f\@:	cla
	.endm

__cmpwi_ne .macro
	cpx	#low(\1)
	clx
	bne	.t\@
	cmp	#high(\1)
	beq	.f\@
.t\@:	ldx	#1
.f\@:	cla
	.endm

__call	.macro
	 call   \1
	.endm

_set_bgpal .macro
	 set_bgpal \1,\2,\3
	.endm

_set_sprpal .macro
	 set_sprpal \1,\2,\3
	.endm

_load_background .macro
	 vload $1000,\1,#$4000
	 vsync
	 set_bgpal #0, \2,#16
	 batcpy #$0, \3, \4, \5
	.endm

