VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'19.4.93'
	ENDM
VERS	MACRO
		dc.b	'Intellifont 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'Intellifont 40.1 (19.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: Intellifont 40.1 (19.4.93)',0
	ENDM
