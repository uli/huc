; ----
; sgx_load_map_8/16
; ----
; transfer a tiled map in VRAM
; ----
; IN :  _AL = x screen coordinate (tile unit)
;       _AH = y screen coordinate
;       _CL = x start coordinate in the map
;       _CH = y start coordinate
;       _DL = nb of column to copy
;       _DH = nb of row
; ----


sgx_load_map:
	lda   sgx_maptiletype
	cmp   #8
	beq   .l1
	; --
	jsr lib2_sgx_load_map_16
	rts
.l1:	jsr	lib2_sgx_load_map_8
	rts


lib2_sgx_load_map_16:

    ; ----
    ; save bank mapping
    ;
	tma   #2
	pha
	tma   #3
	pha
	tma   #4
	pha

    ; ----
    ; init
    ;
	jsr   sgx_load_map_init
	lda   <_ch
	sta   sgx_mapbat_x+1

    ; ----
    ; vertical loop
    ;
.l1:	ldy   <_ah
	lda   <_dl
	sta   <_al
	lda   sgx_mapbat_x+1
	sta   sgx_mapbat_x
	bra   .l5

    ; ----
    ; horizontal loop
    ;
.l2:	lda   sgx_mapbat_x		; bat wrapping
	add   #2
	and   sgx_bat_hmask
	sta   sgx_mapbat_x
	bne   .l3
	; --
	lda   sgx_bat_hmask
	eor   #$ff
	and   <_di
	sta   <_di
	bra   .l4
.l3:
	incw  <_di
	incw  <_di
.l4:
	iny
	; --
	cpy   sgx_mapwidth		; horizontal map wrapping
	bne   .l5
	cly
	ldx   sgx_mapwrap
	bne   .l5
	ldy   sgx_mapwidth
	lda   sgx_maptilebase
	sta   <_cl
	lda   sgx_maptilebase+1
	ora   [_bp]
	sta   <_ch
	dey
	bra   .l6
.l5:
	lda   [_si],Y		; get tile index
	tax			; calculate BAT value (tile + palette)
	sxy
        stz   <_ch
	asl   A
	rol   <_ch
	asl   A
	rol   <_ch
	add   sgx_maptilebase
	sta   <_cl
	lda   <_ch
	adc   sgx_maptilebase+1
	adc   [_bp],Y
	sta   <_ch
	sxy
.l6:
	sgx_vreg  #0		; copy tile
	stw   <_di,sgx_video_data
	sgx_vreg  #2
	stw   <_cx,sgx_video_data
	incw  <_cx
	stw   <_cx,sgx_video_data
	incw  <_cx
	sgx_vreg  #0
	addw  sgx_bat_width,<_di,sgx_video_data
	sgx_vreg  #2
	stw   <_cx,sgx_video_data
	incw  <_cx
	stw   <_cx,sgx_video_data

	dec   <_al		; next tile
	lbne  .l2

    ; ----
    ; next line
    ;
	ldx   #2
	jsr   sgx_load_map_next_line
	dec   <_dh
	lbne  .l1

    ; ----
    ; restore bank mapping
    ;
	jmp   sgx_load_map_exit

lib2_sgx_load_map_8:

    ; ----
    ; save bank mapping
    ;
	tma   #2
	pha
	tma   #3
	pha
	tma   #4
	pha

    ; ----
    ; init
    ;
	jsr   sgx_load_map_init
	bra   .l2

    ; ----
    ; vertical loop
    ;
.l1:	ldx   #1
	jsr   sgx_load_map_next_line
	; --
.l2:	ldy   <_ah
	lda   <_dl
	sta   <_al
	lda   <_ch
	sta   <_cl
	sgx_vreg  #0		; set vram write ptr
	stw   <_di,sgx_video_data
	sgx_vreg  #2
	bra   .l5

    ; ----
    ; horizontal loop
    ;
.l3:	lda   <_cl		; bat wrapping
	inc   A
	and   sgx_bat_hmask
	sta   <_cl
	bne   .l4
	; --
	sgx_vreg  #0
	lda   sgx_bat_hmask
	eor   #$ff
	and   <_di
	sta   sgx_video_data_l
	lda   <_di+1
	sta   sgx_video_data_h
	sgx_vreg  #2
.l4:
	iny			; next tile
	; --
	cpy   sgx_mapwidth		; map wrapping
	bne   .l5
	; --
	cly
	lda   sgx_mapwrap
	bne   .l5
	ldy   sgx_mapwidth
	dey
	cla
	bra   .l6
.l5:
	lda   [_si],Y		; get tile index
.l6:	tax			; calculate BAT value (tile + palette)
	sxy
	add   sgx_maptilebase
	sta   sgx_video_data_l
	lda   sgx_maptilebase+1
	adc   [_bp],Y
	sta   sgx_video_data_h
	sxy

	dec   <_al
	bne   .l3

    ; ----
    ; next line
    ;
	dec   <_dh
	bne   .l1

    ; ----
    ; restore bank mapping
    ;
sgx_load_map_exit:
   	pla
	tam   #4
	pla
	tam   #3
	pla
	tam   #2
	rts



; ----
; sgx_load_map_next_line
; ----
; sgx_load_map sub routine
; ----
; IN :    X = BAT line inc value (1-2)
; ----
; OUT:  _DI = BAT address
;       _SI = map address
; ----
; USE:  _BL = BAT Y pos
;       _BH = map Y pos
;       _SI = map address
; ----

sgx_load_map_next_line:

    ; ----
    ; incremente vram address
    ;
    	txa
	add   <_bl
	cmp   sgx_mapbat_bottom
	blo   .l1
	; --
	sub   sgx_mapbat_bottom	; 1/ vram wrapping
	tax
	inx
	add   sgx_mapbat_top
	sta   <_bl
	lda   sgx_mapbat_ptr
	and   sgx_bat_hmask
	add   sgx_mapbat_top_base
	sta   sgx_mapbat_ptr
	cla
	adc   sgx_mapbat_top_base+1
	sta   sgx_mapbat_ptr+1
	bra   .l3
	; -- 
.l1:	sta   <_bl		; 2/ vram inc
.l2:	lda   sgx_bat_width
	add   sgx_mapbat_ptr
	sta   sgx_mapbat_ptr
	cla
	adc   sgx_mapbat_ptr+1
	sta   sgx_mapbat_ptr+1
	; --
.l3:	dex
	bne   .l2
	; --
	stw   sgx_mapbat_ptr,<_di

    ; ----
    ; incremente map address
    ;
	inc   <_bh
	lda   <_bh
	cmp   sgx_mapheight
	bne   .l4
	; --
	lda   sgx_mapbank		; 1/ map wrapping
	tam   #3
	inc   A
	tam   #4
	stb   sgx_mapaddr,<_si
	lda   sgx_mapaddr+1
	and   #$1F
	ora   #$60
	sta   <_si+1
	stz   <_bh
	bra   .l5
	; --
.l4:	addw  sgx_mapwidth,<_si	; 2/ map inc
	cmp   #$80
	blo   .l5
	sub   #$20
	sta   <_si+1
	tma   #4
	tam   #3
	inc   A
	tam   #4
.l5:
	rts


; ----
; sgx_load_map_init
; ----
; sgx_load_map sub routine
; ----
; OUT:  _DI = BAT address
;       _SI = map address
;       _BP = palette index table ptr
;       _AH = map X pos
;       _BH = map Y pos
;       _CH = BAT X pos
;       _BL = BAT Y pos
; ----

sgx_load_map_init:

    ; ----
    ; calculate vram address
    ;
	ldx   <_al
	lda   <_ah
	ldy   sgx_maptiletype
	cpy   #8
	beq   .l1
	asl   A
	sax
	asl   A
	sax
.l1:	phx
	pha
	jsr   sgx_calc_vram_addr
	stw   <_di,sgx_mapbat_ptr

    ; ----
    ; calculate map address
    ;
	stb   sgx_mapaddr,<_si
	lda   sgx_mapaddr+1
	and   #$1F
	sta   <_si+1
	; --
	ldx   <_cl
	stx   <_ah
	ldy   <_ch
	sty   <_bh
	; --
	lda   sgx_mapwidth+1
	beq   .l2
	tya
	add   <_si+1
	sta   <_si+1
	bra   .l3
    	; --
.l2:	sty   <_al
	lda   sgx_mapwidth
	sta   <_bl
	jsr   mulu8
	addw  <_cx,<_si

    ; ----
    ; calculate map bank
    ;
.l3:	rol   A
	rol   A
	rol   A
	rol   A
	and   #$0F
	add   sgx_mapbank

    ; ----
    ; map data
    ;
    	tam   #3
	inc   A
	tam   #4
	lda   sgx_mapctablebank
	tam   #2

    ; ----
    ; adjust data addresses
    ;
	lda   <_si+1		; tile ptr
	and   #$1F
	ora   #$60
	sta   <_si+1
	; --
	stb   sgx_mapctable,<_bp	; color table ptr
	lda   sgx_mapctable+1
	and   #$1F
	ora   #$40
	sta   <_bp+1

    ; ----
    ; bat pos
    ;
	pla
	and   sgx_bat_vmask
	sta   <_bl
	pla
	and   sgx_bat_hmask
	sta   <_ch
	rts


; ----
; sgx_calc_vram_addr
; ----
; calculate VDC2 VRAM address
; ----
; IN :    X = x coordinates
;         A = y     "
; ----
; OUT:  _DI = VRAM location
; ----

sgx_calc_vram_addr:
	phx
	and   sgx_bat_vmask
	stz   <_di
	ldx   sgx_bat_width
	cpx   #64
	beq   .s64
	cpx   #128
	beq   .s128
	; --
.s32:	lsr   A
	ror   <_di
	; --
.s64:	lsr   A
	ror   <_di
	; --
.s128:	lsr   A
	ror   <_di
	sta   <_di+1
	; --
	pla
	and   sgx_bat_hmask
	ora   <_di
	sta   <_di
	rts
