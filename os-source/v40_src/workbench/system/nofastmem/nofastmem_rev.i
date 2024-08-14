VERSION		EQU	38
REVISION	EQU	4
DATE	MACRO
		dc.b	'22.5.92'
	ENDM
VERS	MACRO
		dc.b	'nofastmem 38.4'
	ENDM
VSTRING	MACRO
		dc.b	'nofastmem 38.4 (22.5.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nofastmem 38.4 (22.5.92)',0
	ENDM
