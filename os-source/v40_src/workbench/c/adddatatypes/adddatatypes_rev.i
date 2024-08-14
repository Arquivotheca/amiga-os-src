VERSION		EQU	39
REVISION	EQU	2
DATE	MACRO
		dc.b	'27.7.92'
	ENDM
VERS	MACRO
		dc.b	'adddatatypes 39.2'
	ENDM
VSTRING	MACRO
		dc.b	'adddatatypes 39.2 (27.7.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: adddatatypes 39.2 (27.7.92)',0
	ENDM
