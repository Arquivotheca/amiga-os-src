VERSION		EQU	35
REVISION	EQU	7
DATE	MACRO
		dc.b	'15 Dec 1989'
	ENDM
VERS	MACRO
		dc.b	'canon_1080a 35.7'
	ENDM
VSTRING	MACRO
		dc.b	'canon_1080a 35.7 (15 Dec 1989)',13,10,0
		ds.w	0
	ENDM