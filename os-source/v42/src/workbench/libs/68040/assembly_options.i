*******************************************************************************
*
* 68040 support library for 68881/68882 FPU emulation
*
* $Id: assembly_options.i,v 1.4 91/07/11 09:49:39 mks Exp $
*
* $Log:	assembly_options.i,v $
* Revision 1.4  91/07/11  09:49:39  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 1.3  91/07/02  15:57:26  mks
* Added two more flags for conditional assembly
*
* Revision 1.2  91/06/24  16:26:37  mks
* Set up to set options for 68040 CPU
*
* Revision 1.1  91/05/22  17:07:42  mks
* Initial revision
*
*******************************************************************************
*
* This file is included in every source file.  It makes sure that we can
* easily change the options needed to assemble the code.
*
* Options for HX68 to turn on 68040 mode...
*
		OPT	p=68040
*
*******************************************************************************
*
* Since the Amiga has a unified address space, we do not need to run COPYIN
* and COPYOUT or mem_read or mem_write.  Set this to use the AMIGA mode.
*
COPY_IN_OUT	SET	1
*
*******************************************************************************
*
* Define DEBUGGING for debugging of code...  (0=NO, 1=YES)
*
DEBUGGING	SET	0
*
*******************************************************************************
*
* A macro for PRINTF that does not touch the registers
* Also, it only produces code when DEBUGGING is defined above
*
		IFND	PRINTF
PRINTF		MACRO	; <string>,...
		IFNE	DEBUGGING
		XREF	KPrintF
PUSHCOUNT	SET	0

		IFNC	'\9',''
		move.l	\9,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\8',''
		move.l	\8,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\7',''
		move.l	\7,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\6',''
		move.l	\6,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\5',''
		move.l	\5,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\4',''
		move.l	\4,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\3',''
		move.l	\3,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\2',''
		move.l	\2,-(a7)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		movem.l a0/a1/d0/d1,-(a7)
		lea.l	PSS\@(pc),a0
		lea.l	16(a7),a1
		jsr	KPrintF
		movem.l (a7)+,a0/a1/d0/d1
		bra.s	PSE\@

PSS\@		dc.b	\1
		dc.b	10
		dc.b	0
		cnop	0,2
PSE\@
		IFNE	PUSHCOUNT
		lea.l	PUSHCOUNT(a7),a7
		ENDC	;IFNE	PUSHCOUNT
		ENDC	;IFD	DEBUGGING
		ENDM	;PRINTF	MACRO
		ENDC	;IFND	PRINTF
*
*******************************************************************************
