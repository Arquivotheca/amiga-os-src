VERSION		EQU	35
REVISION	EQU	58
DATE	MACRO
		dc.b	'15 Dec 1989'
	ENDM
VERS	MACRO
		dc.b	'diablo_adv 35.58'
	ENDM
VSTRING	MACRO
		dc.b	'diablo_adv 35.58 (15 Dec 1989)',13,10,0
		ds.w	0
	ENDM
