VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'9.3.93'
	ENDM
VERS	MACRO
		dc.b	'fstool 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'fstool 37.1 (9.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: fstool 37.1 (9.3.93)',0
	ENDM
