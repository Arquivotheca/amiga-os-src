VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'22.4.93'
	ENDM
VERS	MACRO
		dc.b	'palette 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'palette 40.2 (22.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: palette 40.2 (22.4.93)',0
	ENDM
