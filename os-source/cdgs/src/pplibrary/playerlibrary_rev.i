VERSION		EQU	40
REVISION	EQU	8
DATE	MACRO
		dc.b	'19.3.93'
	ENDM
VERS	MACRO
		dc.b	'playerlibrary 40.8'
	ENDM
VSTRING	MACRO
		dc.b	'playerlibrary 40.8 (19.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: playerlibrary 40.8 (19.3.93)',0
	ENDM
