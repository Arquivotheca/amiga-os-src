VERSION		EQU	40
REVISION	EQU	12
DATE	MACRO
		dc.b	'23.8.93'
	ENDM
VERS	MACRO
		dc.b	'amigaguide 40.12'
	ENDM
VSTRING	MACRO
		dc.b	'amigaguide 40.12 (23.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: amigaguide 40.12 (23.8.93)',0
	ENDM