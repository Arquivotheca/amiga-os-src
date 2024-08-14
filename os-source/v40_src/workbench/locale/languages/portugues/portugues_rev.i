VERSION		EQU	38
REVISION	EQU	4
DATE	MACRO
		dc.b	'3.3.92'
	ENDM
VERS	MACRO
		dc.b	'portugues 38.4'
	ENDM
VSTRING	MACRO
		dc.b	'portugues 38.4 (3.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: portugues 38.4 (3.3.92)',0
	ENDM
