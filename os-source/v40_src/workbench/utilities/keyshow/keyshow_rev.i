VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'16.1.91'
	ENDM
VERS	MACRO
		dc.b	'keyshow 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'keyshow 37.1 (16.1.91)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: keyshow 37.1 (16.1.91)',0
		ds.w	0
	ENDM
