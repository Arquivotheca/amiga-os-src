VERSION		EQU	40
REVISION	EQU	4
DATE	MACRO
		dc.b	'2.8.93'
	ENDM
VERS	MACRO
		dc.b	'cdplayer 40.4'
	ENDM
VSTRING	MACRO
		dc.b	'cdplayer 40.4 (2.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: cdplayer 40.4 (2.8.93)',0
	ENDM
