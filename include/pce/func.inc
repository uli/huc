;
; FUNC.INC  -  MagicKit Standard Functions
;
;

; ----
; BATVAL - create a value for the BAT map
;
; parameters:
; \1 = palette (0-15)
; \2 = VRAM address of tile

BATVAL	 .func	(\1<<12)|(\2>>4)


; ----
; CHAR - create a BAT value for a character defined
;        using '.defchar' (uses default palette)
;
; parameters:
; \1 = name of character
;
CHAR	 .func	(PAL(\1)<<12|VRAM(\1)>>4)

; ----
; SPR_VRAM - return shifted value of sprite-pattern VRAM address
;
; parameters:
; \1 = VRAM address of sprite
;
SPR_VRAM .func	(VRAM(\1)>>5)

