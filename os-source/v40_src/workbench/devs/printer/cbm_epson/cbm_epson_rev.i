VERSION		EQU	35
REVISION	EQU	48
DATE	MACRO
		dc.b	'8.7.91'
	ENDM
VERS	MACRO
		dc.b	'cbm_epson 35.48'
	ENDM
VSTRING	MACRO
		dc.b	'cbm_epson 35.48 (8.7.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: cbm_epson 35.48 (8.7.91)',0
		ds.w	0
	ENDM
