VERSION		EQU	36
REVISION	EQU	1
DATE	MACRO
		dc.b	'12.11.90'
	ENDM
VERS	MACRO
		dc.b	'vers 36.1'
	ENDM
VSTRING	MACRO
		dc.b	'vers 36.1 (12.11.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: vers 36.1 (12.11.90)',0
	ENDM