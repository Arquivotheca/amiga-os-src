VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'1.10.93'
	ENDM
VERS	MACRO
		dc.b	'serial 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'serial 40.1 (1.10.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: serial 40.1 (1.10.93)',0
	ENDM
