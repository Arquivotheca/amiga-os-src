VERSION		EQU	36
REVISION	EQU	1
DATE	MACRO
		dc.b	'13.12.90'
	ENDM
VERS	MACRO
		dc.b	'ed 36.1'
	ENDM
VSTRING	MACRO
		dc.b	'ed 36.1 (13.12.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: ed 36.1 (13.12.90)',0
	ENDM