VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'15.9.93'
	ENDM
VERS	MACRO
		dc.b	'TalkCD 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'TalkCD 40.1 (15.9.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: TalkCD 40.1 (15.9.93)',0
	ENDM
