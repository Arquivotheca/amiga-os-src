
	INCLUDE "exec/types.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "dos/dos.i"
	INCLUDE "dos/dosextens.i"

	XREF	_LVOTypeOfMem
	XDEF	_InProcSegList
	XDEF	_InThisSegList

* prototypes for C code
*
* #define ASM __asm
* #define REG(x) register __## x
* extern BOOL ASM InThisSegList(REG(a0) BPTR seglist, REG(a1) VOID *address, REG(a2) ULONG *hunkp, REG(a3) ULONG *offsp, REG(a6) struct Library *SysBase);
* extern BOOL ASM InProcSegList(REG(a0) struct Task *task, REG(a1) VOID *address, REG(a2) ULONG *hunkp, REG(a3) ULONG *offsp, REG(a6) struct Library *SysBase);
*


*
* Now for the hunk-o-matics...
*
* BOOL InThisSegList(a0=seglist, a1=address, a2=ULONG *hunkp, a3=ULONG *offsp, a6=SysBase)
*
*   	At entry:	
*	a0=SegList
*	a1=address we are trying to find in seglist
*	a2=address of ULONG where hunk number may be stored
*	a3=address of ULONG where hunk offset may be stored
*
*	d0 return: TRUE = found, FALSE = not found.  If FALSE
*	and first passed ULONG returned not 0, means invalid seglist
*
_InThisSegList
		movem.l d2-d6/a2-a6,-(sp)
		moveq.l	#0,d5			; No seglist yet...
		move.l	#0,0(a2)		; no hunk or offset yet	
		move.l	#0,0(a3)
		move.l	a1,d3			; where this code wants address
		move.l	a0,d5			; where this code wants seglist
		beq	nd_InvalidSeg
		bra	nd_HaveSeg

*
* BOOL InProcSegList(a0=process, a1=address, a2=ULONG *hunkp, a3=ULONG *offsp, a6=SysBase)
*
*   	At entry:	
*	a0=Process (we will verify as Process)
*	a1=address we are trying to find in seglist
*	a2=address of ULONG where hunk number may be stored
*	a3=address of ULONG where hunk offset may be stored
*
*	d0 return: TRUE = found, FALSE = not found.  If FALSE
*	and first passed ULONG returned not 0, means invalid seglist
*
_InProcSegList
		movem.l d2-d6/a2-a6,-(sp)
		moveq.l	#0,d5			; No seglist yet...
		move.l	#0,0(a2)		; no hunk or offset yet	
		move.l	#0,0(a3)
		move.l	a1,d3			; where this code wants the address

		cmpi.b	#NT_PROCESS,LN_TYPE(a0) ; process ?
		bne.s	nd_HunkNone		; no

		move.l	pr_CLI(a0),d0		; Check if we have a CLI...
		beq.s	nd_NoCLISeg		; No CLI segment...
		add.l	d0,d0			; CLI BPTR
		add.l	d0,d0			;   to
		move.l	d0,a5			;    CPTR, to a5

		move.l	cli_Module(a5),d5	; Get this SegList
		bne.s	nd_HaveSeg		; If we got one, just skip...
nd_NoCLISeg:	move.l	pr_SegList(a0),d5	; Get seg array...
		beq.s	nd_HunkNone		; If none, we are done...
		addq.l	#3,d5			; Point at element 4...
		add.l	d5,d5			; Make from BPTR
		add.l	d5,d5			; ... into real address...
		move.l	d5,a0			; Get address...
		move.l	(a0),d5			; Get final seglist...
		beq.s	nd_HunkNone		; If none, we are done...
*
* At this point we have:
*	d3=address we are looking for
*	d5=what looks like seglist
*	a2/a3=pointers to two zeroed out ULONGS
*
nd_HaveSeg:	moveq.l	#0,d6			; Hunk counter...
nd_SegLoop:	add.l	d5,d5			; Convert seglist BPTR
		bcs.s	nd_InvalidSeg		; Invalid?
		add.l	d5,d5			; Next...
		bcs.s	nd_InvalidSeg		; Invalid?
		beq.s	nd_HunkNone		; End of seglist?
		move.l	d5,a1			; Get into address reg...
		jsr	_LVOTypeOfMem(a6)	; a6 = SysBase
		tst.l	d0
		beq.s	nd_InvalidSeg		; If invalid, go...

		move.l	d5,a0			; Get into address reg...
*
		move.l	-4(a0),d1		; Get size...
		cmpi.l	#$07FFFFFF,d1		; Test upper 5 bits...
		bcc.s	nd_InvalidSeg		; If larger than 8meg, invalid!
		move.l	d3,d0			; Temp value...
		sub.l	d5,d0			; Subtract...
		bcc.s	nd_MaybeSeg		; Could be...
*
nd_NotThisSeg:	move.l	(a0),d5			; Get next pointer
		addq.l	#1,d6			; Count to next one...
		bra.s	nd_SegLoop		; Keep looping...
*
nd_MaybeSeg:	cmp.l	d1,d0			; Check if this seg...
		bcc.s	nd_NotThisSeg		; If no carry, not here...
		subq.l	#4,d0			; Offset...
		move.l	d0,0(a3)		; Save offset...
		move.l	d6,0(a2)		; Hunk number...
		moveq.l	#1,d0			; Success
		bra.s	nd_HunkDone
*
* Output for when the seglist is invalid
*
nd_InvalidSeg:
		move.l	#-1,0(a2)		; Invalid seglist
nd_HunkNone:
		moveq.l	#0,d0			; Failure		
nd_HunkDone:
		movem.l (sp)+,d2-d6/a2-a6
		rts				; d0=Success or Failure
