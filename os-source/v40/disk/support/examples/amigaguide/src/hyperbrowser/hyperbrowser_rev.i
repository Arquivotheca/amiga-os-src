VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'13.8.92'
	ENDM
VERS	MACRO
		dc.b	'hyperbrowser 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'hyperbrowser 39.1 (13.8.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: hyperbrowser 39.1 (13.8.92)',0
	ENDM
