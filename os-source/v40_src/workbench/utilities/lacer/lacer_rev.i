VERSION		EQU	37
REVISION	EQU	3
DATE	MACRO
		dc.b	'10.5.91'
	ENDM
VERS	MACRO
		dc.b	'lacer 37.3'
	ENDM
VSTRING	MACRO
		dc.b	'lacer 37.3 (10.5.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: lacer 37.3 (10.5.91)',0
		ds.w	0
	ENDM
