VERSION		EQU	0
REVISION	EQU	1
DATE	MACRO
		dc.b	'8.1.93'
	ENDM
VERS	MACRO
		dc.b	'disk_dev 0.1'
	ENDM
VSTRING	MACRO
		dc.b	'disk_dev 0.1 (8.1.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: disk_dev 0.1 (8.1.93)',0
	ENDM
