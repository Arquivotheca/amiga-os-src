VERSION		EQU	37
REVISION	EQU	2
DATE	MACRO
		dc.b	'7.2.91'
	ENDM
VERS	MACRO
		dc.b	'mathieeesingbas 37.2'
	ENDM
VSTRING	MACRO
		dc.b	'mathieeesingbas 37.2 (7.2.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: mathieeesingbas 37.2 (7.2.91)',0
		ds.w	0
	ENDM
