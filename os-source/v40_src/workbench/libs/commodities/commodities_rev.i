VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'11.2.93'
	ENDM
VERS	MACRO
		dc.b	'commodities 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'commodities 40.2 (11.2.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: commodities 40.2 (11.2.93)',0
	ENDM
