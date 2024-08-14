VERSION		EQU	0
REVISION	EQU	1
DATE	MACRO
		dc.b	'10.11.92'
	ENDM
VERS	MACRO
		dc.b	'eucdriver 0.1'
	ENDM
VSTRING	MACRO
		dc.b	'eucdriver 0.1 (10.11.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: eucdriver 0.1 (10.11.92)',0
	ENDM
