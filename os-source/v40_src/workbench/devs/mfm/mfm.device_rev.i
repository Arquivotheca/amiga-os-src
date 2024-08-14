VERSION		EQU	40
REVISION	EQU	9
DATE	MACRO
		dc.b	'21.5.93'
	ENDM
VERS	MACRO
		dc.b	'mfm.device 40.9'
	ENDM
VSTRING	MACRO
		dc.b	'mfm.device 40.9 (21.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: mfm.device 40.9 (21.5.93)',0
	ENDM
