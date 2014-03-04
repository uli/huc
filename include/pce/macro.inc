;
; MACRO.INC  -  MagicKit standard MACRO definitions
;

map	.macro			; map a memory bank into
	 lda   #bank(\1)	; addressable memory
	 tam   #page(\1)
	.endm

;
; MAPLIBFUNC - Re-map a library function into $A000-space if it's
;              not in library bank #1
;
maplibfunc	.macro
	 tay
	 tma  #page(\1)
	 pha
	 lda  #bank(\1)
	 tam  #page(\1)
	 tya
	 jsr  \1
	 tay
	 pla
	 tam  #page(\1)
	 tya
	.endm


;
; STWZ - store a word-sized zero value at stated memory location
;
stwz	.macro
	 stz	LOW_BYTE \1
	 stz	HIGH_BYTE \1
	.endm

;
; STW - store a word-sized value at stated memory location
;
stw	.macro
	 lda	LOW_BYTE \1
	 sta	LOW_BYTE \2
	 lda	HIGH_BYTE \1
	 sta	HIGH_BYTE \2
	.endm

stb	.macro
	 lda	\1
	 sta	\2
	.endm

;
; ADDW - add word-sized value to value at stated memory location,
;        storing result back into stated memory location (or into
;        another destination memory location - third arg)
;
addw	.macro
	.if	(\# = 3)
	 ; 3-arg mode
	 ;
	 clc
	 lda	LOW_BYTE \2
	 adc	LOW_BYTE \1
	 sta	LOW_BYTE \3
	 lda	HIGH_BYTE \2
	 adc	HIGH_BYTE \1
	 sta	HIGH_BYTE \3
	.else
	 ; 2-arg mode
	 ;
	 clc
	 lda	LOW_BYTE \2
	 adc	LOW_BYTE \1
	 sta	LOW_BYTE \2
	 lda	HIGH_BYTE \2
	 adc	HIGH_BYTE \1
	 sta	HIGH_BYTE \2
	.endif
	.endm

;
; ADCW - add word-sized value plus carry to value at stated memory location,
;        storing result back into stated memory location
;
adcw	.macro
	 lda	LOW_BYTE \2
	 adc	LOW_BYTE \1
	 sta	LOW_BYTE \2
	 lda	HIGH_BYTE \2
	 adc	HIGH_BYTE \1
	 sta	HIGH_BYTE \2
	.endm

;
; SUBW - substract word-sized value from value at stated memory location,
;        storing result back into stated memory location
;
subw	.macro
	 sec
	 lda	LOW_BYTE \2
	 sbc	LOW_BYTE \1
	 sta	LOW_BYTE \2
	 lda	HIGH_BYTE \2
	 sbc	HIGH_BYTE \1
	 sta	HIGH_BYTE \2
	.endm

;
; SBCW - substract word-sized value plus carry from value at stated memory
;        location, storing result back into stated memory location
;
sbcw	.macro
	 lda	LOW_BYTE \2
	 sbc	LOW_BYTE \1
	 sta	LOW_BYTE \2
	 lda	HIGH_BYTE \2
	 sbc	HIGH_BYTE \1
	 sta	HIGH_BYTE \2
	.endm

cmpw	.macro
	 lda	HIGH_BYTE \2
	 cmp	HIGH_BYTE \1
	 bne	.x_\@
	 lda	LOW_BYTE \2
	 cmp	LOW_BYTE \1
.x_\@:
	.endm

tstw	.macro			; test if the word-sized 
	 lda   \1		; value at stated memory
	 ora   \1+1		; location is zero
	.endm

incw	.macro			; increment a word-sized
	 inc	\1		; value at stated memory
	 bne	.x_\@		; location
	 inc	\1+1
.x_\@:
	.endm

decw	.macro			; decrement a word-sized
	 sec			; value at stated memory
	 lda	\1		; location
	 sbc	#1
	 sta	\1
	 lda	\1+1
	 sbc	#0
	 sta	\1+1
	.endm

rolw	.macro			; rotate word-sized value
	 rol	\1		; (at stated memory location)
	 rol	\1+1
	.endm

aslw	.macro			; arithmetic shift-left
	 asl	\1		; word-sized value (at stated
	 rol	\1+1		; memory location)
	.endm

lsrw	.macro			; logical shift-right word-sized
	 lsr	\1+1		; value (at stated memory
	 ror	\1		; location)
	.endm

rorw	.macro			; rotate right word-sized value
	 ror	\1+1		; (at stated memory location)
	 ror	\1
	.endm

negw	.macro			; negate word-sized value
	 cla			; (at stated memory location)
	 sub	\1		; 2's complement
	 sta	\1
	 cla
	 sbc	\1+1
	 sta	\1+1
	.endm

neg	.macro			; negate byte-sized value
	 eor	#$FF		; in register A
	 inc	A		; 2's complement
	.endm

add	.macro			; add byte-sized value to
	.if (\# = 2)		; register A (handle carry
	 lda	\2		; flag)
	 clc
	 adc	\1
	 sta	\2
	.else
	 clc
	 adc	\1
	.endif
	.endm

sub	.macro			; subtract byte-sized value
	.if (\# = 2)		; from register A (handle
	 lda	\2		; carry flag)
	 sec
	 sbc	\1
	 sta	\2
	.else
	 sec
	 sbc	\1
	.endif
	.endm

blo	.macro			; branch if 'lower'
	 bcc	\1
	.endm

bhs	.macro			; branch if 'higher or same'
	 bcs	\1
	.endm

bhi	.macro			; branch if 'higher'
	 beq	.x_\@
	 bcs	\1
.x_\@:
	.endm

;-------------------------------

;
; Long branch MACROs
;

lbne	.macro
	 beq	.x_\@
	 jmp	\1
.x_\@
	.endm

lbeq	.macro
	 bne	.x_\@
	 jmp	\1
.x_\@
	.endm

lbpl	.macro
	 bmi	.x_\@
	 jmp	\1
.x_\@
	.endm

lbmi	.macro
	 bpl	.x_\@
	 jmp	\1
.x_\@
	.endm

lbcc	.macro
	 bcs	.x_\@
	 jmp	\1
.x_\@
	.endm

lbcs	.macro
	 bcc	.x_\@
	 jmp	\1
.x_\@
	.endm

lblo	.macro
	 bcs	.x_\@
	 jmp	\1
.x_\@
	.endm

lbhs	.macro
	 bcc	.x_\@
	 jmp	\1
.x_\@
	.endm


;-------------------------------

;
; These MACROs are the same as the MACROs
; without an underscore; the difference
; is these MACROs preserve the state of
; the registers they use (at the expense
; of speed)
;

_stw	.macro
	 pha
	 stw	\1,\2
	 pla
	.endm

_addw	.macro
	 pha
	 addw	\1,\2
	 pla
	.endm

_adcw	.macro
	 pha
	 adcw	\1,\2
	 pla
	.endm

_subw	.macro
	 pha
	 subw	\1,\2
	 pla
	.endm

_sbcw	.macro
	 pha
	 sbcw	\1,\2
	 pla
	.endm

_cmpw	.macro
	 pha
	 cmpw	\1,\2
	 pla
	.endm

_tstw	.macro
	 pha
	 tstw	\1
	 pla
	.endm

_incw	.macro
	 incw	\1
	.endm

_decw	.macro
	 pha
	 decw	\1
	 pla
	.endm

