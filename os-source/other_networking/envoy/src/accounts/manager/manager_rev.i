VERSION		EQU	37
REVISION	EQU	13
DATE	MACRO
		dc.b	'1.3.93'
	ENDM
VERS	MACRO
		dc.b	'manager 37.13'
	ENDM
VSTRING	MACRO
		dc.b	'manager 37.13 (1.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: manager 37.13 (1.3.93)',0
	ENDM