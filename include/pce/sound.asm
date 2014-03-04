; SOUND.ASM
; ---------
;
; A PSG driver which is compatible with the "track data"
; and other bytecodes as used by the CDROM System Card
;


psg_bios:	phx
		phy
		tma	#PAGE(psg_on)
		pha
		lda	#BANK(psg_on)
		tam	#PAGE(psg_on)
		lda	<_dh
		asl	A
		tax
		jsr	.indir_jmp
		tax
		pla
		tam	#PAGE(psg_on)
		txa
		ply
		plx
		rts
.indir_jmp:	jmp	[.functbl,X]

.functbl:	.dw	psg_on		; psg_on       (00)
		.dw	psg_off		; psg_off      (01)
		.dw	psg_init	; psg_init     (02)
		.dw	psg_bank	; psg_bank     (03)
		.dw	psg_track	; psg_track    (04)
		.dw	psg_wave	; psg_wave     (05)
		.dw	psg_env 	; psg_env      (06)
		.dw	psg_fm		; psg_fm       (07)
		.dw	psg_pe		; psg_pe       (08)
		.dw	psg_pc		; psg_pc       (09)
		.dw	psg_settempo	; psg_settempo (0a)
		.dw	psg_play	; psg_play     (0b)
		.dw	psg_mstat	; psg_mstat    (0c)
		.dw	psg_sstat	; psg_sstat    (0d)
		.dw	psg_mstop	; psg_mstop    (0e)
		.dw	psg_sstop	; psg_sstop    (0f)
		.dw	psg_astop	; psg_astop    (10)
		.dw	psg_mvoff	; psg_mvoff    (11)
		.dw	psg_cont	; psg_cont     (12)
		.dw	psg_fdout	; psg_fdout    (13)
		.dw	psg_dcnt	; psg_dcnt     (14)

;
; Origin is at $C000.
;
; This is normally occupied by "START_BANK" when the program
; is destined for CDROM, but should be OK for HuCard-only
; development (note: PSG player exists in system card when
; developing for CDROM)
;

	.bank	PSG_BANK,"PSG Driver"
	.org	$C000


; ----
; PSG_ON
; ----
; CDROM version takes a parameter in _al
; 0=TIMER, 1=VSYNC
;
; This version will only support TIMER interrupt
;
psg_on:
	bbs7	<psg_inhibit,.noturnoff
	bsr	psg_off
	smb7	<psg_inhibit
.noturnoff:
	lda	#1
	sta	timer_ctrl
	rts


; ----
; PSG_OFF
; ----
;
; turn off PSG
; - all sound
; - interrupt service (if TIMER)
;
psg_off:
	bbs7	<psg_inhibit,.nostop
	jsr	psg_astop
.nostop:
	stz	timer_ctrl
	rts


; ----
; PSG_INIT
; ----
;
; Initialize PSG.  CDROM version has a parameter:
; 0=main track only; 60Hz
; 1=sub track only; 60Hz
; 2=both tracks; 60Hz
; 3=both tracks; 120Hz
; 4=both tracks; 240Hz
; 5=both tracks; 300Hz
;
; This version only supports "both channels/60Hz" for now
;
psg_init:

; initialize some IRQ stuff & zero values

	stz	psg_tempo	; clear header info
	tii	psg_tempo, psg_tempo+1, 19

;	stz	psg_trkptr_l	; clear per-track info
;	tii	psg_trkptr_l,psg_trkptr_l+1,????

	stz	<psg_irqflag
	stz	<psg_tmpptr
	stz	<psg_tmpptr+1
	stz	<psg_tmpptr1
	stz	<psg_tmpptr1+1

	smb7	<psg_inhibit

;
; reset hardware
;
	ldx	#5
	lda	#PSG_DDA
.loop1:	stx	psg_ch_value	; $0800 register value
	stx	psg_ch		; set channel
	sta	psg_ctrl	; reset channel
	stz	psg_ctrl	; reset wave buffer
	tin	psg_waveforms,psg_wavebuf,$0020	; simple squarewave
	dex
	bpl	.loop1

	lda	#$ff		; set global hardware values
	sta	psg_mainvol

	lda	#$01
	sta	psg_lfofreq
	stz	psg_lfoctrl

;
; set some basic default values
;
	lda	#$80
	sta	psg_song

	lda	#(PSG_MAINPAUSE | PSG_SUBPAUSE)
	sta	psg_trkctrl

	lda	#$26		; is this right ?
	jsr	psg_settempo

	rts


; ----
; PSG_BANK
; ----

psg_bank:
      __ldw	<_ax
      __stw	psg_bank1
	rts


; ----
; PSG_TRACK
; ----

psg_track:
      __ldw	<_ax
      __stw	psg_trkhdr
	rts


; ----
; PSG_WAVE
; ----

psg_wave:
      __ldw	<_ax
      __stw	psg_wavehdr
	rts


; ----
; PSG_ENV
; ----

psg_env:
      __ldw	<_ax
      __stw	psg_envhdr
	rts


; ----
; PSG_FM
; ----

psg_fm:
      __ldw	<_ax
      __stw	psg_fmhdr
	rts


; ----
; PSG_PE
; ----

psg_pe:
      __ldw	<_ax
      __stw	psg_pehdr
	rts


; ----
; PSG_PC
; ----

psg_pc:
      __ldw	<_ax
      __stw	psg_percusshdr
	rts


; ----
; PSG_SETTEMPO
; ----

psg_settempo:
	lda	<_al
	sub	#$23
	bpl	.okvalue
	cla
.okvalue:
	tax
	lda	psg_tempotbl,X
	sta	psg_tempo
	rts


; ----
; PSG_PLAY
;
; play song # in track list (_al = song #)
; ----

psg_play:
	lda	<_al
	sta	psg_song
	rmb7	<psg_inhibit
	rts


; ----
; PSG_MSTAT
;
; return bitmask of voices in use for main track
; ----

psg_mstat:
	ldy	#5
	cla

.loop:	ldx	psg_voicectrl,Y
	beq	.empty
	sec
	rol	A
	bra	.next
.empty:	clc
	rol	A
.next:	dey
	bpl	.loop
	rts


; ----
; PSG_SSTAT
;
; return bitmask of voices in use for sub track
; ----

psg_sstat:
	ldy	#5
	cla

.loop:	ldx	psg_voicectrl+6,Y
	beq	.empty
	sec
	rol	A
	bra	.next
.empty:	clc
	rol	A
.next:	dey
	bpl	.loop
	rts


; ----
; PSG_MSTOP
;
; Stop voices in main track, as described in bitmask (in _al)
; ----

psg_mstop:
	lda	<_al
	bpl	.nopause

	lda	#PSG_MAINPAUSE	; set pause on MAIN track
	tsb	psg_trkctrl

	lda	<_al

.nopause:
	tay
	clx

.loop:	lda	psg_voicectrl,X
	beq	.nextvoice
	tya
	bmi	.pause		; is it a 'pause' for voice ?

	lsr	A		; no, it is a 'stop'
	bcc	.nextvoice	; this voice not in mask; skip it

	stz	psg_voicectrl,X

.pause:	lda	psg_voicectrl+6,X	; check subtrack
	cmp	#1
	beq	.nextvoice

	stx	psg_ch_value
	stx	psg_ch
	stz	psg_ctrl

.nextvoice:
	inx
	cpx	#6
	bcs	.out
	tya
	bmi	.pause2
	
	lsr	A
	tay
	bra	.loop

.pause2:
	lsr	A
	ora	#$80		; retain high-bit set
	tay
	bra	.loop

.out:	rts


; ----
; PSG_SSTOP
;
; Stop voices in sub track, as described in bitmask (in _al)
; ----

psg_sstop:
	lda	<_al
	bpl	.nopause

	lda	#PSG_SUBPAUSE	; set pause on SUB track
	tsb	psg_trkctrl

	lda	<_al

.nopause:
	tay
	clx

.loop:	lda	psg_voicectrl+6,X
	beq	.nextvoice
	tya
	bmi	.pause		; is it a 'pause' ?

	lsr	A		; no, it is a 'stop'
	bcc	.nextvoice	; voice not in mask; skip it

	stz	psg_voicectrl+6,X
	bra	.pause1
.pause:
	lda	#$ff		; pause control
	sta	psg_voicectrl+6,X

.pause1:
	lda	psg_voicectrl,X
	beq	.skipwave

	lda	psg_wavenum,X
	and	#$80
	sta	psg_wavenum,X

.skipwave:
	lda	psg_trkctrl	; is anything playing on main track ?
	bpl	.nextvoice	; yes... so don't stop it

	stx	psg_ch_value
	stx	psg_ch
	stz	psg_ctrl

.nextvoice:
	inx
	cpx	#6
	bcs	.out
	tya
	bmi	.pause2
	
	lsr	A
	tay
	bra	.loop

.pause2:
	lsr	A
	ora	#$80
	tay
	bra	.loop

.out:	rts


; ----
; PSG_ASTOP
;
; Stop all voices
; ----

psg_astop:
	smb7	<psg_inhibit

	lda	#(PSG_MAINPAUSE | PSG_SUBPAUSE)
	sta	psg_trkctrl

	ldx	#5
.loop:	stz	psg_voicectrl,X		; disable main track
	stz	psg_voicectrl+6,X	; disable sub track
	stx	psg_ch
	stz	psg_ctrl		; disable voice
	dex
	bpl	.loop

	rts

; ----
; PSG_MVOFF
;
; Main volume off for voices specified in bitmask (in _al)
; ----

psg_mvoff:
	lda	<_al
	clx

.loop:	lsr	A
	bcc	.skip

	ldy	psg_voicectrl,X
	beq	.skip

	pha
	lda	#$ff		; turn off sound on voice
	sta	psg_voicectrl,X
	pla

	ldy	psg_voicectrl+6,X
	cpy	#1
	beq	.skip

	stx	psg_ch_value	; turn off voice
	stx	psg_ch
	stz	psg_ctrl

.skip:	inx
	cpx	#6
	bcc	.loop

	rts


; ----
; PSG_CONT
;
; "continue"
; ----
;
; parameter:
; 0 = main track
; 1 = sub track
; 2 = both
;
psg_cont:
	lda	<_al
	cmp	#1
	beq	.sub

	ldx	#5
.loop:	lda	psg_voicectrl,X
	beq	.next			; skip if disabled

	lda	#1			; restart voice
	sta	psg_voicectrl,X

.next:	dex
	bpl	.loop

	lda	#PSG_MAINPAUSE		; release pause on main tracks
	trb	psg_trkctrl

.sub:	lda	<_al
	beq	.end

	ldx	#5
.loop1:	lda	psg_voicectrl+6,X
	beq	.next1			; skip if disabled

	lda	#1			; restart voice
	sta	psg_voicectrl+6,X

	lda	psg_wavenum,X
	and	#$80			; force re-download of waveform
	sta	psg_wavenum,X

.next1:	dex
	bpl	.loop1

	lda	#PSG_SUBPAUSE		; release pause on sub tracks
	trb	psg_trkctrl

.end:	rmb7	<psg_inhibit		; remove pause on irq processing
	rts


; ----
; PSG_FDOUT
; ----

psg_fdout:
	lda	<_al
	bpl	.positive

	eor	#$ff		; get absolute value from negative
	inc	A

.positive:
	sta	psg_fadespeed	; store it as fade speed
	stz	psg_fadecount	; reset fade levels
	stz	psg_fadevolcut

	rts


; ----
; PSG_DCNT
;
;  set delay counter - sets an up-counter to use if interrupt
;  frequency is >60Hz, and this will act as a frequency divider
;  (by ignoring interrupts until up-counter value is hit)
;
;  Not used in this implementation of PSG driver
; ----

psg_dcnt:
	rts


; ----
; psg_drive - driver for MAIN tracks
; ----

psg_drive:
	lda	psg_song
	bmi	.skipsong

	jsr	psg_initsong

.skipsong:
	lda	#11
	sta	psg_currvoice

.loop:	ldx	psg_currvoice
	cpx	#6
	bcc	.chkmain

.chksub:	
	lda	psg_trkctrl
	and	#PSG_SUBPAUSE
	bne	.next
	bra	.dovoice

.chkmain:
	lda	psg_trkctrl
	bmi	.next

.dovoice:
	lda	psg_voicectrl,X
	beq	.next

	jsr	psg_drivevoice

.next:	dec	psg_currvoice
	bpl	.loop
	
; now loop is done...
; so now we look at general items

	jsr	psg_dofade
	jsr	psg_dodata2hdwr

	jsr	psg_mstat
	cmp	#$00
	bne	.mainok

	lda	#PSG_MAINPAUSE
	tsb	psg_trkctrl

.mainok:
	jsr	psg_sstat
	cmp	#$00
	bne	.subok

	lda	#PSG_SUBPAUSE
	tsb	psg_trkctrl

.subok:
	rts


; ----
; psg_initsong - initialize stuff for playing song
; ----

; ???? MAY NOT BE COMPLETE

psg_initsong:
	lda	psg_song

	asl	A		; make index for 16-bit data
	tay

      __ldw	psg_trkhdr

      __stw	<psg_tmpptr1
	lda	[psg_tmpptr1],Y
	sta	<psg_tmpptr
	iny
	lda	[psg_tmpptr1],Y
	sta	<psg_tmpptr+1

	lda	[psg_tmpptr]
	sta	<psg_tmpptr1
	sta	<psg_tmpptr1+1

	bmi	.subtrack	; starting voice = 0 (main), 6 (sub)
	clx
	bra	.processmask
.subtrack:
	ldx	#6

.processmask:
	cly
	lsr	psg_tmpptr1+1
	lbcc	.nextvoice

	lda	#1
	sta	psg_voicectrl,X
	sta	psg_cntdown,X
	stz	psg_strikephase,X
	stz	psg_transpose,X
	stz	psg_mode,X
	stz	psg_stkoff,X
	stz	psg_detune,X
	stz	psg_duratnmult,X
	stz	psg_fmcorrect,X
	stz	psg_fmdelay,X
	stz	psg_fmdelcntdn,X
	stz	psg_pedelay,X
	stz	psg_pedelcntdn,X
	stz	psg_volchg,X
	stz	psg_volchgaccum,X
	stz	psg_panpotchg,X
	stz	psg_panpotaccum,X

	lda	#4
	sta	psg_octave,X
	lda	#8
	sta	psg_keyonratio,X
	sta	psg_keycntdown,X
	lda	#$1f
	sta	psg_perchvol,X

	lda	psg_envtbl	; set to basic envelope
	sta	psg_envptr_l,X
	lda	psg_envtbl+1
	sta	psg_envptr_h,X
	stz	psg_envptr_off,X
	lda	#$84		; set default level to $8400
	stz	psg_envrels_l,X
	sta	psg_envrels_h,X

	iny
	lda	[psg_tmpptr],Y	; put voice address into correct voice
	sta	psg_trkptr_l,X
	sta	psg_savepoint_l,X
	iny
	lda	[psg_tmpptr],Y
	sta	psg_trkptr_h,X
	sta	psg_savepoint_h,X

.nextvoice:
	inx
	lda	<psg_tmpptr1
	bmi	.nextsub

.nextmain:
	cpx	#6
	lbcc	.processmask
	bra	.endloop
	
.nextsub:
	cpx	#12
	lbcc	.processmask

.endloop:
	stz	psg_fadespeed
	stz	psg_fadecount
	stz	psg_fadevolcut

	lda	psg_song
	ora	#$80
	sta	psg_song

	rts


; ----
; psg_dofade
; ----

; ???? MAY NOT BE COMPLETE

psg_dofade:
	rts


; ----
; psg_dodata2hdwr
; ----

; ???? MAY NOT BE COMPLETE

psg_dodata2hdwr:
	rts


; ----
; psg_drivevoice - driver for individual voice
; ----

psg_drivevoice:
	jsr	psg_dobytecodes	; process track bytecodes
	jsr	psg_doreladjust	; adjust relative volume/pan
	jsr	psg_dopercuss
	jsr	psg_doenvelope
	jsr	psg_dofreqstuff
	rts


; ----
; psg_dobytecodes
; ----

; ???? MAY NOT BE COMPLETE

psg_dobytecodes:
	dec	psg_cntdown,X	; is last note done yet ?
	beq	.next
	rts

.next:	lda	psg_trkptr_l,X	; get pointer
	sta	<psg_tmpptr
	lda	psg_trkptr_h,X
	sta	<psg_tmpptr

	lda	[psg_tmpptr]	; read next bytecode
	incw	<psg_tmpptr	; increment

	cmp	#$D0		; if <$d0, it's a tone
	bcc	psg_dotone

	sxy			; use Y as voice index now
	tax			; save byte
	
	sub	#$D0	; table starts with bytecode $D0
	asl	A	; 2 bytes per entry
	sax		; X = index; A = byte value

	bsr	.tablejump
	bsr	psg_trkptr_replace
	rts

.tablejump:
	jmp	[psg_bytefunctbl,X]


psg_dotone:

	bsr	psg_trkptr_replace
	rts

psg_trkptr_replace:
	ldx	psg_currvoice
	lda	<psg_tmpptr
	sta	psg_trkptr_l,X
	lda	<psg_tmpptr+1
	sta	psg_trkptr_h,X
	rts

psg_octavetbl:
	.db	0
	.db	(12*2*1)
	.db	(12*2*2)
	.db	(12*2*3)
	.db	(12*2*4)
	.db	(12*2*5)
	.db	(12*2*6)
	.db	(12*2*7)

psg_bytefunctbl:
	.dw	psgf_timebase	; d0
	.dw	psgf_octave	; d1
	.dw	psgf_octave	; d2
	.dw	psgf_octave	; d3
	.dw	psgf_octave	; d4
	.dw	psgf_octave	; d5
	.dw	psgf_octave	; d6
	.dw	psgf_octave	; d7
	.dw	psgf_octup	; d8
	.dw	psgf_octdown	; d9
	.dw	psgf_tie	; da
	.dw	psgf_tempo	; db
	.dw	psgf_volume	; dc
	.dw	psgf_panpot	; dd
	.dw	psgf_keyratio	; de
	.dw	psgf_relvolume	; df
	.dw	psgf_notused	; e0
	.dw	psgf_contfromsave	; e1
	.dw	psgf_savepoint	; e2
	.dw	psgf_rptbegin	; e3
	.dw	psgf_rptend	; e4
	.dw	psgf_wave	; e5
	.dw	psgf_envelope	; e6
	.dw	psgf_freqmod	; e7
	.dw	psgf_fmdelay	; e8
	.dw	psgf_fmcorrect	; e9
	.dw	psgf_pitchenv	; ea
	.dw	psgf_pedelay	; eb
	.dw	psgf_detune	; ec
	.dw	psgf_sweep	; ed
	.dw	psgf_sweeptime	; ee
	.dw	psgf_jump	; ef
	.dw	psgf_call	; f0
	.dw	psgf_return	; f1
	.dw	psgf_transpose	; f2
	.dw	psgf_reltransp	; f3
	.dw	psgf_fulltransp	; f4
	.dw	psgf_voladjust	; f5
	.dw	psgf_panrtadjust	; f6
	.dw	psgf_panlftadjust	; f7
	.dw	psgf_setmode	; f8
	.dw	psgf_notused	; f9
	.dw	psgf_notused	; fa
	.dw	psgf_notused	; fb
	.dw	psgf_notused	; fc
	.dw	psgf_notused	; fd
	.dw	psgf_fadeout	; fe
	.dw	psgf_dataend	; ff
	

psgf_inctmpptr:
	incw	<psg_tmpptr
psgf_notused:
	rts

;
; $d0
;
psgf_timebase:
	lda	[psg_tmpptr]
	and	#$0f
	sta	psg_duratnmult,Y
	jmp	psgf_inctmpptr

;
; $d1-7
;
psgf_octave:
	and	#$07
	sta	psg_octave,Y
	rts

;
; $d8
;
psgf_octup:
	ldx	psg_currvoice
	inc	psg_octave,X
	rts

;
; $d9
;
psgf_octdown:
	ldx	psg_currvoice
	dec	psg_octave,X
	rts

;
; $da
;
psgf_tie:
	lda	#3
	sta	psg_strikephase,Y
	rts

;
; $db
;
psgf_tempo:
	lda	[psg_tmpptr]
	sub	#$23
	bpl	.okval
	cla
.okval:	tax
	lda	psg_tempotbl,X
	sta	psg_tempo
	jmp	psgf_inctmpptr

;
; $dc
;
psgf_volume:
	lda	[psg_tmpptr]
	sta	psg_perchvol,Y
	cla
	sta	psg_volchg,Y
	sta	psg_volchgaccum,Y
	jmp	psgf_inctmpptr

;
; $dd
;
psgf_panpot:
	lda	[psg_tmpptr]
	sta	psg_panpot,Y
	cla
	sta	psg_panpotchg,Y
	sta	psg_panpotaccum,Y
	jmp	psgf_inctmpptr

;
; $de
;
psgf_keyratio:
	lda	#8
	sub	[psg_tmpptr]
	sta	psg_keyonratio,Y
	jmp	psgf_inctmpptr

;
; $df
;
psgf_relvolume:
	lda	psg_perchvol,Y
	add	[psg_tmpptr]
	and	#$1f
	sta	psg_perchvol,Y
	jmp	psgf_inctmpptr

;
; $e1
;
psgf_contfromsave:
	lda	psg_savepoint_l,Y
	sta	<psg_tmpptr
	lda	psg_savepoint_h,Y
	sta	<psg_tmpptr+1
	rts

;
; $e2
;
psgf_savepoint:
	lda	<psg_tmpptr
	sta	psg_savepoint_l,Y
	lda	<psg_tmpptr+1
	sta	psg_savepoint_h,Y
	rts

;
; $e3
;
psgf_rptbegin:
	rts

;
; $e4
;
psgf_rptend:
	rts

;
; $e5
;
psgf_wave:
	lda	[psg_tmpptr]
	sta	psg_wavenum,Y
	jmp	psgf_inctmpptr

;
; $e6
;
psgf_envelope:
	rts

;
; $e7
;
psgf_freqmod:
	rts

;
; $e8
;
psgf_fmdelay:
	rts

;
; $e9
;
psgf_fmcorrect:
	rts

;
; $ea
;
psgf_pitchenv:
	rts

;
; $eb
;
psgf_pedelay:
	rts

;
; $ec
;
psgf_detune:
	lda	[psg_tmpptr]
	sta	psg_detune,Y
	jmp	psgf_inctmpptr

;
; $ed
;
psgf_sweep:
	rts

;
; $ee
;
psgf_sweeptime:
	rts

;
; $ef
;
psgf_jump:
	lda	[psg_tmpptr]
	tax
	incw	<psg_tmpptr
	lda	[psg_tmpptr]
      __stw	<psg_tmpptr
	rts

;
; $f0
;
psgf_call:
	rts

;
; $f1
;
psgf_return:
	rts

;
; $f2
;
psgf_transpose:
	lda	[psg_tmpptr]
	asl	A
	sta	psg_transpose,Y
	jmp	psgf_inctmpptr

;
; $f3
;
psgf_reltransp:
	lda	[psg_tmpptr]
	asl	A
	ldx	psg_currvoice
	add	psg_transpose,X
	sta	psg_transpose,X
	jmp	psgf_inctmpptr

;
; $f4
;
psgf_fulltransp:
	rts

;
; $f5
;
psgf_voladjust:
	rts

;
; $f6
;
psgf_panrtadjust:
	rts

;
; $f7
;
psgf_panlftadjust:
	rts

;
; $f8
;
psgf_setmode:
	rts

;
; $fe
;
psgf_fadeout:
	lda	[psg_tmpptr]
	bpl	.positive
	eor	#$ff
	inc	A
.positive:
	sta	psg_fadespeed
	stz	psg_fadecount
	stz	psg_fadevolcut
	rts

;
; $ff
;
psgf_dataend:
	lda	#2
	sta	psg_voicectrl
	rts


; ----
; PSG_DORELADJUST
; ----

; NOT IMPLEMENTED YET

psg_doreladjust:
	rts


; ----
; PSG_DOPERCUSS
; ----

; NOT IMPLEMENTED YET

psg_dopercuss:
	rts


; ----
; PSG_DOENVELOPE
; ----

; NOT IMPLEMENTED YET

psg_doenvelope:
	rts


; ----
; PSG_DOFREQSTUFF
; ----

; NOT IMPLEMENTED YET

psg_dofreqstuff:
	rts


;
;;
;; load a waveform into a PSG channel
;; PSG channel must already be set
;;
;snd_loadwave:	lda	#0
;		sta	psg_ctrl
;
;		ldx	#$20
;.l1:		lda	[wavptr]
;		sta	psg_wave
;		incw	<wavptr
;		dex
;		bne	.l1
;
;		lda	#PSG_ENABLE
;		ora	#$1F		; full volume
;		sta	psg_ctrl
;		lda	#$ff
;		sta	psg_pan
;		rts
;		


; careful with these wave data (etc.)

;
; wave #0
;
psg_waveforms:
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f

;
; wave #1
;
	.db $10,$0d,$0a,$07
	.db $05,$03,$02,$01
	.db $00,$01,$02,$03
	.db $05,$07,$09,$0d
	.db $0f,$12,$15,$17
	.db $19,$1b,$1d,$1e
	.db $1e,$1e,$1d,$1b
	.db $19,$17,$15,$13

;
; wave #2
;
	.db $10,$11,$12,$13
	.db $14,$15,$16,$17
	.db $18,$19,$1a,$1b
	.db $1c,$1d,$1e,$1f
	.db $00,$01,$02,$03
	.db $04,$05,$06,$07
	.db $08,$09,$0a,$0b
	.db $0c,$0d,$0e,$0f

;
; wave #3
;
	.db $05,$05,$09,$09
	.db $02,$16,$16,$10
	.db $10,$1f,$1f,$0e
	.db $0e,$04,$04,$12
	.db $19,$19,$09,$09
	.db $02,$02,$13,$13
	.db $07,$07,$1e,$1e
	.db $0b,$0b,$15,$15

;
; wave #4
;
	.db $0f,$0b,$0f,$11
	.db $0f,$0d,$0f,$10
	.db $10,$10,$10,$07
	.db $07,$07,$10,$07
	.db $01,$07,$10,$17
	.db $1e,$17,$10,$07
	.db $01,$07,$10,$17
	.db $0f,$07,$10,$13

;
; wave #5
;
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $00,$00,$00,$1f
	.db $1e,$1c,$1a,$18
	.db $16,$14,$12,$10
	.db $0e,$0c,$0a,$08
	.db $06,$04,$02,$01

;
; wave #6
;
	.db $10,$06,$03,$02
	.db $01,$00,$00,$00
	.db $00,$00,$00,$01
	.db $01,$02,$03,$06
	.db $10,$19,$1c,$1d
	.db $1e,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1e,$1d,$1c,$19

; OK to here -----------------
;
; wave #7
;
	.db $1d,$0f,$09,$05
	.db $03,$01,$03,$05
	.db $09,$0f,$15,$19
	.db $1b,$1d,$1b,$19
	.db $15,$0f,$07,$03
	.db $01,$03,$07,$0f
	.db $17,$1b,$1d,$1b
	.db $17,$0f,$01,$0f

;
; wave #8
;
	.db $10,$13,$16,$18
	.db $1b,$1d,$1e,$1f
	.db $1f,$1f,$1e,$1d
	.db $1b,$18,$16,$13
	.db $10,$0c,$09,$07
	.db $04,$02,$01,$01
	.db $01,$01,$01,$02
	.db $04,$07,$09,$0c

;
; wave #9
;
	.db $1b,$1d,$1b,$17
	.db $0f,$0f,$15,$19
	.db $19,$15,$0f,$0f
	.db $09,$05,$05,$09
	.db $0f,$0f,$0d,$07
	.db $03,$01,$03,$09
	.db $11,$13,$13,$0f
	.db $0b,$0b,$0d,$17

;
; wave #$0a
;
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $01,$01,$01,$01
	.db $01,$01,$01,$01

;
; wave #$0b
;
	.db $1f,$00,$04,$06
	.db $08,$0a,$0c,$0e
	.db $10,$12,$14,$16
	.db $18,$1a,$1c,$1e
	.db $1f,$1e,$1c,$1a
	.db $18,$16,$14,$12
	.db $10,$0e,$0c,$0a
	.db $08,$06,$04,$02

;
; wave #$0c
;
	.db $12,$10,$1a,$1f
	.db $18,$0d,$14,$1c
	.db $18,$12,$18,$1c
	.db $10,$01,$05,$0b
	.db $05,$01,$09,$10
	.db $05,$00,$03,$0d
	.db $09,$07,$10,$1a
	.db $0d,$03,$10,$14


;
; wave #$0d
;
	.db $0f,$15,$17,$18
	.db $1a,$1d,$1e,$10
	.db $1f,$1f,$1d,$1c
	.db $1a,$18,$14,$10
	.db $0f,$0f,$0b,$07
	.db $05,$03,$02,$10
	.db $00,$01,$01,$02
	.db $05,$07,$08,$10

;
; wave #$0e
;
	.db $1f,$1e,$1b,$17
	.db $13,$0f,$0b,$07
	.db $03,$0f,$0c,$09
	.db $07,$04,$02,$01
	.db $00,$00,$01,$02
	.db $04,$07,$09,$0c
	.db $1f,$1b,$17,$13
	.db $0f,$0b,$07,$03

;
; wave #$0f
;
	.db $18,$1f,$1d,$1d
	.db $1f,$1c,$14,$0f
	.db $12,$1c,$1e,$12
	.db $05,$02,$03,$0a
	.db $15,$1c,$1d,$1a
	.db $0d,$01,$03,$0d
	.db $10,$0b,$03,$00
	.db $02,$02,$00,$07

;
; wave #$10
;
	.db $1f,$00,$1f,$00
	.db $1f,$00,$1f,$00
	.db $1f,$00,$1f,$00
	.db $1f,$00,$1f,$00
	.db $1f,$00,$1f,$00
	.db $1f,$00,$1f,$00
	.db $1f,$00,$1f,$00
	.db $1f,$00,$1f,$00

;
; wave #$11
;
	.db $0d,$0f,$0b,$00
	.db $10,$1e,$1d,$1f
	.db $13,$10,$0f,$04
	.db $0e,$0d,$10,$01
	.db $00,$02,$02,$0e
	.db $01,$0d,$0f,$10
	.db $0e,$01,$01,$03
	.db $0e,$01,$01,$00

;
; wave #$12
;
	.db $10,$1a,$1f,$1c
	.db $14,$10,$11,$14
	.db $16,$14,$11,$10
	.db $14,$1c,$1f,$1a
	.db $10,$05,$01,$03
	.db $0b,$0f,$0e,$0b
	.db $09,$0b,$0e,$0f
	.db $0b,$03,$01,$05

;
; wave #$13
;
	.db $16,$1e,$1e,$17
	.db $0d,$06,$01,$00
	.db $00,$00,$00,$00
	.db $01,$03,$07,$0c
	.db $13,$18,$1c,$1e
	.db $1f,$1f,$1f,$1f
	.db $1f,$1e,$19,$12
	.db $08,$01,$01,$09

;
; wave #$14
;
	.db $14,$1a,$1e,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1f,$1e,$1a,$14
	.db $0b,$05,$01,$00
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $00,$01,$05,$0b

;
; wave #$15
;
	.db $17,$1e,$1d,$17
	.db $10,$16,$1f,$1c
	.db $1a,$1e,$1f,$1f
	.db $1c,$0e,$07,$0a
	.db $14,$18,$11,$03
	.db $00,$00,$01,$05
	.db $03,$00,$09,$0f
	.db $08,$02,$01,$08

;
; wave #$16
;
	.db $18,$1e,$19,$0f
	.db $12,$1e,$1d,$1e
	.db $1e,$1d,$1e,$12
	.db $0f,$19,$1e,$18
	.db $07,$01,$06,$10
	.db $0d,$01,$02,$01
	.db $01,$02,$01,$0d
	.db $10,$06,$01,$07

;
; wave #$17
;
	.db $10,$16,$1a,$1c
	.db $1e,$1c,$1a,$16
	.db $10,$0a,$06,$04
	.db $02,$04,$06,$0a
	.db $10,$18,$1c,$1e
	.db $1c,$18,$10,$08
	.db $02,$08,$10,$1e
	.db $10,$02,$10,$02

;
; wave #$18
;
	.db $10,$13,$16,$18
	.db $1b,$1d,$1e,$1f
	.db $1f,$1f,$1e,$1d
	.db $1b,$18,$16,$13
	.db $10,$01,$04,$08
	.db $0c,$10,$14,$18
	.db $1c,$01,$04,$08
	.db $0c,$10,$14,$18

;
; wave #$19
;
	.db $1a,$1c,$1d,$1e
	.db $1f,$1f,$1f,$1f
	.db $1f,$1f,$1f,$1f
	.db $1e,$1d,$1b,$19
	.db $06,$04,$02,$01
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $01,$02,$04,$06

;
; wave #$1a
;
	.db $1b,$14,$10,$0e
	.db $0c,$0b,$16,$09
	.db $02,$07,$0a,$05
	.db $03,$03,$02,$01
	.db $0b,$15,$1c,$1e
	.db $1a,$0c,$15,$19
	.db $0c,$18,$07,$12
	.db $0c,$09,$16,$1a

;
; wave #$1b
;
	.db $0f,$15,$1a,$1c
	.db $1f,$1c,$1a,$15
	.db $0f,$08,$04,$02
	.db $00,$02,$04,$08
	.db $0f,$17,$1c,$1f
	.db $1c,$17,$0f,$06
	.db $01,$02,$07,$0f
	.db $0f,$13,$0f,$00

;
; wave #$1c
;
	.db $1f,$1b,$19,$1c
	.db $1f,$1c,$1a,$17
	.db $0c,$04,$03,$02
	.db $08,$0e,$0d,$04
	.db $00,$06,$09,$05
	.db $00,$00,$00,$01
	.db $0a,$10,$11,$12
	.db $0e,$0c,$0e,$19

;
; wave #$1d
;
	.db $0f,$02,$11,$1f
	.db $0f,$1c,$0d,$00
	.db $0f,$02,$11,$1f
	.db $0f,$1c,$0d,$00
	.db $0f,$02,$11,$1f
	.db $0f,$1c,$0d,$00
	.db $0f,$02,$11,$1f
	.db $0f,$1c,$0d,$00

;
; wave #$1e
;
	.db $10,$0a,$14,$1c
	.db $15,$1d,$16,$10
	.db $18,$11,$18,$1f
	.db $15,$1b,$12,$0a
	.db $10,$08,$0e,$14
	.db $0a,$10,$07,$00
	.db $08,$01,$09,$12
	.db $0a,$12,$0b,$06

;
; wave #$1f
;
	.db $12,$19,$1d,$1e
	.db $1f,$1f,$1f,$1e
	.db $1c,$17,$12,$0c
	.db $0d,$12,$16,$11
	.db $08,$04,$01,$00
	.db $00,$00,$00,$00
	.db $00,$00,$00,$00
	.db $00,$01,$04,$0c

;
; wave #$20
;
	.db $04,$14,$1e,$17
	.db $00,$17,$1e,$16
	.db $10,$1a,$1f,$1c
	.db $12,$16,$18,$10
	.db $06,$12,$1c,$10
	.db $0c,$0e,$10,$06
	.db $02,$08,$12,$0c
	.db $04,$08,$0e,$08

;
; wave #$21
;
	.db $0a,$0c,$0e,$0f
	.db $1f,$1f,$1f,$1f
	.db $14,$14,$13,$12
	.db $1b,$1b,$1b,$1b
	.db $0a,$08,$06,$04
	.db $0e,$0e,$0e,$0e
	.db $00,$00,$00,$02
	.db $13,$13,$13,$13

;
; wave #$22
;
	.db $1b,$1b,$05,$05
	.db $09,$09,$1f,$1f
	.db $1b,$1b,$16,$16
	.db $1a,$1a,$07,$07
	.db $17,$17,$15,$15
	.db $19,$19,$0c,$0c
	.db $1b,$1b,$0c,$0c
	.db $00,$00,$03,$03

;
; wave #$23
;
	.db $1f,$1d,$1b,$19
	.db $17,$15,$13,$11
	.db $0f,$0d,$0b,$09
	.db $07,$05,$03,$01
	.db $00,$0f,$11,$0d
	.db $13,$0b,$15,$09
	.db $17,$07,$19,$05
	.db $1b,$03,$1d,$01

;
; wave #$24
;
	.db $0b,$0e,$11,$13
	.db $1f,$1d,$1a,$17
	.db $09,$05,$02,$00
	.db $0a,$0b,$0e,$11
	.db $0b,$0e,$1b,$1e
	.db $14,$12,$1a,$17
	.db $09,$05,$0d,$0a
	.db $00,$01,$0e,$11

;
; wave #$25
;
	.db $0f,$16,$1b,$1e
	.db $1f,$1e,$1b,$16
	.db $0f,$08,$04,$01
	.db $00,$01,$04,$08
	.db $0f,$00,$00,$00
	.db $1f,$1f,$1f,$1f
	.db $1c,$18,$14,$10
	.db $0c,$07,$03,$00

;
; wave #$26
;
	.db $02,$01,$05,$0b
	.db $11,$17,$1b,$1f
	.db $1e,$1b,$17,$13
	.db $0f,$0b,$09,$0e
	.db $18,$1d,$1c,$17
	.db $0f,$0a,$07,$06
	.db $11,$19,$1a,$19
	.db $10,$0b,$06,$03

;
; wave #$27
;
	.db $1d,$1d,$1d,$10
	.db $10,$10,$1d,$1d
	.db $1d,$1d,$1b,$1b
	.db $1b,$1b,$1b,$1b
	.db $00,$00,$00,$00
	.db $00,$00,$02,$02
	.db $02,$02,$10,$10
	.db $10,$02,$02,$02

;
; wave #$28
;
	.db $19,$16,$10,$12
	.db $12,$09,$16,$1f
	.db $0f,$17,$0f,$10
	.db $1d,$13,$11,$10
	.db $0e,$19,$1b,$00
	.db $0f,$03,$0c,$12
	.db $17,$0f,$16,$11
	.db $12,$05,$0d,$06

;
; wave #$29
;
	.db $00,$07,$0b,$0b
	.db $09,$03,$00,$01
	.db $17,$17,$15,$10
	.db $0c,$0a,$0d,$13
	.db $07,$0e,$12,$11
	.db $0f,$09,$07,$07
	.db $1f,$1e,$1d,$17
	.db $13,$11,$13,$1a

;
; wave #$2a
;
	.db $19,$04,$03,$03
	.db $02,$02,$01,$01
	.db $03,$03,$01,$01
	.db $01,$01,$00,$01
	.db $00,$01,$19,$19
	.db $1b,$1b,$1b,$1b
	.db $19,$1e,$1e,$1e
	.db $0d,$1e,$19,$04

;
; wave #$2b
;
	.db $0f,$15,$19,$1b
	.db $1d,$1e,$1f,$1e
	.db $1d,$1b,$19,$15
	.db $0f,$07,$17,$0a
	.db $14,$0d,$12,$0f
	.db $10,$0a,$06,$04
	.db $02,$01,$00,$01
	.db $02,$04,$06,$0a

;
; wave #$2c
;
	.db $09,$18,$1d,$0b
	.db $00,$06,$13,$1c
	.db $18,$10,$06,$02
	.db $00,$00,$00,$00
	.db $02,$08,$16,$1b
	.db $09,$03,$10,$1e
	.db $1e,$1b,$16,$0b
	.db $05,$01,$00,$00


;
; frequency data
;
psg_freqtbl:
	.dw $0fe4	; A0
	.dw $0f00	; A0#
	.dw $0e28	; B0
	.dw $0d5d	; C1 (octave starts at 'C')
	.dw $0c9d	; C1#
	.dw $0be7	; D1
	.dw $0b3d	; D1#
	.dw $0a9b	; E1
	.dw $0a03	; F1
	.dw $0973	; F1#
	.dw $08eb	; G1
	.dw $086a	; G1#
	.dw $07f2	; A1
	.dw $0780	; A1#
	.dw $0714	; B1
	.dw $06af	; C2
	.dw $064f
	.dw $05f4
	.dw $059e
	.dw $054e
	.dw $0502
	.dw $04b9
	.dw $0476
	.dw $0435
	.dw $03f9
	.dw $03c0
	.dw $038a
	.dw $0357	; C3
	.dw $0327
	.dw $02fa
	.dw $02cf
	.dw $02a7
	.dw $0281
	.dw $025d
	.dw $023b
	.dw $021b
	.dw $01fc
	.dw $01e0
	.dw $01c5
	.dw $01ac	; C4
	.dw $0193
	.dw $017d
	.dw $0168
	.dw $0153
	.dw $0140
	.dw $012e
	.dw $011d
	.dw $010d
	.dw $00fe
	.dw $00f0
	.dw $00e3
	.dw $00d6	; C5
	.dw $00ca
	.dw $00be
	.dw $00b4
	.dw $00aa
	.dw $00a0
	.dw $0097
	.dw $008f
	.dw $0087
	.dw $007f
	.dw $0078
	.dw $0071
	.dw $006b	; C6
	.dw $0065
	.dw $005f
	.dw $005a
	.dw $0055
	.dw $0050
	.dw $004b
	.dw $0046
	.dw $0043
	.dw $0040
	.dw $003c
	.dw $0039
	.dw $0035	; C7
	.dw $0032
	.dw $0030
	.dw $002d
	.dw $002a
	.dw $0028
	.dw $0026
	.dw $0024
	.dw $0022
	.dw $0020
	.dw $001e
	.dw $001c
	.dw $001b	; C8


;
; TEMPO DATA
;
; starting with value $23 (as sent to PSG_TEMPO),
; these numbers are the timer values to use for
; that tempo
;
psg_tempotbl:
	.db $7d,$79,$76,$73
	.db $70,$6d,$6b,$68
	.db $65,$63,$61,$5f
	.db $5d,$5b,$59,$57
	.db $56,$54,$52,$51
	.db $4f,$4e,$4d,$4b
	.db $4a,$49,$48,$46
	.db $45,$44,$43,$42
	.db $41,$40,$3f,$3e
	.db $3e,$3d,$3c,$3b
	.db $3a,$3a,$39,$38
	.db $37,$37,$36,$35
	.db $35,$34,$33,$33
	.db $32,$32,$31,$31
	.db $30,$30,$2f,$2f
	.db $2e,$2e,$2d,$2d
	.db $2c,$2c,$2b,$2b
	.db $2a,$2a,$2a,$29
	.db $29,$28,$28,$28
	.db $27,$27,$27,$26
	.db $26,$26,$25,$25
	.db $25,$24,$24,$24
	.db $24,$23,$23,$23
	.db $22,$22,$22,$22
	.db $22,$21,$21,$21
	.db $20,$20,$20,$20
	.db $1f,$1f,$1f,$1f
	.db $1f,$1e,$1e,$1e
	.db $1e,$1e,$1d,$1d
	.db $1d,$1d,$1d,$1c
	.db $1c,$1c,$1c,$1c
	.db $1b,$1b,$1b,$1b
	.db $1b,$1b,$1a,$1a
	.db $1a,$1a,$1a,$1a
	.db $1a,$19,$19,$19
	.db $19,$19,$19,$19
	.db $18,$18,$18,$18
	.db $18,$18,$18,$17
	.db $17,$17,$17,$17
	.db $17,$17,$17,$17
	.db $16,$16,$16,$16
	.db $16,$16,$16,$16
	.db $16,$15,$15,$15
	.db $15,$15,$15,$15
	.db $15,$15,$15,$14
	.db $14,$14,$14,$14
	.db $14,$14,$14,$14
	.db $14,$14,$13,$13
	.db $13,$13,$13,$13
	.db $13,$13,$13,$13
	.db $13,$13,$12,$12
	.db $12,$12,$12,$12
	.db $12,$12,$12,$12
	.db $12,$12,$12,$11
	.db $11,$11,$11,$11
	.db $11 


;
; Envelope Data
; 


;
; envelope #0
;
psg_env0:
	ENV_RLS	$8410
	ENV_LEV	$7c40
	ENV_DLY $00,$0000
	ENV_END

;
; envelope #1
;
psg_env1:
	ENV_RLS	$fa05
	ENV_LEV $7c40
	ENV_DLY $0c,$fe01
	ENV_END

;
; envelope #2
;
psg_env2:
	ENV_RLS	$fd05
	ENV_LEV $7c40
	ENV_DLY $0c,$fe01
	ENV_END

;
; envelope #3
;
psg_env3:
	ENV_RLS	$ff01
	ENV_LEV $7c40
	ENV_DLY $0c,$fe01
	ENV_END

;
; envelope #4
;
psg_env4:
	ENV_RLS	$fa05
	ENV_LEV $7c40
	ENV_DLY $00,$ff81
	ENV_END

;
; envelope #5
;
psg_env5:
	ENV_RLS	$fa05
	ENV_LEV $7c40
	ENV_DLY $00,$ff01
	ENV_END

;
; envelope #6
;
psg_env6:
	ENV_RLS	$fa05
	ENV_LEV $7c40
	ENV_DLY $00,$fe01
	ENV_END

;
; envelope #7
;
psg_env7:
	ENV_RLS	$fb05
	ENV_LEV $2c40
	ENV_DLY $02,$2801
	ENV_DLY $00,$ff81
	ENV_END

;
; envelope #8
;
psg_env8:
	ENV_RLS	$fb05
	ENV_LEV $3440
	ENV_DLY $03,$1801
	ENV_DLY $00,$ff81
	ENV_END

;
; envelope #9
;
psg_env9:
	ENV_RLS	$fb05
	ENV_LEV $1c40
	ENV_DLY $01,$6001
	ENV_DLY $02,$fe01
	ENV_DLY $08,$fc01
	ENV_DLY $32,$0081
	ENV_END

;
; envelope #$0a
;
psg_enva:
	ENV_RLS	$fd01
	ENV_LEV $0440
	ENV_DLY $03,$2801
	ENV_DLY $04,$fe01
	ENV_END

;
; envelope #$0b
;
psg_envb:
	ENV_RLS	$fd01
	ENV_LEV $0000
	ENV_LEV $0000
	ENV_LEV $7c40
	ENV_DLY $00,$ffc1
	ENV_END

;
; envelope #$0c
;
psg_envc:
	ENV_RLS	$fa05
	ENV_LEV $0000
	ENV_LEV $0000
	ENV_LEV $6840
	ENV_DLY $00,$0061
	ENV_END

;
; envelope #$0d
;
psg_envd:
	ENV_RLS	$fa05
	ENV_LEV $7c40
	ENV_DLY $04,$f801
	ENV_DLY $00,$ff41
	ENV_END

;
; envelope #$0e
;
psg_enve:
	ENV_RLS	$fa05
	ENV_LEV $7c40
	ENV_DLY $08,$f801
	ENV_DLY $00,$ff41
	ENV_END

;
; envelope #$0f
;
psg_envf:
	ENV_RLS	$ff00
	ENV_LEV $7c40
	ENV_DLY $05,$fc01
	ENV_LEV $7040
	ENV_DLY $05,$fc01
	ENV_LEV $6440
	ENV_DLY $05,$fc01
	ENV_LEV $5840
	ENV_DLY $05,$fc01
	ENV_LEV $4c40
	ENV_DLY $05,$fc01
	ENV_LEV $4040
	ENV_DLY $05,$fc01
	ENV_DLY $00,$ff01
	ENV_END


; 
; Envelope Header (pointers to data)
;
psg_envtbl:
	.dw psg_env0
	.dw psg_env1
	.dw psg_env2
	.dw psg_env3
	.dw psg_env4
	.dw psg_env5
	.dw psg_env6
	.dw psg_env7
	.dw psg_env8
	.dw psg_env9
	.dw psg_enva
	.dw psg_envb
	.dw psg_envc
	.dw psg_envd
	.dw psg_enve
	.dw psg_envf


;
; now, return context to bank established in
; file which included this
;
	.bank	START_BANK

