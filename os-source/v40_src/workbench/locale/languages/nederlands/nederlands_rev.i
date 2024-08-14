VERSION		EQU	38
REVISION	EQU	2
DATE	MACRO
		dc.b	'3.3.92'
	ENDM
VERS	MACRO
		dc.b	'nederlands 38.2'
	ENDM
VSTRING	MACRO
		dc.b	'nederlands 38.2 (3.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nederlands 38.2 (3.3.92)',0
	ENDM
