VERSION		EQU	37
REVISION	EQU	4
DATE	MACRO
		dc.b	'1.12.92'
	ENDM
VERS	MACRO
		dc.b	'guessenvoydev 37.4'
	ENDM
VSTRING	MACRO
		dc.b	'guessenvoydev 37.4 (1.12.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: guessenvoydev 37.4 (1.12.92)',0
	ENDM
