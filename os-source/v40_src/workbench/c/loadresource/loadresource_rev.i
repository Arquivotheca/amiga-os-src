VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'17.3.93'
	ENDM
VERS	MACRO
		dc.b	'loadresource 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'loadresource 40.2 (17.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: loadresource 40.2 (17.3.93)',0
	ENDM