VERSION		EQU	42
REVISION	EQU	3
DATE	MACRO
		dc.b	'16.3.94'
	ENDM
VERS	MACRO
		dc.b	'filesysres 42.3'
	ENDM
VSTRING	MACRO
		dc.b	'filesysres 42.3 (16.3.94)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: filesysres 42.3 (16.3.94)',0
	ENDM
