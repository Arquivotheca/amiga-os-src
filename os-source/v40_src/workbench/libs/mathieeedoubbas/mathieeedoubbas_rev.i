VERSION		EQU	38
REVISION	EQU	2
DATE	MACRO
		dc.b	'24.1.92'
	ENDM
VERS	MACRO
		dc.b	'mathieeedoubbas 38.2'
	ENDM
VSTRING	MACRO
		dc.b	'mathieeedoubbas 38.2 (24.1.92)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: mathieeedoubbas 38.2 (24.1.92)',0
		ds.w	0
	ENDM
