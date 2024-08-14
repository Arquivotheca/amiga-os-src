*************************************************************************
*
* Handy macros for writing Debug strings
* Bryce Nesbitt, 12-29-88
*
*************************************************************************


PRINTF	       MACRO	; level,<string>,...
	XREF	printit
	       IFGE	INFODEPTH-\1
PUSHCOUNT	SET	0

		IFNC	'\9',''
		move.l	\9,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\8',''
		move.l	\8,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\7',''
		move.l	\7,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\6',''
		move.l	\6,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\5',''
		move.l	\5,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\4',''
		move.l	\4,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\3',''
		move.l	\3,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		movem.l a0/a1,-(sp)
		lea.l	PSS\@(pc),A0
		lea.l	4*2(SP),A1
		BSR	printit
		movem.l (sp)+,a0/a1
		bra.s	PSE\@

PSS\@		dc.b	\2
		IFEQ	(\1&1)  ;If even, add CR/LF par...
		   dc.b 10,13
		ENDC
		dc.b	0
		ds.w	0
PSE\@
		adda.l	#PUSHCOUNT,sp
	       ENDC
	       ENDM

