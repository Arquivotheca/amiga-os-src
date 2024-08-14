VERSION		EQU	37
REVISION	EQU	5
DATE	MACRO
		dc.b	'8.5.91'
	ENDM
VERS	MACRO
		dc.b	'potgo 37.5'
	ENDM
VSTRING	MACRO
		dc.b	'potgo 37.5 (8.5.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: potgo 37.5 (8.5.91)',0
		ds.w	0
	ENDM
