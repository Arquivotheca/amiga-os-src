VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'4.11.92'
	ENDM
VERS	MACRO
		dc.b	'prepcard 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'prepcard 39.1 (4.11.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: prepcard 39.1 (4.11.92)',0
	ENDM
