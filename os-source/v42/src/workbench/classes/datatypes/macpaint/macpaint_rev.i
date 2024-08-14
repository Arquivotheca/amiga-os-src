VERSION		EQU	42
REVISION	EQU	1
DATE	MACRO
		dc.b	'2.8.93'
	ENDM
VERS	MACRO
		dc.b	'macpaint 42.1'
	ENDM
VSTRING	MACRO
		dc.b	'macpaint 42.1 (2.8.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: macpaint 42.1 (2.8.93)',0
	ENDM
