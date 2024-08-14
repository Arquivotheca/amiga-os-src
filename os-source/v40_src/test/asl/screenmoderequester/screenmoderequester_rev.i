VERSION		EQU	38
REVISION	EQU	7
DATE	MACRO
		dc.b	'11.6.92'
	ENDM
VERS	MACRO
		dc.b	'screenmoderequester 38.7'
	ENDM
VSTRING	MACRO
		dc.b	'screenmoderequester 38.7 (11.6.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: screenmoderequester 38.7 (11.6.92)',0
	ENDM
