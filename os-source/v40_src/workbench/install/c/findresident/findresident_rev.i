VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'26.5.93'
	ENDM
VERS	MACRO
		dc.b	'findresident 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'findresident 40.1 (26.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: findresident 40.1 (26.5.93)',0
	ENDM
