VERSION		EQU	2
REVISION	EQU	411
DATE	MACRO
		dc.b	'4.2.92'
	ENDM
VERS	MACRO
		dc.b	'PCWindow 2.411'
	ENDM
VSTRING	MACRO
		dc.b	'PCWindow 2.411 (4.2.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: PCWindow 2.411 (4.2.92)',0
	ENDM
