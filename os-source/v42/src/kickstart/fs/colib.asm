		SECTION filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	GetPubMem,_LVOFreeMem,_LVOAllocMem,Qpkt

		XDEF	InitCo,CreateCo,CallCo,WaitCo,DeleteCo,ResumeCo,StartCo

; Register set for saving. These are for my environment with a5 and a6 always
; valid and never changed.  d0/d1/a0/a1 are usually always trashed anyway.
regset		REG	d2-d7/a2-a4
	IFND DEBUG_CODE
COSTACK		EQU	512
	ENDC
	IFD DEBUG_CODE
COSTACK		EQU	2048
	ENDC
;COLUMP		EQU	COSTACK*10

;==============================================================================
; coroutine = CreateCo( function )
;    d0			   a0
;
; Creates and adds a co-routine with a COSTACK byte stack and given start
; function.
;==============================================================================
CreateCo	movem.l regset,-(sp)		save all registers
		movea.l	a0,a2			stash start routine address
1$		move.l	StackQueue(a5),d0	get next available stack
		bne.s	5$			one was available

		move.l	#COSTACK,d0		need to allocate a stack
		bsr	GetPubMem		we count on it being cleared!
		tst.l	d0			did we get it
		bne.s	5$			yes
		movem.l (sp)+,regset		else, restore registers
		rts				return with d0 = 0

5$		movea.l	d0,a1			a1 = new coroutine base
		move.l	(a1),StackQueue(a5)	unlink from the list

	IFD DEBUG_CODEX
	movea.l	a1,a0
	move.l	#$DEADBEEF,d0
	move.w	#(COSTACK/4)-1,d1
6$	move.l	d0,(a0)+
	dbra	d1,6$
	ENDC

		movea.l	CurrentCo(a5),a0	a0 = current coroutine
		move.l	co_link(a0),co_link(a1) link new coroutine into chain
		move.l	a1,co_link(a0)
		move.l	a0,co_parent(a1)	set parent to current co
		move.l	a2,co_func(a1)		set function pointer
		move.l	a1,co_dsem+dsem_OwnCo(a1) mark who owns the coroutine
		clr.l	co_dsem+dsem_Block(a1)	we don't own a dirlock
		move.l	a1,CurrentCo(a5)	make new coroutine current
		move.l	sp,co_sp(a0)		save sp, parent is now suspended
		lea.l	COSTACK(a1),sp		swap to new stack
		move.l	a1,d0			return new coroutine address
		movea.l	a1,a2			and save it for us too
10$		bsr.s	WaitCo			waitco back to parent
		move.l	co_func(a2),a0		a0 = initial function
		jsr	(a0)			call it (Arg in d0)
		bra.s	10$			and loop forever if it returns

;==============================================================================
; Result = CallCo( coroutine,arg )
;   d0			a0    d0
;
; Starts up a coroutine that was just created or did a WaitCo to return an arg.
;==============================================================================
CallCo		movea.l	a0,a1			stash entering address
		movea.l	CurrentCo(a5),a0	a0 = current coroutine
		move.l	a0,co_parent(a1)	adopt the coroutine

* coming here we assume:
* D0 = return/argument value
* A0 = coroutine we are leaving
* A1 = coroutine we are entering
coenter		movem.l regset,-(sp)		save this coroutines registers
		move.l	sp,co_sp(a0)		save stack pointer
		movea.l	co_sp(a1),sp		move to new coroutine stack
		movem.l (sp)+,regset		restore registers
		move.l	a1,CurrentCo(a5)	set new current coroutine
		rts				and go into it

;==============================================================================
; Arg = WaitCo( arg )
; d0		d0
;
; Returns control back to the parent with required argument/return code in d0.
; Arg will eventually be returned when the coroutine doing the WaitCo is called
; again with CallCo(coroutine,ARG) or ResumeCo(coroutine,ARG)
;==============================================================================
WaitCo		movea.l	CurrentCo(a5),a0	a0 = current coroutine
		movea.l	co_parent(a0),a1	a1 = parent
		clr.l	co_parent(a0)		become an orphan
		bra.s	coenter			go do switch

;==============================================================================
; ResumeCo( coroutine, arg )
;		a0      d0
;
; Passes control across to a waiting co-routine on the same level as current.
;==============================================================================
ResumeCo	movea.l	a0,a1			save routine we are going to
		movea.l	CurrentCo(a5),a0	a0 = current coroutine
		move.l	co_parent(a0),co_parent(a1)	get adopted parent
		clr.l	co_parent(a0)		and this one is orphaned now
		bra.s	coenter			go call it

;==============================================================================
; success = DeleteCo( coroutine )
;   d0			 a0
;
; Deletes the stack area being used by a coroutine that is no longer needed.
;==============================================================================
DeleteCo	movea.l	RootCo(a5),a1		a1 = coroutine chain
		bra.s	15$			skip first link
10$		movea.l	(a1),a1			indirect
15$		cmpa.l	(a1),a0			is next the one we want ?
		bne.s	10$			if not loop	
		move.l	(a0),(a1)		unlink from co-routine list

	IFD DEBUG_CODEX
	lea.l	COSTACK(a0),a1
	move.l	#$DEADBEEF,d1
	moveq.l	#-4,d0
20$	addq.l	#4,d0
	cmp.l	-(a1),d1
	bne.s	20$
	printf <'used %ld bytes\n'>,d0
	ENDC

		move.l	StackQueue(a5),(a0)	link into free stack list
		move.l	a0,StackQueue(a5)
		moveq.l	#-1,d0			return TRUE
		rts

;==============================================================================
; Success = InitCo()
;   d0
;
; Initialises a root co-routine that never goes away.  It corresponds directly
; to the main level of the program and is really just a list header for all
; other co-routines that get started.  The memory allocation could go in the
; main allocator in Init() but I've left it here for clarity.
;==============================================================================
InitCo		moveq.l	#co_SIZEOF,d0		get root co-routine
		bsr	GetPubMem
		tst.l	d0
		beq.s	10$			it didn't work so fail
		movea.l	d0,a0
		move.l	a0,RootCo(a5)		this will always point here
		move.l	a0,CurrentCo(a5)	and sometimes here
		move.l	a0,co_parent(a0)	I'm my own parent
		moveq.l	#-1,d0			return success
10$		rts

;==============================================================================
; success = StartCo( coroutine,pkt )
;   d0			a0     a1
;
; Creates and starts up a co-routine.  Returns true if it was successful or 0.
;==============================================================================
StartCo		move.l	a1,-(sp)		save packet address
		bsr	CreateCo		create the co-routine
		tst.l	d0			did it work
		bne.s	10$			yes, prepare to callco it

; we didn't get memory for this co-routine so send it back with an error code
		move.l	(sp)+,a0		get packet back
		move.l	a0,d0			is there really a packet ?
		beq.s	20$			nope, so just return an error
		clr.l	dp_Res1(a0)		false return code
		move.l	#ERROR_NO_FREE_STORE,dp_Res2(a0)
		bsr	Qpkt
		moveq.l	#0,d0			return false
		bra.s	20$

10$		movea.l	d0,a0			set up to callco the co-routine
		move.l	(sp)+,d0		pkt = arg for callco
		bsr	CallCo			call it
		moveq.l	#-1,d0			return TRUE
20$		rts

;==============================================================================
; success = MakeCoStacks()
;   d0
;
; Allocates a 5K chunk of memory and chops it up into 5 co-routine stacks on
; the StackQueue.  Returns FALSE if the memory wasn't available.  This is a
; new addition to try and speedup FFS by the last few percent.  No need to call
; AllocMem and frag up memory now. SMB 1/26/88  WORKS REAL WELL!
;==============================================================================
;MakeCoStacks	move.l	#COLUMP+4,d0		amount to get
;		bsr	GetPubMem
;		tst.l	d0			did we get it ?
;		beq.s	CoStacksMade		no, return false

;		movea.l	d0,a0			link into CoChunkList
;		move.l	CoChunkList(a5),(a0)
;		move.l	a0,CoChunkList(a5)

; now break up the chunk into COLUMP/COSTACK pieces and add them into the list
;		lea.l	4(a0),a0
;		moveq.l	#(COLUMP/COSTACK)-1,d0	number of chunks
;10$		move.l	StackQueue(a5),(a0)
;		move.l	a0,StackQueue(a5)
;		lea.l	COSTACK(a0),a0		move to next stack
;		dbra	d0,10$			will return TRUE now
;CoStacksMade	rts

		END
