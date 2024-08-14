VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'12.5.93'
	ENDM
VERS	MACRO
		dc.b	'con-handler 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'con-handler 40.2 (12.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: con-handler 40.2 (12.5.93)',0
	ENDM
