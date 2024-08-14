VERSION		EQU	1
REVISION	EQU	24
DATE	MACRO
		dc.b	'1.9.92'
	ENDM
VERS	MACRO
		dc.b	'installer 1.24'
	ENDM
VSTRING	MACRO
		dc.b	'installer 1.24 (1.9.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: installer 1.24 (1.9.92)',0
	ENDM
