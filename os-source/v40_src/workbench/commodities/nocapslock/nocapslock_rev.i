VERSION		EQU	38
REVISION	EQU	4
DATE	MACRO
		dc.b	'25.2.92'
	ENDM
VERS	MACRO
		dc.b	'nocapslock 38.4'
	ENDM
VSTRING	MACRO
		dc.b	'nocapslock 38.4 (25.2.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nocapslock 38.4 (25.2.92)',0
	ENDM
