VERSION		EQU	1
REVISION	EQU	4
DATE	MACRO
		dc.b	'8.4.93'
	ENDM
VERS	MACRO
		dc.b	'KBInt 1.4'
	ENDM
VSTRING	MACRO
		dc.b	'KBInt 1.4 (8.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: KBInt 1.4 (8.4.93)',0
	ENDM