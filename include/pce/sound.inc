;
; PSG.INC  -  MagicKit PSG Player Equates
;
;

; --------
; defines
;

PSG_INHIBIT =	$80
PSG_DDA =	$40

PSG_MAINPAUSE =	$80
PSG_SUBPAUSE =	$40

;PSG_NOISE =	$80
;PSG_LFO_ON =	$80


; --------
; useful macros:
;

ENV_RLS	.macro
	 .db $fb
	 .dw \1
	.endm

ENV_LEV	.macro
	 .db $fc
	 .dw \1
	.endm

ENV_DLY	.macro
	 .db \1
	 .dw \2
	.endm

ENV_END	.macro
	 .db $ff
	.endm


; --------
; This block defines standard system variables
; in zero-page.
;

psg_irqflag	=	$20e6
psg_inhibit	=	$20e7
psg_tmpptr	=	$20e8
psg_tmpptr1	=	$20ea



		.org	$22d0

; --------
; "global" data (common to all voices)
;
psg_tempo	.ds	1
psg_trkhdr	.ds	2
psg_wavehdr	.ds	2
psg_envhdr	.ds	2
psg_percusshdr	.ds	2
psg_fmhdr	.ds	2
psg_pehdr	.ds	2
psg_trkctrl	.ds	1

psg_ch_value	.ds	1
psg_currvoice	.ds	1
psg_song	.ds	1
psg_bank1	.ds	1
psg_bank2	.ds	1


; "local" data array (one item for each voice)
;
psg_voicectrl	.ds	12
psg_trkptr_l	.ds	12
psg_trkptr_h	.ds	12
psg_savepoint_l	.ds	12
psg_savepoint_h	.ds	12
psg_cntdown	.ds	12
psg_octave	.ds	12
psg_freq_l	.ds	12
psg_freq_h	.ds	12
psg_perchvol	.ds	12
psg_keyonratio	.ds	12
psg_keycntdown	.ds	12
psg_transpose	.ds	12
psg_duratnmult	.ds	12
psg_envptr_l	.ds	12
psg_envptr_h	.ds	12
psg_envptr_off	.ds	12
psg_envlevel_l	.ds	12
psg_envlevel_h	.ds	12
psg_envduratn_l	.ds	12
psg_envduratn_h	.ds	12
psg_envduratn	.ds	12
psg_envrels_l	.ds	12
psg_envrels_h	.ds	12
psg_fmptr_l	.ds	12
psg_fmptr_h	.ds	12
psg_fmptr_off	.ds	12
psg_fmcorrect	.ds	12
psg_fmdelay	.ds	12
psg_fmdelcntdn	.ds	12
psg_peptr_l	.ds	12
psg_peptr_h	.ds	12
psg_peptr_off	.ds	12
psg_pedelay	.ds	12
psg_pedelcntdn	.ds	12
psg_detune	.ds	12
psg_volchg	.ds	12
psg_volchgaccum	.ds	12
psg_panpotchg	.ds	12
psg_panpotaccum	.ds	12
psg_sweep	.ds	12
psg_sweepamt_l	.ds	12
psg_sweepamt_h	.ds	12
psg_swptime	.ds	12
psg_swptmcntdn	.ds	12
psg_percuss_l	.ds	12
psg_percuss_h	.ds	12

psg_stack_0	.ds	11
psg_stack_1	.ds	11
psg_stack_2	.ds	11
psg_stack_3	.ds	11
psg_stack_4	.ds	11
psg_stack_5	.ds	11
psg_stack_6	.ds	10
psg_stack_7	.ds	10
psg_stack_8	.ds	10
psg_stack_9	.ds	10
psg_stack_10	.ds	10
psg_stack_11	.ds	10

psg_mode	.ds	12
psg_stkoff	.ds	12
psg_tone_l	.ds	12
psg_tone_h	.ds	12
psg_volume	.ds	12
psg_panpot	.ds	12
psg_wavenum	.ds	12
psg_noisefreq	.ds	12
psg_strikephase	.ds	12
psg_fadespeed	.ds	2
psg_fadecount	.ds	2
psg_fadevolcut	.ds	2

