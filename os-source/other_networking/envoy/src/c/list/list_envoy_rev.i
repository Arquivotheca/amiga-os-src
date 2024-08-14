VERSION		EQU	37
REVISION	EQU	9
DATE	MACRO
		dc.b	'13.10.92'
	ENDM
VERS	MACRO
		dc.b	'list_envoy 37.9'
	ENDM
VSTRING	MACRO
		dc.b	'list_envoy 37.9 (13.10.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: list_envoy 37.9 (13.10.92)',0
	ENDM
