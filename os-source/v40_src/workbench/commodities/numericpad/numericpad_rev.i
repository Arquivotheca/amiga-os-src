VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'31.5.92'
	ENDM
VERS	MACRO
		dc.b	'numericpad 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'numericpad 39.1 (31.5.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: numericpad 39.1 (31.5.92)',0
	ENDM
