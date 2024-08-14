VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'31.3.92'
	ENDM
VERS	MACRO
		dc.b	'screensaver 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'screensaver 39.1 (31.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: screensaver 39.1 (31.3.92)',0
	ENDM
