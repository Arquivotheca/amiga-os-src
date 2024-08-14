VERSION		EQU	36
REVISION	EQU	5
DATE	MACRO
		dc.b	'4.12.90'
	ENDM
VERS	MACRO
		dc.b	'mouseserv 36.5'
	ENDM
VSTRING	MACRO
		dc.b	'mouseserv 36.5 (4.12.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: mouseserv 36.5 (4.12.90)',0
	ENDM
