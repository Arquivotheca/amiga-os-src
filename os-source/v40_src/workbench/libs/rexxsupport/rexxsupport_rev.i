VERSION		EQU	34
REVISION	EQU	9
DATE	MACRO
		dc.b	'29.8.90'
	ENDM
VERS	MACRO
		dc.b	'rexxsupport 34.9'
	ENDM
VSTRING	MACRO
		dc.b	'rexxsupport 34.9 (29.8.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rexxsupport 34.9 (29.8.90)',0
	ENDM
