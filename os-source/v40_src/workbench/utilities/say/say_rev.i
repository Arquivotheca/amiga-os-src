VERSION		EQU	38
REVISION	EQU	6
DATE	MACRO
		dc.b	'25.3.92'
	ENDM
VERS	MACRO
		dc.b	'say 38.6'
	ENDM
VSTRING	MACRO
		dc.b	'say 38.6 (25.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: say 38.6 (25.3.92)',0
	ENDM