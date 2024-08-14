;
; PSP definition
;
PSPdef  STRUC

	db	2ch DUP (?)
EnvSeg	dw	?
	db	2eh DUP (?)
FCB1	db	10h DUP (?)
FCB2	db      14h DUP (?)
CmdLen	db	?
CmdLin	db	7fh DUP (?)

PSPdef	ENDS



