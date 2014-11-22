; Arcade card functions library
;
; Tomaitheous '07

	.data
XFER_TYPE:	.ds 2
XFER_SRC:	.ds 2
XFER_DEST:	.ds 2
XFER_LEN:	.ds 2
XFER_RTS:	.ds 2
	.code

	.bank	LIB1_BANK

; Functions
; ----


; ac_cd_xfer(char AC_reg [bl], char sector_h [cl],char sector_m [ch], char sector_l [dl],
;		 char sectors [al] )
; ----
; Call system card CD_READ to transfer directly to AC memory. Max sectors is 4 (8k).
; ----

_ac_cd_xfer.4:
	cly
	bra	maplib3_group_3

; ac_init();
; ----
; Detect and initialise Arcade Card. Returns 1 if detected and 0 if not.
; ----

_ac_init:
	ldy	#1
	bra	maplib3_group_3


; ac_vram_xfer(char AC_reg [al], int vram_addr [bx], int num_bytes [cx], char chunk_size [dl] )
; ----
; ac_vram_xfer(char AC_reg [al], int vram_addr [bx], int num_bytes [cx], char chunk_size [dl], char SGX [NULL] )
; ----
.ifdef _SGX
_ac_vram_xfer.5:
	ldy	#2
	bra	maplib3_group_3
.endif

_ac_vram_xfer.4:
	ldy	#3
	bra	maplib3_group_3


; ac_vram_dma( char AC_reg [al], int vram_addr [bx], int size [cx] )
; ----
; ac_vram_dma( char AC_reg [al], int vram_addr [bx], int size [cx], const SGX [ah] )
; ----

_ac_vram_dma.3:
	ldy	#4
	bra	maplib3_group_3

.ifdef _SGX
_ac_vram_dma.4:
	ldy	#5
	bra	maplib3_group_3
.endif

; ac_vram_copy( char AC_reg [al], int vram_addr [bx], int size [cx] )
; ----
; ac_vram_copy( char AC_reg [al], int vram_addr [bx], int size [cx], const SGX [ah] )
; ----

_ac_vram_copy.4:
	ldy	#6
	bra	maplib3_group_3

.ifdef _SGX
_ac_vram_copy.5:
	ldy	#7
	bra	maplib3_group_3
.endif


; ac_vce_copy( char AC_reg [al], int color_offset [bx], int size [cx] )
; ----

_ac_vce_copy.3:
	ldy	#8
	bra	maplib3_group_3


maplib3_group_3:
	maplibfunc_y	lib3_group_3
	rts

	.bank	LIB3_BANK
lib3_group_3:
	cpy	#1
	bcc	lib3_group_3_lbl0
	beq	lib3_group_3_lbl1
	cpy	#3
	bcc	lib3_group_3_lbl2
	beq	lib3_group_3_lbl3
	cpy	#5
	bcc	lib3_group_3_lbl4
	beq	lib3_group_3_lbl5
	cpy	#7
	bcc	lib3_group_3_lbl6
	beq	lib3_group_3_lbl7
	cpy	#9
	bcc	lib3_group_3_lbl8
	bcs	lib3_group_3_lbl9

lib3_group_3_lbl0:
	jmp	lib3_ac_cd_xfer.4
lib3_group_3_lbl1:
	jmp	lib3_ac_init
lib3_group_3_lbl2:
	jmp	lib3_ac_vram_xfer.5
lib3_group_3_lbl3:
	jmp	lib3_ac_vram_xfer.4
lib3_group_3_lbl4:
	jmp	lib3_ac_vram_dma.3
lib3_group_3_lbl5:
	jmp	lib3_ac_vram_dma.4
lib3_group_3_lbl6:
	jmp	lib3_ac_vram_copy.4
lib3_group_3_lbl7:
	jmp	lib3_ac_vram_copy.5
lib3_group_3_lbl8:
	jmp	lib3_ac_vce_copy.3
lib3_group_3_lbl9:
	rts

	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ---
lib3_ac_vram_copy.5:
	ldx	#$10	;SGX version
	bra	lib3_ac_vram_copy.main

lib3_ac_vram_copy.4:
	clx

lib3_ac_vram_copy.main:
	lda	<cl
	eor	#$ff
	clc
	adc	#$01
	sta	<cl
	lda	<ch
	eor	#$ff
	adc	#$00
	sta	<ch

	lda	<al		;the AC reg
	and	#$03
	asl a
	asl a
	asl a
	asl a
	tay
	stz	$0000,x
	cpx	#$10
	stz	<vdc_reg
	lda	<bl
	sta	$0002,x
	lda	<bh
	sta	$0003,x
	lda	#$02
	cpx	#$10
	sta	vdc_reg
	sta	$0000,x

.loop
	lda	$1a00,y
	sta	$0002,x
	lda	$1a01,y
	sta	$0003,x
	inc	<cl
	bne	.loop
	inc	<ch
	bne	.loop
	rts



; ---
lib3_ac_vce_copy.3:
	lda	<cl
	eor	#$ff
	clc
	adc	#$01
	sta	<cl
	lda	<ch
	eor	#$ff
	adc	#$00
	sta	<ch

	lda	<al		;the AC reg
	and	#$03
	asl a
	asl a
	asl a
	asl a
	tay
	lda	<bl
	sta	$402
	lda	<bh
	sta	$403

.loop
	lda	$1a00,y
	sta	$404
	lda	$1a01,y
	sta	$405
	inc	<cl
	bne	.loop
	inc	<ch
	bne	.loop
	rts


	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----
lib3_ac_cd_xfer.4:

	ldx	<dh ;->mid
	stx	<ch ; mid->ch

	; setup for MPR5
	lda	#$05
	sta	<dh

	; setup AC bank
	stz	<bh
	lda	<bl
	and	#$03
	ora	#$40
	sta	<bl

	; make sure not more than 4 sectors(8k) are transfered
	lda	<al
	cmp	#$05
	bcc	.skip
	lda	#$04
.skip
	sta	<al
	stz	<ah

	jsr	$E009

	rts
	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----

lib3_ac_init:
	jsr	ac_init

	bcs	.no_ac_card
;else
	jmp	.ac_card

.no_ac_card:
	ldx	#$00
	cla
	jmp	.out_ac_init

.ac_card:
	ldx	#$01
	cla
.out_ac_init:
	rts

ac_init:
.include "ac_init.asm"
	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----
lib3_ac_vram_xfer.5:

	; setup AC reg
	lda	<al
	and	#$03
	clc
	adc	#$40
	sta	<al

	; prep opcode for TIA block transfer
	lda	#$E3
	sta	XFER_TYPE+1
	lda	#$60
	sta	XFER_RTS

	; load length. Not needed in the main loop.
	lda	<dl
	asl a			;convert words to bytes
	sta	XFER_LEN
	stz	XFER_LEN+1

	; load source address. Not need in main loop.
	stz	XFER_SRC
	lda	#$C0
	sta	XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$12
	sta	XFER_DEST
	stz	XFER_DEST+1

	; setup vram write address. Save current VDC reg.
	cla
	;sta	<vdc_reg	;need to due SGX equiv
	sta	$0010
	lda	<bl
	sta	$0012
	lda	<bh
	sta	$0013

	lda	#$02
	;sta	<vdc_reg	;need to due SGX equiv
	sta	$0010
	jmp	loop_ac_vram


lib3_ac_vram_xfer.4:

	; setup AC reg
	lda	<al
	and	#$03
	clc
	adc	#$40
	sta	<al


	; prep opcode for TIA block transfer
	lda	#$E3
	sta	XFER_TYPE+1
	lda	#$60
	sta	XFER_RTS

	; load length. Not needed in the main loop.
	lda	<dl
	asl a			; convert words to bytes
	sta	XFER_LEN
	stz	XFER_LEN+1

	; load source address. Not need in main loop.
	stz	XFER_SRC
	lda	#$C0
	sta	XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$02
	sta	XFER_DEST
	stz	XFER_DEST+1

	; setup vram write address. Save current VDC reg.
	cla
	sta	<vdc_reg
	st0	#$00
	lda	<bl
	sta	$0002
	lda	<bh
	sta	$0003

	lda	#$02
	sta	<vdc_reg
	st0	#$02

	; main loop. Decrements until by fourth arguement
loop_ac_vram:
	lda	<cl
	pha
	sec
	sbc	<dl
	sta	<cl
	lda	<ch
	sbc	#$00
	cmp	#$FF
	beq	.last_chunk
	sta	<ch
	pla

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<al
	tam	#$06

	; call XFER instruction
	jsr	XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; do it all again
	jmp	loop_ac_vram

.last_chunk:
	pla
	beq	.ac_vram_xfer_out

	; load length. This is less than base value in dl so use cl instead
	lda	<cl
	asl a
	sta	XFER_LEN
	stz	XFER_LEN+1

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<al
	tam	#$06

	; call XFER instruction
	jsr	XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; that's all.
.ac_vram_xfer_out:
	rts
	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----
lib3_ac_vram_dma.4:
.ifdef _SGX
	; setup AC reg
	lda	<al
	and	#$03
	clc
	adc	#$40
	sta	<al

	; prep opcode for TIA block transfer
	lda	#$E3
	sta	XFER_TYPE+1
	lda	#$60
	sta	XFER_RTS

	; load source address. Not need in main loop. [XXX: Huh?]
	stz	XFER_SRC
	lda	#$C0
	sta	XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$12
	sta	XFER_DEST
	stz	XFER_DEST+1

	; setup vram write address
	cla
	sta	<sgx_vdc_reg
	stz	$0010
	lda	<bl
	sta	$0012
	lda	<bh
	sta	$0013

	lda	#$02
	sta	<sgx_vdc_reg
	sta	$0010

	; Setup fixed length outside main loop.
	stz	XFER_LEN
	lda	#$20
	sta	XFER_LEN+1
	jmp	loop_ac_dma
.endif

lib3_ac_vram_dma.3:

	; setup AC reg
	lda	<al
	and	#$03
	clc
	adc	#$40
	sta	<al

	; prep opcode for TIA block transfer
	lda	#$E3
	sta	XFER_TYPE+1
	lda	#$60
	sta	XFER_RTS

	; load source address. Not need in main loop.
	stz	XFER_SRC
	lda	#$C0
	sta	XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$02
	sta	XFER_DEST
	stz	XFER_DEST+1

	; setup vram write address
	cla
	sta	<vdc_reg
	st0	#$00
	lda	<bl
	sta	$0002
	lda	<bh
	sta	$0003

	lda	#$02
	sta	<vdc_reg
	st0	#$02

	; Setup fixed length outside main loop.
	stz	XFER_LEN
	lda	#$20
	sta	XFER_LEN+1

	; check to see if transfer size is 8k or less
loop_ac_dma:
	lda	<ch
	beq	.last_blck_2nd_chk
	cmp	#$10	; compare MSB of 0x1000 words
	beq	.ac_2nd_check_LL1
	bcs	.oversize_8k_LL1
;else (is less then)
	bra	.last_block_dma
.ac_2nd_check_LL1:
	lda	<cl
	beq	.last_block_dma

.oversize_8k_LL1:
	; main loop. Decrements by $2000
	lda	<ch
	sec
	sbc	#$10	; subtract 0x10 words (0x20 bytes)
	sta	<ch

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<al
	tam	#$06

	; call XFER instruction
	jsr	XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; do it all again
	jmp	loop_ac_dma

	; CH was zero, so check to see what CL is. If zer0, then done.
.last_blck_2nd_chk:
	lda	<cl
	beq	.ac_vram_dma_out

.last_block_dma:
	; load length. This is less than bas value in dl so use cl instead
	lda	<cl
	asl a
	rol	<ch
	sta	XFER_LEN
	lda	<ch
	sta	XFER_LEN+1

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<al
	tam	#$06

	; call XFER instruction
	jsr	XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; that's all.
.ac_vram_dma_out:
	rts

	.bank	LIB1_BANK
