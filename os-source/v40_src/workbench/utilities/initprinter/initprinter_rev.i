VERSION		EQU	38
REVISION	EQU	2
DATE	MACRO
		dc.b	'26.2.92'
	ENDM
VERS	MACRO
		dc.b	'initprinter 38.2'
	ENDM
VSTRING	MACRO
		dc.b	'initprinter 38.2 (26.2.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: initprinter 38.2 (26.2.92)',0
	ENDM
