VERSION		EQU	40
REVISION	EQU	4
DATE	MACRO
		dc.b	'16.3.93'
	ENDM
VERS	MACRO
		dc.b	'mathieeesingbas 40.4'
	ENDM
VSTRING	MACRO
		dc.b	'mathieeesingbas 40.4 (16.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: mathieeesingbas 40.4 (16.3.93)',0
	ENDM
