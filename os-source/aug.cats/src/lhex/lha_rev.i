VERSION		EQU	40
REVISION	EQU	6
DATE	MACRO
		dc.b	'17.2.94'
	ENDM
VERS	MACRO
		dc.b	'lha 40.6'
	ENDM
VSTRING	MACRO
		dc.b	'lha 40.6 (17.2.94)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: lha 40.6 (17.2.94)',0
	ENDM
