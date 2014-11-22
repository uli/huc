;
; VDC.INC  -  MagicKit PC-Engine video MACRO definitions
;


;
; VREG - set up video register to be read/written
;

.macro vreg
	lda	\1
	sta	<vdc_reg
.if (\?1 = ARG_IMMED)
	st0	\1
.else
	sta	video_reg
.endif
.endm

;
; SETVINC - set VRAM auto-increment step
;

.macro setvinc
	vreg	#5
.if (\?1 = ARG_IMMED)
	st2	\1
.else
	lda	\1
	sta	video_data_h
.endif
.endm

; standard increment step

VADDR_INC_1	= %00_000
VADDR_INC_32	= %01_000
VADDR_INC_64	= %10_000
VADDR_INC_128	= %11_000

;
; SETBGMAP - set the bgmap virtual size
;

.macro setbgmap
	lda	\1
	jsr	set_bat_size
.endm

; standard bgmap size

BGMAP_SIZE_32x32	= %000
BGMAP_SIZE_64x32	= %001
BGMAP_SIZE_128x32	= %011
BGMAP_SIZE_32x64	= %100
BGMAP_SIZE_64x64	= %101
BGMAP_SIZE_128x64	= %111

;
; These values represent a bit in the VCE which
; does color-correction to fix leading/trailing
; edges of images
;
; Default to XRES_SOFT
;
XRES_SOFT	= 4
XRES_SHARP	= 0
