VERSION		EQU	37
REVISION	EQU	9
DATE	MACRO
		dc.b	'28.12.93'
	ENDM
VERS	MACRO
		dc.b	'HP_550C 37.9'
	ENDM
VSTRING	MACRO
		dc.b	'HP_550C 37.9 (28.12.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: HP_550C 37.9 (28.12.93)',0
	ENDM