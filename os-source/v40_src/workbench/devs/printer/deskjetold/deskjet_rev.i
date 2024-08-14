VERSION		EQU	35
REVISION	EQU	31
DATE	MACRO
		dc.b	'15.1.92'
	ENDM
VERS	MACRO
		dc.b	'deskjet 35.31'
	ENDM
VSTRING	MACRO
		dc.b	'deskjet 35.31 (15.1.92)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: deskjet 35.31 (15.1.92)',0
		ds.w	0
	ENDM
