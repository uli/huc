;
; HUC_MISC.ASM  -  HuC Misc Library
;


; joy(char number)
; ----

_joy:
	sxy
	ldx	joy,Y
	lda	joy6,Y
	rts

; joytrg(char number)
; ----

_joytrg:
	sxy
	ldx	joytrg,Y
	lda	joytrg6,Y
	rts

; joyold(char number)
; ----

_joyold:
	sxy
	ldx	joyold,Y
	lda	joyold6,Y
	rts

; joybuf(char number)
; ----

_joybuf:
	sxy
	ldx	joybuf,Y
	lda	joybuf6,Y
	rts


; mouse functions
; available if SUPPORT_MOUSE is defined (default)
; ----

.ifdef SUPPORT_MOUSE

; mouse_exists() - returns true/false (true = 1)
; ----

_mouse_exists:
	ldx	msflag
	cla
	rts

; mouse_disable() - turns off mouse support
; ----

_mouse_disable:
	; XXX: This doesn't really need a return value, does it?
	cla
	clx
	sta	msflag
	rts

; mouse_enable() - turns on mouse support, if mouse detected
;	- return true/false (true=1) if mouse exists
; ----

_mouse_enable:
	jsr	mousinit
	sax
	cla
	rts

; mouse_x() - returns x/y delta (int)
; ----

_mouse_x:
	ldx	mshorz
	bpl	mouspos
mousneg:
	lda	#$ff
	rts

; mouse_y() - returns y delta (int)
; ----

_mouse_y:
	ldx	msvert
	bmi	mousneg
mouspos:
	cla
	rts

.endif ; SUPPORT_MOUSE


; set_joy_callback(char num [dl], char mask [al], char keys [ah], int proc [bl:si])
; ----

_set_joy_callback.4:
	stz	joycallback
	lda	<bl
	sta	joycallback+3
	__ldw	<si
	__stw	joycallback+4
	ora	<si
	beq	.l1
	lda	<ah
	sta	joycallback+2
	lda	<al
	sta	joycallback+1
	lda	#$80
	sta	joycallback
.l1:	rts

; get_joy_events(char num, int rst)
; ----
; return all the collected joypad events
; ----

_get_joy_events.2:
	ldy	<al
	cpx	#0
	bne	_get_joy_events.sub
	ldx	joybuf,Y
	cla
	rts
_get_joy_events.1:
	sxy
_get_joy_events.sub:
	ldx	joybuf,Y
	cla
	sta	joybuf,Y
	rts

; clear_joy_events(char mask)
; ----

_clear_joy_events:
	stx	<al
	cly
	sei
.l1:	lsr	<al
	bcc	.l2
	cla
	sta	joybuf,Y
	sta	joybuf6,Y
	sta	joytrg,Y
	sta	joytrg6,Y
	lda	#$FF
	sta	joyold,Y
	sta	joyold6,Y
.l2:	iny
	cpy	#5
	blo	.l1
	cli
	rts


; clock_hh()
; ----
_clock_hh:
	cla
	ldx	clock_hh
	rts

; clock_mm()
; ----
_clock_mm:
	cla
	ldx	clock_mm
	rts

; clock_ss()
; ----
_clock_ss:
	cla
	ldx	clock_ss
	rts

; clock_tt()
; ----
_clock_tt:
	cla
	ldx	clock_tt
	rts

; clock_reset()
; ----
_clock_reset:
	stz	clock_hh
	stz	clock_mm
	stz	clock_ss
	stz	clock_tt
	rts


; poke(int offset bx, char val)
; ----

_poke.2:
	txa
	sta	[bx]
	rts

; poke/pokew(int offset bx, int val)
; ----

_pokew.2:
	sax
	sta	[bx]
	ldy	#1
	sax
	sta	[bx],Y
	rts

; peek(int offset)
; ----

_peek:
	__stw	<__ptr
	lda	[__ptr]
	tax
	cla
	rts

; peekw(int offset)
; ----

_peekw:
	__stw	<__ptr
	lda	[__ptr]
	tax
	ldy	#1
	lda	[__ptr],Y
	rts

; farpeekb(far void *base)
; ----

_farpeekb.1:
	lda	<fbank
	tam	#3
	lda	<fptr+1
	and	#$1F
	ora	#$60
	sta	<fptr+1
	lda	[fptr]
	tax
	cla
	rts

; farpeekw(far void *base)
; ----

_farpeekw.1:
	lda	<fbank
	tam	#3
	lda	<fptr+1
	and	#$1F
	ora	#$60
	sta	<fptr+1
	bra	_farpeekw.sub
_farpeekw.fast:
	tam	#3
	txa
	and	#$1F
	ora	#$60
	sta	<fptr+1
_farpeekw.sub:
	lda	[fptr]
	tax
	inc	<fptr
	bcc	.l1
	inc	<fptr+1
	bpl	.l1
	lda	#$60
	sta	<fptr+1
.l1:
	lda	[fptr]
	rts

; farmemget(void *dst, far void *base, int len)
; ----

_farmemget.3:
	maplibfunc	lib2_farmemget.3
	rts

; The following function was too large to stay
; in LIB1_BANK, so it is placed in LIB2_BANK
; and must be mapped in and out when used.

; Code after this routine should be back in LIB1_BANK

	.bank	LIB2_BANK

lib2_farmemget.3:

	__stw	<cx
	lda	<fbank
	tam	#3

	; ----
	; split transfer if needed
	;
	; -- clip length (max. 8KB)
	cmpw	#$2000,<cx
	blo	.t1
	stw	#$2000,<cx
	; -- check length
.t1:	lda	<fptr
	add	<cl
	sta	<al
	lda	<fptr+1
	and	#$1F
	adc	<ch
	sta	<ah
	cmp	#$20
	blo	.t2
	; -- calculate second-half size
	and	#$1F
	sta	<dh
	lda	<al
	sta	<dl
	subw	<dx,<cx
	; -- remap src ptr
.t2:	lda	<fptr+1
	and	#$1F
	ora	#$60
	sta	<fptr+1

	; ----
	; copy a block
	;
	clx
	cly
	dec	<ch
	bmi	.l4
	; -- main loop
.l1:	lda	[fptr],Y
	sta	[bx],Y
	iny
	dex
	bne	.l1
	; -- inc dst ptr
	cpy	#0
	beq	.l2
	tya
	add	<bl
	sta	<bl
	bcc	.l3
.l2:	inc	<bh
	; -- inc src ptr
.l3:	inc	<fptr+1
	; -- next chunk
	dec	<ch
	bpl	.l1
.l4:	ldx	<cl
	stz	<cl
	bne	.l1

	; ----
	; second half
	;
	tstw	<dx
	beq	.l5
	; -- reload dst and cnt
	stw	<dx,<cx
	stw	#$6000,<fptr
	; -- inc bank
	tma	#3
	inc	A
	tam	#3
	bra	.l1

	; ----
	; exit
	;
.l5:	rts

; The preceding function was too large to stay
; in LIB1_BANK, so it is placed in LIB2_BANK
; and must be mapped in and out when used.

; Code after this routine should be back in LIB1_BANK

	.bank	LIB1_BANK


; srand(int seed)
; srand32.2(int seed1 [dx], int seed2 [cx])
; ---------------
; set the seed number for the pseudo-random number generator

_srand:
	__stw	<dx
	__stw	<cx

_srand32.2:
	jsr	srand
	rts

; rand()
; ----
; get a 16-bit random number

_rand:
	jsr	rand
	__ldw	<dx
	rts


; random(char limit)
; --- 
; get a random number where 0 <= n < limit
; (and limit < 128)

_random:
	sax
	and	#$7f
	jsr	random
	sax
	cla
	rts

