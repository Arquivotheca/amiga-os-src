VERSION		EQU	36
REVISION	EQU	10
DATE	MACRO
		dc.b	'20.3.90'
	ENDM
VERS	MACRO
		dc.b	'Magic 36.10'
	ENDM
VSTRING	MACRO
		dc.b	'Magic 36.10 (20.3.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: Magic 36.10 (20.3.90)',0
	ENDM
