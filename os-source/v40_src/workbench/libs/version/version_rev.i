VERSION		EQU	40
REVISION	EQU	43
DATE	MACRO
		dc.b	'25.3.94'
	ENDM
VERS	MACRO
		dc.b	'version 40.43'
	ENDM
VSTRING	MACRO
		dc.b	'version 40.43 (25.3.94)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: version 40.43 (25.3.94)',0
	ENDM