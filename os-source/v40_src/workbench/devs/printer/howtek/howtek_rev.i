VERSION		EQU	35
REVISION	EQU	7
DATE	MACRO
		dc.b	'29.8.90'
	ENDM
VERS	MACRO
		dc.b	'howtek 35.7'
	ENDM
VSTRING	MACRO
		dc.b	'howtek 35.7 (29.8.90)',13,10,0
		ds.w	0
	ENDM
VERTAG	MACRO
		dc.b	0,'$VER: howtek 35.7 (29.8.90)',0
		ds.w	0
	ENDM
