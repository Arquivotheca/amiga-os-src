VERSION		EQU	1
REVISION	EQU	0
DATE	MACRO
		dc.b	'9.7.92'
	ENDM
VERS	MACRO
		dc.b	'configinet 1.0'
	ENDM
VSTRING	MACRO
		dc.b	'configinet 1.0 (9.7.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: configinet 1.0 (9.7.92)',0
	ENDM
