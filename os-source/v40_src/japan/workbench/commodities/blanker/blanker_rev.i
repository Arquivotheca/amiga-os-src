VERSION		EQU	38
REVISION	EQU	18
DATE	MACRO
		dc.b	'25.3.92'
	ENDM
VERS	MACRO
		dc.b	'blanker 38.18'
	ENDM
VSTRING	MACRO
		dc.b	'blanker 38.18 (25.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: blanker 38.18 (25.3.92)',0
	ENDM
