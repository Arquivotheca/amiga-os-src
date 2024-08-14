VERSION		EQU	39
REVISION	EQU	3
DATE	MACRO
		dc.b	'5.8.92'
	ENDM
VERS	MACRO
		dc.b	'extractkickstart 39.3'
	ENDM
VSTRING	MACRO
		dc.b	'extractkickstart 39.3 (5.8.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: extractkickstart 39.3 (5.8.92)',0
	ENDM
