VERSION		EQU	39
REVISION	EQU	2
DATE	MACRO
		dc.b	'5.2.93'
	ENDM
VERS	MACRO
		dc.b	'showanim 39.2'
	ENDM
VSTRING	MACRO
		dc.b	'showanim 39.2 (5.2.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: showanim 39.2 (5.2.93)',0
	ENDM
