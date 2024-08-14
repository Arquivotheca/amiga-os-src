VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'1.5.91'
	ENDM
VERS	MACRO
		dc.b	'changedefault 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'changedefault 37.1 (1.5.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: changedefault 37.1 (1.5.91)',0
	ENDM
