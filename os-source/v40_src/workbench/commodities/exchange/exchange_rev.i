VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'14.6.93'
	ENDM
VERS	MACRO
		dc.b	'exchange 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'exchange 40.1 (14.6.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: exchange 40.1 (14.6.93)',0
	ENDM