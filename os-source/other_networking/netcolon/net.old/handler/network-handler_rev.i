VERSION		EQU	37
REVISION	EQU	3
DATE	MACRO
		dc.b	'17.1.91'
	ENDM
VERS	MACRO
		dc.b	'network-handler 37.3'
	ENDM
VSTRING	MACRO
		dc.b	'network-handler 37.3 (17.1.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: network-handler 37.3 (17.1.91)',0
	ENDM
