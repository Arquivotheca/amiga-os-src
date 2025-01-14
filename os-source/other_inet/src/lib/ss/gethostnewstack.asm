
	INCLUDE	"exec/types.i"
	INCLUDE "exec/tasks.i"

	XDEF	_gethostbyname
	XDEF	_gethostbyaddr

	XREF	__ghbyname
	XREF	__ghbyaddr

jsrlib	MACRO
	IFND	_LVO\1
	XREF	_LVO\1
	ENDC
	jsr	_LVO\1(a6)
	ENDM


	;Call __gethbyname() with a new stack.
	;
	;

_gethostbyname:
	movem.l	a2-a6,-(sp)
	movea.l	a0,a2		; Preserve name
	suba.l	a3,a3		; Return value

	movea.l	a6,a5		; Save SocketBase

	;Allocate new temp stack space...

	movea.l	4,a6		; SysBase
	move.l	#8192+12,d0	; 8k of stack plus enough for the StackSwap Structure
	moveq.l	#0,d1		; any memory will do

	jsrlib	AllocMem
	tst.l	d0
	beq.s	1$
	movea.l	d0,a4		; pointer to base of StackSwap Structure
	add.l	#12,d0		; Skip over StackSwap Structure
	move.l	d0,stk_Lower(a4) ; Base of stack space
	beq.s	1$

	add.l	#8192,d0	; stack size bound
	move.l	d0,stk_Upper(a4)	; Upper bound
	subq.l	#4,d0		; top of stack
	move.l	d0,stk_Pointer(a4)

	movea.l	a4,a0

	jsrlib	StackSwap

	movea.l	a5,a6		; Restore SocketBase
	movea.l	a2,a0		; Restore name pointer

	jsr	__ghbyname
	movea.l	d0,a3		; Save return value

	movea.l	a4,a0		; Restore old stack
	movea.l	4,a6
	jsrlib	StackSwap	; *POOF*

	movea.l	a4,a1		; Free StackSwap and Stack mem
	move.l	#8192+12,d0
	jsrlib	FreeMem

1$	move.l	a3,d0
	movem.l	(sp)+,a2-a6	; restore everything else
	rts

	;Call __gethbyaddr() with a new stack.
	;
	; Need to preserve a0, d0, d1

_gethostbyaddr:
	movem.l	d2-d3/a2-a6,-(sp)
	movea.l	a0,a2		; Preserve name
	move.l	d0,d2
	move.l	d1,d3
	suba.l	a3,a3		; Return value

	movea.l	a6,a5		; Save SocketBase

	;Allocate new temp stack space...

	movea.l	4,a6		; SysBase
	move.l	#8192+12,d0	; 8k of stack plus enough for the StackSwap Structure
	moveq.l	#0,d1		; any memory will do

	jsrlib	AllocMem
	tst.l	d0
	beq.s	1$
	movea.l	d0,a4		; pointer to base of StackSwap Structure
	add.l	#12,d0		; Skip over StackSwap Structure
	move.l	d0,stk_Lower(a4) ; Base of stack space
	beq.s	1$

	add.l	#8192,d0	; stack size bound
	move.l	d0,stk_Upper(a4)	; Upper bound
	subq.l	#4,d0		; top of stack
	move.l	d0,stk_Pointer(a4)

	movea.l	a4,a0

	jsrlib	StackSwap

	movea.l	a5,a6		; Restore SocketBase
	movea.l	a2,a0		; Restore name pointer
	move.l	d2,d0
	move.l	d3,d1

	jsr	__ghbyaddr

	movea.l	d0,a3		; Save return value

	movea.l	a4,a0		; Restore old stack
	movea.l	4,a6
	jsrlib	StackSwap	; *POOF*

	movea.l	a4,a1		; Free StackSwap and Stack mem
	move.l	#8192+12,d0
	jsrlib	FreeMem

1$	move.l	a3,d0
	movem.l	(sp)+,d2-d3/a2-a6	; restore everything else
	rts

