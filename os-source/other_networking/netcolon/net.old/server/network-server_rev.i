VERSION		EQU	37
REVISION	EQU	7
DATE	MACRO
		dc.b	'21.1.91'
	ENDM
VERS	MACRO
		dc.b	'network-server 37.7'
	ENDM
VSTRING	MACRO
		dc.b	'network-server 37.7 (21.1.91)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: network-server 37.7 (21.1.91)',0
	ENDM
