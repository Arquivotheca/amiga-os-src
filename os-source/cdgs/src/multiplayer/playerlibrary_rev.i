VERSION		EQU	40
REVISION	EQU	13
DATE	MACRO
		dc.b	'15.12.93'
	ENDM
VERS	MACRO
		dc.b	'playerlibrary 40.13'
	ENDM
VSTRING	MACRO
		dc.b	'playerlibrary 40.13 (15.12.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: playerlibrary 40.13 (15.12.93)',0
	ENDM