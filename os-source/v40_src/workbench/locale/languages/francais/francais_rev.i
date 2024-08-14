VERSION		EQU	38
REVISION	EQU	1
DATE	MACRO
		dc.b	'3.3.92'
	ENDM
VERS	MACRO
		dc.b	'francais 38.1'
	ENDM
VSTRING	MACRO
		dc.b	'francais 38.1 (3.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: francais 38.1 (3.3.92)',0
	ENDM
