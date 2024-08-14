VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'2.6.92'
	ENDM
VERS	MACRO
		dc.b	'makebuildlist 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'makebuildlist 39.1 (2.6.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: makebuildlist 39.1 (2.6.92)',0
	ENDM
