; Arcade Card detect and initialize program 
; ripped from PopFul Mail.
;
; Tomaitheous '06
;

	;.org $0000  - disassembled 
	;.org $46C9  - in game

	LDA   $1AFF			;00000: AD FF 1A      
	CMP   #$51			;00003: C9 51         
	BNE   fail			;00005: D0 57         
	CLX 				;00007: 82
 LL_01:            
	STZ   $1A02,X		;00008: 9E 02 1A      
	STZ   $1A12,X		;0000B: 9E 12 1A      
	STZ   $1A22,X		;0000E: 9E 22 1A      
	STZ   $1A32,X		;00011: 9E 32 1A      
	INX 				;00014: E8            
	CPX   #$08			;00015: E0 08         
	BNE   LL_01			;00017: D0 EF         
	CLX 				;00019: 82            
 LL_02:
	STZ   $1AE0,X		;0001A: 9E E0 1A      
	INX 				;0001D: E8            
	CPX   #$06			;0001E: E0 06         
	BNE   LL_02			;00020: D0 F8         
	CLX 				;00022: 82            
 LL_03:
	LDA   $1A02,X		;00023: BD 02 1A      
	ORA   $1A12,X		;00026: 1D 12 1A      
	ORA   $1A22,X		;00029: 1D 22 1A      
	ORA   $1A32,X		;0002C: 1D 32 1A      
	BNE   fail			;0002F: D0 2D         
	INX 				;00031: E8            
	CPX   #$07			;00032: E0 07         
	BNE   LL_03			;00034: D0 ED         
	LDA   $1A02,X		;00036: BD 02 1A      
	ORA   $1A12,X		;00039: 1D 12 1A      
	ORA   $1A22,X		;0003C: 1D 22 1A      
	ORA   $1A32,X		;0003F: 1D 32 1A      
	AND   #$7F			;00042: 29 7F         
	BNE   fail			;00044: D0 18         
	CLX 				;00046: 82            
 LL_04:
	LDA   $1AE0,X		;00047: BD E0 1A      
	BNE   fail			;0004A: D0 12         
	INX 				;0004C: E8            
	CPX   #$04			;0004D: E0 04         
	BNE   LL_04			;0004F: D0 F6         
	LDA   $1AE4			;00051: AD E4 1A      
	ORA   $1AE5			;00054: 0D E5 1A      
	AND   #$0F			;00057: 29 0F         
	BNE   fail			;00059: D0 03         
	CLA 				;0005B: 62            
	CLC 				;0005C: 18            
	RTS 				;0005D: 60            

 fail:
	CLX 				;0005E: 82            
	LDA   #$FF			;0005F: A9 FF         
	SEC 				;00061: 38            
	RTS 				;00062: 60            

