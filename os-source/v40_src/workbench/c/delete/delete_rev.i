VERSION		EQU	37
REVISION	EQU	8
DATE	MACRO
		dc.b	'6.6.91'
	ENDM
VERS	MACRO
		dc.b	'delete 37.8'
	ENDM
VSTRING	MACRO
		dc.b	'delete 37.8 (6.6.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: delete 37.8 (6.6.91)',0
	ENDM
