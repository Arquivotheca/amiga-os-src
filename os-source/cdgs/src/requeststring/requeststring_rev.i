VERSION		EQU	39
REVISION	EQU	5
DATE	MACRO
		dc.b	'13.7.93'
	ENDM
VERS	MACRO
		dc.b	'requeststring 39.5'
	ENDM
VSTRING	MACRO
		dc.b	'requeststring 39.5 (13.7.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: requeststring 39.5 (13.7.93)',0
	ENDM
