**
**      $Id: assembly.i,v 39.2 92/06/08 15:52:08 mks Exp $
**
**	Private include file sucked in by _all_ Exec assembly files.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**

; IDNT	exec
; must go before EQU's  (comments after SECTION break)
	SECTION exec

NO		EQU     0
OFF		EQU     0
ON		EQU     1
YES		EQU     1
TRUE		EQU	-1
FALSE		EQU	0
NULL		EQU	0

;--Assemble time switches

DEVELOPMENT	EQU OFF	;development features-OFF FOR RELEASE!
INFODEPTH	SET 0	;detail level of debugging-0 FOR RELEASE!
AZ_LYZER	SET 0	;detail level for analyzer support-0 FOR RELEASE!

TORTURE_TEST	EQU NO	;General torture.
TORTURE_TEST_0	EQU NO	;Location zero.
TORTURE_TEST_1	EQU NO	;Memmung.
TORTURE_TEST_2	EQU NO	;Include stringent error checking?
TORTURE_TEST_3	EQU NO	;Include stringent error checking?
IO_TORTURE	EQU NO	;Enable special IO checks?
HARDTESTS	EQU NO	;power-on hardware tests

ABSEXECBASE	EQU 4

*------ Special Externals:

EXTERN_BASE MACRO
	    ENDM

EXTERN_SYS  MACRO
            XREF    _LVO\1
            ENDM

EXTERN_CODE MACRO
            XREF    \1
            ENDM

EXTERN_DATA MACRO
            XREF    \1
            ENDM

EXTERN_STR  MACRO
            XREF    \1
            ENDM



*------ pseudo-instruction macros:

CLEAR       MACRO
            MOVEQ.L #0,\1
            ENDM

EVEN	    MACRO
	    ds.w 0
	    ENDM

BLINK		MACRO
		bchg.b	#1,$bfe001
		ENDM

*************************************************************************
*
* Handy macros for writing Debug strings
* Bryce Nesbitt, 12-29-88
*
*************************************************************************

	IFNE	INFODEPTH
	IFND	RAWDOFMTKLUDGE
		XREF	RawDoFmt
	ENDC
	ENDC	;damn metacomco bugs.... grumble.


NPRINTF		MACRO	; level,<string>,...
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

		BSR	printit
		bra.s	PSE\@

		dc.b	\2
		IFEQ	(\1&1)	;If even, add CR/LF par...
		   dc.b	13,10
		ENDC
		dc.b	0
		ds.w	0
PSE\@
		add	#PUSHCOUNT,sp		;point optimizer at it!
		ENDC
		ENDM



;Special for BusErrorHandler
ZPRINTF		MACRO	; level,<string>,...
		XREF	PSCODE
		IFGE	INFODEPTH-\1

		movem.l A0/A1/A2/A3/D0/D1,-(SP)	;Note calculation below
		lea.l	PSS\@(pc),A0
		lea.l	\3,A1
		lea.l	PSCODE,a2		;Address of output function
		suba.l	a3,a3			;No data pointer
		BSR	RawDoFmt
		movem.l (SP)+,A0/A1/A2/A3/D0/D1
		bra.s	PSE\@

PSS\@		dc.b	\2
		IFEQ	(\1&1)	;If even, add CR/LF par...
		   dc.b	13,10
		ENDC
		dc.b	0
		ds.w	0
PSE\@
		ENDC
		ENDM


;
; Write to location $2FC, a trigger for the analyzer (switched to longword
; because of 68020/30 analyzer limitations.
;
AZ_TRIG		MACRO	;level
	        IFGE   AZ_LYZER-\1
		  MOVE.L #$55555555,$2FC
		ENDC
		ENDM


;
; Write to location $2FC, a trigger for the analyzer.  Then stop the CPU!
;
AZ_STOP		MACRO	;level
	        IFGE   AZ_LYZER-\1
		  NOP
		  MOVE.L #$55555555,$2FC
		  STOP	#$2000
		  NOP
		ENDC
		ENDM


