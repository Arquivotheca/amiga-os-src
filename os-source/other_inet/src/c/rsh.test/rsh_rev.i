VERSION		EQU	3
REVISION	EQU	1
DATE	MACRO
		dc.b	'4.4.92'
	ENDM
VERS	MACRO
		dc.b	'rsh 3.1'
	ENDM
VSTRING	MACRO
		dc.b	'rsh 3.1 (4.4.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rsh 3.1 (4.4.92)',0
	ENDM
