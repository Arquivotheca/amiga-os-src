VERSION	EQU	38
REVISION	EQU	4
DATE	MACRO
	dc.b	'16.7.91'
	ENDM
VERS	MACRO
	dc.b	'printer 38.4'
	ENDM
VSTRING	MACRO
	dc.b	'printer 38.4 (16.7.91)',13,10,0
	ds.w	0
	ENDM
VERTAG	MACRO
	dc.b	0,'$VER: printer 38.4 (16.7.91)',0
	ds.w	0
	ENDM
