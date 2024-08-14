VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'30.7.93'
	ENDM
VERS	MACRO
		dc.b	'envoyprint 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'envoyprint 40.1 (30.7.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: envoyprint 40.1 (30.7.93)',0
	ENDM
