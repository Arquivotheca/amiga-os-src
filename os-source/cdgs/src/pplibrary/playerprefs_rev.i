VERSION		EQU	40
REVISION	EQU	12
DATE	MACRO
		dc.b	'7.4.93'
	ENDM
VERS	MACRO
		dc.b	'playerprefs 40.12'
	ENDM
VSTRING	MACRO
		dc.b	'playerprefs 40.12 (7.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: playerprefs 40.12 (7.4.93)',0
	ENDM
