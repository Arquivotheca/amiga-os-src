VERSION		EQU	40
REVISION	EQU	3
DATE	MACRO
		dc.b	'2.6.93'
	ENDM
VERS	MACRO
		dc.b	'printerps 40.3'
	ENDM
VSTRING	MACRO
		dc.b	'printerps 40.3 (2.6.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: printerps 40.3 (2.6.93)',0
	ENDM