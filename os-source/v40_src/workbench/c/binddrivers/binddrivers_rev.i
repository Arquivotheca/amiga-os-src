VERSION		EQU	38
REVISION	EQU	2
DATE	MACRO
		dc.b	'31.3.92'
	ENDM
VERS	MACRO
		dc.b	'binddrivers 38.2'
	ENDM
VSTRING	MACRO
		dc.b	'binddrivers 38.2 (31.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: binddrivers 38.2 (31.3.92)',0
	ENDM
