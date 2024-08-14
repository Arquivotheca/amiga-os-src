VERSION		EQU	2
REVISION	EQU	2
DATE	MACRO
		dc.b	'19.10.92'
	ENDM
VERS	MACRO
		dc.b	'showmount 2.2'
	ENDM
VSTRING	MACRO
		dc.b	'showmount 2.2 (19.10.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: showmount 2.2 (19.10.92)',0
	ENDM
