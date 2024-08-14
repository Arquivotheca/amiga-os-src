VERSION		EQU	37
REVISION	EQU	4
DATE	MACRO
		dc.b	'21.2.91'
	ENDM
VERS	MACRO
		dc.b	'speak-handler 37.4'
	ENDM
VSTRING	MACRO
		dc.b	'speak-handler 37.4 (21.2.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: speak-handler 37.4 (21.2.91)',0
		ds.w	0
	ENDM
