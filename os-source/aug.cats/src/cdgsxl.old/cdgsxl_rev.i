VERSION		EQU	1
REVISION	EQU	28
DATE	MACRO
		dc.b	'14.5.93'
	ENDM
VERS	MACRO
		dc.b	'cdgsxl 1.28'
	ENDM
VSTRING	MACRO
		dc.b	'cdgsxl 1.28 (14.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: cdgsxl 1.28 (14.5.93)',0
	ENDM
