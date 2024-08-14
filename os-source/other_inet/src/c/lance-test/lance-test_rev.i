VERSION		EQU	36
REVISION	EQU	2
DATE	MACRO
		dc.b	'4.12.90'
	ENDM
VERS	MACRO
		dc.b	'lance-test 36.2'
	ENDM
VSTRING	MACRO
		dc.b	'lance-test 36.2 (4.12.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: lance-test 36.2 (4.12.90)',0
	ENDM
