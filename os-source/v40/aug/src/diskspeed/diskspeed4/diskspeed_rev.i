VERSION		EQU	4
REVISION	EQU	2
DATE	MACRO
		dc.b	'17.5.92'
	ENDM
VERS	MACRO
		dc.b	'DiskSpeed 4.2'
	ENDM
VSTRING	MACRO
		dc.b	'DiskSpeed 4.2 (17.5.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: DiskSpeed 4.2 (17.5.92)',0
	ENDM
