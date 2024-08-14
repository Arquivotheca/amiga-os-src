VERSION		EQU	34
REVISION	EQU	29
DATE	MACRO
		dc.b	'3.11.90'
	ENDM
VERS	MACRO
		dc.b	'janus.library 34.29'
	ENDM
VSTRING	MACRO
		dc.b	'janus.library 34.29 (3.11.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: janus.library 34.29 (3.11.90)',0
	ENDM
