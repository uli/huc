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
; only available if '#define SUPPORT_MOUSE 1' is used
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
	  cla
	  clx
	  sta	msflag
	  rts

; mouse_enable() - turns on mouse support, if mouse detected
;                - return true/false (true=1) if mouse exists
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
	  bpl   mouspos
mousneg:  lda   #$ff
	  rts

; mouse_y() - returns y delta (int)
; ----

_mouse_y:
	  ldx	msvert
	  bmi   mousneg
mouspos:  cla
	  rts

	.endif		; SUPPORT_MOUSE


; set_joy_callback(char num [dl], char mask [al], char keys [ah], int proc [bl:si])
; ----

_set_joy_callback.4:
	  stz	joycallback
	  lda	<_bl
	  sta	joycallback+3
	__ldw	<_si
	__stw	joycallback+4
	  ora   <_si
	  beq   .l1
	  lda	<_ah
	  sta	joycallback+2
	  lda	<_al
	  sta	joycallback+1
	  lda	#$80
	  sta	joycallback
.l1:	  rts

; get_joy_events(char num, int rst)
; ----
; return all the collected joypad events
; ----

_get_joy_events.2:
	  ldy   <_al
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
	  stx	<_al
	  cly
	  sei
.l1:	  lsr	<_al
	  bcc	.l2
	  cla
	  sta	joybuf,Y
	  sta	joybuf6,Y
	  sta	joytrg,Y
	  sta	joytrg6,Y
	  lda	#$FF
	  sta	joyold,Y
	  sta	joyold6,Y
.l2:	  iny
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
	  sta	[_bx]
	  rts

; poke/pokew(int offset bx, int val)
; ----

_pokew.2:
	  sax
	  sta	[_bx]
	  ldy	#1
	  sax
	  sta	[_bx],Y
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
	  lda	<__fbank
	  tam	#3
	  lda	<__fptr+1
	  and	#$1F
	  ora	#$60
	  sta	<__fptr+1
	  lda	[__fptr]
	  tax
	  cla
	  rts

; farpeekw(far void *base)
; ----

_farpeekw.1:
	  lda	<__fbank
	  tam	#3
	  lda	<__fptr+1
	  and	#$1F
	  ora	#$60
	  sta	<__fptr+1
	  bra	_farpeekw.sub
_farpeekw.fast:
	  tam	#3
	  txa
	  and	#$1F
	  ora	#$60
	  sta	<__fptr+1
_farpeekw.sub:
	  lda	[__fptr]
	  tax
	  inc	<__fptr
	  bcc	.l1
	  inc	<__fptr+1
	  bpl	.l1
	  lda	#$60
	  sta	<__fptr+1
.l1:
	  lda	[__fptr]
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

	__stw	<_cx
	  lda	<__fbank
	  tam	#3

    ; ----
    ; split transfer if needed
    ;
	  ; -- clip length (max. 8KB)
	  cmpw	#$2000,<_cx
	  blo	.t1
	  stw	#$2000,<_cx
	  ; -- check length
.t1:	  lda	<__fptr
	  add	<_cl
	  sta	<_al
	  lda	<__fptr+1
	  and	#$1F
	  adc	<_ch
	  sta	<_ah
	  cmp	#$20
	  blo	.t2
	  ; -- calculate second-half size
	  and	#$1F
	  sta	<_dh
	  lda	<_al
	  sta	<_dl
	  subw	<_dx,<_cx
	  ; -- remap src ptr
.t2:	  lda	<__fptr+1
	  and	#$1F
	  ora	#$60
	  sta	<__fptr+1

    ; ----
    ; copy a block
    ;
	  clx
	  cly
	  dec	<_ch
	  bmi	.l4
	  ; -- main loop
.l1:	  lda	[__fptr],Y
	  sta	[_bx],Y
	  iny
	  dex
	  bne	.l1
	  ; -- inc dst ptr
	  cpy	#0
	  beq	.l2
	  tya
	  add	<_bl
	  sta	<_bl
	  bcc	.l3
.l2:	  inc	<_bh
	  ; -- inc src ptr
.l3:	  inc	<__fptr+1
	  ; -- next chunk
	  dec	<_ch
	  bpl	.l1
.l4:	  ldx	<_cl
	  stz	<_cl
	  bne	.l1

    ; ----
    ; second half
    ;
	  tstw	<_dx
	  beq	.l5
	  ; -- reload dst and cnt
	  stw	<_dx,<_cx
	  stw	#$6000,<__fptr
	  ; -- inc bank
	  tma	#3
	  inc	A
	  tam	#3
	  bra	.l1

    ; ----
    ; exit
    ;
.l5:	  rts

; The preceding function was too large to stay
; in LIB1_BANK, so it is placed in LIB2_BANK
; and must be mapped in and out when used.

; Code after this routine should be back in LIB1_BANK

	 .bank	LIB1_BANK


; srand(int seed)
; srand32.2(int seed1 [dx], int seed2 [cx])
; ---------------
; set the seed number for the psuedo-random number generator

_srand:
	__stw	<_dx
	__stw	<_cx

_srand32.2:
	  jsr	srand
	  rts

; rand()
; ----
; get a 16-bit random number

_rand:
	  jsr	rand
	__ldw	<_dx
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

