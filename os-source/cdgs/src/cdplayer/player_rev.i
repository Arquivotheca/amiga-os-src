VERSION		EQU	40
REVISION	EQU	3
DATE	MACRO
		dc.b	'29.7.93'
	ENDM
VERS	MACRO
		dc.b	'player 40.3'
	ENDM
VSTRING	MACRO
		dc.b	'player 40.3 (29.7.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: player 40.3 (29.7.93)',0
	ENDM