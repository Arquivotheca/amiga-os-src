VERSION		EQU	2
REVISION	EQU	9
DATE	MACRO
		dc.b	'22.2.93'
	ENDM
VERS	MACRO
		dc.b	'ftp 2.9'
	ENDM
VSTRING	MACRO
		dc.b	'ftp 2.9 (22.2.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: ftp 2.9 (22.2.93)',0
	ENDM