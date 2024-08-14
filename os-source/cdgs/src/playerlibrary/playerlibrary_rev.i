VERSION		EQU	40
REVISION	EQU	12
DATE	MACRO
		dc.b	'19.5.93'
	ENDM
VERS	MACRO
		dc.b	'playerlibrary 40.12'
	ENDM
VSTRING	MACRO
		dc.b	'playerlibrary 40.12 (19.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: playerlibrary 40.12 (19.5.93)',0
	ENDM
