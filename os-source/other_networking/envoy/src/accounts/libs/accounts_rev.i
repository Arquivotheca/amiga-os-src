VERSION		EQU	37
REVISION	EQU	5
DATE	MACRO
		dc.b	'1.3.93'
	ENDM
VERS	MACRO
		dc.b	'accounts 37.5'
	ENDM
VSTRING	MACRO
		dc.b	'accounts 37.5 (1.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: accounts 37.5 (1.3.93)',0
	ENDM
