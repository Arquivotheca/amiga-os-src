VERSION		EQU	1
REVISION	EQU	1
DATE	MACRO
		dc.b	'4.4.92'
	ENDM
VERS	MACRO
		dc.b	'rcmd 1.1'
	ENDM
VSTRING	MACRO
		dc.b	'rcmd 1.1 (4.4.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rcmd 1.1 (4.4.92)',0
	ENDM
