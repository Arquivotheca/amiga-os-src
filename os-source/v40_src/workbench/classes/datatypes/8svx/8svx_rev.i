VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'29.4.93'
	ENDM
VERS	MACRO
		dc.b	'8svx 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'8svx 40.1 (29.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: 8svx 40.1 (29.4.93)',0
	ENDM
