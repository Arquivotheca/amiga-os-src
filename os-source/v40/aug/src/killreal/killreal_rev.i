VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'1.3.91'
	ENDM
VERS	MACRO
		dc.b	'killreal 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'killreal 37.1 (1.3.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: killreal 37.1 (1.3.91)',0
	ENDM
