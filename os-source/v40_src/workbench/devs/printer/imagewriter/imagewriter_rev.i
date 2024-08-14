VERSION		EQU	35
REVISION	EQU	50
DATE	MACRO
		dc.b	'15 Dec 1989'
	ENDM
VERS	MACRO
		dc.b	'imagewriter 35.50'
	ENDM
VSTRING	MACRO
		dc.b	'imagewriter 35.50 (15 Dec 1989)',13,10,0
		ds.w	0
	ENDM
