VERSION		EQU	36
REVISION	EQU	170
DATE	MACRO
		dc.b	'19.4.90'
	ENDM
VERS	MACRO
		dc.b	'bootmenu 36.170'
	ENDM
VSTRING	MACRO
		dc.b	'bootmenu 36.170 (19.4.90)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: bootmenu 36.170 (19.4.90)',0
	ENDM