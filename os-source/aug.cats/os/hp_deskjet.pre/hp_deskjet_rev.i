VERSION		EQU	37
REVISION	EQU	4
DATE	MACRO
		dc.b	'1.4.93'
	ENDM
VERS	MACRO
		dc.b	'hp_deskjet 37.4'
	ENDM
VSTRING	MACRO
		dc.b	'hp_deskjet 37.4 (1.4.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: hp_deskjet 37.4 (1.4.93)',0
	ENDM
