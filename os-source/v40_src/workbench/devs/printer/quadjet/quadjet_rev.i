VERSION		EQU	35
REVISION	EQU	12
DATE	MACRO
		dc.b	'17 Dec 1989'
	ENDM
VERS	MACRO
		dc.b	'quadjet 35.12'
	ENDM
VSTRING	MACRO
		dc.b	'quadjet 35.12 (17 Dec 1989)',13,10,0
		ds.w	0
	ENDM