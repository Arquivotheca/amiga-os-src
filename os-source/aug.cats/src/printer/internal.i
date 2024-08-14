;
; Internal defines for the printer device.
;

;flags for PrtInfo structure.
PRT_BW			EQU	$01	; non-color picture
PRT_BLACKABLE		EQU	$02	; printer has black capabilities
PRT_BW_BLACKABLE	EQU	$03	; combination of above two
PRT_HAM			EQU	$04	; printing a ham picture
PRT_INVERT		EQU	$08	; invert picture
PRT_NOBLIT		EQU	$10	; can't use blitter in ReadPixelLine
PRT_RENDER0		EQU	$20	; render (case 0) has been called
PRT_NORPL		EQU	$40	; can't use ReadPixelLine
PRT_BELOW		EQU	$80	; there is a line below us

	IFNE	0	* defined in prtgfx.i
PRT_24BIT               EQU	$100    ; printer wants 24-bit info
PRT_DITHER8             EQU	$200    ; printer wants 8x8 dither matrix
PRT_DITHER16            EQU	$400    ; printer wants 16x16 dither matrix
	ENDC

SPECIAL_FIX_RGB_MASK	EQU	SPECIAL_FIX_RED!SPECIAL_FIX_GREEN!SPECIAL_FIX_BLUE

MAXBLITSIZE		EQU	1008	; max # of pixels blitter can transfer
MAXDEPTH		EQU	8	; max # of planes ClipBlit can blit
