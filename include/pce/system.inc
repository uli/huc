;
; SYSTEM.INC  -  CD-ROM System standard definitions
;
;


; --------
; This block defines types of supported CD-ROM
;

STANDARD_CD	= 1
SUPER_CD	= 2


; --------
; This block defines standard system variables
; in RAM page.
;

usrvec		= $2200
usr_jmptbl	= $2200
irq2_jmp	= $2200 ;2
irq1_jmp	= $2202 ;2	formerly called irq_jmp
timer_jmp	= $2204 ;2	formerly called timer_jmp
nmi_jmp		= $2206 ;2
vsync_hook	= $2208 ;2	formerly called sync_jmp
hsync_hook	= $220A ;2	formerly called rcr_jmp
bg_x1		= $220C ;2	formerly called bgx1
bg_x2		= $220E ;2
bg_y1		= $2210 ;2
bg_y2		= $2212 ;2
satb_addr	= $2214 ;2	formerly called sat_adr
sprptr		= $2216 ;1
spryl		= $2217 ;1
spryh		= $2218 ;1
sprxl		= $2219 ;1
sprxh		= $221A ;1
sprnl		= $221B ;1
sprnh		= $221C ;1
spral		= $221D ;1
sprah		= $221E ;1
color_cmd	= $221F ;1
bgc_ptr		= $2220 ;2
bgc_len		= $2222 ;1
sprc_ptr	= $2223 ;2
sprc_len	= $2225 ;1
joyena		= $2227 ;1
joy		= $2228 ;5
joytrg		= $222D ;5
joyold		= $2232 ;5
irq_cnt		= $2241 ;1
vdc_mwr		= $2242 ;1	formerly called mwr_m
vdc_dcr		= $2243 ;1	formerly called dcr_m
rndseed		= $2249 ;1
rndl		= $2249 ;1
rndh		= $224A ;1
rndm		= $224B ;1


; --------
; This block defines CD-ROM BIOS routine
; entry points.
;

cd_boot		= $E000
cd_reset	= $E003
cd_base		= $E006
cd_read		= $E009
cd_seek		= $E00C
cd_exec		= $E00F
cd_play		= $E012
cd_search	= $E015
cd_pause	= $E018
cd_stat		= $E01B
cd_subq		= $E01E
cd_dinfo	= $E021
cd_contnts	= $E024
cd_subrd	= $E027
cd_pcmrd	= $E02A
cd_fade		= $E02D

ad_reset	= $E030
ad_trans	= $E033
ad_read		= $E036
ad_write	= $E039
ad_play		= $E03C
ad_cplay	= $E03F
ad_stop		= $E042
ad_stat		= $E045

bm_format	= $E048
bm_free		= $E04B
bm_read		= $E04E
bm_write	= $E051
bm_delete	= $E054
bm_files	= $E057

ex_getver	= $E05A
ex_setvec	= $E05D
ex_getfnt	= $E060
ex_joysns	= $E063
ex_joyrep	= $E066
ex_scrsiz	= $E069
ex_dotmod	= $E06C
ex_scrmod	= $E06F
ex_imode	= $E072
ex_vmode	= $E075
ex_hmode	= $E078
ex_vsync	= $E07B
ex_rcron	= $E07E
ex_rcroff	= $E081
ex_irqon	= $E084
ex_irqoff	= $E087
ex_bgon		= $E08A
ex_bgoff	= $E08D
ex_spron	= $E090
ex_sproff	= $E093
ex_dspon	= $E096
ex_dspoff	= $E099
ex_dmamod	= $E09C
ex_sprdma	= $E09F
ex_satclr	= $E0A2
ex_sprput	= $E0A5
ex_setrcr	= $E0A8
ex_setred	= $E0AB
ex_setwrt	= $E0AE
ex_setdma	= $E0B1
ex_binbcd	= $E0B4
ex_bcdbin	= $E0B7
ex_rnd		= $E0BA
ex_memopen	= $E0DE
ex_colorcmd	= $E0E4

ma_mul8u	= $E0BD
ma_mul8s	= $E0C0
ma_mul16u	= $E0C3
ma_div16s	= $E0C6
ma_div16u	= $E0C9
ma_sqrt		= $E0CC
ma_sin		= $E0CF
ma_cos		= $E0D2
ma_atni		= $E0D5

psg_bios	= $E0D8
grp_bios	= $E0DB
psg_driver	= $E0E1


; --------
; This block defines aliases for library routines
; that have their equivalent in the CD-ROM BIOS.
;

		.if (CDROM)
mulu8		= ma_mul8u
mulu16		= ma_mul16u
set_intvec	= ex_setvec
		.endif

; --------
; This block defines PSG BIOS function
; number.
;

PSG_ON		= 0
PSG_OFF		= 1
PSG_INIT	= 2
PSG_BANK	= 3
PSG_TRACK	= 4
PSG_WAVE	= 5
PSG_ENV		= 6
PSG_FM		= 7
PSG_PE		= 8
PSG_PC		= 9
PSG_TEMPO	= 10
PSG_PLAY	= 11
PSG_MSTAT	= 12
PSG_SSTAT	= 13
PSG_MSTOP	= 14
PSG_SSTOP	= 15
PSG_ASTOP	= 16
PSG_MVOFF	= 17
PSG_CONT	= 18
PSG_FDOUT	= 19
PSG_DCNT	= 20


; --------
; This block defines GRP BIOS function
; number.
;

VI_GINIT	= 0
VI_CASHCLR	= 1
VI_STRTADR	= 2
VI_GETADRS	= 3
VI_CLS		= 4
VI_PSET		= 5
VI_POINT	= 6
VI_LINE		= 7
VI_BOX		= 8
VI_BOXF		= 9
VI_FLOOD	= 10
VI_PAINT	= 11
VI_GWINDOW	= 12
VI_GFONT	= 13
VI_PUTFONT	= 14
VI_SYMBOL	= 15

