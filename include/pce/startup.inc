;
; STARTUP.INC  -  MagicKit startup code equates and macros
;

; ----
; this block defines names for joypad
; direction and button bits

JOY_I		= $01
JOY_II		= $02
JOY_SEL		= $04
JOY_RUN		= $08
JOY_UP		= $10
JOY_RIGHT	= $20
JOY_DOWN	= $40
JOY_LEFT	= $80

JOY_TYPE6	= $50
JOY_III		= $01
JOY_IV		= $02
JOY_V		= $04
JOY_VI		= $08

JOY_BITI	= $0
JOY_BITII	= $1
JOY_BITSEL	= $2
JOY_BITRUN	= $3
JOY_BITUP	= $4
JOY_BITRT	= $5
JOY_BITDN	= $6
JOY_BITLT	= $7

JOY_BITIII	= $0
JOY_BITIV	= $1
JOY_BITV	= $2
JOY_BITVI	= $3

; ----
; this block defines names for interrupt
; vectors
;
IRQ2		= 0
IRQ1		= 1
TIMER		= 2
NMI		= 3
VSYNC		= 4
HSYNC		= 5
SOFT_RESET	= 6


; ----
; setvec(num, addr)
; ----
; num,	vector number (see defines above)
; addr,	address of the new routine
; ----

setvec	.macro
	 lda	\1
	 ldx	#LOW(\2)
	 ldy	#HIGH(\2)
	 jsr	set_intvec
.endm

; ----
; vec_on(num)
; ----
; num, vector to enable
; ----

vec_on	.macro
.if (\1 = 5)
	 smb	#6,<irq_m
.else
	 smb	\1,<irq_m
.endif
.endm

; ----
; vec_off(num)
; ----
; num, vector to disable
; ----

vec_off	.macro
.if (\1 = 5)
	 rmb	#6,<irq_m
.else
	 rmb	\1,<irq_m
.endif
.endm

; ----
; vsync([nb])
; ----
; nb, number of frames to be sync'ed on
; ----

vsync	.macro
.if (\# = 0)
	 lda #1
.else
	 lda \1
.endif
	 jsr	wait_vsync
.endm
