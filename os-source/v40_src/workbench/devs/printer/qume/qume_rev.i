VERSION		EQU	35
REVISION	EQU	47
DATE	MACRO
		dc.b	'15 Dec 1989'
	ENDM
VERS	MACRO
		dc.b	'qume 35.47'
	ENDM
VSTRING	MACRO
		dc.b	'qume 35.47 (15 Dec 1989)',13,10,0
		ds.w	0
	ENDM
