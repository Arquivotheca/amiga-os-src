VERSION		EQU	1
REVISION	EQU	3
DATE	MACRO
		dc.b	'5.3.93'
	ENDM
VERS	MACRO
		dc.b	'TimerInt 1.3'
	ENDM
VSTRING	MACRO
		dc.b	'TimerInt 1.3 (5.3.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: TimerInt 1.3 (5.3.93)',0
	ENDM
