VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'26.8.93'
	ENDM
VERS	MACRO
		dc.b	'gradientslider 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'gradientslider 40.2 (26.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: gradientslider 40.2 (26.8.93)',0
	ENDM