VERSION		EQU	37
REVISION	EQU	1
DATE	MACRO
		dc.b	'11.3.92'
	ENDM
VERS	MACRO
		dc.b	'checkdisk 37.1'
	ENDM
VSTRING	MACRO
		dc.b	'checkdisk 37.1 (11.3.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: checkdisk 37.1 (11.3.92)',0
	ENDM
