VERSION		EQU	42
REVISION	EQU	1
DATE	MACRO
		dc.b	'19.7.93'
	ENDM
VERS	MACRO
		dc.b	'blanker 42.1'
	ENDM
VSTRING	MACRO
		dc.b	'blanker 42.1 (19.7.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: blanker 42.1 (19.7.93)',0
	ENDM