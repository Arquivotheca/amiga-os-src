VERSION		EQU	40
REVISION	EQU	14
DATE	MACRO
		dc.b	'22.2.94'
	ENDM
VERS	MACRO
		dc.b	'playerlibrary 40.14'
	ENDM
VSTRING	MACRO
		dc.b	'playerlibrary 40.14 (22.2.94)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: playerlibrary 40.14 (22.2.94)',0
	ENDM
