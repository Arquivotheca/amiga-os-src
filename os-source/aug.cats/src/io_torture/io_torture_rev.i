VERSION		EQU	37
REVISION	EQU	7
DATE	MACRO
		dc.b	'24.5.93'
	ENDM
VERS	MACRO
		dc.b	'io_torture 37.7'
	ENDM
VSTRING	MACRO
		dc.b	'io_torture 37.7 (24.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: io_torture 37.7 (24.5.93)',0
	ENDM
