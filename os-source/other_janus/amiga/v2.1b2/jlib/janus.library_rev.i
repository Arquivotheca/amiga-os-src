VERSION		EQU	36
REVISION	EQU	52
DATE	MACRO
		dc.b	'8.3.91'
	ENDM
VERS	MACRO
		dc.b	'janus.library 36.52'
	ENDM
VSTRING	MACRO
		dc.b	'janus.library 36.52 (8.3.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: janus.library 36.52 (8.3.91)',0
	ENDM
