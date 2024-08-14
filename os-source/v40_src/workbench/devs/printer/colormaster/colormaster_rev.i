VERSION		EQU	35
REVISION	EQU	56
DATE	MACRO
		dc.b	'15 Dec 1989'
	ENDM
VERS	MACRO
		dc.b	'colormaster 35.56'
	ENDM
VSTRING	MACRO
		dc.b	'colormaster 35.56 (15 Dec 1989)',13,10,0
		ds.w	0
	ENDM
