**
** $Id: wbtag.asm,v 37.3 92/04/20 10:14:20 mks Exp $
**
** $Log:	wbtag.asm,v $
*   Revision 37.3  92/04/20  10:14:20  mks
*   Cleaned up and now uses TaggedOpenLibrary()
*   
*   Revision 37.2  91/01/16  10:41:41  mks
*   Cleaned up and shortened WBTag since icon.library is now in ROM
*
*   Revision 37.1  91/01/13  21:12:02  mks
*   Updated for V37
*
*   Revision 36.10  90/12/08  16:04:01  mks
*   Fixed spelling of the tag string
*

	SECTION	wbtag,code

**	Included Files

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE "exec/resident.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"internal/librarytags.i"
	LIST

	INCLUDE	"wbtag_rev.i"

	INCLUDE	"wbtag.i"

ABSEXECBASE	EQU	4

XLVO	MACRO	(function)
	XREF	_LVO\1
	ENDM

CALLLVO	MACRO	(function)
	jsr	_LVO\1(a6)
	ENDM

	XLVO	CloseLibrary
	XLVO	FindName
	XLVO	GetMsg
	XLVO	TaggedOpenLibrary
	XLVO	ReplyMsg
	XLVO	WaitPort

	XLVO	StartWorkbench


		dc.w	0		; longword align pseudo segments below
ResidentTag:
					; STRUCTURE RT,0
		dc.w	RTC_MATCHWORD	;    UWORD RT_MATCHWORD
		dc.l	ResidentTag	;    APTR  RT_MATCHTAG
		dc.l	EndTag		;    APTR  RT_ENDSKIP
		dc.b	0		;    BYTE  RT_FLAGS
		dc.b	VERSION		;    UBYTE RT_VERSION
		dc.b	NT_TASK		;    UBYTE RT_TYPE
		dc.b	-120		;    BYTE  RT_PRI
		dc.l	WTName		;    APTR  RT_NAME
		dc.l	WTID		;    APTR  RT_IDSTRING
		dc.l	segment		;    APTR  RT_INIT
					;    LABEL RT_SIZE
segment:
		; segment structure for v1.1 'loadwb' entry point.
		dc.l	0
		moveq	#0,d2
		bra.s	jmpstart

		; segment structure for v1.1 'loadwb -debug' entry point.
		dc.l	0
		moveq	#1,d2
		bra.s	jmpstart

		; segment structure for >= v1.2 'loadwb' entry point.
		dc.l	0
		moveq	#2,d2
		; fall thru to jmpstart

;------ Workbench tag code -------------------------------------------
;
;   d2	which loadwb started us flag (0, 1, or 2)
;   a2	current Process
;   a3	LoadWB argument message
;   a6	current library base
;
jmpstart:
		movem.l	d2/a2-a3/a6,-(a7)
		move.l	ABSEXECBASE,a6

	    ;-- set this process/task name
		move.l	ThisTask(a6),a2
		lea	WTName(pc),a0
		move.l	a0,LN_NAME(a2)

		; this fixes the loadwbV1.1 does not work under V2.0 bug
		; note - we can branch directly to 'openWB' here since the
		; code after the 'WaitPort' and 'GetMsg' is special LoadWBV1.3
		; code which we can ignore since we know we were fired up via
		; LoadWBV1.1 or earlier.
		cmpi	#2,d2	; expecting a message?
		bne.s	openWB	; no, so do not wait for one.

	    ;-- get the argument message from LoadWB
		lea	pr_MsgPort(a2),a0
		CALLLVO	WaitPort
		lea	pr_MsgPort(a2),a0
		CALLLVO	GetMsg
		move.l	d0,a3


	    ;-- try to find a proper workbench library
openWB:
		moveq.l	#OLTAG_WORKBENCH,d0
		CALLLVO	TaggedOpenLibrary
		tst.l	d0
		beq.s	noWB

	    ;-- pass argument message to workbench startup function
		move.l	d0,a6

		;-- call StartWorkbench
		cmpi	#2,d2	; did we get a msg?
		beq.s	gotmsg	; yes, so send it to StartWorkbench
		move	d2,d0	; arg is either 0(loadwb) or 1(loadwb -debug)
		moveq	#0,d1	; null path specification
		bra.s	nomsg
gotmsg:
		move.l	lm_Arg(a3),d0
		move.l	lm_Path(a3),d1
nomsg:
		CALLLVO	StartWorkbench

		move.l	a6,a1
		move.l	ABSEXECBASE,a6
		CALLLVO	CloseLibrary

noWB:
		cmpi	#2,d2	; did we get a msg?
		bne.s	noreply	; no, so do not reply to it

	    ;-- reply the message to allow LoadWB to terminate
		move.l	a3,a1
		CALLLVO	ReplyMsg
noreply:
	    ;-- kill this process and terminate
		movem.l	(a7)+,d2/a2-a3/a6
		rts

WTName		dc.b	'workbench.task',0
WTID		VSTRING
EndTag:

	END
