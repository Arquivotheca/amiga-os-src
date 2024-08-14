VERSION		EQU	35
REVISION	EQU	40
DATE	MACRO
		dc.b	'15.4.91'
	ENDM
VERS	MACRO
		dc.b	'generic 35.40'
	ENDM
VSTRING	MACRO
		dc.b	'generic 35.40 (15.4.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: generic 35.40 (15.4.91)',0
		ds.w	0
	ENDM
