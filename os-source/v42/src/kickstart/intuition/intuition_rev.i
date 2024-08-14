VERSION		EQU	42
REVISION	EQU	1
DATE	MACRO
		dc.b	'15.2.94'
	ENDM
VERS	MACRO
		dc.b	'intuition 42.1'
	ENDM
VSTRING	MACRO
		dc.b	'intuition 42.1 (15.2.94)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: intuition 42.1 (15.2.94)',0
	ENDM
