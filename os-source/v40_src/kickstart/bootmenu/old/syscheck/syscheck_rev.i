VERSION		EQU	36
REVISION	EQU	13
DATE	MACRO
		dc.b	'19 Jan 1990'
	ENDM
VERS	MACRO
		dc.b	'syscheck 36.13'
	ENDM
VSTRING	MACRO
		dc.b	'syscheck 36.13 (19 Jan 1990)',13,10,0
	ENDM
