VERSION		EQU	40
REVISION	EQU	4
DATE	MACRO
		dc.b	'10.6.93'
	ENDM
VERS	MACRO
		dc.b	'amigaguide 40.4'
	ENDM
VSTRING	MACRO
		dc.b	'amigaguide 40.4 (10.6.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: amigaguide 40.4 (10.6.93)',0
	ENDM
