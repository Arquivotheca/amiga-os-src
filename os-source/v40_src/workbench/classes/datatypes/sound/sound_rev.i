VERSION		EQU	40
REVISION	EQU	6
DATE	MACRO
		dc.b	'12.5.93'
	ENDM
VERS	MACRO
		dc.b	'sound 40.6'
	ENDM
VSTRING	MACRO
		dc.b	'sound 40.6 (12.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: sound 40.6 (12.5.93)',0
	ENDM
