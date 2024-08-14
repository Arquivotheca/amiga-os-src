VERSION		EQU	40
REVISION	EQU	2
DATE	MACRO
		dc.b	'17.3.94'
	ENDM
VERS	MACRO
		dc.b	'nofastmem 40.2'
	ENDM
VSTRING	MACRO
		dc.b	'nofastmem 40.2 (17.3.94)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: nofastmem 40.2 (17.3.94)',0
	ENDM
