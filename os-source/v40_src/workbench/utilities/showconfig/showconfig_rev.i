VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'24.8.93'
	ENDM
VERS	MACRO
		dc.b	'showconfig 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'showconfig 40.2 (24.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: showconfig 40.2 (24.8.93)',0
	ENDM
