VERSION		EQU	40
REVISION	EQU	3
DATE	MACRO
		dc.b	'7.12.93'
	ENDM
VERS	MACRO
		dc.b	'text 40.3'
	ENDM
VSTRING	MACRO
		dc.b	'text 40.3 (7.12.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: text 40.3 (7.12.93)',0
	ENDM