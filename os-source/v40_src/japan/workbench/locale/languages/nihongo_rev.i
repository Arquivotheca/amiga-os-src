VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'18.1.93'
	ENDM
VERS	MACRO
		dc.b	'nihongo 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'nihongo 39.1 (18.1.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nihongo 39.1 (18.1.93)',0
	ENDM