VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'11.1.91'
	ENDM
VERS	MACRO
		dc.b	'trap-handler 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'trap-handler 37.1 (11.1.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: trap-handler 37.1 (11.1.91)',0
	ENDM
