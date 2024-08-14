VERSION		EQU	40
REVISION	EQU	1
DATE	MACRO
		dc.b	'12.3.93'
	ENDM
VERS	MACRO
		dc.b	'trackdisk 40.1'
	ENDM
VSTRING	MACRO
		dc.b	'trackdisk 40.1 (12.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: trackdisk 40.1 (12.3.93)',0
	ENDM
