VERSION		EQU	36
REVISION	EQU	42
DATE	MACRO
		dc.b	'18.12.90'
	ENDM
VERS	MACRO
		dc.b	'mungwall 36.42a'
	ENDM
VSTRING	MACRO
		dc.b	'mungwall 36.42a (18.12.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: mungwall 36.42a (18.12.90)',0
	ENDM
