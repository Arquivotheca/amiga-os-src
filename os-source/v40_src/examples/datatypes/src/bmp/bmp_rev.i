VERSION		EQU	39
REVISION	EQU	4
DATE	MACRO
		dc.b	'10.11.92'
	ENDM
VERS	MACRO
		dc.b	'bmp 39.4'
	ENDM
VSTRING	MACRO
		dc.b	'bmp 39.4 (10.11.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: bmp 39.4 (10.11.92)',0
	ENDM
