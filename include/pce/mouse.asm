;- The mouse is read 4 successive times, in order to assemble the mouse data
;(horiz most-significant-nybble/horiz LSN/vert MSN/vert LSN).
;- values are deltas, and reading the mouse every VSYNC is normal
;- For vertical movement, up is positive, down is negative
;- For horizontal movement, left is positive, right is negative
;- I was not able to yield a delta value greater than 0x25 (hexadecimal)
;during a 1-VSYNC interval  (I didn't try all *that* hard though).  This
;should give you an idea of the sensitivity.

;
;  PCE mouse driver source
;

	.bss
msflag:	.ds 1
msvert:	.ds 1
mshorz:	.ds 1

	.code

;
; These stub interfaces are available from LIB1_BANK
; But the real guts are in LIB2_BANK
;
mousinit:
	maplibfunc	lib2_mousinit
	rts

mousread:
	maplibfunc	lib2_mousread
	rts


;
; These functions are available in bank 2 of the
; library:
;
	.bank	LIB2_BANK

;
; detect and initialize the mouse
;
lib2_mousinit:
	lda   #$20	; reset resource-usage flag
	tsb   <irq_m	; to skip joystick read portion of vsync

	stz   joy	; clear joypad memory area
	tii   joy, joy+1, $4

	stz   joytrg	; clear joypad memory area
	tii   joytrg, joytrg+1, $4

	stz   joyold	; clear joypad memory area
	tii   joyold, joyold+1, $4

	stz   joybuf	; clear joypad memory area
	tii   joybuf, joybuf+1, $4

	stz   joy6	; clear joypad memory area
	tii   joy6, joy6+1, $4

	stz   joytrg6	; clear joypad memory area
	tii   joytrg6, joytrg6+1, $4

	stz   joyold6	; clear joypad memory area
	tii   joyold6, joyold6+1, $4

	stz   joybuf6	; clear joypad memory area
	tii   joybuf6, joybuf6+1, $4

	stz   msvert
	stz   mshorz
	stz   msflag

	cla		; counter of 'good' reads (where vertical == 0)
	ldx   #$0A	; try 10 iterations
.loop1:	pha
	phx
	lda   #1
;	jsr   wait_vsync	; wait for 1 vsync frame
	jsr   lib2_mousread	; read mouse
	plx
	pla
	ldy   msvert	; read mouse vertical axis movement
	bne   .l1	; if (val == 0), inc counter
	inc   a
.l1:	dex
	bne   .loop1	; next iteration

	cmp   #$00	; if #$00 value found even once (out of 10 times)
	bne   .mous	; then return(1)

	lda   #$20	; reset #$20 bit of $F5
	trb   <irq_m
	cla		; bad return code
	sta   msflag
	stz   msvert
	stz   mshorz
	rts

.mous:
	lda   #$20	; reset #$20 bit of $F5
	trb   <irq_m
	lda   #$01	; good return code
	sta   msflag
	rts

;
; actual mechanics of reading mouse
;
lib2_mousread:
	ldx   #$04	; # iterations (actually 5)
.loop1:	lda   joy,X	; copy 'current' value to 'previous' value
	sta   joyold,X
	dex
	bpl  .loop1

	stz   joy	; initialize joypad #1's value

	lda   #$01	; reset joypad port# to joystick #1
	sta   joyport
	lda   #$03
	sta   joyport
	lda   #$01
	sta   joyport

	jsr   delay240	; delay 240 CPU cycles

	lda   joyport	; read joystick port
	asl   a	; upper nybble of 8-bit value - shift it
	asl   a
	asl   a
	asl   a
	sta   mshorz	; store it
	jsr   msbutt	; read buttons (toggle port/read other nybble)

	lda   #$01	; reset joystick port again (to stick #1)
	sta   joyport
	lda   #$03
	sta   joyport
	lda   #$01
	sta   joyport

	jsr   delay14	; wait 14 cycles to settle (reference code says 9)

	lda   joyport	; read port
	and   #$0F	; lower nybble of 8-bit value
	tsb   mshorz	; 'or' it into memory
	bsr   msbutt	; read buttons (toggle port/read other nybble)

	lda   #$01	; reset joystick port again
	sta   joyport
	lda   #$03
	sta   joyport
	lda   #$01
	sta   joyport

	jsr   delay14	; wait 14 cycles to settle (reference code says 9)

	lda   joyport	; read port
	asl   a	; upper nybble of 8-bit value - shift it
	asl   a
	asl   a
	asl   a
	sta   msvert
	bsr   msbutt	; read buttons (toggle port/read other nybble)

	lda   #$01	; reset joystick port again
	sta   joyport
	lda   #$03
	sta   joyport
	lda   #$01
	sta   joyport

	jsr   delay14	; wait 14 cycles to settle (reference code says 9)

	lda   joyport	; read port
	and   #$0F	; lower nybble of 8-bit value
	tsb   msvert	; 'or' it into value
	bsr   msbutt	; read buttons (toggle port/read other nybble)

	lda   joytrg	; check joystick buttons
	cmp   #$04	; is 'select' newly-pressed ?
	bne  .exit
	lda   joy	; if so, are both run & select pressed ?
	cmp   #$0C
	bne  .exit
	jmp   [soft_reset]	; if yes, reboot
.exit:	rts		; else return

delay240:
	pha		; delay loop for 240 processor cycles
	phx		; (including call/return overhead)
	cla
	nop
.lp:	nop
	inc	a
	cmp	#21
	bcc	.lp
	plx
	pla
delay14:
	rts

msbutt:
	stz   joyport	; toggle joystick port to read buttons

	jsr   delay14	; wait 14 cycles to settle (reference code says 9)

	lda   joyport	; read value
	eor   #$FF	; change low-active to high-active
	and   #$0F	; only 4 bits
	tsb   joy	; 'or' it into value

	ora   joyold	; determine 'newly-pressed' buttons
	eor   joyold
	sta   joytrg	; put them into 'delta'
	rts

	.bank	LIB1_BANK	; restore bank-context


