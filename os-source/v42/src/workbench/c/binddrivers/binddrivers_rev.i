VERSION		EQU	42
REVISION	EQU	1
DATE	MACRO
		dc.b	'27.8.93'
	ENDM
VERS	MACRO
		dc.b	'binddrivers 42.1'
	ENDM
VSTRING	MACRO
		dc.b	'binddrivers 42.1 (27.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: binddrivers 42.1 (27.8.93)',0
	ENDM
