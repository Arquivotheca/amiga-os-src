VERSION		EQU	37
REVISION	EQU	7
DATE	MACRO
		dc.b	'5.1.93'
	ENDM
VERS	MACRO
		dc.b	'smconfig 37.7'
	ENDM
VSTRING	MACRO
		dc.b	'smconfig 37.7 (5.1.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: smconfig 37.7 (5.1.93)',0
	ENDM
