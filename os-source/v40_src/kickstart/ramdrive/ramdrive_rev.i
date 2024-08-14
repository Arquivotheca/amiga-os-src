VERSION		EQU	39
REVISION	EQU	35
DATE	MACRO
		dc.b	'21.5.92'
	ENDM
VERS	MACRO
		dc.b	'ramdrive 39.35'
	ENDM
VSTRING	MACRO
		dc.b	'ramdrive 39.35 (21.5.92)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: ramdrive 39.35 (21.5.92)',0
		ds.w	0
	ENDM
