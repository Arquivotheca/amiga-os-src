

;****************************************************************************
;
; alloc.asm -- low level janus interrupt handling code
;
; Copyright (c) 1986, Commodore Amiga Inc.  All rights reserved.
;
;****************************************************************************


		INCLUDE 'assembly.i'

		NOLIST
		INCLUDE 'exec/types.i'
		INCLUDE 'exec/nodes.i'
		INCLUDE 'exec/libraries.i'
		INCLUDE 'exec/interrupts.i'
		INCLUDE 'exec/memory.i'
		INCLUDE 'exec/ables.i'
		INCLUDE 'exec/alerts.i'
		INCLUDE 'hardware/intbits.i'
		INCLUDE 'janus.i'
		INCLUDE 'janusvar.i'
		INCLUDE 'janusreg.i'
		INCLUDE 'setupsig.i'
		LIST

		INCLUDE 'asmsupp.i'


		XDEF	AllocJanusMem
		XDEF	FreeJanusMem
		XDEF	SetupJanusSig
		XDEF	CleanupJanusSig
		XDEF	JanusMemType
		XDEF	JanusMemBase
		XDEF	JanusMemToOffset
		XDEF	JanusLock
		XDEF	JanusUnLock 

		XLIB	AllocJanusMem
		XLIB	SetJanusHandler
		XLIB	GetParamOffset
		XLIB	SetParamOffset
		XLIB	FreeJanusMem
		XLIB	SetJanusEnable
		XLIB	SetJanusRequest
		XLIB	JanusMemBase
		XLIB	JanusMemType
		XLIB	JanusMemToOffset
		XLIB	Alert
		XLIB	FindTask
		XLIB	Signal
		XLIB	AllocMem
		XLIB	FreeMem
		XLIB	Permit
		XLIB	Forbid
		XLIB	Debug


	       INT_ABLES


; ptr = AllocJanusMem( size, type )
; d0		       d0    d1
;
; get memory from the memory pools.  A (68000) 32 bit pointer is
; returned if the allocation could be done; if the allocation failed
; then a null is returned.

AllocJanusMem:
		movem.l  d2/d3/d4/a2,-(sp)

	IFGE	INFOLEVEL-80
	movem.l d0/d1,-(sp)
	PUTMSG	80,<'%s/AllocJanusMem( %ld, %ld )'>
	movem.l (sp)+,d0/d1
	ENDC

		;------ adjust the size -- rounding & hidden size
		addq.l	#7,d0		    ; room for hidden size + rounding
		and.b	#$fc,d0 	    ; turn off lower two bits

		LINKEXE Forbid

		;------ check for overflow
		swap	d0
		tst.w	d0
		bne	ajm_fail
		swap	d0

		move.l	ja_ParamMem(a6),a2

		;------ get the memory header in here
		cmp.b	#MEMF_PARAMETER,d1
		beq.s	ajm_parammem
		cmp.b	#MEMF_BUFFER,d1
		bne	ajm_fail

		;------ it is buffer memory
		lea	jb_BufferMem(a2),a0
		bra.s	ajm_gottype

ajm_parammem:
		;------ it is param mem
		lea	jb_ParamMem(a2),a0

ajm_gottype:

	IFGE	INFOLEVEL-80
	movem.l a0/a2,-(sp)
	PUTMSG	80,<'%s/ajm: membase 0x%lx, param 0x%lx'>
	addq.l	#8,sp
	ENDC

		;------ we now have our MemHead.  The registers from here
		;------ will be: a2 -- ptr to addressible memory.
		;------ a0 -- ptr to JanusMemHead structure.
		;------ a1 -- pointer to last JanusMemChunk
		;------ d2 -- index of CURRENT JanusMemChunk

		;------ lock out the ibm-pc
		LOCK	jmh_Lock(a0)
		add.l	#WordAccessOffset,a0

		move.l	d0,d4
		subq.w	#1,d4
		lea	jmh_First(a0),a1    ; back pointer to previous pointer
		CLEAR	d2
		move.l	jmh_68000Base(a0),a2
		add.l	#WordAccessOffset,a2
		bra.s	ajm_begin

ajm_loop:

	IFGE	INFOLEVEL-80
	movem.l d2/a1/a2,-(sp)
	PUTMSG	80,<'%s/ajm_loop: cur 0x%lx, last 0x%lx, base 0x%lx'>
	lea	12(sp),sp
	ENDC


		;------ get the next pointer
		lea	0(a2,d2.l),a1	    ; save block
		move.w	jmc_Next(a1),d2

ajm_begin:
		;------ get the next pointer
		move.w	(a1),d2

		;------ if next is -1 then it is all over
		cmp.w	#-1,d2
		beq.s	ajm_unlockandfail

		;------ see if this chunk is big enough
		cmp.w	jmc_Size(a2,d2.l),d4
		bhi.s	ajm_loop

		;------ there is enough room.  See if we need to
		;------ split it
		blo.s	ajm_split

		;------ it is exactly right.  make previous point to next
		move.w	jmc_Next(a2,d2.l),(a1)
		bra.s	ajm_gotmem

ajm_split:
		;------ the chunk is too big.  make it smaller.  we will be
		;------ a bit tricky by doing a long move instead of two words
		move.l	d2,d3
		add.w	d0,d3
		move.l	jmc_Next(a2,d2.l),jmc_Next(a2,d3.l)
		sub.w	d0,jmc_Size(a2,d3.l)

		;------ make previous point to new position
		add.w	d0,(a1)

ajm_gotmem:
		;------ we now have mem.  put in the size.
		move.l	d0,0(a2,d2.l)
		add.w	#4,d2

		;------ reflect the allocated block in the free mem count
		sub.w	d0,jmh_Free(a0)


		;------ now for some magic: based on the access area he
		;------ he asked for, we will return him the right pointer.
		add.l	jmh_68000Base(a0),d2

		and.l	#MEM_ACCESSMASK,d1
		lsl.l	#TYPEACCESSTOADDR,d1
		add.l	d1,d2

		;------ unlock the memory list
		sub.l	#WordAccessOffset,a0
		UNLOCK	jmh_Lock(a0)

		;------ all done!
		move.l	d2,d0

ajm_end:

		LINKEXE Permit

		movem.l (sp)+,d2/d3/d4/a2
		rts

ajm_unlockandfail:
		sub.l	#WordAccessOffset,a0
		UNLOCK	jmh_Lock(a0)

ajm_fail:
		moveq	#0,d0
		bra.s	ajm_end





; FreeJanusMem( ptr, size )
;		a1   d0
;
; returns ptr to the memory free pool.	checks to make sure that
; size is an allocated chunk.  Will coallesse adjacent free blocks.
; If a bad pointer is freed, the routine will cause and Alert().

FreeJanusMem:
		movem.l d2/d3/a2,-(sp)

		move.l	ja_ParamMem(a6),a2
		add.l	#WordAccessOffset,a2

		;------ adjust the size (round up)
		addq.l	#7,d0
		and.b	#$fc,d0 	    ; unset lower two bits

		;------ adjust the pointer to allow for hidden size
		subq.l	#4,a1


		LINKEXE Forbid

		;------ check to make sure it is a valid address.
		move.l	a1,d1
		sub.l	ja_ExpanBase(a6),d1
		blo	fjm_err0

		cmp.l	#JANUSTOTALSIZE,d1
		bhs	fjm_err1

		;------ strip off the bank address part
		move.l	a1,d1
		and.l	#~JANUSBANKMASK,d1
		move.l	d1,a1

		;------ now try and find the address in a free map
		lea	jb_ParamMem(a2),a0
		bsr	fjm_checkhead
		bne.s	fjm_headfound

		lea	jb_BufferMem(a2),a0
		bsr	fjm_checkhead 
		beq	fjm_err2

fjm_headfound:
		;------ we found a proper mem head structure.  link us in
		;------ (registers now: a1 -- ptr, a0 -- mem head, word access
		;------ d0 -- size)

	IFGE	INFOLEVEL-80
	movem.l d0/a0/a1,-(sp)
	PUTMSG	80,<'%s/fjm_headfound: size %ld, mh 0x%lx, ptr 0x%lx'>
	lea	12(sp),sp
	ENDC

		;------ lock out the ibm-pc
		sub.l	#WordAccessOffset,a0
		LOCK	jmh_Lock(a0)
		add.l	#WordAccessOffset,a0

		;------ get us a base of the memory region, word access
		move.l	jmh_68000Base(a0),a2
		sub.l	a2,a1
		add.l	#WordAccessOffset,a2

		CLEAR	d2
		move.l	a1,d1

		;------ bump the free count
		add.w	d0,jmh_Free(a0)

		;------ some error checking: make sure the right size
		;------ is saved
		cmp.w	2(a2,d1.l),d0
		bne	fjm_err3


		;------ get a pointer to the first index
		lea	jmh_First(a0),a1
		move.l	a1,d3		    ; copy of head of list
		bra.s	fjm_begin

fjm_loop:
		;------ advance d2 to point to next free list entry
		lea	jmc_Next(a2,d2.l),a1
fjm_begin:
		;------ get a pointer to the next element
		move.w	(a1),d2

		;------ see if current is still less than mem to be freed
		cmp.w	d2,d1
		bhi.s	fjm_loop

		;------ this is the first mem chunk that is beyond d1
		;------ if d1 == d2 then we are freeing a freed chunk
		beq	fjm_err4

		;------ set up this block to point to d2, and adjust size
		move.w	d2,jmc_Next(a2,d1.l)
		subq.w	#1,jmc_Size(a2,d1.l)

		;------ make previous point to memory being freed
		move.w	d1,(a1)

		;------ make sure we do not coallesce the head
		cmp.l	a1,d3
		beq.s	fjm_checknext

		;------ see if we can coalesce the previous and current
		;------ see if prev + prevsize = current.  This is made
		;------ a bit trickier since prev is a 68000 ptr, and
		;------ current is an offset from membase.
		move.l	a1,d3
		sub.l	a2,d3
		add.w	jmc_Size(a1),d3
		addq.w	#1,d3		    ; jmc_Size is size-1
		cmp.l	d1,d3
		bhi	fjm_err5	    ; freeing something already freed
		blo.s	fjm_checknext

		;------ join previous and current
		add.w	jmc_Size(a1),d0     ; update size
		move.w	d0,jmc_Size(a1)
		addq.w	#1,d0		    ; jmc_Size+1 is real size
		move.w	d2,(a1) 	    ; prev->next = next
		move.l	a1,d1		    ; recompute current (d1)
		sub.l	a2,d1

fjm_checknext:
		;------ see if current and next can be joined
		cmp.w	#-1,d2
		beq.s	fjm_end 	    ; there is no next

		;------ combine if cur + curr.size = next
		move.l	d1,d3
		add.w	d0,d3
		cmp.w	d2,d3
		blo.s	fjm_end 	    ; can't combine
		bhi	fjm_err6	    ; freeing something alread freed

		;------ join the memory regions
		add.w	jmc_Size(a2,d2.l),d0
		move.w	d0,jmc_Size(a2,d1.l)
		move.w	jmc_Next(a2,d2.l),jmc_Next(a2,d1.l)

fjm_end:
		sub.l	#WordAccessOffset,a0
		UNLOCK	jmh_Lock(a0)

fjm_errexit:

		LINKEXE Permit

		movem.l (sp)+,d2/d3/a2
		rts

fjm_err0:
		PUTMSG	1,<'%s/fjm_err0'>
		bra	fjm_err


fjm_err1:
		PUTMSG	1,<'%s/fjm_err1'>
		bra	fjm_err

fjm_err2:
		PUTMSG	1,<'%s/fjm_err2'>
		bra	fjm_err

fjm_err3:
		UNLOCK	jmh_Lock(a0)
		PUTMSG	1,<'%s/fjm_err3'>
		bra	fjm_err

fjm_err4:
		UNLOCK	jmh_Lock(a0)
		PUTMSG	1,<'%s/fjm_err4'>
		bra	fjm_err

fjm_err5:
		UNLOCK	jmh_Lock(a0)
		PUTMSG	1,<'%s/fjm_err5'>
		bra	fjm_err

fjm_err6:
		UNLOCK	jmh_Lock(a0)
		PUTMSG	1,<'%s/fjm_err6'>
		bra	fjm_err


fjm_err:
******		PUTMSG	1,<'%s/fjm_err'>
		LINKEXE Permit
fjm_errloop:
		LINKEXE Debug
		ALERT	$7fffffff
		LINKEXE Forbid
		bra	fjm_errexit

;
; check memory head to see if this address fits in
; must preserve d0, a1, a0
;
; returns 0 for failure & 1 for success.  condition codes must be set.

fjm_checkhead:	; ( a1 -- ptr, a0 -- mem head )
		move.l	a1,d1
		sub.l	jmh_68000Base(a0),d1
		blo.s	fjmch_fail

		cmp.w	jmh_Max(a0),d1
		bhi.s	fjmch_fail

		;------ make sure no bit in the high word are set
		swap	d1
		tst.w	d1
		bne.s	fjmch_fail

		;------ success!
		moveq	#1,d1
		rts

fjmch_fail:
		moveq	#0,d1
		rts



; setupSigPtr = SetupJanusSig( jintnum, signum, paramsize, paramtype )
; d0			    d0	     d1      d2 	d3
;
; this routine does much of the 'standard' things that one would
; need to do to use the janus subsystem.  It makes some assumptions
; about how you would want to use the system, however.
;
; The primary assumption it makes is that you want to be told about
; an interrupt by getting a signal.  The routine will set up an interrupt
; to deliver the signal "signum" whenever a janus interrupt comes through.
;
; The routine will also allocate parameter memory for you, and set up
; the parameter area to point to the memory.  It returns a (68000)
; pointer to the parameter area.
;
; The call does a fair amount of error checking.  If signum is -1 it fails.
; This allows you to pass the results of AllocSignal() directly to
; SetupJanusSig().  If there already is a handler for jintnum an error
; is returned.	If there already is a parameter in the parameter area
; the routine fails.  If it cannout allocate the parameter memory then
; the routine fails.
;
; If the call succeeded then a pointer to a SetupSig structure is returned.
; An null is returned if there was an error.
;
; If paramsize is null then no parameter memory is allocated.

SJS_SAVEREGS	REG	d2/d3/d4/d5/a2/a6
SJS_NUMREGS	EQU	6


SetupJanusSig:
		movem.l SJS_SAVEREGS,-(sp)
		move.l	d0,d4
		move.l	d1,d5
	
	movem.l d0-d3,-(a7)
	INFOMSG 50,<'Janus: SetupJanusSig(%ld,0x%lx,%ld,%ld)'>
	add.w	#16,a7
		LINKEXE Forbid

		;------ see if the signal is -1
		tst.b	d1
		bmi	sjs_fail

		;------ see if there is already a parameter area
		tst.l	d2
		beq.s	sjs_handlercheck

		CALLSYS GetParamOffset
		addq.w	#1,d0	    ; test for $ffff - uninitialized offset
		bne	sjs_fail    ;	I want it to be uninitialized!

sjs_handlercheck:
		;------ see if there is already a handler
		suba.l	a1,a1
		move.l	d4,d0

		DISABLE a0

		CALLSYS SetJanusHandler
		tst.l	d0
		beq	sjs_allocmem

		;------ fix up the damage
		move.l	d0,a1
		move.l	d4,d0
		CALLSYS SetJanusHandler
					 
		ENABLE	a0
		bra	sjs_fail

sjs_allocmem:
		ENABLE	a0

		;------ allocate some amiga memory to hold the server
		;------ structure + additional data needed
		moveq	#SetupSig_SIZEOF,d0
		move.l	#MEMF_CLEAR,d1
		LINKEXE AllocMem
		tst.l	d0
		beq	sjs_fail
		move.l	d0,a2

		;------ get the current task
		suba.l	a1,a1
		LINKEXE FindTask
		move.l	d0,ss_TaskPtr(a2)
		move.l	d0,a0
		move.l	LN_NAME(a0),LN_NAME(a2)

		;------ set up the signal number
		moveq	#1,d0
		lsl.l	d5,d0
		move.l	d0,ss_SigMask(a2)

		;------ set up the code and data fields
		move.l	#intToSigHandler,IS_CODE(a2)
		move.l	a2,IS_DATA(a2)

		;------ record the jintnum and paramsize
		move.w	d4,ss_JanusIntNum(a2)
		move.l	d2,ss_ParamSize(a2)

		;------ if no param mem was wanted we are all done.
		beq.s	sjs_success

		;------ allocate the parameter area
		move.l	d2,d0
		move.l	d3,d1
		CALLSYS AllocJanusMem
	movem.l d0/d2/d3,-(a7)
	INFOMSG 50,<'Janus: $%lx=AllocJanusMem(%ld,%ld)'>
	add.w	#12,a7
		move.l	d0,ss_ParamPtr(a2)
		bne.s	sjs_setoffset

		;------ the alloc failed.  free our mem and return an error
		move.l	a2,a1
		moveq	#SetupSig_SIZEOF,d0
		LINKEXE FreeMem
		bra	sjs_fail

sjs_setoffset:
		;------ set up the parameter block
		move.l	ss_ParamPtr(a2),d0
		CALLSYS JanusMemToOffset
		move.l	d0,d1
		move.l	d4,d0
		CALLSYS SetParamOffset
sjs_success:
		;------ set up the interrupt handler
		move.l	d4,d0
		move.l	a2,a1
		CALLSYS SetJanusHandler

		;------ clear any pending requests
		move.l	d4,d0
		CLEAR	d1
		CALLSYS SetJanusRequest

		;------ and enable the interrupt
		move.l	d4,d0
		moveq	#1,d1
		CALLSYS SetJanusEnable



		;------ set up the return code
		move.l	a2,d0


sjs_end:

		LINKEXE Permit

		movem.l (sp)+,SJS_SAVEREGS
		rts

sjs_fail:
		moveq	#0,d0
		bra.s	sjs_end

intToSigHandler:
		move.l	ss_SigMask(a1),d0
		move.l	ss_TaskPtr(a1),a1
		LINKEXE Signal
		rts




; CleanupJanusSig( setupSigPtr )
;		   a0
;
; this routine undoes everything that SetupJanusSig does.

CleanupJanusSig:

		movem.l d2/d3/a2,-(sp)
		move.l	a0,a2

		;------ make sure SetupSig is really there 
		move.l	a0,d0
		beq.s	cjs_end

		CLEAR	d2
		move.w	ss_JanusIntNum(a2),d2

		;------ free up the parameter memory (if any)
		move.l	ss_ParamSize(a2),d3
		beq.s	cjs_disable

		move.l	d2,d0
		moveq	#-1,d1
		CALLSYS SetParamOffset

		move.l	d3,d0
		move.l	ss_ParamPtr(a2),a1
		CALLSYS FreeJanusMem

cjs_disable:
		move.l	d2,d0
		suba.l	a1,a1
		CALLSYS SetJanusHandler

		move.l	d2,d0
		CLEAR	d1
		CALLSYS SetJanusEnable

		;------ free up the SetupSig structure
		move.l	a2,a1
		moveq	#SetupSig_SIZEOF,d0
		LINKEXE FreeMem

cjs_end:
		movem.l (sp)+,d2/d3/a2
		rts



; type = JanusMemType( ptr )
; d0		       d0
;
; this routine turns a 68000 memory pointer into a janus memory type descriptor
; (suitable for being fed to AllocJanusMem).  Havoc will break out if an
; invalid pointer is fed to the routine (havoc often looking like an
; Alert()).


JanusMemType:

		sub.l	ja_ExpanBase(a6),d0
		blo.s	jmt_err

		cmp.l	#JANUSTOTALSIZE,d0
		bhs.s	jmt_err

		;------ compute the bank bits
		move.l	d0,d1
		and.l	#JANUSBANKMASK,d1
		eor.l	d1,d0		    ; turn off bank bits in d0
		lsr.l	#TYPEACCESSTOADDR,d1

		;------ figure out what type of memory it is
		cmp.l	#ColorOffset,d0
		blo.s	jmt_buffermem

		cmp.l	#ParameterOffset,d0
		blo.s	jmt_err

		cmp.l	#MonoVideoOffset,d0
		bhs.s	jmt_err

		;------ it is parameter mem
		bset	#MEMB_PARAMETER,d1
		bra.s	jmt_success

jmt_buffermem:
		bset	#MEMB_BUFFER,d1

jmt_success:
		move.l	d1,d0
		rts

jmt_err:
		ALERT	$7ffffffe
		rts



; ptr = JanusMemBase( type )
; d0		      d0
;
; this routine will return a 68000 pointer to the janus memory described
; by the type specifier.
;


JanusMemBase:
		move.l	d0,d1
		btst	#MEMB_BUFFER,d1
		bne.s	jmb_buffermem

		btst	#MEMB_PARAMETER,d1
		beq.s	jmb_err

		;------ it is parameter memory
		move.l	#ParameterOffset,d0
		bra.s	jmb_access

jmb_buffermem:
		move.l	#BufferOffset,d0

jmb_access:
		and.l	#MEM_ACCESSMASK,d1
		lsl.l	#TYPEACCESSTOADDR,d1
		or.l	d1,d0

		add.l	ja_ExpanBase(a6),d0

		rts

jmb_err:
		ALERT	$7ffffffd
		LINKEXE Debug
		rts


; offset = JanusMemToOffset( ptr )
; d0			     d0
;
; turn a 68000 pointer into an offset from the memory segment base.

JanusMemToOffset:
		move.l	d2,-(sp)

		move.l	d0,d2
		CALLSYS JanusMemType
		move.l	d0,-(sp)		save the memory type
		CALLSYS JanusMemBase
		sub.l	d0,d2
;==============================================================================
; if this is an AT with the memory set to $d000 then all offsets should be 
; backed up by $4000 because the PC will be taking all offsets off a segment
; base address of $d400.  SHIT !!!!
;==============================================================================
		move.l	(sp)+,d0		get back the type
		btst.l	#MEMB_BUFFER,d0		and see if it's buffer mem
		beq.s	10$			nope, then do nothing special
; this is that funny buffer mem, see if it's at segment offset $d400
		cmpi.w	#$d400,ja_ATOffset(a6)	are we in the funny bank ?
		bne.s	10$			no, leave everything alone
		subi.l	#$4000,d2		yes, back it up by 4000

10$		move.l	d2,d0
		move.l	(sp)+,d2
		rts


; JanusLock( ptr )
;	     a0
;
; busy wait for a janus lock on the byte poined to by the 68000 pointer.
JanusLock:
		LOCK	(a0)
		rts
			 

; JanusLock( ptr )
;	     a0
;
; unlock a janus lock at the byte pointed to by the 68000 pointer.
JanusUnLock: 
		UNLOCK	(a0)
		rts

       END

