VERSION		EQU	36
REVISION	EQU	35
DATE	MACRO
		dc.b	'4.12.90'
	ENDM
VERS	MACRO
		dc.b	'janus.library 36.35'
	ENDM
VSTRING	MACRO
		dc.b	'janus.library 36.35 (4.12.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: janus.library 36.35 (4.12.90)',0
	ENDM
