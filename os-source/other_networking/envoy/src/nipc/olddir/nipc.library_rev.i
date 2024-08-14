VERSION		EQU	1
REVISION	EQU	11
DATE	MACRO
		dc.b	'27.2.92'
	ENDM
VERS	MACRO
		dc.b	'nipc.library 1.11'
	ENDM
VSTRING	MACRO
		dc.b	'nipc.library 1.11 (27.2.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nipc.library 1.11 (27.2.92)',0
	ENDM
