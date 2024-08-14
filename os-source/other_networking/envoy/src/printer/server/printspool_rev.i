VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'12.8.93'
	ENDM
VERS	MACRO
		dc.b	'printspool 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'printspool 40.2 (12.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: printspool 40.2 (12.8.93)',0
	ENDM
