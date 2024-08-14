
* *** alloc.asm ***************************************************************
* 
* alloc.asm -- low level janus code
* 
* Copyright (C) 1986, 1987, 1988, Commodore Amiga Inc.
* All rights reserved.
* 
* Date        Name               Description
* ----------  -----------------  ----------------------------------------------
* 15 Jul-88   -RJ                Changed all files to work with new includes
* 18-Feb-88   -RJ Mical-         Added autodoc headers.
*                                AllocJanusMem() now returns value in both 
*                                D0 and A0.
*                                JIntNum and SigNum of SetupJanusSig() are
*                                now 16 bits.
* Early 1986  Katin/Burns clone  Created this file!
* 
* *****************************************************************************


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

		INCLUDE 'janus/janusbase.i'
		INCLUDE 'janus/janusvar.i'
		INCLUDE 'janus/janusreg.i'
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
		XDEF	JanusUnlock 
		XDEF	JanusLockAttempt

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


AllocJanusMem:
* === AllocJanusMem() =========================================================
* 
* NAME
*     AllocJanusMem  --  Allocate memory from the special Janus RAM
* 
* 
* SYNOPSIS
*     MemPtr =  AllocJanusMem(Size, Type);
*     D0,A0                   D0    D1
*         APTR    MemPtr;
*         ULONG   Size;
*         ULONG   Type;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Allocates memory of the specified size and type from the special 
*     Janus RAM.  The type argument is used to describe the memory area 
*     from which you want the allocation made (e.g. buffer, parameter, 
*     or others as they come available) and into which address space you 
*     want the pointer to point (e.g. byte-access, word-access, others).  
*     Please refer to the Janus reference documentation for a description 
*     of Janus memory types and address spaces.  
*     
*     If memory of the desired size and type is not available, this routine 
*     returns NULL.  
*     
*     Assembly programmers note that this routine returns the result in 
*     both D0 and A0.  
*     
*     
* EXAMPLE
*     UBYTE    *ByteParameterPtr;
*     USHORT   *WordParameterPtr;
*     UBYTE    *ByteBufferPtr;
*     USHORT   *WordBufferPtr;
*         ByteParameterPtr = (UBYTE *)AllocJanusMem(100, 
*                 MEMF_PARAMETER | MEM_BYTEACCESS);
*         WordParameterPtr = (UBYTE *)AllocJanusMem(100, 
*                 MEMF_PARAMETER | MEM_WORDACCESS);
*         ByteBufferPtr = (UBYTE *)AllocJanusMem(100, 
*                 MEMF_BUFFER | MEM_BYTEACCESS);
*         WordBufferPtr = (UBYTE *)AllocJanusMem(100, 
*                 MEMF_BUFFER | MEM_WORDACCESS);
*     
*     
* INPUTS
*     Size = size of memory (in bytes) that you wish to allocate
*     Type = type of Janus memory that you wish to allocate, which 
*         type specifier describes both the area of memory and 
*         the address space of the resultant pointer
*     
*     
* RESULTS
*     MemPtr = pointer to your newly allocated memory, or NULL if the 
*         allocation failed.  Assembly programmers note that the 
*         result is returned in both D0 and A0
*     
*     
* SEE ALSO
*     FreeJanusMem(), AllocJRemember(), AllocServiceMem()

		movem.l  d2/d3/d4/a2,-(sp)

	IFGE	INFOLEVEL-59
	movem.l d0/d1,-(sp)
	PUTMSG	59,<'%s/AllocJanusMem( %ld, %ld )'>
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

		move.l	jb_ParamMem(a6),a2

		;------ get the memory header in here
		cmp.b	#MEMF_PARAMETER,d1
		beq.s	ajm_parammem
		cmp.b	#MEMF_BUFFER,d1
		bne	ajm_fail

		;------ it is buffer memory
		lea	ja_BufferMem(a2),a0
		bra.s	ajm_gottype

ajm_parammem:
		;------ it is param mem
		lea	ja_ParamMem(a2),a0

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
		beq	ajm_unlockandfail

		;------ see if this chunk is big enough
		cmp.w	jmc_Size(a2,d2.l),d4
		bhi	ajm_loop

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
		MOVE.L	D0,A0		; RJ adds this, to return both D0/A0
		LINKEXE Permit

		movem.l (sp)+,d2/d3/d4/a2
		rts

ajm_unlockandfail:
		sub.l	#WordAccessOffset,a0
		UNLOCK	jmh_Lock(a0)

ajm_fail:
		moveq	#0,d0
		bra	ajm_end



FreeJanusMem:
* === FreeJanusMem() ==========================================================
* 
* NAME
*     FreeJanusMem  --  Free Janus memory allocated with AllocJanusMem() 
* 
* 
* SYNOPSIS
*     VOID FreeJanusMem(Ptr, Size);
*                       A1   D0
*         APTR    Ptr;
*         ULONG   Size;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Free a block of Janus memory that was allocated with a call to 
*     AllocJanusMem().  
*     
*     
* INPUTS
*     Ptr = address of the Janus memory to be freed.  Note that this 
*         pointer can point to any type or address space of Janus memory 
*     Size = the size of the block of memory to be freed
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     FreeJanusMem(), FreeJRemember(), FreeServiceMem()

		movem.l d2/d3/a2,-(sp)

	MOVE.L	D0,-(SP)
	MOVE.L	A1,-(SP)
	PUTMSG	80,<'%s/FreeJanusMem( 0x%lx %ld )'>
	LEA	2*4(SP),SP

		move.l	jb_ParamMem(a6),a2
		add.l	#WordAccessOffset,a2

		;------ adjust the size (round up)
		addq.l	#7,d0
		and.b	#$fc,d0 	    ; unset lower two bits

		;------ adjust the pointer to allow for hidden size
		subq.l	#4,a1


		LINKEXE Forbid

		;------ check to make sure it is a valid address.
		move.l	a1,d1
		sub.l	jb_ExpanBase(a6),d1
		blo	fjm_err0

		cmp.l	#JANUSTOTALSIZE,d1
		bhs	fjm_err1

		;------ strip off the bank address part
		move.l	a1,d1
		and.l	#~JANUSBANKMASK,d1
		move.l	d1,a1

		;------ now try and find the address in a free map
		lea	ja_ParamMem(a2),a0
		bsr	fjm_checkhead
		bne.s	fjm_headfound

		lea	ja_BufferMem(a2),a0
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



SetupJanusSig:
* === SetupJanusSig() =========================================================
* 
* NAME
*     SetupJanusSig  --  Set up to receive a Janus interrupt from the PC
* 
* 
* SYNOPSIS
*     SetupSigPtr = SetupJanusSig(JanusInterruptNum, SignalNumber, Size, Type)
*     D0,A0                       D0:0-15            D1:0-15       D2    D3
* 
*         struct  SetupSig *SetupSigPtr;
*         USHORT  JanusInterruptNumber;
*         USHORT  SignalNumber;
*         ULONG   Size;
*         ULONG   Type;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     This routine sets up an interrupt handler to receive PC interrupts 
*     for you.  The interrupt handler that this routine creates does all the 
*     underlying interrupt management for you.  When an interrupt from the 
*     PC arrives, the interrupt code does all the grunt work for you and 
*     then signals your task.  Meanwhile, you Wait() on the signal bit 
*     described by your SignalNumber argument, and when your task awakens 
*     with that bit set, voila, you've received an interrupt from the PC.  
*     This allows you to ignore all the complexities of writing interrupt 
*     code, which is very good, believe us.  If it doesn't sound as if we're 
*     saving you a lot of work, then you've never written interrupt code.  
*     
*     You specify the JanusInterruptNumber that you want to connect with 
*     and the SignalNumber of the signal bit to be sent to your task.  
*     The Janus interrupt numbers are defined in services.[hi].  
*     Your signal number will most probably be gotten via a call to 
*     Exec's AllocSignal().  
*     
*     This routine can set up a parameter area for your 
*     interrupt according to your specifications.  If you specify a Size 
*     argument of anything other than 0, then 'Size' bytes of 'Type' Janus 
*     memory will be allocated for you and the ss_ParamPtr field of your 
*     SetupSig data structure will point to the memory.  
*     
*     There is some error checking being done while all this is going on.  
*     If SignalNumber is -1 the call fails (-1 is the error return from 
*     AllocSignal...).  If there is already an interrupt handler then the 
*     call fails.  If Size is non-zero and there is already a parameter 
*     area the call fails.  If it cannot allocate enough memory the call 
*     fails.  
*     
*     If all goes well then the return value from this function a pointer 
*     to an initialized SetupSig structure.  This structure is defined 
*     in setupsig.[hi].  The call returns a NULL if an error occurs.  
*     
*     This is a mid-level Janus system routine.  You would probably have 
*     a nicer time and be better served by using the Services routines, 
*     which hide all the hassle of Janus signals from you.  
*     
*     
* EXAMPLE
*     struct SetupSig *sig;
*     sig = SetupJanusSig(JSERV_READPC, AllocSignal(-1), 
*             100, MEMF_BUFFER | MEM_WORDACCESS);
*     if (sig) CleanupJanusSig(sig);
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt with 
*         which you want to connect
*     SignalNumber = the number of the signal bit that your task 
*         will Wait() on until the PC generates your interrupt
*     Size = the size of the Janus memory that you want allocated for 
*         your interrupt.  If 0, no memory will be allocated
*     Type = the type of Janus memory you want allocated for your 
*         interrupt.  If Size above is 0, this argument is ignored
*     
*     
* RESULTS
*     If all goes well, returns the address of an initialized 
*     SetupSig structure.  If any error, returns NULL
*     
*     
* SEE ALSO
*     CleanupJanusSig()


SJS_SAVEREGS	REG	d2/d3/d4/d5/a2/a6
SJS_NUMREGS	EQU	6


		movem.l SJS_SAVEREGS,-(sp)
		move.l	d0,d4
		move.l	d1,d5

		;------	RJ sez:  make the interrupt number and the 
		;------	signal number 16 bit values for sure, as advertised
		MOVE.L	D0,-(SP)
		MOVE.L	#$0000FFFF,D0
		AND.L	D0,D4
		AND.L	D0,D5
		MOVE.L	(SP)+,D0

	IFGE	INFOLEVEL-1
	movem.l d2-d3,-(a7)
	movem.l d4-d5,-(a7)
	INFOMSG 1,<'Janus: SetupJanusSig(%ld,0x%lx,%ld,%ld)'>
	LEA.L	4*4(SP),SP
	ENDC

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
		MOVE.L	D0,A0		; Return both D0 and A0

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



CleanupJanusSig:
* === CleanupJanusSig() =======================================================
* 
* NAME
*     CleanupJanusSig  --  Clean up everything that SetupJanusSig() created
* 
* 
* SYNOPSIS
*     VOID CleanupJanusSig(SetupSigPtr);
*                          A0
*         struct  SetupSig *SetupSigPtr;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     This routine cleans up everything that SetupJanusSig() creates.  
*     All structures are unlinked and freed, and everyone gets to go home.  
*     
*     
* INPUTS
*     SetupSigPtr = address of the SetupSig structure created by a call 
*         to SetupJanusSig()
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     SetupJanusSig()

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



JanusMemType:
* === JanusMemType() ==========================================================
* 
* NAME
*     JanusMemType  --  Return the Janus memory type of the specified pointer
* 
* 
* SYNOPSIS
*     Type = JanusMemType(MemPtr);
*     D0                  D0
* 
*         ULONG   Type;
*         APTR    MemPtr;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Accepts a Janus memory pointer as an argument and returns the 
*     Janus memory type specifier that describes the Janus memory 
*     to which the pointer points.  The type includes the Janus memory 
*     area (e.g. buffer, parameter) and the memory-access address space 
*     (e.g. byte-access, word-access).  
*     
*     Remember, garbage in gets garbage out.  The MemPtr argument must 
*     point to Janus memory for the result to be valid.  If you insist on 
*     MemPtr pointing to non-Janus memory, be sure to bake a guru cake first.  
*     
*     
* INPUTS
*     MemPtr = pointer to Janus memory
*     
*     
* RESULTS
*     Type = the Janus memory type specifier that describes the 
*         Janus memory to which the MemPtr arg refers.  
*     
*     
* SEE ALSO
*     JanusMemBase(), JanusMemToOffset, JanusOffsetToMem(), 
*     TranslateJanusPtr()

	IFGE	INFOLEVEL-59
	MOVEM.L	D0,-(SP)
	PUTMSG	59,<'%s/JanusMemType( $%lx )'>
	LEA.L	1*4(SP),SP
	ENDC

		sub.l	jb_ExpanBase(a6),d0
		blo	jmt_err

		cmp.l	#JANUSTOTALSIZE,d0
		bhs	jmt_err

		;------ compute the bank bits
		move.l	d0,d1
		and.l	#JANUSBANKMASK,d1
		eor.l	d1,d0		    ; turn off bank bits in d0
		lsr.l	#TYPEACCESSTOADDR,d1

		;------ figure out what type of memory it is
		cmp.l	#ColorOffset,d0
		blo	jmt_buffermem

		cmp.l	#ParameterOffset,d0
		blo	jmt_err

		cmp.l	#MonoVideoOffset,d0
		bhs	jmt_err

		;------ it is parameter mem
		bset	#MEMB_PARAMETER,d1
		bra	jmt_success

jmt_buffermem:
		bset	#MEMB_BUFFER,d1

jmt_success:
		move.l	d1,d0
		rts

jmt_err:
	PUTMSG	1,<'%s/JanusMemType error, about to ALERT'>
		ALERT	$7ffffffe
		rts



JanusMemBase:
* === JanusMemBase() ==========================================================
* 
* NAME
*     JanusMemBase  --  Return the base of the specified type of Janus memory
* 
* 
* SYNOPSIS
*     MemPtr = JanusMemBase(Type);
*     D0,A0                 D0
* 
*         APTR    MemPtr;
*         ULONG   Type;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Accepts a Janus memory type as an argument and returns the base of 
*     Janus memory of the specified type.  By "base" we mean the address 
*     of the first memory element (which has a Janus memory offset of 0x0000, 
*     don'cha know).  The type includes the Janus memory area (e.g. buffer, 
*     parameter) and the memory-access address space (e.g. byte-access, 
*     word-access).  
*     
*     Assembly language programmers note that the result is returned 
*     in both registers D0 and A0.
* 
*     
* INPUTS
*     Type = the Janus memory type specifier that describes the 
*         Janus memory area of interest
*     
*     
* RESULTS
*     MemPtr = pointer to the first element of the Type of Janus memory
*     
*     
* SEE ALSO
*     JanusMemType(), JanusMemToOffset, JanusOffsetToMem(), 
*     TranslateJanusPtr()

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

		add.l	jb_ExpanBase(a6),d0
		MOVE.L	D0,A0
		rts

jmb_err:
		ALERT	$7ffffffd
		LINKEXE Debug
		MOVE.L	D0,A0
		rts



JanusMemToOffset:
* === JanusMemToOffset() ======================================================
* 
* NAME
*     JanusMemToOffset  --  Create a Janus memory offset from a Janus pointer
* 
* 
* SYNOPSIS
*     Offset =  JanusMemToOffset(MemPtr)
*     D0:0-15                    D0
*         USHORT  Offset;
*         APTR    MemPtr;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Turns a Janus memory pointer into a Janus memory offset.  
* 
*     Remember, garbage in gets garbage out.  The MemPtr argument must 
*     point to Janus memory for the result to be valid.  If MemPtr points 
*     to non-Janus memory, you will be enjoined to meditate on the error 
*     of your ways.
* 
* 
* INPUTS
*     MemPtr = the Janus memory pointer to be translated into a Janus offset
*     
*     
* RESULTS
*     Offset = 16-bit Janus memory offset of the MemPtr argument
*     
*     
* SEE ALSO
*     AllocJanusMem(), JanusMemType(), JanusOffsetToMem(), SetParamOffset()

		move.l	d2,-(sp)

	IFGE	INFOLEVEL-1
	MOVE.L	D0,-(SP)
	PUTMSG	80,<'%s/JanusMemToOffset( %lx )'>
	LEA	1*4(SP),SP
	ENDC

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
		beq.s	l10			nope, then do nothing special
; this is that funny buffer mem, see if it's at segment offset $d400
		cmpi.w	#$d400,jb_ATOffset(a6)	are we in the funny bank ?
		bne.s	l10			no, leave everything alone
		subi.l	#$4000,d2		yes, back it up by 4000
l10:

		move.l	d2,d0
		move.l	(sp)+,d2
		rts


JanusLock:
* === JanusLock() =============================================================
* 
* NAME
*     JanusLock  --  Get a Janus-style lock on a byte of Janus memory
* 
* 
* SYNOPSIS
*     VOID JanusLock(BytePtr);
*                    A0
*         UBYTE   *BytePtr;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Gets a Janus-style lock on a byte of Janus memory.  
*     
*     This lock cannot be achieved by both the Amiga and the PC 
*     simultaneously, so it's safe for the two processors to use this 
*     as a mechanism by which they can share memory.  By requiring 
*     both sides to get a lock on a certain byte of memory before 
*     accessing the shared memory, each can avoid interfering with 
*     the other's usage of the memory.  
*     
*     Note that the pointer you specify as the argument to this routine 
*     must point to a byte-access address space of Janus memory.  
*     
*     The lock byte must be initialized to 0x7F before the first time it's 
*     used as a lock.  After being used as a lock byte, the system manages 
*     the value of the byte and you should not modify it.  All lock bytes 
*     provided by the system (the one in the ServiceData structure, for 
*     instance) are initialized for you.  
*     
*     Note that the lock mechanism is appropriate only for Amiga/PC locks.  
*     Amiga programs that wish to lock one another out of Janus memory should 
*     use either Forbid()/Permit() or ObtainSemaphore()/ReleaseSemaphore().
*     
*     This routine tries for the lock until it's gotten.  You may 
*     wish to consider using JanusLockAttempt(), which tries just once 
*     to get the lock.  
*     
*     A note for assembly language programmers:  there is a macro, named 
*     LOCK, which performs the same function as the JanusLock() routine.
*     You should use this macro rather than make a system call to JanusLock().
*     
*     
* EXAMPLE
*     struct ServiceData *SData;
*         if (GetService(&SData, ...) == JSERV_OK)
*             {
*             SData = (struct ServiceData *)MakeBytePtr(SData);
*             JanusLock(&SData->ServiceDataLock);
*             /* Now the ServiceData and its memory buffers are yours 
*              * to play with with impugnity.
*              */
*             JanusUnlock(&SData->ServiceDataLock);
*             ReleaseService(SData);
*             SData = NULL;
*             }
*     
*             
* INPUTS
*     BytePtr = pointer to a lock byte in byte-access address space of 
*         Janus memory.  This byte must be initialized to 0x7F before the 
*         first time it's used as a lock byte
*     
*     
* RESULTS
*     When control returns to you, you have achieved the desired lock
*     
*     
* SEE ALSO
*     JanusLockAttempt(), JanusUnlock()

		LOCK	(a0)
		rts
			 

JanusLockAttempt:
* === JanusLockAttempt() ======================================================
* 
* NAME
*     JanusLockAttempt  --  Try once to get a Janus-style lock on a byte
* 
* 
* SYNOPSIS
*     result = JanusLockAttempt(BytePtr);
*     D0:0-15                   A0
*         BOOL    Result;
*         UBYTE   *BytePtr;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Tries once to gets a Janus-style lock on a byte of Janus memory.  
*     
*     This function performs the same function as the janusLock() function, 
*     except that JanusLock() will try until it succeeds, whereas 
*     this routine will try just once and will return a BOOL TRUE or FALSE 
*     to you depending on whether it was successful or not respectively.  
*     See the JanusLock() description for details about getting a lock 
*     on Janus memory.  
*     
*     Note that the pointer you specify as the argument to this routine 
*     must point to a byte-access address space of Janus memory.  
*     
*     A note for assembly language programmers:  there is a macro, named 
*     LOCKTRY, which performs the same function as the JanusLockAttempt() 
*     routine.  You should use this macro rather than make a system call 
*     to JanusLockAttempt().
*     
*     
* EXAMPLE
*     struct ServiceData *SData;
*         if (GetService(&SData, ...) == JSERV_OK)
*             {
*             SData = (struct ServiceData *)MakeBytePtr(SData);
*             if (JanusLockAttempt(&SData->ServiceDataLock))
*                 {
*                 /* Now the ServiceData and its memory buffers are yours 
*                  * to play with with impugnity.
*                  */
*                 JanusUnlock(&SData->ServiceDataLock);
*                 }
*             ReleaseService(SData);
*             SData = NULL;
*             }
*     
*             
* INPUTS
*     BytePtr = pointer to a lock byte in byte-access address space of 
*         Janus memory.  This byte must be initialized to 0x7F before the 
*         first time it's used as a lock byte
*     
*     
* RESULTS
*     result = BOOL TRUE if you got the lock, FALSE if you didn't
*     
*     
* SEE ALSO
*     JanusLock(), JanusUnlock()

		LOCKTRY	(A0)
		RTS



JanusUnlock: 
* === JanusUnlock() ===========================================================
* 
* NAME
*     JanusUnlock  --  Release a Janus-style lock on a byte of Janus memory
* 
* 
* SYNOPSIS
*     VOID JanusUnlock(BytePtr);
*                      A0
*         UBYTE   *BytePtr;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Release a Janus-style lock on a byte of Janus memory that has 
*     been locked via a call to JanusLock() or JanusLockAttempt().  
*     
*     Note that the pointer you specify as the argument to this routine 
*     must point to a byte-access address space of Janus memory.  
*     
*     A note for assembly language programmers:  there is a macro, named 
*     UNLOCK, which performs the same function as the JanusUnlock() routine.
*     You should use this macro rather than make a system call to JanusUnlock().
*     
*     
* EXAMPLE
*     struct ServiceData *SData;
*         if (GetService(&SData, ...) == JSERV_OK)
*             {
*             SData = (struct ServiceData *)MakeBytePtr(SData);
*             JanusLock(&SData->ServiceDataLock);
*             /* Now the ServiceData and its memory buffers are yours 
*              * to play with with impugnity.
*              */
*             JanusUnlock(&SData->ServiceDataLock);
*             ReleaseService(SData);
*             SData = NULL;
*             }
*     
*             
* INPUTS
*     BytePtr = pointer to a lock byte in byte-access address space of 
*         Janus memory
*     
*     
* RESULTS
*     When control returns to you, you have released the lock
*     
*     
* SEE ALSO
*     JanusLock(), JanusLockAttempt()

		UNLOCK	(a0)
		rts

       END

