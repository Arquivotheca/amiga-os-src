VERSION		EQU	36
REVISION	EQU	12
DATE	MACRO
		dc.b	'8.3.91'
	ENDM
VERS	MACRO
		dc.b	'flipper 36.12'
	ENDM
VSTRING	MACRO
		dc.b	'flipper 36.12 (8.3.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: flipper 36.12 (8.3.91)',0
	ENDM
