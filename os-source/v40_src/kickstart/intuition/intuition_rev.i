VERSION		EQU	40
REVISION	EQU	85
DATE	MACRO
		dc.b	'5.5.93'
	ENDM
VERS	MACRO
		dc.b	'intuition 40.85'
	ENDM
VSTRING	MACRO
		dc.b	'intuition 40.85 (5.5.93)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: intuition 40.85 (5.5.93)',0
		ds.w	0
	ENDM
