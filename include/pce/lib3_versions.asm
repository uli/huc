	.bank LIB3_BANK

; sgx_load_vram
; ----
lib3_sgx_load_vram:
lib3_sgx_load_vram.3:

	; ----
	; map data
	;
	jsr	lib3_map_data

	; ----
	; setup call to TIA operation (fastest transfer)
	;
	; (instruction setup done during bootup...)

	stw	#sgx_video_data, _ram_hdwr_tia_dest
;	stw	<_si, _ram_hdwr_tia_src
;
;	asl	<_cl		; change from words to bytes (# to xfer)
;	rol	<_ch

	; ----
	; set vram address
	;
	jsr	lib3_sgx_set_write

	; ----
	; copy data
	;
	cly
	ldx	<_cl
	beq	.l3
	; --
.l1:	lda	[_si],Y
	sta	sgx_video_data_l
	iny
	lda	[_si],Y
	sta	sgx_video_data_h
	iny
	bne	.l2
	inc	<_si+1
	; --
.l2:	dex
	bne	.l1
	; --
	jsr	lib3_remap_data
	; --
.l3:	dec	<_ch
	bpl	.l1

;.l1:	lda	<_ch		; if zero-transfer, exit
;	ora	<_cl
;	beq	.out
;
;	lda	<_ch
;	cmp	#$20		; if more than $2000, repeat xfers of $2000
;	blo	.l2		; while adjusting banks
;	sub	#$20		; reduce remaining transfer amount
;	sta	<_ch
;
;	stw	#$2000, _ram_hdwr_tia_size
;	jsr	_ram_hdwr_tia
;
;	lda	<_si+1		; force bank adjust
;	add	#$20		; and next move starts at same location
;	sta	<_si+1
;
;	jsr	lib3_remap_data	; adjust banks
;	bra	.l1
;
;.l2:	sta	HIGH_BYTE _ram_hdwr_tia_size	; 'remainder' transfer of < $2000
;	lda	<_cl
;	sta	LOW_BYTE  _ram_hdwr_tia_size
;	jsr	_ram_hdwr_tia

	; ----
	; unmap data
	;

.out:
	; restore PCE VDC address
	stw	#video_data, _ram_hdwr_tia_dest
	jmp	lib3_unmap_data

; ----
; unmap_data
; ----
; IN :	_BX = old banks
; ----

lib3_unmap_data:

	lda	<_bl
	tam	#3
	lda	<_bh
	tam	#4
	rts

; ----
; remap_data
; ----

lib3_remap_data:
	lda	<_bp
	bne	.l1
	lda	<_si+1
	bpl	.l1
	sub	#$20
	sta	<_si+1
	tma	#4
	tam	#3
	inc a
	tam	#4
.l1:
	rts

; ----
; map_data
; ----
; map data in page 3-4 ($6000-$9FFF)
; ----
; IN :	_BL = data bank
;	_SI = data address
; ----
; OUT:	_BX = old banks
;	_SI = remapped data address
; ----

lib3_map_data:
	ldx	<_bl

	; ----
	; save current bank mapping
	;
	tma	#3
	sta	<_bl
	tma	#4
	sta	<_bh
	; --
	cpx	#$FE
	bne	.l1
	; --
	stx	<_bp
	rts

	; ----
	; map new bank
	;
.l1:	stz	<_bp
	; --
	txa
	tam	#3
	inc a
	tam	#4

	; ----
	; remap data address to page 3
	;
	lda	<_si+1
	and	#$1F
	ora	#$60
	sta	<_si+1
	rts


; ----
; sgx_set_write
; ----
; set the SGX VDC VRAM write pointer
; ----
; IN :	_DI = VRAM location
; ----

lib3_sgx_set_write:
	lda #$00
	sta	sgx_video_reg
	lda	<_di
	sta	sgx_video_data_l
.ifdef HUC
	sta	_sgx_vdc
.endif
	lda	<_di+1
	sta	sgx_video_data_h
.ifdef HUC
	sta	_sgx_vdc+1
.endif
	lda	#$02
	sta	sgx_video_reg
	rts


	.bank LIB1_BANK
