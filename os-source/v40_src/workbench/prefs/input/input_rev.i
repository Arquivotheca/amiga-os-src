VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'27.4.93'
	ENDM
VERS	MACRO
		dc.b	'input 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'input 40.2 (27.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: input 40.2 (27.4.93)',0
	ENDM
