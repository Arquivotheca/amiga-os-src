VERSION		EQU	38
REVISION	EQU	8
DATE	MACRO
		dc.b	'14.4.92'
	ENDM
VERS	MACRO
		dc.b	'clipboard 38.8'
	ENDM
VSTRING	MACRO
		dc.b	'clipboard 38.8 (14.4.92)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: clipboard 38.8 (14.4.92)',0
		ds.w	0
	ENDM
