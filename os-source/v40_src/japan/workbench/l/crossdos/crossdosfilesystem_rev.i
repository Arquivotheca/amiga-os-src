VERSION		EQU	39
REVISION	EQU	1
DATE	MACRO
		dc.b	'23.4.93'
	ENDM
VERS	MACRO
		dc.b	'CrossDOSFileSystem 39.1'
	ENDM
VSTRING	MACRO
		dc.b	'CrossDOSFileSystem 39.1 (23.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: CrossDOSFileSystem 39.1 (23.4.93)',0
	ENDM
