;
; Hu-C internal include file
;

; optimized macros
; ----

; phw
;
	.macro __phw
	 ; #1 (bytes :  9, cycles : 16)
	 ; #2 (bytes : 16, cycles : 28)

	.if (\# = 2)
	 tay
	 lda	<__arg_idx
	 add	#\2
	 sta	<__arg_idx
	 say
	.else
	 ldy	<__arg_idx
	.endif

	 sta	__arg-(\1),Y
	 txa
	 sta	__arg-(\1-1),Y
	.endm

; phb
;
	.macro __phb
	 ; #1 (bytes :  6, cycles : 11)
	 ; #2 (bytes : 12, cycles : 21)

	.if (\# = 2)
	 lda	<__arg_idx
	 add	#\2
	 sta	<__arg_idx
	 tay
	.else
	 ldy	<__arg_idx
	.endif

	 txa
	 sta	__arg-(\1),Y
	.endm

; plw
;
	.macro __plw
	 ; (bytes : 9/12, cycles : 16/22)

	 dey
	 lda	__arg,Y
	.if (\#)
	 sta	\1
	.else
	 tax
	.endif
	 dey
	 lda	__arg,Y
	.if (\#)
	 sta	\1+1
	.endif
	.endm

; plb
;
	.macro __plb
	 ; (bytes : 6/7, cycles : 11/12)

	.if (\# = 2)
	 dey
	 ldx	__arg,Y
	 stx	\1
	.else
	 dey
	 lda	__arg,Y
	.if (\#)
	 sta	\1
	.endif
	.endif
	.endm

; lea_s
;
	.macro __lea_s
	 ; (bytes : 10, cycles : 16)

	 lda	<__sp
	 clc
	 adc	#LOW(\1)
	 tax
	 lda	<__sp+1
	 adc	#HIGH(\1)
	.endm

; pea_s
;
	.macro __pea_s
	 ; (bytes : 25, cycles : 44)

	 lda	<__sp
	 sec
	 sbc	#2
	 bcs	.x_\@
	 dec	<__sp+1
.x_\@:	 sta	<__sp
	 clc
	 adc	#LOW(\1+2)
	 sta	[__sp]
	 tax
	 lda	<__sp+1
	 adc	#HIGH(\1+2)
	 ldy	#1
	 sta	[__sp],Y
	.endm

; ldb_s
;
	.macro __ldb_s
	.if (\1 < 256)
	 ; short index (bytes :  9, cycles : 13/17)

	 ldy	#\1
	 lda	[__sp],Y
	 tax
	 cla
	 bpl	.x_\@	; signed
	 dec	A
.x_\@:
	.else
	 ; long index  (bytes : 20, cycles : 37/41)

	 addw	#\1,<__sp,<__ptr	; 13,26
	 lda	[__ptr]
	 tax
	 cla
	 bpl	.x_\@	; signed
	 dec	A
.x_\@:
	.endif
	.endm

; addb_s
;
	.macro __addb_s
	 ; short index (bytes :  9, cycles : 13/17)

	 stx	<__temp
	 sta	<__temp+1
	.if (\1 < 256)
	 ldy	#\1
	 lda	[__sp],Y
	.else
	 addw	#\1,<__sp,<__ptr	; 13,26
	 lda	[__ptr]
	.endif
	 tax
	 __extw
	 sax
	 clc
	 adc	<__temp
	 sax
	 adc	<__temp+1
	.endm

; ldub_s
;
	.macro __ldub_s
	.if (\1 < 256)
	 ; short index (bytes :  9, cycles : 13/17)

	 ldy	#\1
	 lda	[__sp],Y
	 tax
	 cla
	.else
	 ; long index  (bytes : 20, cycles : 37/41)

	 addw	#\1,<__sp,<__ptr	; 13,26
	 lda	[__ptr]
	 tax
	 cla
	.endif
	.endm

; addub_s
;
	.macro __addub_s
	.if (\1 < 256)
	 ; short index (bytes :  9, cycles : 13/17)

	 ldy	#\1
	 sax
	 clc
	 adc	[__sp],Y
	 sax
	 adc	#0
	.else
	 ; long index  (bytes : 20, cycles : 37/41)

	 sta	<__temp
	 addw	#\1,<__sp,<__ptr	; 13,26
	 txa
	 clc
	 adc	[__ptr]
	 tax
	 lda	<__temp
	 adc	#0
	.endif
	.endm

; ldb_p
;
	.macro __ldb_p
	 ; (bytes : 11, cycles : 19/23)
	 stx	<__ptr
	 sta	<__ptr+1
	 lda	[__ptr]
	 tax
	 cla
	 bpl	.x_\@	; signed
	 dec	A
.x_\@:
	.endm

; addbi_p
	.macro __addbi_p
	 stx	<__ptr
	 sta	<__ptr+1
	 lda	[__ptr]
	 clc
	 adc	#\1
	 sta	[__ptr]
	 tax
	 cla
	.endm

; ldub_p
;
	.macro __ldub_p
	 ; (bytes : 11, cycles : 19/23)
	 stx	<__ptr
	 sta	<__ptr+1
	 lda	[__ptr]
	 tax
	 cla
	.endm

; ldw_s
;
	.macro __ldw_s
	.if (\1 < 255)
	 ; short index (bytes :  7/8, cycles : 18/20)
	 .if (\1 = 0)
	  lda	[__sp]
	  tax
	  ldy	#1
	  lda	[__sp],Y
	 .else
	  ldy	#\1
	  lda	[__sp],Y
	  tax
	  iny
	  lda	[__sp],Y
	 .endif
	.else
	 ; long index  (bytes : 20, cycles : 44)

	 addw	#\1,<__sp,<__ptr	; 13,26
	 lda	[__ptr]
	 tax
	 ldy	#1
	 lda	[__ptr],Y
	.endif
	.endm

; stbi_s
;
	.macro __stbi_s
	.if (\2 < 256)
	 ; short index (bytes :  8/ 9, cycles : 15)

	 lda	#LOW(\1)
	 ldy	#\2
	 sta	[__sp],Y
	 tax
;	 lda	#HIGH(\1)
	 cla
	.else
	 ; long index  (bytes : 19/20, cycles : 39)

	 addw	#\2,<__sp,<__ptr	; 13,26
	 lda	#LOW(\1)
	 sta	[__ptr]
	 tax
;	 lda	#HIGH(\1)
         cla
	.endif
	.endm

; stwi_s
;
	.macro __stwi_s
	.if (\2 < 255)
	 ; short index (bytes : 12, cycles : 24)

	 lda	#LOW(\1)
	 ldy	#\2
	 sta	[__sp],Y
	 tax
	 lda	#HIGH(\1)
	 iny
	 sta	[__sp],Y
	.else
	 ; long index  (bytes : 24, cycles : 48)

	 addw	#\2,<__sp,<__ptr	; 13,26
	 lda	#LOW(\1)
	 sta	[__ptr]
	 tax
	 lda	#HIGH(\1)
	 ldy	#1
	 sta	[__ptr],Y
	.endif
	.endm

; stb_s
;
	.macro __stb_s
	.if (\1 < 255)
	 ; short index (bytes :  6, cycles : 13)

	 sax
	 ldy	#\1
	 sta	[__sp],Y
	 sax
	.else
	 ; long index  (bytes : 18, cycles : 42)

	 pha
	 addw	#\1,<__sp,<__ptr	; 13,26
	 txa
	 sta	[__ptr]
	 pla
	.endif
	.endm

; stw_s
;
	.macro __stw_s
	.if (\1 < 255)
	 ; short index (bytes :  9, cycles : 22)

	 sax
	 ldy	#\1
	 sta	[__sp],Y
	 sax
	 iny
	 sta	[__sp],Y
	.else
	 ; long index  (bytes : 22, cycles : 51)

	 pha
	 addw	#\1,<__sp,<__ptr	; 13,26
	 txa
	 sta	[__ptr]
	 pla
	 ldy	#1
	 sta	[__ptr],Y
	.endif
	.endm

; addw_s
;
	.macro __addw_s
	.if (\1 < 255)
	 ; short index (bytes : 10, cycles : 24)

	 ldy	#\1
	 clc
	 sax
	 adc	[__sp],Y
	 sax
	 iny
	 adc	[__sp],Y
	.else
	 ; long index  (bytes : 24, cycles : 55)

	 pha				;  1, 3
	 addw	#\1,<__sp,<__ptr	; 13,26
	 pla				;  1, 4
	 clc
	 sax
	 adc	[__ptr]
	 sax
	 ldy	#1
	 adc	[__ptr],Y
	.endif
	.endm

; incw_s
;
	.macro __incw_s
	 ; (bytes : 16, cycles : 24/40)

	 pha
	 ldy    #\1
	 txa
	 inc    A
	 sta	[__sp],Y
	 bne    .x_\@
	 iny
	 lda	[__sp],Y
	 inc    A
	 sta	[__sp],Y
.x_\@:
	 pla
	.endm

; ldd_i
;
	.macro __ldd_i
	 stw	#(\1) & 0xFFFF,<\2
	 stw	#(\1) >> 16,<\3
	 ldx	#low(\2)
	 lda	#high(\2)
	.endm

; ldd_b
;
	.macro __ldd_b
	 lda	 \1
	 sta	<\2
	 stz	<\2+1
	 stwz	<\3
	 ldx	#low(\2)
	 lda	#high(\2)
	.endm

; ldd_w
;
	.macro __ldd_w
	 stw	 \1,<\2
	 stwz	<\3
	 ldx	#low(\2)
	 lda	#high(\2)
	.endm

; ldd_s_b
;
	.macro __ldd_s_b
       __ldb_s	\1
	 stx	<\2
	 sta	<\2+1
	 stwz	<\3
	 ldx	#low(\2)
	 lda	#high(\2)
	.endm

; ldd_s_w
;
	.macro __ldd_s_w
       __ldw_s	\1
	 stx	<\2
	 sta	<\2+1
	 stwz	<\3
	 ldx	#low(\2)
	 lda	#high(\2)
	.endm

