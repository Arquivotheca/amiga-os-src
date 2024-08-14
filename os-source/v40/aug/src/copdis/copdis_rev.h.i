VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'20.11.92'
	ENDM
VERS	MACRO
		dc.b	'copdis_rev.h 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'copdis_rev.h 39.1 (20.11.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: copdis_rev.h 39.1 (20.11.92)',0
	ENDM
