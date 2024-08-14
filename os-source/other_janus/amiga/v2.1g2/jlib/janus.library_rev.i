VERSION		EQU	36
REVISION	EQU	77
DATE	MACRO
		dc.b	'25.7.91'
	ENDM
VERS	MACRO
		dc.b	'janus.library 36.77'
	ENDM
VSTRING	MACRO
		dc.b	'janus.library 36.77 (25.7.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: janus.library 36.77 (25.7.91)',0
	ENDM
