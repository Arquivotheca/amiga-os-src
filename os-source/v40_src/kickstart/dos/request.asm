*
* This module implements the Requestor function for DOS.
* It also provides an OpenWindow call.
* 
* It defines BCPL callable functions with the following syntax
*
*    Bool := requestor ( line1, line2, line3 )
*    Bool := requestor2( line1, line2, line3, IDCMPFlags )
*
* The three lines are displayed within an intuition requestor
* box. Two gadgets are also presented, labelled Retry and Cancel
* This call requires about 2K of stack, which is obtained dynamically
* from the free pool. If there is no stack left it returns FALSE anyway.
*
* Changed to only allocating stack if there is less than 2K available.
* If called from BCPL it forces allocation.
*
* Change: we now allow EasyRequest to do the stack magic.  This _may_ cause
* problems with BCPL programs with small stacks.
*
*    Window := OpenWindow( Xpos, Ypos, Width, Height, StripName )
*
* The window specified is opened.
*
* Also defined is XRequest(buttonstring,linearray,IDCMPFlags)
* This allows more buttons with different names, and more lines of text.
*  

	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/execbase.i"
	INCLUDE	"intuition/intuition.i"

	INCLUDE	"dos/dos.i"
	INCLUDE	"dos/dosextens.i"
	
	INCLUDE	"libhdr.i"
	INCLUDE "fault.i"

FUNCDEF	MACRO
	XREF	_LVO\1
	ENDM

	INCLUDE "exec/exec_lib.i"

	XREF	_LVOOpenWindow
	XREF	_LVOEasyRequestArgs
	XREF	@getstring

*
* External definitions
*
	XDEF	@requester
	XDEF	_requester
	XDEF	@requester2
	XDEF	_requester2
	XDEF	@openwindow
	XDEF	REQ0
	XDEF	REQUEST2

* Version 1.0 TJK July 11th 85
* Version 1.1 TJK August 25th 85
* Version 1.2 NHG September 9th 86
* revised to use easyrequest REJ

ABSEXECBASE	EQU	4

* minimum stack required for requester
*
*
* args in d1/d2/d3/d4/ - d1-d3 are char *ptrs, d4 is LONG
*

REQ0
	MOVE.L	#DISKINSERTED,D4
REQUEST2
	ADDQ.L	#1,D1		; null-terminated BSTRs!
	ADDQ.L	#1,D2		; (fakeblib/calldos shifted them)
	ADDQ.L	#1,D3
	bra.s	@requester2	; calldos doesn't care if d2-d4 are trashed!
				; called with C conventions by calldos

@requester
_requester
	MOVE.L	#DISKINSERTED,D4		set DISKINSERTED flag
@requester2
_requester2
	MOVEM.L D2-D7/A2-A6,-(SP)		Save regs
	movem.l	d1-d3/d5,-(sp)			; array of strings for EasyRequest
	MOVE.L	SP,A3				; addr of args for ezreq
	MOVE.L  ABSEXECBASE,A6 	       		Exec pointer
	MOVEQ	#0,D6				And set result to FALSE
	moveq	#0,d7			; use window title by default
*
	MOVE.L	ThisTask(a6),a0			Find my task
	MOVE.L  pr_WindowPtr(A0),a4	        Window Pointer
	MOVE.L	a4,d1
	ADDQ.L	#1,D1
	BEQ     EXIT1				== -1, return FALSE

	move.l  a7,d5				; save old SP in D5

* get intuitionbase
	move.l	pr_GlobVec(a0),a0
	move.l	G_INTUITION(a0),a0	; intuitionbase

* Build intuition arguments (struct EasyStruct, and registers)
*
* Handle three arguments
					; a3 has ptr to arg list (3 strings)
					; 2nd string may be NULL?! FIX!
					; they are followed by the longword
					; errorcode for Abort()
	move.l	a6,a5			; save execbase
	move.l	a0,a6			; ibase in a6

	;-- 2nd argument may be NULL - reorder and use different string
	;-- apparently 3rd can be null also (at least in the old code)
	lea	dos_formatstr(pc),a2	; "%s\n%s\n%s"
	tst.l	8(a3)			; is 3rd arg NULL?
	bne.s	arg3_not_null
	addq.w	#3,a2			; skip over a '%s/n'
arg3_not_null:
	tst.l	4(a3)			; is 2nd arg NULL?
	bne.s	arg2_not_null
	move.l	8(a3),4(a3)		; won't hurt our reg return
					; (we do this because #3 might not be 0)
	addq.w	#3,a2			; skip over a '%s/n'

arg2_not_null:
	moveq	#STR_RETRY_CANCEL,d1
	btst.l	#14,d4			; check for NEWPREFS IDCMP
					; since this no longer is generated,
					; and noone waits on it anyways, it
					; means that we chould use the Reboot|	
					; Suspend.
	beq.s	use_retry
	bclr.l	#14,d4			; remove it just in case
	lea	dos_errorstr(pc),a2	; "%s\n%s%ld)\n%s"
	move.l	8(a3),d0		; swap string 3 and error code
	move.l	12(a3),8(a3)
	move.l	d0,12(a3)
	move.l	#STR_SOFTWARE_FAILURE,d1
	bsr	@getstring		; window title Software Failure
	move.l	d0,d7
	move.l	#STR_REBOOT_SUSPEND,d1

use_retry:
	bsr	@getstring
	move.l	d0,d6			; ptr to "Retry|Cancel"
	moveq	#es_SIZEOF,d0
	sub.l	d0,sp			; space for easystruct on stack
	move.l	sp,a1			; register for call
	move.l	a1,a0			; must leave a1 alone
	move.l	d0,(a0)+		; es_StructSize (=es_SIZEOF)
	clr.l	(a0)+			; es_Flags
	move.l	d7,(a0)+		; es_Title (use window title)
	move.l	a2,(a0)+		; es_TextFormat (%s, etc)
	move.l	d6,(a0)			; es_GadgetFormat
	move.l	d4,-(sp)		; save idcmp flags on stack 
					; ER() may modify this value!
	move.l	sp,a2			; argument to ER()
	move.l  a4,a0			; Window Pointer (argument)

* Well thats finished. Try calling intuition at last.
	jsr	_LVOEasyRequestArgs(a6)
	move.l	d0,d6			; result
	bpl.s	selected_gadget

* finished by IDCMP, must be a retry
	moveq	#1,d6

* Get execbase back in a6
selected_gadget:
	move.l a5,a6			; Get ExecBase

* test to see if the requester aborted due to lack of memory (suspend only)
	tst.l	d7			; 0 for retry/cancel
	beq.s	EXIT0			; not suspend/cancel

	move.l	ThisTask(a6),a0
	move.l	pr_Result2(a0),d0
	beq.s	EXIT0			; no error

* Ran out of memory, select retry instead of cancel
	moveq	#1,d6

* Give back stack space used
EXIT0
	MOVE.L	D5,SP				Get back previous SP

* Restore registers and return
EXIT1
	MOVE.L	D6,D0				result
	LEA	4*4(sp),sp		; pop off the 3 string ptrs + error code
	MOVEM.L (SP)+,D2-D7/A2-A6
	RTS
*
* The gadget text is stored here
* 
dos_formatstr	dc.b	'%s',$a,'%s',$a,'%s',0
dos_errorstr	dc.b	'%s',$a,'%s%lx).',$a,'%s',0

	CNOP    0,2
	PAGE
**********************************************************************
*                                                                    *
*       OpenWindow(Xpos,Ypos,Width,Height,StripName)                 *
*                                                                    *
*    Returns a window pointer obtained from Intuition.               *
*    No checks are made on whether the arguments are valid.          *
*                                                                    *
* BCPL-only routine						     *
* Note that stripname MUST have null-termination when called from    *
* BCPL (and always has been this way).				     *
* counts on BCPL interface to move 5th parm into D5		     *
* Calldos guarantees a2 == globvec on entry			     *
**********************************************************************
OW_TYPE EQU	1
OW_MAXW	EQU	$FFFF
OW_MAXH EQU	$FFFF
OW_MINW	EQU	50
OW_MINH	EQU	120
*
OW_FLGS EQU     WINDOWSIZING!WINDOWDRAG!WINDOWDEPTH!ACTIVATE!NOCAREREFRESH
*
@openwindow:
	MOVE.L  A6,-(SP)                Save regs (d5/d6 not saved, bcpl only)

	MOVEQ.L #-1,D6      		Pens
* Start constructing parameter area on stack
	MOVE.W  #OW_TYPE,-(SP)		Type
	MOVE.W	d6,-(SP)		Window bounds
	MOVE.W	d6,-(SP)		; d6 is -1
	MOVE.W	#OW_MINW,-(SP)
	MOVE.W	#OW_MINH,-(SP)
	CLR.L   -(SP)			Bitmap
	CLR.L   -(SP)			Screen
	MOVE.L  D5,-(SP)		Next arg is name
	CLR.L	-(SP)			Check
	CLR.L	-(SP)			Gadget
	MOVE.L  #OW_FLGS,-(SP)		Flags
	CLR.L	-(SP)			idcmp
	MOVE.W	D6,-(SP)		Pens
	MOVEM.W D1-D4,-(SP)		Xpos/Ypos/Width/Height
	MOVEA.L G_INTUITION(A2),A6      Get intuition library pointer
	MOVEA.L SP,A0                   Into A0
	JSR	_LVOOpenWindow(A6)      Call Intuition
	LEA.L   nw_SIZE(SP),SP          Restore stack
        MOVE.L  (SP)+,A6                Restore regs
	RTS
