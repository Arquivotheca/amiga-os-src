VERSION		EQU	37
REVISION	EQU	2
DATE	MACRO
		dc.b	'27.10.92'
	ENDM
VERS	MACRO
		dc.b	'a2060.device 37.2'
	ENDM
VSTRING	MACRO
		dc.b	'a2060.device 37.2 (27.10.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: a2060.device 37.2 (27.10.92)',0
	ENDM