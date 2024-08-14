VERSION		EQU	37
REVISION	EQU	7
DATE	MACRO
		dc.b	'23.11.92'
	ENDM
VERS	MACRO
		dc.b	'envoyprint 37.7'
	ENDM
VSTRING	MACRO
		dc.b	'envoyprint 37.7 (23.11.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: envoyprint 37.7 (23.11.92)',0
	ENDM
