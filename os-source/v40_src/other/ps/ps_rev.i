VERSION		EQU	36
REVISION	EQU	2
DATE	MACRO
		dc.b	'27 Jun 1991'
	ENDM
VERS	MACRO
		dc.b	'ps 36.2'
	ENDM
VSTRING	MACRO
		dc.b	'ps 36.2 (27 Jun 1991)',13,10,0
	ENDM
