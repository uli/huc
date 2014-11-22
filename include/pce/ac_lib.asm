; Arcade card functions library
;
; Tomaitheous '07

	.data
_XFER_TYPE:	.ds 2
_XFER_SRC:	.ds 2
_XFER_DEST:	.ds 2
_XFER_LEN:	.ds 2
_XFER_RTS:	.ds 2
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
	lda	<_cl
	eor	#$ff
	clc
	adc	#$01
	sta	<_cl
	lda	<_ch
	eor	#$ff
	adc	#$00
	sta	<_ch

	lda	<_al		;the AC reg
	and	#$03
	asl a
	asl a
	asl a
	asl a
	tay
	stz	$0000,x
	cpx	#$10
	stz	<vdc_reg
	lda	<_bl
	sta	$0002,x
	lda	<_bh
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
	inc	<_cl
	bne	.loop
	inc	<_ch
	bne	.loop
	rts



; ---
lib3_ac_vce_copy.3:
	lda	<_cl
	eor	#$ff
	clc
	adc	#$01
	sta	<_cl
	lda	<_ch
	eor	#$ff
	adc	#$00
	sta	<_ch

	lda	<_al		;the AC reg
	and	#$03
	asl a
	asl a
	asl a
	asl a
	tay
	lda	<_bl
	sta	$402
	lda	<_bh
	sta	$403

.loop
	lda	$1a00,y
	sta	$404
	lda	$1a01,y
	sta	$405
	inc	<_cl
	bne	.loop
	inc	<_ch
	bne	.loop
	rts


	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----
lib3_ac_cd_xfer.4:

	ldx	<_dh ;->mid
	stx	<_ch ; mid->ch

	; setup for MPR5
	lda	#$05
	sta	<_dh

	; setup AC bank
	stz	<_bh
	lda	<_bl
	and	#$03
	ora	#$40
	sta	<_bl

	; make sure not more than 4 sectors(8k) are transfered
	lda	<_al
	cmp	#$05
	bcc	.skip
	lda	#$04
.skip
	sta	<_al
	stz	<_ah

	jsr	$E009

	rts
	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----

lib3_ac_init:
	jsr	__ac_init

	bcs	__no_ac_card
;else
	jmp	__ac_card

__no_ac_card:
	ldx	#$00
	cla
	jmp	__out_ac_init

__ac_card:
	ldx	#$01
	cla
__out_ac_init:
	rts

__ac_init:
.include "ac_init.asm"
	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----
lib3_ac_vram_xfer.5:

	; setup AC reg
	lda	<_al
	and	#$03
	clc
	adc	#$40
	sta	<_al

	; prep opcode for TIA block transfer
	lda	#$E3
	sta	_XFER_TYPE+1
	lda	#$60
	sta	_XFER_RTS

	; load length. Not needed in the main loop.
	lda	<_dl
	asl a			;convert words to bytes
	sta	_XFER_LEN
	stz	_XFER_LEN+1

	; load source address. Not need in main loop.
	stz	_XFER_SRC
	lda	#$C0
	sta	_XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$12
	sta	_XFER_DEST
	stz	_XFER_DEST+1

	; setup vram write address. Save current VDC reg.
	cla
	;sta	<vdc_reg	;need to due SGX equiv
	sta	$0010
	lda	<_bl
	sta	$0012
	lda	<_bh
	sta	$0013

	lda	#$02
	;sta	<vdc_reg	;need to due SGX equiv
	sta	$0010
	jmp	__loop_ac_vram


lib3_ac_vram_xfer.4:

	; setup AC reg
	lda	<_al
	and	#$03
	clc
	adc	#$40
	sta	<_al


	; prep opcode for TIA block transfer
	lda	#$E3
	sta	_XFER_TYPE+1
	lda	#$60
	sta	_XFER_RTS

	; load length. Not needed in the main loop.
	lda	<_dl
	asl a			; convert words to bytes
	sta	_XFER_LEN
	stz	_XFER_LEN+1

	; load source address. Not need in main loop.
	stz	_XFER_SRC
	lda	#$C0
	sta	_XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$02
	sta	_XFER_DEST
	stz	_XFER_DEST+1

	; setup vram write address. Save current VDC reg.
	cla
	sta	<vdc_reg
	st0	#$00
	lda	<_bl
	sta	$0002
	lda	<_bh
	sta	$0003

	lda	#$02
	sta	<vdc_reg
	st0	#$02

	; main loop. Decrements until by fourth arguement
__loop_ac_vram:
	lda	<_cl
	pha
	sec
	sbc	<_dl
	sta	<_cl
	lda	<_ch
	sbc	#$00
	cmp	#$FF
	beq	__last_chunk
	sta	<_ch
	pla

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<_al
	tam	#$06

	; call XFER instruction
	jsr	_XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; do it all again
	jmp	__loop_ac_vram

__last_chunk:
	pla
	beq	__ac_vram_xfer_out

	; load length. This is less than base value in _dl so use _cl instead
	lda	<_cl
	asl a
	sta	_XFER_LEN
	stz	_XFER_LEN+1

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<_al
	tam	#$06

	; call XFER instruction
	jsr	_XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; that's all.
__ac_vram_xfer_out:
	rts
	.bank	LIB1_BANK


	.bank	LIB3_BANK
; ----
lib3_ac_vram_dma.4:
.ifdef _SGX
	; setup AC reg
	lda	<_al
	and	#$03
	clc
	adc	#$40
	sta	<_al

	; prep opcode for TIA block transfer
	lda	#$E3
	sta	_XFER_TYPE+1
	lda	#$60
	sta	_XFER_RTS

	; load source address. Not need in main loop. [XXX: Huh?]
	stz	_XFER_SRC
	lda	#$C0
	sta	_XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$12
	sta	_XFER_DEST
	stz	_XFER_DEST+1

	; setup vram write address
	cla
	sta	<sgx_vdc_reg
	stz	$0010
	lda	<_bl
	sta	$0012
	lda	<_bh
	sta	$0013

	lda	#$02
	sta	<sgx_vdc_reg
	sta	$0010

	; Setup fixed length outside main loop.
	stz	_XFER_LEN
	lda	#$20
	sta	_XFER_LEN+1
	jmp	__loop_ac_dma
.endif

lib3_ac_vram_dma.3:

	; setup AC reg
	lda	<_al
	and	#$03
	clc
	adc	#$40
	sta	<_al

	; prep opcode for TIA block transfer
	lda	#$E3
	sta	_XFER_TYPE+1
	lda	#$60
	sta	_XFER_RTS

	; load source address. Not need in main loop.
	stz	_XFER_SRC
	lda	#$C0
	sta	_XFER_SRC+1

	; load destination address (only need to do this once)
	lda	#$02
	sta	_XFER_DEST
	stz	_XFER_DEST+1

	; setup vram write address
	cla
	sta	<vdc_reg
	st0	#$00
	lda	<_bl
	sta	$0002
	lda	<_bh
	sta	$0003

	lda	#$02
	sta	<vdc_reg
	st0	#$02

	; Setup fixed length outside main loop.
	stz	_XFER_LEN
	lda	#$20
	sta	_XFER_LEN+1

	; check to see if transfer size is 8k or less
__loop_ac_dma:
	lda	<_ch
	beq	__last_blck_2nd_chk
	cmp	#$10	; compare MSB of 0x1000 words
	beq	__ac_2nd_check_LL1
	bcs	__oversize_8k_LL1
;else (is less then)
	bra	__last_block_dma
__ac_2nd_check_LL1:
	lda	<_cl
	beq	__last_block_dma

__oversize_8k_LL1:
	; main loop. Decrements by $2000
	lda	<_ch
	sec
	sbc	#$10	; subtract 0x10 words (0x20 bytes)
	sta	<_ch

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<_al
	tam	#$06

	; call XFER instruction
	jsr	_XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; do it all again
	jmp	__loop_ac_dma

	; CH was zero, so check to see what CL is. If zer0, then done.
__last_blck_2nd_chk:
	lda	<_cl
	beq	__ac_vram_dma_out

__last_block_dma:
	; load length. This is less than bas value in _dl so use _cl instead
	lda	<_cl
	asl a
	rol	<_ch
	sta	_XFER_LEN
	lda	<_ch
	sta	_XFER_LEN+1

	; setup correct bank and disable interrupts
	sei
	tma	#$06
	pha
	lda	<_al
	tam	#$06

	; call XFER instruction
	jsr	_XFER_TYPE+1

	; restore bank and interrupts
	pla
	tam	#$06
	cli

	; that's all.
__ac_vram_dma_out:
	rts

	.bank	LIB1_BANK
