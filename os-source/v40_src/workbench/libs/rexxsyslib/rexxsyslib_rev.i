VERSION		EQU	36
REVISION	EQU	23
DATE	MACRO
		dc.b	'17.4.91'
	ENDM
VERS	MACRO
		dc.b	'rexxsyslib 36.23'
	ENDM
VSTRING	MACRO
		dc.b	'rexxsyslib 36.23 (17.4.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rexxsyslib 36.23 (17.4.91)',0
	ENDM