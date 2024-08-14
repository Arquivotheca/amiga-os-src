VERSION		EQU	35
REVISION	EQU	33
DATE	MACRO
		dc.b	'15.1.92'
	ENDM
VERS	MACRO
		dc.b	'deskjetOld 35.33'
	ENDM
VSTRING	MACRO
		dc.b	'deskjetOld 35.33 (15.1.92)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: deskjetOld 35.33 (15.1.92)',0
		ds.w	0
	ENDM
