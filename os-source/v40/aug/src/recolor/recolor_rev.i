VERSION		EQU	37
REVISION	EQU	2
DATE	MACRO
		dc.b	'2.5.91'
	ENDM
VERS	MACRO
		dc.b	'recolor 37.2'
	ENDM
VSTRING	MACRO
		dc.b	'recolor 37.2 (2.5.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: recolor 37.2 (2.5.91)',0
	ENDM
