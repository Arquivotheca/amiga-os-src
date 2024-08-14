VERSION		EQU	39
REVISION	EQU	4
DATE	MACRO
		dc.b	'29.7.92'
	ENDM
VERS	MACRO
		dc.b	'sendreport 39.4'
	ENDM
VSTRING	MACRO
		dc.b	'sendreport 39.4 (29.7.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: sendreport 39.4 (29.7.92)',0
	ENDM
