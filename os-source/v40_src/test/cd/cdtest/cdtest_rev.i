VERSION		EQU	1
REVISION	EQU	3
DATE	MACRO
		dc.b	'4.6.93'
	ENDM
VERS	MACRO
		dc.b	'cdtest 1.3'
	ENDM
VSTRING	MACRO
		dc.b	'cdtest 1.3 (4.6.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: cdtest 1.3 (4.6.93)',0
	ENDM
