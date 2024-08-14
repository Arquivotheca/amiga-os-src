VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'31.12.92'
	ENDM
VERS	MACRO
		dc.b	'postscript 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'postscript 39.1 (31.12.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: postscript 39.1 (31.12.92)',0
	ENDM
