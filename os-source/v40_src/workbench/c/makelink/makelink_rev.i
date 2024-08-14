VERSION		EQU	37
REVISION	EQU	4
DATE	MACRO
		dc.b	'6.5.91'
	ENDM
VERS	MACRO
		dc.b	'makelink 37.4'
	ENDM
VSTRING	MACRO
		dc.b	'makelink 37.4 (6.5.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: makelink 37.4 (6.5.91)',0
	ENDM
