VERSION		EQU	37
REVISION	EQU	8
DATE	MACRO
		dc.b	'13.12.93'
	ENDM
VERS	MACRO
		dc.b	'HP_550C 37.8'
	ENDM
VSTRING	MACRO
		dc.b	'HP_550C 37.8 (13.12.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: HP_550C 37.8 (13.12.93)',0
	ENDM
