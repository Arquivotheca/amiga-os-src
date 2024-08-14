VERSION		EQU	40
REVISION	EQU	4
DATE	MACRO
		dc.b	'2.2.94'
	ENDM
VERS	MACRO
		dc.b	'hdtoolbox 40.4'
	ENDM
VSTRING	MACRO
		dc.b	'hdtoolbox 40.4 (2.2.94)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: hdtoolbox 40.4 (2.2.94)',0
	ENDM
