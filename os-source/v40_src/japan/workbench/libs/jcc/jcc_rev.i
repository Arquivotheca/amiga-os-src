VERSION		EQU	39
REVISION	EQU	3
DATE	MACRO
		dc.b	'12.2.93'
	ENDM
VERS	MACRO
		dc.b	'jcc 39.3'
	ENDM
VSTRING	MACRO
		dc.b	'jcc 39.3 (12.2.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: jcc 39.3 (12.2.93)',0
	ENDM