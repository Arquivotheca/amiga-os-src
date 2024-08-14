*************************************************************************
*                                                                       *
* Copyright (C) 1985-1991 Commodore Amiga Inc.  All rights reserved.    *
*                                                                       *
*************************************************************************

*************************************************************************
*
* $Id: utils.asm,v 39.1 92/07/01 20:12:39 mks Exp $
*
* $Log:	utils.asm,v $
* Revision 39.1  92/07/01  20:12:39  mks
* Fixed autodoc typos
* 
* Revision 39.0  91/10/28  16:25:39  mks
* First release of native build V39 expansion code
*
*************************************************************************

	NOLIST
	include	"exec/types.i"
	include	"exec/nodes.i"
	include	"exec/lists.i"
	include	"exec/libraries.i"
	include	"exec/ables.i"
	include	"exec/memory.i"
	include	"exec/alerts.i"

	include	"configregs.i"
	include	"configvars.i"
	include	"private_base.i"

	include "messages.i"
	include	"asmsupp.i"
	LIST


	XLIB	AllocMem
	XLIB	FreeMem
	XLIB	AllocExpansionMem
	XLIB	FreeExpansionMem
	XLIB	Enqueue
	XLIB	ReadExpansionByte
	XLIB	Alert

	XSEM	ObtainSemaphore
	XSEM	ReleaseSemaphore

	XDEF	SlotsNeeded
	XDEF	AllocConfigDev
	XDEF	FreeConfigDev
	XDEF	AllocExpansionMem
	XDEF	AllocBoardMem
	XDEF	FreeBoardMem
	XDEF	FreeExpansionMem
	XDEF	AddConfigDev
	XDEF	RemConfigDev
	XDEF	FindConfigDev
	XDEF	ReadExpansionByte
	XDEF	WriteExpansionByte
	XDEF	WriteExpansionWord
	XDEF	ReadExpansionRom
	XDEF	ObtainConfigBinding
	XDEF	ReleaseConfigBinding
	XDEF	SetCurrentBinding
	XDEF	GetCurrentBinding

	TASK_ABLES
	INT_ABLES


******* expansion.library/AllocConfigDev ********************
*
*   NAME
*	AllocConfigDev - allocate a ConfigDev structure
*
*   SYNOPSIS
*	configDev = AllocConfigDev()
*	D0
*
*   FUNCTION
*	This routine returns the address of a ConfigDev structure.
*	It is provided so new fields can be added to the structure
*	without breaking old, existing code.  The structure is cleared
*	when it is returned to the user.
*
*   INPUTS
*
*   RESULTS
*	configDev - either a valid ConfigDev structure or NULL.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	FreeConfigDev
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


AllocConfigDev:
		move.l	#MEMF_CLEAR!MEMF_CHIP,D1
		moveq	#ConfigDev_SIZEOF,D0
		LINKEXE	AllocMem
		rts


******* expansion.library/FreeConfigDev *********************
*
*   NAME
*	FreeConfigDev - free a ConfigDev structure
*
*   SYNOPSIS
*	FreeConfigDev( configDev )
*	               A0
*
*   FUNCTION
*	This routine frees a ConfigDev structure as returned by
*	AllocConfigDev.
*
*   INPUTS
*	configDev - a valid ConfigDev structure.
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	AllocConfigDev
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

FreeConfigDev:
		move.l	a0,a1
		moveq	#ConfigDev_SIZEOF,D0

		LINKEXE	FreeMem

		RTS

******* expansion.library/AllocExpansionMem *****************
*
*   NAME
*	AllocExpansionMem - allocate expansion memory
*
*   SYNOPSIS
*	startSlot = AllocExpansionMem( numSlots, slotOffset )
*	D0                             D0        D1
*
*   FUNCTION
*	(Not typically called by user code)
*
*	This function allocates numslots of expansion space (each slot
*	is E_SLOTSIZE bytes).  It returns the slot number of the
*	start of the expansion memory.  The EC_MEMADDR macro may be
*	used to convert this to a memory address.
*
*	Boards that fit the expansion architecture have alignment
*	rules.  Normally a board must be on a binary boundary of its
*	size.  Four and Eight megabyte boards have special rules.
*	User defined boards might have other special rules.
*
*	If AllocExpansionMem() succeeds, the startSlot will satisfy
*	the following equation:
*
*		(startSlot - slotOffset) MOD slotAlign = 0
*
*   INPUTS
*	numSlots - the number of slots required.
*	slotOffset - an offset from that boundary for startSlot.
*
*   RESULTS
*	startSlot - the slot number that was allocated, or -1 for error.
*
*   EXAMPLES
*
*		AllocExpansionMem( 2, 0 )
*
*	Tries to allocate 2 slots on a two slot boundary.
*
*		AllocExpansionMem( 64, 32 )
*
*	This is the allocation rule for 4 meg boards.  It allocates
*	4 megabytes (64 slots) on an odd 2 meg boundary.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	FreeExpansionMem
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

AllocExpansionMem:

		movem.l	d2/d3,-(sp)
		move.l	d0,d2
		move.l	d1,d3

		;------ some simple range checking
		add.l	d1,d0
		cmp.l	#TOTALSLOTS,d0
		bhs.s	AllocExpansionMem_Zorro_III

		;------ first try from the higher expansion area
		cmp.w	#7,d2
		bhi.s	AllocExpansionMem_8meg

		move.l	#$f0,d1
		move.l	#$e8,d0
		bsr.s	AllocExpansionSlot

		;------ quit if that one succeeded
		tst.l	d0
		bpl.s	AllocExpansionMem_End

		;------ try to allocate from the big space
AllocExpansionMem_8meg:
		move.l	#TOTALSLOTS,d1
		move.l	d3,d0
		bsr.s	AllocExpansionSlot

AllocExpansionMem_End:
		movem.l	(sp)+,d2/d3
		rts

*********************************************************************
*
*	D2.L 			- slots needed
*	eb_Z3_ConfigAddr(a6)	- current slot
*
*	(CurrentSlot + needed-1) & ~(needed-1)
*
AllocExpansionMem_Zorro_III:
		moveq	#0,d0
		move.w	eb_Z3_ConfigAddr(a6),d0	;(current)
		move.l	d2,d1			;(needed)
		subq.l	#1,d1			;(needed-1)
		add.l	d1,d0			;(current + needed-1)
		not.l	d1
		and.l	d1,d0			;(current + needed) & mask
		;[D0=result of funciton]

		add.l	d0,d2			;D2=needed + result
		move.w	d2,eb_Z3_ConfigAddr(a6)	;add needed to current
		bra.s	AllocExpansionMem_End


*********************************************************************

AllocExpansionSlot:	; ( first:d0, last:d1, numslots:d2 )

		movem.l	a2/a3,-(sp)

		lea.l	eb_AllocTable(a6),a0
		lea.l	0(a0,d0.l),a2
		lea.l	0(a0,d1.l),a3
		bra.s	AllocExpansionSlot_MainLoop

AllocExpansionSlot_Increment:
		;------ add the requested size to the current pointer
		add.l	d2,a2

		;------ main loop: try until we run out of expansion space
AllocExpansionSlot_MainLoop:
		cmp.l	a2,a3
		bls.s	AllocExpansionSlot_Fail

		;------ see if we fit in this range
		move.l	a2,a1
		move.l	d2,d1
		subq.l	#1,d1

AllocExpansionSlot_CheckLoop:
		tst.b	(a1)+		; if zero then slot is allocated
		dbeq	d1,AllocExpansionSlot_CheckLoop
		beq.s	AllocExpansionSlot_Increment

		;------ success! mark the slots as taken
		CLEAR	d0
		move.l	d2,d1
		move.l	a2,a1
		bra.s	AllocExpansionSlot_TLoopEntry

AllocExpansionSlot_TakenLoop:
		move.b	d0,(a1)+
AllocExpansionSlot_TLoopEntry:
		dbra	d1,AllocExpansionSlot_TakenLoop

		;------ return the slot number
		sub.l	a0,a2
		move.l	a2,d0
		bra.s	AllocExpansionSlot_End


AllocExpansionSlot_Fail:
		moveq	#-1,d0
AllocExpansionSlot_End:
		movem.l	(sp)+,a2/a3
		rts



*****i* expansion.library/AllocBoardMem *********************
*
*   NAME
*	AllocBoardMem - allocate memory for a Zorro II card
*
*   SYNOPSIS
*	startSlot = AllocBoardMem( slotSpec )
*	D0                         D0:8
*
*   FUNCTION
*	(Not typically called by user code)
*
*	This function allocates numslots of expansion space (each slot
*	is E_SLOTSIZE bytes).  It returns the slot number of the
*	start of the expansion memory.  The EC_MEMADDR macro may be
*	used to convert this to a memory address.
*
*	AllocBoardMem() knows about the intricacies of expansion
*	board hardware and will allocate the proper expansion
*	memory for each board type.
*
*   INPUTS
*	slotSpec - the entire er_Type byte of the Zorro II board needing
*		space (passing just the memory size field is also ok).
*
*   RESULTS
*	startSlot - the slot number that was allocated, or -1 for error.
*
*   EXAMPLES
*		struct ExpansionRom *er;
*		slot = AllocBoardMem( er->er_Type )
*
*   EXCEPTIONS
*
*   SEE ALSO
*	AllocExpansionMem, FreeExpansionMem, FreeBoardMem
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*/


boardMapTable:	; size,offset
	DC.B	128,32
	DC.B	1,0
	DC.B	2,0
	DC.B	4,0
	DC.B	8,0
	DC.B	16,0
	DC.B	32,0
	DC.B	64,32


AllocBoardMem:
		and.w	#ERT_MEMMASK,d0

		lsl.w	#1,d0		;:OPTIMIZE:
		lea	boardMapTable(pc,d0.w),a0

		CLEAR	d0
		CLEAR	d1

		move.b	(a0)+,d0	; size
		move.b	(a0)+,d1	; offset

		JMPSYS	AllocExpansionMem


*****i* expansion.library/FreeBoardMem **********************
*
*   NAME
*	FreeBoardMem - free standard Zorro II expansion memory
*
*   SYNOPSIS
*	FreeBoardMem( startSlot, slotSpec )
*	              D0         D1
*
*   FUNCTION
*	(Not typically called by user code)
*
*	This function frees numslots of expansion space (each slot
*	is E_SLOTSIZE bytes).  It is the inverse function of
*	AllocBoardMem().
*
*   INPUTS
*	startSlot - a slot number in expansion space.
*	slotSpec - the entire er_Type byte of the Zorro II board
*		(passing just the memory size field is also ok).
*
*
*   RESULTS
*
*   EXAMPLES
*
*	struct ExpansionRom *er;
*	int startSlot;
*	int slotSpec;
*
*	slotSpec = er->er_Type & ERT_MEMMASK;
*	startSlot = AllocBoardMem( er->er_Type & ERT_MEMMAK );
*
*	if( startSlot != -1 ) {
*	    FreeBoardMem( startSlot, slotSpec );
*	}
*
*   EXCEPTIONS
*	If the caller tries to free a slot that is already in the
*	free list, FreeBoardMem will Alert() (e.g. crash the
*	system).
*
*
*   SEE ALSO
*	AllocExpansionMem, FreeExpansionMem, AllocBoardMem
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


FreeBoardMem:
		;------ we don't assume that slotspec is OK (e.g. < 7)
		and.w	#ERT_MEMMASK,d1
		lsl.w	#1,d1
		move.b	boardMapTable(pc,d1.w),d1

		JMPSYS	FreeExpansionMem


SlotsNeeded:	moveq	#ERT_MEMMASK,d1
		and.l	d1,d0
		lsl.w	#1,d0
		move.b	boardMapTable(pc,d0.w),d0
		rts


******* expansion.library/FreeExpansionMem ******************
*
*   NAME
*	FreeExpansionMem - allocate standard device expansion memory
*
*   SYNOPSIS
*	FreeExpansionMem( startSlot, numSlots )
*	                  D0         D1
*
*   FUNCTION
*	(Not typically called by user code)
*
*	This function allocates numslots of expansion space (each slot
*	is E_SLOTSIZE bytes).  It is the inverse function of
*	AllocExpansionMem().
*
*   INPUTS
*	startSlot - the slot number that was allocated, or -1 for error.
*	numSlots - the number of slots to be freed.
*
*   RESULTS
*
*   EXAMPLES
*
*   EXCEPTIONS
*	If the caller tries to free a slot that is already in the
*	free list, FreeExpansionMem will Alert() (e.g. crash the
*	system).
*
*   SEE ALSO
*	AllocExpansionMem
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


FreeExpansionMem:

		;------ simple sanity check on slot to be freed
		move.l	d0,a0
		add.l	d1,a0
		cmp.w	#TOTALSLOTS,a0
		bls.s	FreeExpansionMem_DoIt

		;------ something is wrong!
FreeExpansionMem_Alert:
		ALERT	AN_BadExpansionFree
		bra.s	FreeExpansionMem_End

FreeExpansionMem_DoIt:

		lea	eb_AllocTable(a6,d0.l),a0
		moveq	#1,d0

		bra.s	FreeExpansionMem_Entry

FreeExpansionMem_Loop:
		tst.b	(a0)
		bne.s	FreeExpansionMem_Alert	; already free
		move.b	d0,(a0)+
FreeExpansionMem_Entry
		dbra	d1,FreeExpansionMem_Loop

FreeExpansionMem_End:
		rts



******* expansion.library/AddConfigDev **********************
*
*   NAME
*	AddConfigDev - add a new ConfigDev structure to the system
*
*   SYNOPSIS
*	AddConfigDev( configDev )
*	              A0
*
*   FUNCTION
*	(Not typically called by user code)
*
*	This routine adds the specified ConfigDev structure to the
*	list of Configuration Devices in the system.
*
*   INPUTS
*	configDev - a valid ConfigDev structure.
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	RemConfigDev
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

AddConfigDev:
		move.l	a6,-(sp)

	IFGE	INFOLEVEL-300
	move.l	a0,-(sp)
	INFOMSG	300,<'AddConfigDev: adding 0x%lx'>
	addq.l	#4,sp
	ENDC

		move.l	a0,a1
		lea	eb_BoardList(a6),a0
		move.l	ABSEXECBASE,a6

		FORBID
		CALLSYS	Enqueue
		PERMIT

		move.l	(sp)+,a6
		rts


******* expansion.library/RemConfigDev **********************
*
*   NAME
*	RemConfigDev - remove a ConfigDev structure from the system
*
*   SYNOPSIS
*	RemConfigDev( configDev )
*	              A0
*
*   FUNCTION
*	(Not typically called by user code)
*
*	This routine removes the specified ConfigDev structure from the
*	list of Configuration Devices in the system.
*
*   INPUTS
*	configDev - a valid ConfigDev structure.
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	AddConfigDev
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

RemConfigDev:
		move.l	a6,-(sp)

		move.l	ABSEXECBASE,a6
		move.l	a0,a1

		FORBID
		REMOVE
		PERMIT

		move.l	(sp)+,a6
		rts

******* expansion.library/FindConfigDev *********************
*
*   NAME
*	FindConfigDev - find a matching ConfigDev entry
*
*   SYNOPSIS
*	configDev = FindConfigDev( oldConfigDev, manufacturer, product )
*	D0                         A0            D0            D1
*
*   FUNCTION
*	This routine searches the list of existing ConfigDev
*	structures in the system and looks for one that has
*	the specified manufacturer and product codes.
*
*	If the oldConfigDev is NULL the the search is from the
*	start of the list of configuration devices.  If it is
*	not null then it searches from the first configuration
*	device entry AFTER oldConfigDev.
*
*	A code of -1 is treated as a wildcard -- e.g. it matches
*	any manufacturer (or product)
*
*   INPUTS
*	oldConfigDev - a valid ConfigDev structure, or NULL to start
*		from the start of the list.
*	manufacturer - the manufacturer code being searched for, or
*		-1 to ignore manufacturer numbers.
*	product - the product code being searched for, or -1 to
*		ignore product numbers.
*
*   RESULTS
*	configDev - the next ConfigDev entry that matches the
*		manufacturer and product codes, or NULL if there
*		are no more matches.
*
*   EXCEPTIONS
*
*   EXAMPLES
*	/* to find all configdevs of the proper type */
*	struct ConfigDev *cd = NULL;
*
*	while( cd = FindConfigDev( cd, MANUFACTURER, PRODUCT ) ) {
*		/* do something with the returned ConfigDev */
*	}
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

FindConfigDev:
		move.l	d2,-(sp)

	IFGE	INFOLEVEL-200
	movem.l	d0/d1,-(sp)
	move.l	a0,-(sp)
	INFOMSG	200,<'FindConfigDev 0x%lx, %ld/%ld'>
	lea	12(sp),sp
	ENDC

		;------ set a1 to -1 for use in wildcard checking
		lea	-1,a1

		;------ see if the user specified a config dev structure
		move.l	a0,d2
		bne.s	FindConfigDev_GotCD

		;------ start from the beginning of the list
		lea	eb_BoardList(a6),a0

FindConfigDev_GotCD:
		move.l	(a0),d2

FindConfigDev_Loop:
		;------ see if we are at the end of the loop
		move.l	d2,a0
		move.l	(a0),d2
		beq.s	FindConfigDev_Fail


		;------ do all the checking.  -1 (a1) is a wild card.
		cmp.l	a1,d0
		beq.s	FindConfigDev_CheckProduct

		cmp.w	cd_Rom+er_Manufacturer(a0),d0
		bne.s	FindConfigDev_Loop

FindConfigDev_CheckProduct:
		cmp.l	a1,d1
		beq.s	FindConfigDev_Success

		cmp.b	cd_Rom+er_Product(a0),d1
		bne.s	FindConfigDev_Loop

FindConfigDev_Success:
		move.l	a0,d0
		bra.s	FindConfigDev_End

FindConfigDev_Fail:
		CLEAR	d0

FindConfigDev_End:

	IFGE	INFOLEVEL-200
	move.l	d0,-(sp)
	INFOMSG	200,<'FindConfigDev: returning 0x%lx'>
	addq.l	#4,sp
	ENDC

		move.l	(sp)+,d2
		rts


******* expansion.library/ReadExpansionByte *****************
*
*   NAME
*	ReadExpansionByte - read a byte nybble by nybble.
*
*   SYNOPSIS
*	byte = ReadExpansionByte( board, offset )
*	D0                        A0     D0
*
*   FUNCTION
*	(Not typically called by user code)
*
* 	ReadExpansionByte reads a byte from a new-style expansion
*	board.  These boards have their readable data organized
*	as a series of nybbles in memory.  This routine reads
*	two nybbles and returns the byte value.
*
*	In general, this routine will only be called by ReadExpansionRom.
*
*	The offset is a byte offset, as if into a ExpansionRom structure.
*	The actual memory address read will be four times larger.
*	The macros EROFFSET and ECOFFSET are provided to help get
*	these offsets from C.
*
*   INPUTS
*	board - a pointer to the base of a new style expansion board.
*	offset - a logical offset from the board base
*
*   RESULTS
*	byte - a byte of data from the expansion board.
*
*   EXAMPLES
*	byte = ReadExpansionByte( cd->BoardAddr, EROFFSET( er_Type ) );
*	ints = ReadExpansionByte( cd->BoardAddr, ECOFFSET( ec_Interrupt ) );
*
*   SEE ALSO
*	WriteExpansionByte, ReadExpansionRom
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


ReadExpansionByte:
		lsl.w	#2,d0		;times 4
		lea.l	0(a0,d0.w),a0

		move.l	a0,d1		;Cheap test for EZ3_EXPANSIONBASE
		bmi.s	reb_ZorroIII
		move.b	$002(a0),d1	;Zorro II
		bra.s	reb_Common
reb_ZorroIII:	move.b	$100(a0),d1	;Zorro III
reb_Common:	lsr.b	#4,d1

		CLEAR	d0
		move.b	(a0),d0
		and.b	#$f0,d0
		or.b	d1,d0
		rts



******* expansion.library/WriteExpansionByte ****************
*
*   NAME
*	WriteExpansionByte - write a byte nybble by nybble.
*
*   SYNOPSIS
*	WriteExpansionByte( board, offset, byte )
*			    A0     D0      D1
*
*   FUNCTION
*	(Not typically called by user code)
*
*	WriteExpansionByte writes a byte to a new-style expansion
*	board.  These boards have their writeable data organized
*	as a series of nybbles in memory.  This routine writes
*	two nybbles in a very careful manner to work with all
*	types of new expansion boards.
*
*	To make certain types of board less expensive, an expansion
*	board's write registers may be organized as either a
*	byte-wide or nybble-wide register.  If it is nybble-wide
*	then it must latch the less significant nybble until the
*	more significant nybble is written.  This allows the
*	following algorithm to work with either type of board:
*
*		write the low order nybble to bits D15-D12 of
*			byte (offset*4)+2
*
*		write the entire byte to bits D15-D8 of
*			byte (offset*4)
*
*	The offset is a byte offset into a ExpansionRom structure.
*	The actual memory address read will be four times larger.
*	The macros EROFFSET and ECOFFSET are provided to help get
*	these offsets from C.
*
*   INPUTS
*	board - a pointer to the base of a new style expansion board.
*	offset - a logical offset from the configdev base
*	byte - the byte of data to be written to the expansion board.
*
*   EXAMPLES
*	WriteExpansionByte( cd->BoardAddr, ECOFFSET( ec_Shutup ),  0 );
*	WriteExpansionByte( cd->BoardAddr, ECOFFSET( ec_Interrupt ), 1 );
*
*   SEE ALSO
*	ReadExpansionByte, ReadExpansionRom
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


WriteExpansionByte:
		lsl.w	#2,d0		;times 4
		lea.l	0(a0,d0.w),a0

		move.b	d1,d0		;D1=high bits
		lsl.b	#4,d0		;D0=low bits

		cmpa.l	#EZ3_EXPANSIONBASE,a0
		bhs.s	web_ZorroIII
		move.b	d0,$002(a0)
		bra.s	web_Common
web_ZorroIII:	move.b	d0,$100(a0)	;write low nibble to addr+
web_Common:

		move.b	d1,(a0)		;write entire byte to addr

		CLEAR	d0
		rts



*****i* expansion.library/WriteExpansionWord ****************
*
*   NAME
*	WriteExpansionWord - write a word in special ways  (V37)
*
*   SYNOPSIS
*	WriteExpansionWord( board, offset, word )
*			    A0     D0      D1
*
*   FUNCTION
*	(Not typically called by user code)
*
*	WriteExpansionWord writes a word to a Zorro III expansion
*	board.
*
*   INPUTS
*	board - a pointer to the base of a Zorro III expansion board.
*	offset - a logical offset from the configdev base
*	word - the word of data to be written to the expansion board.
*
*   SEE ALSO
*	WriteExpansionByte, ReadExpansionByte, ReadExpansionRom
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


WriteExpansionWord:
		lsl.w	#2,d0		;times 4
		lea.l	0(a0,d0.w),a0
		move.b  d1,4(a0)	;write low byte as byte to addr+4
		move.w	d1,(a0)		;write     word as word to addr
		CLEAR	d0
		rts


******* expansion.library/ReadExpansionRom ******************
*
*   NAME
*	ReadExpansionRom - read a boards configuration ROM space
*
*   SYNOPSIS
*	error = ReadExpansionRom( board, configDev )
*	D0                        A0     A1
*
*   FUNCTION
*	(Not typically called by user code)
*
*	ReadExpansionRom reads a the ROM portion of an expansion
*	device in to cd_Rom portion of a ConfigDev structure.
*	This routine knows how to detect whether or not there is
*	actually a board there,
*
*	In addition, the ROM portion of a new style expansion board
*	is encoded in ones-complement format (except for the first
*	two nybbles -- the er_Type field).  ReadExpansionRom knows
*	about this and un-complements the appropriate fields.
*
*   INPUTS
*	board - a pointer to the base of a new style expansion board.
*	configDev - the ConfigDev structure that will be read in.
*	offset - a logical offset from the configdev base
*
*   RESULTS
*	error - If the board address does not contain a valid new style
*		expansion board, then error will be non-zero.
*
*   EXAMPLES
*
*	configDev = AllocConfigDev();
*	if( ! configDev ) panic();
*
*	error = ReadExpansionBoard( board, configDev );
*	if( ! error ) {
*		configDev->cd_BoardAddr = board;
*		ConfigBoard( configDev );
*	}
*
*   SEE ALSO
*	ReadExpansionByte, WriteExpansionByte
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


ReadExpansionRom:
		movem.l	d2/a2/a3,-(sp)

		move.l	a0,a2		; board
		move.l	a1,a3		; config dev structure

		;------ room for second rom structure
		lea	-ExpansionRom_SIZEOF(sp),sp

		;------ read in the rom image
		lea	cd_Rom(a3),a1
		bsr.s	readrom

		;------ some sanity checks
		;move.b	cd_Rom+er_Flags(a3),d0	;Zorro III defines these
		;and.b	#$3f,d0			;Zorro III defines these
		;bne.s	ReadExpansionRom_Err	;Zorro III defines these

		tst.b	cd_Rom+er_Reserved03(a3)
		bne.s	ReadExpansionRom_Err

		move.w	cd_Rom+er_Manufacturer(a3),d0
		beq.s	ReadExpansionRom_Err
		cmp.w	#-1,d0
		beq.s	ReadExpansionRom_Err

		;------ make sure it reads the same
		moveq	#10,d2

ReadExpansionRom_CheckLoop:
		move.l	a2,a0
		move.l	sp,a1
		bsr.s	readrom

		lea	cd_Rom(a3),a0
		move.l	sp,a1
		moveq	#ExpansionRom_SIZEOF,d0
		bsr.s	bcmp
		tst.l	d0
		bne.s	ReadExpansionRom_Err

		dbra	d2,ReadExpansionRom_CheckLoop

		;------ we made it through the tests */
		CLEAR	d0

ReadExpansionRom_End:

	IFGE	INFOLEVEL-600
	move.l	d0,-(sp)
	INFOMSG	600,<'ReadExpansionRom: returning %ld'>
	addq.l	#4,sp
	ENDC


		;------ clean up temp structure
		lea	ExpansionRom_SIZEOF(sp),sp

		movem.l	(sp)+,d2/a2/a3
		rts

ReadExpansionRom_Err:
		moveq	#EE_NOBOARD,d0
		bra.s	ReadExpansionRom_End


**************************

readrom:	; ( board:a0, ExpansionRom: a1 )
		movem.l	d2/a2/a3,-(sp)
		move.l	a0,a2
		move.l	a1,a3


		;------ read the first byte
		CLEAR	d0
		CALLSYS	ReadExpansionByte
		move.b	d0,(a3)+

		;------ read all the other bytes (they are inverted)
		MOVEQ	#1,d2

readrom_loop:
		move.l	d2,d0
		move.l	a2,a0
		CALLSYS	ReadExpansionByte
		not.b	d0
		move.b	d0,(a3)+

		addq.w	#1,d2
		cmp.w	#ExpansionRom_SIZEOF,d2
		bls.s	readrom_loop

		movem.l	(sp)+,d2/a2/a3
		rts


*************************

bcmp:	; ( a: a0, b: a1, len : d0 )

		;------ special case for length of zero
		tst.l	d0
		beq.s	bcmp_end

		;------ adjust loop count
		subq.l	#1,d0

bcmp_loop:

		cmpm.b	(a0)+,(a1)+
		dbne	d0,bcmp_loop
		beq.s	bcmp_success

		blo.s	bcmp_lo
		moveq	#1,d0
		bra.s	bcmp_end
bcmp_lo:
		moveq	#-1,d0
		bra.s	bcmp_end

bcmp_success:
		CLEAR	d0

bcmp_end:
		rts


************************************

bcopy_loop:
		move.b	(a0)+,(a1)+
bcopy:	; ( a: a0, b: a1, len : d0 )
		dbra	d0,bcopy_loop

		rts


******* expansion.library/ObtainConfigBinding ***************
*
*   NAME
*	ObtainConfigBinding - try to get permission to bind drivers
*
*   SYNOPSIS
*	ObtainConfigBinding()
*
*
*   FUNCTION
*	ObtainConfigBinding gives permission to bind drivers to
*	ConfigDev structures.  It exists so two drivers at once
*	do not try and own the same ConfigDev structure.  This
*	call will block until it is safe proceed.
*
*	It is crucially important that people lock out others
*	before loading new drivers.  Much of the data that is used
*	to configure things is statically kept, and others need
*	to be kept from using it.
*
*	This call is built directly on Exec SignalSemaphore code
*	(e.g. ObtainSemaphore).
*
*   INPUTS
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	ReleaseConfigBinding()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

ObtainConfigBinding:
		lea	eb_BindSemaphore(a6),a0
		LINKSEM	ObtainSemaphore
		rts


******* expansion.library/ReleaseConfigBinding **************
*
*   NAME
*	ReleaseConfigBinding - allow others to bind to drivers
*
*   SYNOPSIS
*	ReleaseConfigBinding()
*
*
*   FUNCTION
*	This call should be used when you are done binding drivers
*	to ConfigDev entries.  It releases the SignalSemaphore; this
*	allows others to bind their drivers to ConfigDev structures.
*
*   SEE ALSO
*	ObtainConfigBinding()
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

ReleaseConfigBinding:
		lea	eb_BindSemaphore(a6),a0
		LINKSEM	ReleaseSemaphore
		rts


******* expansion.library/SetCurrentBinding *****************
*
*   NAME
*	SetCurrentBinding - sets static board configuration area
*
*   SYNOPSIS
*	SetCurrentBinding( currentBinding, size )
*	                   A0		   D0:16
*
*   FUNCTION
*	This function records the contents of the "currentBinding"
*	structure in a private place.  It may be read via
*	GetCurrentBinding().  This is really a kludge, but it is
*	the only way to pass extra arguments to a newly configured
*	device.
*
*	A CurrentBinding structure has the name of the currently
*	loaded file, the product string that was associated with
*	this driver, and a pointer to the head of a singly linked
*	list of ConfigDev structures (linked through the cd_NextCD
*	field).
*
*	Many devices may not need this information; they have hard
*	coded into themselves their manufacture number.  It is
*	recommended that you at least check that you can deal with
*	the product code in the linked ConfigDev structures.
*
*   INPUTS
*	currentBinding - a pointer to a CurrentBinding structure
*
*	size - The size of the user's binddriver structure.  No
*	    more than this much data will be copied.  If size is
*	    less than the library's idea a CurrentBinding size,
*	    then the library's structure will be null padded.
*
*   SEE ALSO
*	GetCurrentBinding
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/

SetCurrentBinding:

		lea	eb_CurrentBinding(a6),a1
		bra.s	bncopy
		;rts


******* expansion.library/GetCurrentBinding *****************
*
*   NAME
*	GetCurrentBinding - sets static board configuration area
*
*   SYNOPSIS
*	actual = GetCurrentBinding( currentBinding, size )
*	                            A0              D0:16
*
*   FUNCTION
*	This function writes the contents of the "currentBinding"
*	structure out of a private place.  It may be set via
*	SetCurrentBinding().  This is really a kludge, but it is
*	the only way to pass extra arguments to a newly configured
*	device.
*
*	A CurrentBinding structure has the name of the currently
*	loaded file, the product string that was associated with
*	this driver, and a pointer to the head of a singly linked
*	list of ConfigDev structures (linked through the cd_NextCD
*	field).
*
*	Many devices may not need this information; they have hard
*	coded into themselves their manufacture number.  It is
*	recommended that you at least check that you can deal with
*	the product code in the linked ConfigDev structures.
*
*   INPUTS
*	currentBinding - a pointer to a CurrentBinding structure
*
*	size - The size of the user's binddriver structure.
*	    Do not pass in less than sizeof(struct CurrentBinding).
*
*   RESULTS
*	actual - the true size of a CurrentBinding structure is returned.
*
*   SEE ALSO
*	GetCurrentBinding
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	Note bug:  If the passed size parameter (d0) is less than the
*	CurrentBinding size, this clears the user's buffer to
*	the CurrentBinding size anyway.  Not fixed for 2.0 (no big
*	deal; the CurrentBinding size did not change).
*
*   REGISTER USAGE
*
*/


GetCurrentBinding:
		move.l	a0,a1
		lea.l	eb_CurrentBinding(a6),a0


		;[used above]
bncopy:		; ( a0:source, a1:dest, d0:copysize)

		moveq	#CurrentBinding_SIZEOF,d1


		;------ copy can be no larger than max
		cmp.l	d0,d1
		bhi.s	10$
		move.l	d1,d0
10$:		;------ let d1 be the number of bytes to pad
		sub.l	d0,d1


		;------ copy the structure
		bra.s	bncopy_copyentry
bncopy_copyloop:
		move.b	(a0)+,(a1)+
bncopy_copyentry:
		dbra	d0,bncopy_copyloop


		;------ now fill the structure
		bra.s	bncopy_pad
bncopy_padloop:	clr.b	(a1)+
bncopy_pad:	dbra	d1,bncopy_padloop


		moveq	#CurrentBinding_SIZEOF,d0
		rts


		END
