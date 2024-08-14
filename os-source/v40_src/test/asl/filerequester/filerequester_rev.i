VERSION		EQU	38
REVISION	EQU	4
DATE	MACRO
		dc.b	'30.6.92'
	ENDM
VERS	MACRO
		dc.b	'filerequester 38.4'
	ENDM
VSTRING	MACRO
		dc.b	'filerequester 38.4 (30.6.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: filerequester 38.4 (30.6.92)',0
	ENDM
