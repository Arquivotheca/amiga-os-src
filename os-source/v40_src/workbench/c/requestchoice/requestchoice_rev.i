VERSION		EQU	39
REVISION	EQU	4
DATE	MACRO
		dc.b	'13.8.92'
	ENDM
VERS	MACRO
		dc.b	'requestchoice 39.4'
	ENDM
VSTRING	MACRO
		dc.b	'requestchoice 39.4 (13.8.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: requestchoice 39.4 (13.8.92)',0
	ENDM
