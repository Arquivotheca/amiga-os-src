VERSION		EQU	35
REVISION	EQU	31
DATE	MACRO
		dc.b	'15 Dec 1989'
	ENDM
VERS	MACRO
		dc.b	'diablo_c 35.31'
	ENDM
VSTRING	MACRO
		dc.b	'diablo_c 35.31 (15 Dec 1989)',13,10,0
		ds.w	0
	ENDM