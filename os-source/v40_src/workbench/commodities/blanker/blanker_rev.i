VERSION		EQU	40
REVISION	EQU	3
DATE	MACRO
		dc.b	'17.8.93'
	ENDM
VERS	MACRO
		dc.b	'blanker 40.3'
	ENDM
VSTRING	MACRO
		dc.b	'blanker 40.3 (17.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: blanker 40.3 (17.8.93)',0
	ENDM