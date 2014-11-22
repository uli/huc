;
; EQU.INC  -  MagicKit Standard Equates
;


; --------
; This block defines standard system variables
; in zero-page.
;

vdc_crl	= $20F3	; VDC control register (copy of)
vdc_crh	= $20F4	;
irq_m	= $20F5	; interrupt control mask (copy of)
vdc_sr	= $20F6	; VDC status register (copy of)
vdc_reg	= $20F7	; VDC register index (copy of)

; old aliases (for compatibility only, don't use them)
.ifndef HUC
_irq_m	= $20F5
_vsr	= $20F6
_vreg	= $20F7
.endif


; --------
; This block defines standard parameter-passing
; areas (in zero-page) for subroutines.
;

bp	= $20EC	; base pointer
si	= $20EE	; source address
di	= $20F0	; destination address
ax	= $20F8
al	= $20F8
ah	= $20F9
bx	= $20FA
bl	= $20FA
bh	= $20FB
cx	= $20FC
cl	= $20FC
ch	= $20FD
dx	= $20FE
dl	= $20FE
dh	= $20FF

.ifdef C_REGS
; C aliases
___bp	= bp
___si	= si
___di	= di
___ax	= ax
___al	= al
___ah	= ah
___bx	= bx
___bl	= bl
___bh	= bh
___cx	= cx
___cl	= cl
___ch	= ch
___dx	= dx
___dl	= dl
___dh	= dh
.endif

.ifdef DEPRECATED
; aliases for old assembler code
_bp	= bp
_si	= si
_di	= di
_ax	= ax
_al	= al
_ah	= ah
_bx	= bx
_bl	= bl
_bh	= bh
_cx	= cx
_cl	= cl
_ch	= ch
_dx	= dx
_dl	= dl
_dh	= dh
.endif


; --------
; This block defines names for standard
; devices and equipment on the PC-Engine.
; (they should be self-explanatory...)
;

; ----
; VDC (Video Display Controller)

videoport	.equ $0000

video_reg	.equ videoport
video_reg_l	.equ video_reg
video_reg_h	.equ video_reg+1

video_data	.equ videoport+2
video_data_l	.equ video_data
video_data_h	.equ video_data+1


.ifdef _SGX
; ----
; SGX VDC (Video Display Controller)

sgx_videoport		.equ $0010

sgx_video_reg		.equ sgx_videoport
sgx_video_reg_l		.equ sgx_video_reg
sgx_video_reg_h		.equ sgx_video_reg+1

sgx_video_data		.equ sgx_videoport+2
sgx_video_data_l	.equ sgx_video_data
sgx_video_data_h	.equ sgx_video_data+1


; ----
; SGX VPC (Video Priority Controller)

sgx_vpcport	.equ $0008

vpc_ctrl_1	.equ sgx_vpcport
vpc_ctrl_2	.equ sgx_vpcport+1

vpc_window_1	.equ sgx_vpcport+2
vpc_window_1_l	.equ vpc_window_1
vpc_window_1_h	.equ vpc_window_1+1

vpc_window_2	.equ sgx_vpcport+4
vpc_window_2_l	.equ vpc_window_2
vpc_window_2_h	.equ vpc_window_2+1

vpc_vdc_redir	.equ sgx_vpcport+6

.endif ; _SGX


.ifdef _AC
; Arcade Card equates
; ----

ac_port_1		.equ $1A00
ac_reg_1		.equ $1A02
ac_reg_1_low		.equ ac_reg_1
ac_reg_1_mid		.equ ac_reg_1+1
ac_reg_1_high		.equ ac_reg_1+2
ac_reg_1_offset_l	.equ ac_reg_1+3
ac_reg_1_offset_h	.equ ac_reg_1+4
ac_reg_1_incrmt_l	.equ ac_reg_1+5
ac_reg_1_incrmt_h	.equ ac_reg_1+6
ac_reg_1_control	.equ ac_reg_1+7

ac_port_2		.equ $1A10
ac_reg_2		.equ $1A12
ac_reg_2_low		.equ ac_reg_2
ac_reg_2_mid		.equ ac_reg_2+1
ac_reg_2_high		.equ ac_reg_2+2
ac_reg_2_offset_l	.equ ac_reg_2+3
ac_reg_2_offset_h	.equ ac_reg_2+4
ac_reg_2_incrmt_l	.equ ac_reg_2+5
ac_reg_2_incrmt_h	.equ ac_reg_2+6
ac_reg_2_control	.equ ac_reg_2+7

ac_port_3		.equ $1A20
ac_reg_3		.equ $1A22
ac_reg_3_low		.equ ac_reg_3
ac_reg_3_mid		.equ ac_reg_3+1
ac_reg_3_high		.equ ac_reg_3+2
ac_reg_3_offset_l	.equ ac_reg_3+3
ac_reg_3_offset_h	.equ ac_reg_3+4
ac_reg_3_incrmt_l	.equ ac_reg_3+5
ac_reg_3_incrmt_h	.equ ac_reg_3+6
ac_reg_3_control	.equ ac_reg_3+7

ac_port_4		.equ $1A30
ac_reg_4		.equ $1A32
ac_reg_4_low		.equ ac_reg_4
ac_reg_4_mid		.equ ac_reg_4+1
ac_reg_4_high		.equ ac_reg_4+2
ac_reg_4_offset_l	.equ ac_reg_4+3
ac_reg_4_offset_h	.equ ac_reg_4+4
ac_reg_4_incrmt_l	.equ ac_reg_4+5
ac_reg_4_incrmt_h	.equ ac_reg_4+6
ac_reg_4_control	.equ ac_reg_4+7

.endif ; _AC


; ----
; VCE (Video Color Encoder)

colorport	.equ $0400
color_ctrl	.equ colorport

color_reg	.equ colorport+2
color_reg_l	.equ color_reg
color_reg_h	.equ color_reg+1

color_data	.equ colorport+4
color_data_l	.equ color_data
color_data_h	.equ color_data+1

.ifdef HUC
_color_reg	.equ colorport+2
_color_data	.equ colorport+4
.endif


; ----
; PSG (Programmable Sound Generator)

psgport		.equ $0800
psg_ch		.equ psgport
psg_mainvol	.equ psgport+1
psg_freqlo	.equ psgport+2
psg_freqhi	.equ psgport+3
psg_ctrl	.equ psgport+4
psg_pan		.equ psgport+5
psg_wavebuf	.equ psgport+6
psg_noise	.equ psgport+7
psg_lfofreq	.equ psgport+8
psg_lfoctrl	.equ psgport+9


; ----
; TIMER

timerport	.equ $0C00
timer_cnt	.equ timerport
timer_ctrl	.equ timerport+1


; ----
; I/O port

joyport		.equ $1000


; ----
; IRQ ports

irqport		.equ $1400
irq_disable	.equ irqport+2
irq_status	.equ irqport+3


; ----
; CDROM/Expansion ports

cd_port		.equ $1800

bram_lock	.equ cd_port+3	; a read access here will do it
bram_unlock	.equ cd_port+7	; actually, bit #$80 of this byte


; ----
; Arcade Card ports

ac_port		.equ $1A00
ac_data1	.equ ac_port
ac_data1_alt	.equ ac_port+1
ac_base1_l	.equ ac_port+2
ac_base1_m	.equ ac_port+3
ac_base1_h	.equ ac_port+4
ac_offset1_l	.equ ac_port+5
ac_offset1_m	.equ ac_port+6
ac_cntrol1	.equ ac_port+9
ac_addoffset1	.equ ac_port+$0A

ac_shftreg	.equ $1AE0		; actually, probably rotate register
ac_shftreg_0	.equ ac_shftreg
ac_shftreg_1	.equ ac_shftreg+1
ac_shftreg_2	.equ ac_shftreg+2
ac_shftreg_3	.equ ac_shftreg+3
ac_shft_bits	.equ ac_shftreg+4	; positive = shift left

ac_identbase	.equ $1AFD
ac_identver_l	.equ ac_identbase
ac_identver_h	.equ ac_identbase+1
ac_identflag	.equ ac_identbase+2

AC_IDENT	.equ $51	; if ac_identflag = AC_IDENT, then AC in use


; --------
; This block defines names for macro
; argument types (\?x).
;

ARG_NONE	.equ 0
ARG_REG		.equ 1
ARG_IMMED	.equ 2
ARG_ABS		.equ 3
ARG_ABSOLUTE	.equ 3
ARG_INDIRECT	.equ 4
ARG_STRING	.equ 5
ARG_LABEL	.equ 6
