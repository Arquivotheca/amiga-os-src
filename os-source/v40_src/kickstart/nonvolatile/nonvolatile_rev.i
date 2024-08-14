VERSION		EQU	40
REVISION	EQU	36
DATE	MACRO
		dc.b	'14.9.93'
	ENDM
VERS	MACRO
		dc.b	'nonvolatile 40.36'
	ENDM
VSTRING	MACRO
		dc.b	'nonvolatile 40.36 (14.9.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nonvolatile 40.36 (14.9.93)',0
	ENDM
