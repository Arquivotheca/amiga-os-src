VERSION		EQU	40
REVISION	EQU	14
DATE	MACRO
		dc.b	'9.12.93'
	ENDM
VERS	MACRO
		dc.b	'videocd 40.14'
	ENDM
VSTRING	MACRO
		dc.b	'videocd 40.14 (9.12.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: videocd 40.14 (9.12.93)',0
	ENDM
