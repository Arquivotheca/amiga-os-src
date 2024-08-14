VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'12.1.91'
	ENDM
VERS	MACRO
		dc.b	'graphicdump 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'graphicdump 37.1 (12.1.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: graphicdump 37.1 (12.1.91)',0
	ENDM
