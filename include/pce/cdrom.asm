;
;  PCE CDROM access routines' sources
;

	.include "cdrom.inc"

	.bss

cdplay_end_ctl	.ds  1 ; saved 'cd_play end' information - type
cdplay_end_h	.ds  1 ; high byte of address
cdplay_end_m	.ds  1 ; mid byte
cdplay_end_l	.ds  1 ; low byte
cd_buf          .ds  4 ; Return buffer from some BIOS commands

	.code

;
; cd_reset(void)
; ----
; Reset CDROM
; ----
;
_cd_reset:	jsr	cd_reset
		rts

;
; cd_pause(void)
; ----
; Pause CDROM drive
; ----
;
_cd_pause:	jsr	cd_pause
		tax
		cla
		rts


;
; cd_unpause(void)
; ----
; Continue playing CDROM audio after pause
; ----
;
 .ifndef _LIB3
_cd_unpause:	lda	cdplay_end_ctl
 .else ; _LIB3
_cd_unpause:
		maplibfunc	lib3_cd_unpause
		rts

	.bank LIB3_BANK
; cd_unpause(void)
; ----
lib3_cd_unpause:	lda	cdplay_end_ctl
 .endif ; _LIB3
		sta	<_dh
		lda	cdplay_end_h
		sta	<_cl
		lda	cdplay_end_m
		sta	<_ch
		lda	cdplay_end_l
		sta	<_dl
		lda	#CD_CURRPOS
		sta	<_bh
		jsr	cd_play
		tax
		cla
		rts
 .ifdef _LIB3
	.bank LIB1_BANK
 .endif ; _LIB3

;
; cd_fade(char type)
;
;   type = $00 -> cancel fade
;          $08 -> PCM fadeout 6 seconds
;          $0A -> ADPCM fadeout 6 seconds
;          $0C -> PCM fadeout 2.5 seconds
;          $0E -> ADPCM fadeout 2.5 seconds
; ----
; Fade-out PCM/ADPCM audio
; ----
;
_cd_fade:	txa
		jsr	cd_fade
		tax
		cla
		rts


;
; cd_playtrk(int start_track [bx], int end_track [cx], int mode [acc])
;   mode = CDPLAY_MUTE / CDPLAY_REPEAT / CDPLAY_NORMAL
; ----
; Play CDROM audio track
; ----
;
 .ifndef _LIB3
_cd_playtrk.3:	txa
 .else ; _LIB3
_cd_playtrk.3:
		maplibfunc lib3_cd_playtrk.3
		rts

	.bank LIB3_BANK
; cd_playtrk
; ----
lib3_cd_playtrk.3:	txa
 .endif ; _LIB3
		and	#CD_PLAYMODE
		ora	#CD_TRACK
		sta	<_dh		; end type + play mode
		sta	cdplay_end_ctl
		lda	#CD_TRACK
		sta	<_bh		; start type
		
	        lda	<_cl
		bne	.endtrk

.endofdisc:	lda	<_dh		; repeat to end of disc
		ora	#CD_LEADOUT
		sta	<_dh
		sta	cdplay_end_ctl
		bra	.starttrk

.endtrk:	jsr	ex_binbcd
		sta	<_cl		; end track
		sta	cdplay_end_h
		stz	<_ch
		stz	cdplay_end_m
		stz	<_dl
		stz	cdplay_end_l

.starttrk:	lda	<_bl		; track #
		jsr	ex_binbcd
		sta	<_al		; from track
		stz	<_ah
		stz	<_bl
		jsr	cd_play
		tax
		cla
		rts

 .ifdef _LIB3
	.bank LIB1_BANK

 .endif ; _LIB3

;
; cd_playmsf(int start_minute [al], int start_second [ah], int start_frame [bl],
;            int end_minute [cl], int end_second [ch], int end_frame [dl], int mode [acc])
;   mode = CDPLAY_MUTE / CDPLAY_REPEAT / CDPLAY_NORMAL
; ----
; Play CDROM from/to 'minute/second/frame'
; ----
;
 .ifndef _LIB3
_cd_playmsf.7:	txa
 .else ; _LIB3
_cd_playmsf.7:
		maplibfunc	lib3_cd_playmsf.7
		rts
		
	.bank LIB3_BANK
; cd_playmsf	
; ----
lib3_cd_playmsf.7:
		txa
 .endif ; _LIB3
		and	#CD_PLAYMODE
		ora	#CD_MSF
		sta	<_dh		; end type + play mode
		sta	cdplay_end_ctl
		lda	#CD_MSF
		sta	<_bh		; start type
		
.endmsf:	lda	<_dl		; end frame
		jsr	ex_binbcd
		sta	<_dl
		sta	cdplay_end_l

	        lda	<_ch		; end second
		jsr	ex_binbcd
		sta	<_ch
		sta	cdplay_end_m

	        lda	<_cl		; end minute
		jsr	ex_binbcd
		sta	<_cl
		sta	cdplay_end_h

.startmsf:
	        lda	<_bl		; start frame
		jsr	ex_binbcd
		sta	<_bl

	        lda	<_ah		; start second
		jsr	ex_binbcd
		sta	<_ah

	        lda	<_al		; start minute
		jsr	ex_binbcd
		sta	<_al

		jsr	cd_play
		tax
		cla
		rts
 .ifdef _LIB3
	.bank LIB1_BANK


 .endif ; _LIB3

;
; char cd_numtrk(void)
; ----
; return number of tracks on CDROM
; ----
;
_cd_numtrk:	stw	#cd_buf,<_bx
		stz	<_al		; request type 0
		jsr	cd_dinfo
		cmp	#$00
		bne	.err
		lda	cd_buf+1
		jsr	ex_bcdbin
		clx
		sax
		rts
.err:		lda	#$ff
		tax
		rts

;
; char cd_trkinfo(char track [ax], char *min [cx], char *sec [dx], char *frm [bp])
; ----
; Return information about track
; ----
;
_cd_trkinfo.4:
 .ifdef _LIB3
		maplibfunc	lib3_cd_trkinfo.4
		rts
		
	.bank LIB3_BANK
; char cd_trkinfo(char track [ax], char *min [cx], char *sec [dx], char *frm [bp])
; ----
lib3_cd_trkinfo.4:

 .endif ; _LIB3
	      __ldw	<_ax
		jsr	_cd_trktype
		phx
		lda	cd_buf
		jsr	ex_bcdbin
		sta	[_cx]

		lda	cd_buf+1
		jsr	ex_bcdbin
		sta	[_dx]

		lda	cd_buf+2
		jsr	ex_bcdbin
		sta	[_bp]

		plx
		rts
 .ifdef _LIB3
	.bank LIB1_BANK
 .endif ; _LIB3

;
; char cd_trktype(char track)
; ----
; Return type of track (data/audio)
; ----
;
 .ifndef _LIB3
_cd_trktype:	sax
 .else ; _LIB3
_cd_trktype:
		maplibfunc	lib3_cd_trktype
		rts
		
	.bank LIB3_BANK
; char cd_trktype(char track)
; ----
lib3_cd_trktype:	sax
 .endif ; _LIB3
		jsr	ex_binbcd
		sta	<_ah		; track #
		stw	#cd_buf,<_bx
		cmp	#0
		beq	.discnottrk
		lda	#2
		bra	.go
.discnottrk	lda	#1
.go:		sta	<_al		; request type 2
		jsr	cd_dinfo
		cmp	#$00
		bne	.err
		ldx	cd_buf+3	; track type
		cla
		rts
.err:		lda	#$ff
		tax
		rts
 .ifdef _LIB3
	.bank LIB1_BANK
 .endif ; _LIB3

;
; char cd_execoverlay(int ovl_index)
; ----
; Execute program overlay from disc
; ----
;
_cd_execoverlay:
 .ifdef _LIB3
		maplibfunc	lib3_cd_execoverlay
		rts
		
	.bank LIB3_BANK		
; char cd_execoverlay
; ----
lib3_cd_execoverlay:
 .endif ; _LIB3
		jsr	cd_overlay
		cmp	#0
		bne	.error
		jmp	$C000		; loaded fine... now run it
.error:		pha
		ldx	ovl_running	; failed; reload old segment for error recovery
		jsr	cd_overlay
		plx			; return error
		cla
		rts

cd_overlay:
		jsr	prep_rdsect
		stz	<_cl	; sector (offset from base of track)
		sta	<_ch
		stx	<_dl
		iny
		lda	ovlarray,Y
		sta	<_al	; # sectors
		tma	#6
		sta	<_bl	; Bank #
		lda	#3
		sta	<_dh	; MPR #
		jsr	cd_read
		rts
 .ifdef _LIB3
	.bank LIB1_BANK		

 .endif ; _LIB3

;
; char cd_loadvram(int ovl_index [di], int sect_offset [si], int vramaddr [bx], int bytes [acc])
; ----
; Load CDROM data directly into VRAM
; ----
;
_cd_loadvram.4:
 .ifdef _LIB3
		maplibfunc	lib3_cd_loadvram.4
		rts
		
	.bank LIB3_BANK
; char cd_loadvram(int ovl_index [di], int sect_offset [si], int vramaddr [bx], int bytes [acc])
; ----
lib3_cd_loadvram.4:
 .endif ; _LIB3
	      __stw	<_ax
	      __ldw	<_di
		jsr	prep_rdsect
	      __addw	<_si
		stz	<_cl
		sta	<_ch
		stx	<_dl
		lda	#$FE
		sta	<_dh
		jsr	cd_read
		tax
		cla
		rts
 .ifdef _LIB3
	.bank LIB1_BANK


 .endif ; _LIB3
;
; char cd_loaddata(int ovl_index [di], int sect_offset [si], farptr array [bl:bp], int bytes [acc])
; ----
; Load CDROM data directly into data area, replacing predefined data
; ----
;
	.bss
cdtemp_l	.ds	1
cdtemp_m	.ds	1
cdtemp_bank	.ds	1
cdtemp_savbnk60	.ds	1
cdtemp_savbnk80	.ds	1
cdtemp_addr	.ds	2
cdtemp_bytes	.ds	2
	.code

_cd_loaddata.4:
 .ifdef _LIB3
		maplibfunc	lib3_cd_loaddata.4
		rts
		
	.bank LIB3_BANK
; char cd_loaddata(int ovl_index [di], int sect_offset [si], farptr array [bl:bp], int bytes [acc])
; ----
lib3_cd_loaddata.4:
 .endif ; _LIB3
	      __stw	cdtemp_bytes
	      __ldw	<_di
		jsr	prep_rdsect
	      __addw	<_si
		stx	cdtemp_l	; calculate sector adddress
		sta	cdtemp_m

		tma	#3		; save entry banks
		sta	cdtemp_savbnk60
		tma	#4
		sta	cdtemp_savbnk80

		lda	<_bl
		sta	cdtemp_bank	; load addr (bank/address)
	      __ldw	<_bp
		and	#$1f		; correct to a $6000-relative addr.
		ora	#$60
	      __stw	cdtemp_addr

.loop:
		lda	cdtemp_bank	; get 2 adjacent banks just in case
		tam	#3
		inc	A
		tam	#4

	      __ldw	cdtemp_addr	; load address
	      __stw	<_bx

		stz	<_cl		; sector address
		lda	cdtemp_m
		sta	<_ch
		lda	cdtemp_l
		sta	<_dl
		stz	<_dh		; address type (local, # bytes)

	      __ldw	cdtemp_bytes
		sub	#$20
		bmi	.less2000
	      __stw	cdtemp_bytes
	      __ldwi	$2000
		bra	.read

.less2000:	add	#$20
		stwz	cdtemp_bytes
.read:	      __stw	<_ax
	
		jsr	cd_read
		cmp	#0
		bne	.error

	      __tstw	cdtemp_bytes	; if still some bytes to read
		beq	.error		; but A = 0 so no error

		addw	#4,cdtemp_l	; add 4 sectors (with 16-bit carry)
		inc	cdtemp_bank	; go back for next bank
		bra	.loop

.error:		tax
		lda	cdtemp_savbnk60
		tam	#3
		lda	cdtemp_savbnk80
		tam	#4
		cla
		rts
 .ifdef _LIB3
	.bank LIB1_BANK
 .endif ; _LIB3

;
; prepare the sector address
;
prep_rdsect:	txa
		asl	A
		asl	A
		tay
		map	ovlarray	; in DATA_BANK
		ldx	ovlarray,Y
		iny
		lda	ovlarray,Y
		rts

;--------------------
; NOT IMPLEMENTED YET
;--------------------
;
;
; cd_status()
; ----
; Get CDROM status
;
;   input $00     (= busy check)  -> return $00 if not busy, else busy
;         (other) (= ready check) -> return $00 if ready, else sub error code
; ----
;
_cd_status:
		txa
		jsr	cd_stat
		tax
		cla
		rts


;--------------------


;
; int cd_getver()
; ----
; get CDROM system card version number
; MSB = major number
; LSB = minor number
; ----
;
_cd_getver:
		jsr	ex_getver
		tya
		sax
		rts


;
; int ac_exists(void)
; ----
; Detect Arcade Card (return 1 if true)
; ----
;
_ac_exists:	lda	ac_identflag
		ldx	#1
		cmp	#AC_IDENT
		beq	.true
		clx
.true:		cla
		rts

;
; ad_reset(void)
; ----
; Reset ADPCM device
; ----
;
_ad_reset:	jsr	ad_reset
		rts

;
; ad_stop(void)
; ----
; stop ADPCM playing
; ----
;
_ad_stop:	jsr	ad_stop
		rts

;
; char ad_stat(void)
; ----
; Get ADPCM status
; ----
;
_ad_stat:	jsr	ad_stat
		tax
		cla
		rts

;
; char ad_trans(int ovl_index [di], int sect_offset [si], char nb_sectors [al], int ad_addr [bx])
; ----
; Load CDROM data directly into ADPCM RAM
; ----
;
_ad_trans.4:
	      __ldw	<_di
		jsr	prep_rdsect
	      __addw	<_si
		stz	<_cl
		sta	<_ch
		stx	<_dl
		stz	<_dh
		jsr	ad_trans
		tax
		cla
		rts

;
; char ad_read(int ad_addr [cx], char mode [dh], int buf [bx], int bytes [ax])
; ----
; copy data from ADPCM RAM directly to RAM or VRAM
; ----
;
_ad_read.4:
		jsr	ad_read
		tax
		cla
		rts

;
; char ad_write(int ad_addr [cx], char mode [dh], int buf [bx], int bytes [ax])
; ----
; copy data from RAM or VRAM directly to ADPCM VRAM
; ----
;
_ad_write.4:
		jsr	ad_write
		tax
		cla
		rts

;
; char ad_play(int ad_addr [bx], int bytes [ax], char freq [dh], char mode [dl])
; ----
; play ADPCM sample
; ----
;
_ad_play.4:
		jsr	ad_play
		tax
		cla
		rts
 .ifdef _LIB3

 .endif ; _LIB3
