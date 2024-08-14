VERSION		EQU	39
REVISION	EQU	2
DATE	MACRO
		dc.b	'30.5.92'
	ENDM
VERS	MACRO
		dc.b	'requestfile 39.2'
	ENDM
VSTRING	MACRO
		dc.b	'requestfile 39.2 (30.5.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: requestfile 39.2 (30.5.92)',0
	ENDM
