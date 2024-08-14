VERSION		EQU	42
REVISION	EQU	1
DATE	MACRO
		dc.b	'12.7.93'
	ENDM
VERS	MACRO
		dc.b	'commodities 42.1'
	ENDM
VSTRING	MACRO
		dc.b	'commodities 42.1 (12.7.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: commodities 42.1 (12.7.93)',0
	ENDM
