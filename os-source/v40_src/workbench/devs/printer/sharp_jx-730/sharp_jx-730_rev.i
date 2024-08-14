VERSION		EQU	35
REVISION	EQU	22
DATE	MACRO
		dc.b	'9.7.91'
	ENDM
VERS	MACRO
		dc.b	'sharp_jx-730 35.22'
	ENDM
VSTRING	MACRO
		dc.b	'sharp_jx-730 35.22 (9.7.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: sharp_jx-730 35.22 (9.7.91)',0
		ds.w	0
	ENDM
