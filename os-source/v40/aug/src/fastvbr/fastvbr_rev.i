VERSION		EQU	1
REVISION	EQU	1
DATE	MACRO
		dc.b	'4.2.92'
	ENDM
VERS	MACRO
		dc.b	'FastVBR 1.1'
	ENDM
VSTRING	MACRO
		dc.b	'FastVBR 1.1 (4.2.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: FastVBR 1.1 (4.2.92)',0
	ENDM
