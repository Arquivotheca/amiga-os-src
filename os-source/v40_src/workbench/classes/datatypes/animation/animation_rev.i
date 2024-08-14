VERSION		EQU	40
REVISION	EQU	7
DATE	MACRO
		dc.b	'28.9.93'
	ENDM
VERS	MACRO
		dc.b	'animation 40.7'
	ENDM
VSTRING	MACRO
		dc.b	'animation 40.7 (28.9.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: animation 40.7 (28.9.93)',0
	ENDM
