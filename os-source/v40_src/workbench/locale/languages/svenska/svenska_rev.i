VERSION		EQU	38
REVISION	EQU	5
DATE	MACRO
		dc.b	'12.3.92'
	ENDM
VERS	MACRO
		dc.b	'svenska 38.5'
	ENDM
VSTRING	MACRO
		dc.b	'svenska 38.5 (12.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: svenska 38.5 (12.3.92)',0
	ENDM
