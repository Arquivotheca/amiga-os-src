*** sysreqstack.asm *********************************************************
*
*  switch to dynamically allocated stack
*
*  $Id: sysreqstack.asm,v 38.1 92/05/21 17:51:33 peter Exp $
*
*  Confidential Information: Commodore-Amiga, Inc.
*  Copyright (C) 1990, Commodore-Amiga, Inc.
*  All Rights Reserved
*
****************************************************************************/

    INCLUDE "exec/types.i"
    INCLUDE "exec/tasks.i"
    INCLUDE "exec/memory.i"
    INCLUDE "exec/execbase.i"
    INCLUDE "exec/macros.i"
    INCLUDE "ibase.i"

    xdef    _stackSwap

    xref    _AllocMem		; smaller to use library stubs
    xref    _FreeMem
    xref    _Debug

; BE VERY CAREFUL to make sure we're allocating enough stack for
; the routine we call, and copying over all the parameters used

NUMPARAMS	EQU	10	; commonBSR uses 9 parameters!
STACKSWAPSIZE	EQU	2400	; pick a number :-)

AbsExecBase	EQU	4

_stackSwap:
	; oldstack on entry:
	;	parameters
	;	routine to call
	;	caller return addr
	;
	; on exit the routine to call is no longer on the stack
	;

	; First, decide if we need to switch the stack
	; we must be careful of people who switch the stack without updating
	; TC_SPxxxxx
	move.l	ib_SysBase(a6),a0	; a6 has IntuitionBase ptr in it
	move.l	ThisTask(a0),a0		; get my task structure
	move.l	TC_SPLOWER(a0),a1	; lower stack bound
	cmp.l	sp,a1			; use unsigned cmp's for the hell of it
	bcc.s	alloc_stack		; splower >= sp, stack was switched
	cmp.l	TC_SPUPPER(a0),sp
	bhi.s	alloc_stack		; spupper < sp, stack was switched

	; within stack ptrs, see if we have enough space
	add.l	#STACKSWAPSIZE,a1	; stack ptr must be above here
	cmp.l	sp,a1
	bcc.s	alloc_stack		; splower+size >= sp, not enough space

	; we have enough space on the stack.  Call the function without switch
	; we must be tricky.  We'll copy the params to the top of the stack.
	move.l	(sp)+,a1		; Get return address...
	move.l	(sp)+,a0		; Get function address...
	move.l	a1,-(sp)		; Save return address again...
	jmp	(a0)			; Do function...
*
*******************************************************************************
*
alloc_stack:
	; we'll keep new stack in a0, old stack in a1 and at
	; top of newstack for restoration

	movem.l	d2/a4/a5,-(sp)		; will use it for newstack
	move.l	ib_SysBase(a6),a5	; Get ExecBase...

	;Allocate memory for the new stack
	moveq.l	#MEMF_ANY,d1		; Any memory for the stack...
	move.l	#STACKSWAPSIZE,d0	; Size of stack...
	exg	a5,a6			; Get exec...
	JSRLIB	AllocMem		; Call AllocMem
	exg	a5,a6			; Back to normal...

	move.l  d0,a4			; I'm stashing it here
	move.l  a4,d0			; set the flags
	bne.s	newStack		; We have a new stack...
****** Out of Memory 'handler'
out_of_memory:		; fail and bail
	moveq.l	#0,d2
swap_exit:
	move.l	d2,d0		; Set return result...
	movem.l	(sp)+,d2/a4/a5	; restore
	move.l	(sp)+,a0	; Get return address
	addq.l	#4,sp		; remove the function pointer
	jmp	(a0)		; "return"
*
*******************************************************************************
*
newStack:
	lea	STACKSWAPSIZE(a4),a1	; Point at end of stack (lower)
	lea	-8(a1),a0		; Point at stack frame...
	move.l	a1,-(a0)		; stk_Upper
	move.l	a4,-(a0)		; stk_Lower
	move.l	a0,stk_Pointer(a0)	; stk_Pointer
	move.l	a0,a4			; We will store this here...
	exg	a5,a6			; Get execbase
	JSRLIB	StackSwap		; Switch stacks
	exg	a5,a6			; Get back...
	move.l	stk_Pointer(a4),a0	; Get old stack pointer...
	;
	; push the last n parameters onto the new stack...
	; a0 is the old stack.  It contains:
	; <args> <function pointer> <return address> <a4/a5/d2>
	;
	lea.l	NUMPARAMS*4+5*4(a0),a0	; Move down...
	moveq.l	#NUMPARAMS-1,d0		; number to copy...
copy:	move.l	-(a0),-(sp)		; Copy them...
	dbra	d0,copy			; loop around...
	move.l	-(a0),a0		; Function pointer...
	jsr	(a0)			; Call the function
	move.l	d0,d2			; Store the result...
	move.l	a4,a0			; Get StackSwap structure
	exg	a5,a6			; Get execbase...
	JSRLIB	StackSwap		; Swap it back in
	move.l	(a4),a1			; Free mem work...
	move.l	#STACKSWAPSIZE,d0	; Size of the memory
	JSRLIB	FreeMem			; Free the memory...
	exg	a5,a6			; Restore a6...
	bra.s	swap_exit		; and done...
*
*******************************************************************************
*
    end

