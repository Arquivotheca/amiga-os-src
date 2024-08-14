VERSION		EQU	39
REVISION	EQU	2
DATE	MACRO
		dc.b	'22.1.93'
	ENDM
VERS	MACRO
		dc.b	'asltester 39.2'
	ENDM
VSTRING	MACRO
		dc.b	'asltester 39.2 (22.1.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: asltester 39.2 (22.1.93)',0
	ENDM
