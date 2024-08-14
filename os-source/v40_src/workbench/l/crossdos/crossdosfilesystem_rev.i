REVISION	EQU	18
VSTRING	MACRO
		dc.b	'CrossDOSFileSystem '
    ifd V33
VERSION		EQU	33
        dc.b     '33'
    else
VERSION		EQU	40
        dc.b     '40'
    endc
        dc.b     '.18 (19.5.93)',13,10,0
    ENDM
