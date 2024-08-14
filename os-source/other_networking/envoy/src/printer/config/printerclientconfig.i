VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'8.8.93'
	ENDM
VERS	MACRO
		dc.b	'printerclientconfig 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'printerclientconfig 40.1 (8.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: printerclientconfig 40.1 (8.8.93)',0
	ENDM
