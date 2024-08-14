VERSION		EQU	39
REVISION	EQU	3
DATE	MACRO
		dc.b	'6.8.92'
	ENDM
VERS	MACRO
		dc.b	'guessbootdev 39.3'
	ENDM
VSTRING	MACRO
		dc.b	'guessbootdev 39.3 (6.8.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: guessbootdev 39.3 (6.8.92)',0
	ENDM
