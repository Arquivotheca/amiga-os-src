VERSION		EQU	1
REVISION	EQU	2
DATE	MACRO
		dc.b	'3.11.93'
	ENDM
VERS	MACRO
		dc.b	'MidiPlay 1.2'
	ENDM
VSTRING	MACRO
		dc.b	'MidiPlay 1.2 (3.11.93)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: MidiPlay 1.2 (3.11.93)',0
	ENDM