VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'11.5.91'
	ENDM
VERS	MACRO
		dc.b	'showconfig 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'showconfig 37.1 (11.5.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: showconfig 37.1 (11.5.91)',0
	ENDM
