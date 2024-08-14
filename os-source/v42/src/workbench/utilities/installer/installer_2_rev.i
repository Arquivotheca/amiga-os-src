;
;		installer_2_rev.i
;
;



VERSION		EQU	2
REVISION	EQU	16
DATE	MACRO
		dc.b	'1.9.92'
	ENDM
VERS	MACRO
		dc.b	'installer_2 2.16'
	ENDM
VSTRING	MACRO
		dc.b	'installer_2 2.16 (1.9.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: installer_2 2.16 (1.9.92)',0
	ENDM
