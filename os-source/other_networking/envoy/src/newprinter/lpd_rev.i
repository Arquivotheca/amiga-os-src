VERSION		EQU	1
REVISION	EQU	1
DATE	MACRO
		dc.b	'16.5.92'
	ENDM
VERS	MACRO
		dc.b	'lpd 1.1'
	ENDM
VSTRING	MACRO
		dc.b	'lpd 1.1 (16.5.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: lpd 1.1 (16.5.92)',0
	ENDM
