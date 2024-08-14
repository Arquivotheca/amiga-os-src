VERSION		EQU	37
REVISION	EQU	3
DATE	MACRO
		dc.b	'12.11.91'
	ENDM
VERS	MACRO
		dc.b	'showconfig 37.3'
	ENDM
VSTRING	MACRO
		dc.b	'showconfig 37.3 (12.11.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: showconfig 37.3 (12.11.91)',0
	ENDM
