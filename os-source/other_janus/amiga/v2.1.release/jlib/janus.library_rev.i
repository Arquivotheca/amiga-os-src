VERSION		EQU	36
REVISION	EQU	83
DATE	MACRO
		dc.b	'21.8.91'
	ENDM
VERS	MACRO
		dc.b	'janus.library 36.83'
	ENDM
VSTRING	MACRO
		dc.b	'janus.library 36.83 (21.8.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: janus.library 36.83 (21.8.91)',0
	ENDM
