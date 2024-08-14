VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'1.9.92'
	ENDM
VERS	MACRO
		dc.b	'graffiti 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'graffiti 39.1 (1.9.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: graffiti 39.1 (1.9.92)',0
	ENDM
