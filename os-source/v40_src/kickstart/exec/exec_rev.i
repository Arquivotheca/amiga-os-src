VERSION		EQU	40
REVISION	EQU	10
DATE	MACRO
		dc.b	'15.7.93'
	ENDM
VERS	MACRO
		dc.b	'exec 40.10'
	ENDM
VSTRING	MACRO
		dc.b	'exec 40.10 (15.7.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: exec 40.10 (15.7.93)',0
	ENDM