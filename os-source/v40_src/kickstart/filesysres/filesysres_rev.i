VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'15.2.93'
	ENDM
VERS	MACRO
		dc.b	'filesysres 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'filesysres 40.1 (15.2.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: filesysres 40.1 (15.2.93)',0
	ENDM
