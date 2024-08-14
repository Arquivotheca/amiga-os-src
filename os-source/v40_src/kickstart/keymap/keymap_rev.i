VERSION		EQU	40
REVISION	EQU	4
DATE	MACRO
		dc.b	'12.3.93'
	ENDM
VERS	MACRO
		dc.b	'keymap 40.4'
	ENDM
VSTRING	MACRO
		dc.b	'keymap 40.4 (12.3.93)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: keymap 40.4 (12.3.93)',0
		ds.w	0
	ENDM
