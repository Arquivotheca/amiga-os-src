VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'9.3.93'
	ENDM
VERS	MACRO
		dc.b	'expansion 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'expansion 40.2 (9.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: expansion 40.2 (9.3.93)',0
	ENDM
