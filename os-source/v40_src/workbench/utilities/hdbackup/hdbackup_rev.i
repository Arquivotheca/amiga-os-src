VERSION		EQU	37
REVISION	EQU	8
DATE	MACRO
		dc.b	'24.4.91'
	ENDM
VERS	MACRO
		dc.b	'hdbackup 37.8'
	ENDM
VSTRING	MACRO
		dc.b	'hdbackup 37.8 (24.4.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: hdbackup 37.8 (24.4.91)',0
	ENDM