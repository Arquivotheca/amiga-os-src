VERSION		EQU	36
REVISION	EQU	92
DATE	MACRO
		dc.b	'19.12.91'
	ENDM
VERS	MACRO
		dc.b	'flipper 36.92'
	ENDM
VSTRING	MACRO
		dc.b	'flipper 36.92 (19.12.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: flipper 36.92 (19.12.91)',0
	ENDM