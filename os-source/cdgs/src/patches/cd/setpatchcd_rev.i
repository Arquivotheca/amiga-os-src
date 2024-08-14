VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'3.5.93'
	ENDM
VERS	MACRO
		dc.b	'setpatchcd 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'setpatchcd 40.1 (3.5.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: setpatchcd 40.1 (3.5.93)',0
	ENDM
