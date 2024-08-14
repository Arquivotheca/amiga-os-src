VERSION		EQU	36
REVISION	EQU	5
DATE	MACRO
		dc.b	'9.4.91'
	ENDM
VERS	MACRO
		dc.b	'rexxmast 36.5'
	ENDM
VSTRING	MACRO
		dc.b	'rexxmast 36.5 (9.4.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rexxmast 36.5 (9.4.91)',0
	ENDM
