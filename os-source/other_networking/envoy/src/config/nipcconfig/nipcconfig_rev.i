VERSION		EQU	37
REVISION	EQU	9
DATE	MACRO
		dc.b	'3.9.93'
	ENDM
VERS	MACRO
		dc.b	'nipcconfig 37.9'
	ENDM
VSTRING	MACRO
		dc.b	'nipcconfig 37.9 (3.9.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nipcconfig 37.9 (3.9.93)',0
	ENDM
