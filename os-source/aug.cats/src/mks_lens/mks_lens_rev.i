VERSION		EQU	36
REVISION	EQU	2
DATE	MACRO
		dc.b	'7.1.91'
	ENDM
VERS	MACRO
		dc.b	'mks_lens 36.2'
	ENDM
VSTRING	MACRO
		dc.b	'mks_lens 36.2 (7.1.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: mks_lens 36.2 (7.1.91)',0
	ENDM
