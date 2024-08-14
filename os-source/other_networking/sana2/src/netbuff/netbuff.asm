**********************************************************************
*
* NetBuff.asm
*
* Copyright 1991 Raymond S. Brand, All rights reserved.
*
* 19910208
*
* $Id$
*
**********************************************************************

	INCLUDE	"internal.i"


	SECTION	NetBuff

	XDEF	AllocSegments		; works
	XDEF	IntAllocSegments	; works
	XDEF	FreeSegments		; works
	XDEF	IntFreeSegments		; works
	XDEF	SplitNetBuff		; works
	XDEF	TrimNetBuff		; works
	XDEF	CopyToNetBuff		; works
	XDEF	CopyFromNetBuff		; works
	XDEF	CopyNetBuff		; works
	XDEF	CompactNetBuff		;
	XDEF	ReadyNetBuff		; works
	XDEF	IsContiguous		; works
	XDEF	NetBuffAppend		; works
	XDEF	PrependNetBuff		; works
	XDEF	ReclaimSegments		;

	XSYS	AllocMem
	XSYS	FreeMem

	XREF	DeferedFree
	XREF	CopyBytes
	XREF	InsertList
	XREF	MoveSubList


******* netbuff.library **********************************************
*
*	Netbuff.library maintains the free pool of NetBuffSegments for
*	use by the various network modules. It also contains several
*	utility functions for dealing with NetBuffs as a data
*	structure for "holding" network packet data as well as
*	allocators and deallocators for NetBuffSegments.
*
*	The intent is for all of the network modules to use a common
*	pool of buffers so as to reduce the memory requirements of the
*	network software, and for the network modules to be able to
*	pass network data (packets) with minimal copying.
*
*   STRUCTURES
*	A NetBuff is a data structure representing a logical array of
*	of bytes.
*
*	    struct NetBuff
*	        {
*	        struct MinList List;
*	        ULONG Count;
*	        };
*
*	    List
*	        List of NetBuffSegments.
*
*	    Count
*	        The number of bytes of data the NetBuff is said to
*	        contain.
*
*
*	A NetBuffSegment is a data structure used to store and keep
*	track of all or part of the data in a NetBuff.
*
*	    struct NetBuffSegment
*	        {
*	        struct MinNode Node;
*	        ULONG PhysicalSize;
*	        ULONG DataOffset;
*	        ULONG DataCount;
*	        UBYTE *Buffer;
*	        };
*
*	    Node
*	        Node structure linking the NetBuffSegments together.
*
*	    PhysicalSize
*	        The size of the data area that Buffer points to. If
*	        this field is zero, the actual size of the data area
*	        unknown and it is managed (allocated and freed) by
*	        some other entity. Only 'Count' bytes starting at
*	        '(Buffer+Offset)' are known to be valid.
*
*	    DataOffset
*	        Offset into the Buffer where the data starts.
*
*	    DataCount
*	        Number of data bytes that this NetBuffSegment
*	        contains.
*
*	    Buffer
*	        Pointer to the start of the data area for this
*	        NetBuffSegment.
*
*   PHYSICALSIZE ZERO SEGMENTS
*	Any software making use of NetBuffs must correctly handle
*	NetBuffs with PhysicalSize zero segments. The general rules to
*	follow are as follows:
*
*	+   "Send" NetBuffs, those NetBuffs where the data is created
*	    and passed to a lower network layer, are returned to the
*	    creator with the data intact. This class of NetBuffs may
*	    have segments of PhysicalSize zero.
*
*	+   "Receive" NetBuffs, those NetBuffs where the data is
*	    created and passed to a higher network layer, are not
*	    returned to the creator. This class of NetBuffs may not
*	    have segments of PhysicalSize zero.
*
*	+   Lower network layers will eventually return the actual
*	    NetBuff structure (pointer) to the layer that created the
*	    data.
*
*	+   NetBuffs returned to higher layers from lower layers may
*	    have the "physical" layout of the data changed. The layout
*	    of the "logical" data will not have changed.
*
*	+   Track NetBuff structures to know when it is safe to reuse/
*	    deallocate storage for PhysicalSize zero segments.
*
*	In to above, higher and lower network layers refers to the
*	usual diagram of the OSI network layering model; where the
*	application layer is at the top of the diagram and the
*	physical layer is at the bottom.
*
*	This model for handling PhysicalSize zero segments also has
*	obvious advantages in situations where time-outs and
*	retransmittions occure.
*
*   CREDITS
*	Raymond S. Brand   rsbx@cbmvax.commodore.com   (215) 431-9100
*	Martin Hunt      martin@cbmvax.commodore.com   (215) 431-9100
*	Perry Kivolowitz           ASDG Incorporated   (608) 273-6585
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*


******* netbuff.library/AllocSegments() *******************************
*
*   NAME
*	AllocSegments -- Get a list of NetBuffSegments.
*
*   SYNOPSIS
*	AllocSegments( Count, Segment_List )
*	               D0     A0
*
*	void AllocSegments( ULONG, struct List * );
*
*   FUNCTION
*	This function returns a list of NetBuffSegments sufficient to
*	hold Count bytes.
*
*   INPUTS
*	Count           Numbers of bytes for which space is needed.
*	Segment_List    Pointer to an empty (initialized) list to hold
*	                    the allocated NetBuffSegments.
*
*   RESULTS
*
*   NOTES
*	The function cannot be called from interrupts.
*
*   SEE ALSO
*	netbuff.library/IntAllocSegments(),
*	netbuff.library/FreeSegments(),
*	netbuff.library/ReadyNetBuff()
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
* D0 --	count	scratch	segment
* D1 --	?????	?????	?????
* D2 --	?????	togo	restored
* A0 --	list	pool	?????
* A1 --	?????	scratch	?????
* A2 --	?????	list	restored
* A5 --	?????	exec	restored
* A6 --	library	library	library
*

AllocSegments:
*!* debug
*	ERRMSG	"AllocSegments"
*!* debug
	bsr.s	AllocSegmentsCommon
	bra.w	DeferedFree		; preserves registers!
	; rft


******* netbuff.library/IntAllocSegments() ****************************
*
*   NAME
*	IntAllocSegments -- Get a list of NetBuffSegments.
*
*   SYNOPSIS
*	IntAllocSegments( Count, Segment_List )
*	                  D0     A0
*
*	void IntAllocSegments( ULONG, struct List * );
*
*   FUNCTION
*	This function returns a list of NetBuffSegments sufficient to
*	hold Count bytes.
*
*   INPUTS
*	Count           Numbers of bytes for which space is needed.
*	Segment_List    Pointer to an empty (initialized) list to hold
*	                    the allocated NetBuffSegments.
*
*   RESULTS
*
*   NOTES
*	The function should be called only from interrupts.
*
*   SEE ALSO
*	netbuff.library/AllocSegments(),
*	netbuff.library/IntFreeSegments(),
*	netbuff.library/ReadyNetBuff()
*
*   BUGS
*	Since this function may be called from interrupts, it can not
*	actually allocate memory from the system. It, therefor, relies
*	on a "friendly" task or process to add NetBuffSegments to the
*	free pool via the netbuff.library/FreeSegments() function.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	count	scratch	segment
* D1 --	?????	?????	?????
* D2 --	?????	togo	restored
* A0 --	list	pool	?????
* A1 --	?????	scratch	?????
* A2 --	?????	list	restored
* A5 --	?????	exec	restored
* A6 --	library	library	library
*

IntAllocSegments:
*!* debug
*	ERRMSG	"IntAllocSegments"
*!* debug

AllocSegmentsCommon
*!* debug
*	ERRMSG	"AllocSegmentsCommon"
*!* debug
	movem.l	a2/a5/d2,-(sp)
	movem.l	NBL_SYSLIB(a6),a5
	move.l	a0,a2
	move.l	d0,d2

ASCLoop
	tst.l	d2
	ble.s	ASCDone

	lea.l	NBL_FREEPOOL(a6),a0
	DISABLE_V36	a5,NOFETCH
	REMHEAD
	ENABLE_V36	a5,NOFETCH
	tst.l	d0
	beq.w	ASCBad

	move.l	d0,a1
	move.l	#0,NBS_DATAOFFSET(a1)
	move.l	#0,NBS_DATACOUNT(a1)
	sub.l	NBS_PHYSICALSIZE(a1),d2
	move.l	a2,a0
	ADDTAIL
	bra.s	ASCLoop

ASCDone
	movem.l	(sp)+,a2/a5/d2
	rts

ASCBad
	move.l	a2,a0
	movem.l	(sp)+,a2/a5/d2
	bra.w	FreeSegmentsCommon
	;rtf


******* netbuff.library/FreeSegments() ********************************
*
*   NAME
*	FreeSegments -- Return a list of NetBuffSegments.
*
*   SYNOPSIS
*	FreeSegments( Segment_list )
*	              A0
*
*	void FreeSegments( struct List * );
*
*   FUNCTION
*	This function gives a list of NetBuffSegments to the system
*	free pool.
*
*   INPUTS
*	Segment_list    Pointer to the list of NetBuffSegments to add
*	                    to the system free NetBuffSegment pool.
*
*   NOTES
*	When this routine encounters a NetBuffSegment with
*	PhysicalSize of 0, the data area is left untouched but the
*	NetBuffSegment structure which points to the data is freed
*	using exec.library/FreeMem().
*
*   SEE ALSO
*	netbuff.library/IntFreeSegments(),
*	netbuff.library/AllocSegments(),
*	netbuff.library/ReadyNetBuff()
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
* D0 --	?????	scratch	?????
* D1 --	?????	scratch	?????
* A0 --	list	scratch	?????
* A1 --	?????	scratch	?????
* A6 --	library	library	library
*

FreeSegments:
*!* debug
*	ERRMSG	"FreeSegments"
*!* debug
	bsr.s	FreeSegmentsCommon
	bra.w	DeferedFree		; preserves registers!
	; rtf


******* netbuff.library/IntFreeSegments() *****************************
*
*   NAME
*	IntFreeSegments -- Return a list of NetBuffSegments.
*
*   SYNOPSIS
*	IntFreeSegments( Segment_list )
*	                 A0
*
*	void IntFreeSegments( struct List * );
*
*   FUNCTION
*	This function gives a list of NetBuffSegments to the system
*	free pool.
*
*   INPUTS
*	Segment_list    Pointer to the list of NetBuffSegments to add
*	                    to the system free NetBuffSegment pool.
*
*   NOTES
*	The function should be called only from interrupts.
*
*	When this routine encounters a NetBuffSegment with
*	PhysicalSize of 0, that data area is left untouched but the
*	NetBuffSegment structure which points to the data is freed
*	using exec.library/FreeMem().
*
*   SEE ALSO
*	netbuff.library/FreeSegments(),
*	netbuff.library/IntAllocSegments(),
*	netbuff.library/ReadyNetBuff()
*
*   BUGS
*	This function relies on the non-interrupt functions to perform
*	garbage collection of segments of physical size zero.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	?????	scratch	?????
* D1 --	?????	scratch	?????
* A0 --	list	scratch	?????
* A1 --	?????	node	?????
* A2 --	?????	list	restored
* A5 --	?????	exec	restored
* A6 --	library	library	library
*

IntFreeSegments:
*!* debug
*	ERRMSG	"IntFreeSegments"
*!* debug

FreeSegmentsCommon
*!* debug
*	ERRMSG	"FreeSegmentsCommon"
*!* debug
	movem.l	a2/a5,-(sp)
	move.l	NBL_SYSLIB(a6),a5
	move.l	a0,a2

IFSCLoop
	move.l	a2,a0
	REMHEAD
	tst.l	d0
	beq.w	IFSCDone
	move.l	d0,a1
	tst.l	NBS_PHYSICALSIZE(a1)
	bne.s	IFSCLFree

	DISABLE_V36	a5,NOFETCH
	move.l	NBL_DEFERED(a6),a0
	move.l	a0,NBS_NODE+MLN_SUCC(a1)
	move.l	a1,NBL_DEFERED(a6)
	ENABLE_V36	a5,NOFETCH
	bra.s	IFSCLoop

IFSCLFree
	DISABLE_V36	a5,NOFETCH
	lea.l	NBL_FREEPOOL(a6),a0
	ADDTAIL
	ENABLE_V36	a5,NOFETCH
	bra.w	IFSCLoop

IFSCDone
	movem.l	(sp)+,a2/a5
	moveq.l	#0,d0
	rts


******* netbuff.library/SplitNetBuff() *******************************
*
*   NAME
*	SplitNetBuff -- Split a NetBuff in to two NetBuffs.
*
*   SYNOPSIS
*	error = SplitNetBuff( Netbuff0, Count, Netbuff1 )
*	D0                    A0        D0     A1
*
*	LONG SplitNetBuff( struct NetBuff *, LONG, struct NetBuff * );
*
*   FUNCTION
*	This function takes a NetBuff and splits it at the specified
*	offset. The sign of Count determines which bytes will be
*	removed from Netbuff0 and placed in Netbuff1. If Count is
*	non-negative, the first 'Offset' bytes in Netbuff0 will be
*	moved to Netbuff1. If Offset is negative, the last
*	'abs(Offset)' bytes in Netbuff0 will be moved to Netbuff1.
*
*	All data in Netbuff1 will be lost when this function is
*	called.
*
*   INPUTS
*	Netbuff0        Pointer to NetBuff structure with data to
*	                    split.
*	Count           Number of bytes to split off.
*	Netbuff1        Pointer to a NetBuff structure to receive
*	                    split-off data.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function might be used within a protocol stack to split
*	a packet into smaller NetBuffs so as to fall below a specific
*	transport medium's maximum transfer unit.
*	
*   SEE ALSO
*	netbuff.library/TrimNetBuff(),
*	netbuff.library/NetBuffAppend(),
*	netbuff.library/PrependNetBuff()
*
*   BUGS
*	If the split point is in a NetBuffSegment of PhysicalSize
*	zero, exec/AllocMem() will be called to create a new segment
*	of PhysicalSize zero.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
* D0 --	offset	?????	error
* D1 --	?????	?????	?????
* D2 --	?????	?????	restored
* A0 --	netbuf0	?????	?????
* A1 --	netbuf1	?????	?????
* A2 --	?????	netbuf0	restored
* A3 --	?????	netbuf1	restored
* A6 --	library	?????	library
*

SplitNetBuff:
*!* debug
*	ERRMSG	"SplitNetBuff"
*!* debug
	movem.l	d2/a2-a3,-(sp)
	move.l	a0,a2
	move.l	a1,a3
	tst.l	d0
	beq.w	SNBZero

	move.l	d0,d1
	bpl.s	SNBCPos
	neg.l	d1
SNBCPos
	move.l	d1,d2
	cmp.l	NB_COUNT(a0),d1
	beq.s	SNBAll
	bhi.s	SNBBad

;	lea	NB_LIST(a2),a0
	cmp.l	MLH_TAILPRED(a0),a0
	beq.s	SNBBad

	tst.l	d0
	bmi.w	SNBNeg
	bpl.s	SNBPos

SNBGood
	moveq.l	#0,d0

SNBDone
	movem.l	(sp)+,d2/a2-a3
	bsr.w	DeferedFree		; preserves registers!
	rts

SNBBad	moveq.l	#1,d0
	bra.s	SNBDone

SNBZero
	move.l	a3,a0
	bsr.w	ReadyNetBuff
	bra.s	SNBGood

SNBAll
	lea	NB_LIST(a3),a0
	move.l	#0,NB_COUNT(a3)
	bsr.w	FreeSegmentsCommon
	move.l	a3,a0
	move.l	a2,a1
	bsr.w	NetBuffAppend
	bra.s	SNBGood


SNBPos
	move.l	NB_LIST+MLH_HEAD(a2),a0
SNBPLoop
	move.l	NBS_DATACOUNT(a0),d1
	cmp.l	d1,d0			; find segment of interest
	bcs.s	SNBPLDone
	sub.l	d1,d0
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	tst.l	NBS_NODE+MLN_SUCC(a0)
	beq.s	SNBBad
	bra.s	SNBPLoop

SNBPLDone
; assert:
;	segment{a0} != 0
;	count{d0} < NBS_DATACOUNT{d1}
;	count{d0} >= 0
;	count{d0} bytes to copy
;
;	segment{a0} will be the first segment in NetBuff0 returned
;	data to copy starts at NBS_BUFFER(a0)+NBS_DATAOFFSET(a0)

	tst.l	d0
	beq.w	SNBPLDZero

	tst.l	NBS_PHYSICALSIZE(a0)
	bne.s	SNBPLDPSNonZero

	movem.l	d0/a0,-(sp)
	move.l	#NBS_SIZE,d0
	move.l	#MEMF_CLEAR,d1
	LINKSYS	AllocMem
	move.l	d0,d1
	movem.l	(sp)+,d0/a0
	tst.l	d1
	beq.s	SNBPLDPSNonZero

	move.l	d1,a1
	move.l	NBS_BUFFER(a0),d1
	add.l	NBS_DATAOFFSET(a0),d1
	move.l	d1,NBS_BUFFER(a1)
	move.l	d0,NBS_DATACOUNT(a1)
	add.l	d0,NBS_DATAOFFSET(a0)
	sub.l	d0,NBS_DATACOUNT(a0)

	move.l	d0,NB_COUNT(a3)
	move.l	a0,-(sp)
	move.l	a1,-(sp)
	lea	NB_LIST(a3),a0
	bsr.w	FreeSegmentsCommon
	move.l	(sp)+,a1
	lea	NB_LIST(a3),a0
	ADDHEAD
	move.l	(sp)+,a0
	bra.s	SNBPLDAssemble

SNBPLDPSNonZero
	movem.l	d0/a0,-(sp)
	move.l	a3,a0
	bsr.w	ReadyNetBuff
	tst.l	d0
	bne.w	SNBCLBad

	movem.l	(sp),d1/a0
	move.l	NBS_BUFFER(a0),a1
	add.l	NBS_DATAOFFSET(a0),a1
	moveq.l	#0,d0
	move.l	a3,a0
	bsr.w	CopyToNetBuff
	tst.l	d0
	bne.w	SNBCLBad

	movem.l	(sp)+,d0/a0
	add.l	d0,NBS_DATAOFFSET(a0)
	sub.l	d0,NBS_DATACOUNT(a0)
	bra.s	SNBPLDAssemble

SNBPLDZero
	move.l	a0,-(sp)
	lea	NB_LIST(a3),a0
	bsr.w	FreeSegmentsCommon
	move.l	(sp)+,a0

SNBPLDAssemble
	move.l	d2,NB_COUNT(a3)
	sub.l	d2,NB_COUNT(a2)

;	lea	NB_LIST(a3),d0
	move.l	a3,d0
	move.l	NB_LIST+MLH_HEAD(a2),a1
	exg.l	a0,a1
	bsr.w	MoveSubList
	bra.w	SNBGood


SNBNeg
	neg.l	d0
	move.l	NB_LIST+MLH_TAILPRED(a2),a0
SNBNLoop
	move.l	NBS_DATACOUNT(a0),d1
	cmp.l	d1,d0			; find segment of interest
	bcs.s	SNBNLDone
	sub.l	d1,d0
	move.l	NBS_NODE+MLN_PRED(a0),a0
	tst.l	NBS_NODE+MLN_PRED(a0)
	beq.w	SNBBad
	bra.s	SNBNLoop

SNBNLDone
; assert:
;	segment{a0} != 0
;	count{d0} < NBS_DATACOUNT{d1}
;	count{d0} >= 0
;	count{d0} bytes to copy
;
;	segment{a0} will be the last segment in NetBuff0 returned
;	data to copy starts at NBS_BUFFER(a0)+NBS_DATAOFFSET(a0)+NBS_DATACOUNT(a0)-count{d0}

	tst.l	d0
	beq.w	SNBNLDZero

	tst.l	NBS_PHYSICALSIZE(a0)
	bne.s	SNBNLDPSNonZero

	movem.l	d0/a0,-(sp)
	move.l	#NBS_SIZE,d0
	move.l	#MEMF_CLEAR,d1
	LINKSYS	AllocMem
	move.l	d0,d1
	movem.l	(sp)+,d0/a0
	tst.l	d1
	beq.s	SNBNLDPSNonZero

	move.l	d1,a1
	move.l	NBS_BUFFER(a0),d1
	add.l	NBS_DATAOFFSET(a0),d1
	add.l	NBS_DATACOUNT(a0),d1
	sub.l	d0,d1
	move.l	d1,NBS_BUFFER(a1)
	move.l	d0,NBS_DATACOUNT(a1)
	sub.l	d0,NBS_DATACOUNT(a0)

	move.l	d0,NB_COUNT(a3)
	move.l	a0,-(sp)
	move.l	a1,-(sp)
	lea	NB_LIST(a3),a0
	bsr.w	FreeSegmentsCommon
	move.l	(sp)+,a1
	lea	NB_LIST(a3),a0
	ADDHEAD
	move.l	(sp)+,a0
	bra.s	SNBNLDAssemble

SNBNLDPSNonZero
	movem.l	d0/a0,-(sp)
	move.l	a3,a0
	bsr.w	ReadyNetBuff
	tst.l	d0
	bne.w	SNBCLBad

	movem.l	(sp),d1/a0
	move.l	NBS_BUFFER(a0),a1
	add.l	NBS_DATAOFFSET(a0),a1
	add.l	NBS_DATACOUNT(a0),a1
	sub.l	d1,a1
	moveq.l	#0,d0
	move.l	a3,a0
	bsr.w	CopyToNetBuff
	tst.l	d0
	bne.w	SNBCLBad

	movem.l	(sp)+,d0/a0
	sub.l	d0,NBS_DATACOUNT(a0)
	bra.s	SNBNLDAssemble

SNBNLDZero
	move.l	a0,-(sp)
	lea	NB_LIST(a3),a0
	bsr.w	FreeSegmentsCommon
	move.l	(sp)+,a0

SNBNLDAssemble
	sub.l	d2,NB_COUNT(a2)
	move.l	d2,NB_COUNT(a3)

	move.l	NB_LIST+MLH_TAILPRED(a3),d0
	lea	NB_LIST+MLH_TAIL(a2),a1
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	bsr.w	MoveSubList
	bra.w	SNBGood


SNBCLBad
	movem.l	(sp)+,d0/a0
	bra.w	SNBBad


******* netbuff.library/TrimNetBuff() ********************************
*
*   NAME
*	TrimNetBuff -- Elliminate leading or trailing data.
*
*   SYNOPSIS
*	error = TrimNetBuff( Netbuff, Count )
*	D0                   A0        D0
*
*	LONG TrimNetBuff( struct NetBuff *, LONG );
*
*   FUNCTION
*	This function takes a NetBuff and elliminates 'abs(Count)'
*	bytes of data from the first or last bytes of the NetBuff data
*	depending on the sign of Count. If Count is positive, the
*	first 'Count' bytes of data are elliminated. If Count is
*	negative, the last 'abs(Count)' bytes are elliminated.
*
*	Emptied NetBuffSegments (which may result) will be returned to
*	the free pool.
*
*   INPUTS
*	Netbuff         Pointer to NetBuff to be trimmed.
*	Count           Number of data bytes to remove.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function might be used within a protocol stack to remove
*	levels of protocol wrapping from either side of a packet.
*
*   SEE ALSO
*	netbuff.library/SplitNetBuff(),
*	netbuff.library/NetBuffAppend(),
*	netbuff.library/PrependNetBuff()
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
* D0 --	count	count	error
* D1 --	?????	scratch	?????
* A0 --	netbuff	segment	?????
* A1 --	?????	netbuff	?????
* A6 --	library	library	library
*

TrimNetBuff:
*!* debug
*	ERRMSG	"TrimNetBuff"
*!* debug
	tst.l	d0
	beq.w	TNBGood

	move.l	d0,d1
	bpl.s	TNBCPos
	neg.l	d1
TNBCPos
	cmp.l	NB_COUNT(a0),d1
	bhi.s	TNBBad
;	lea	NB_LIST(a0),a0
	cmp.l	MLH_TAILPRED(a0),a0
	beq.s	TNBBad

	sub.l	d1,NB_COUNT(a0)
	move.l	a0,a1
	tst.l	d0
	bmi.s	TNBNeg
	bpl.s	TNBPos

TNBGood
	moveq.l	#0,d0
	rts

TNBBad
	moveq.l	#1,d0
	rts


TNBPos
	move.l	NB_LIST+MLH_HEAD(a1),a0
TNBPLoop
	move.l	NBS_DATACOUNT(a0),d1
	cmp.l	d1,d0			; find segment of interest
	bcs.s	TNBPLDone
	sub.l	d1,d0
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	tst.l	NBS_NODE+MLN_SUCC(a0)
	beq.s	TNBLCommon

	bra.s	TNBPLoop

TNBPLDone
; assert:
;	segment{a0} != 0
;	count{d0} < NBS_DATACOUNT{d1}
;	count{d0} >= 0
;	netbuff{a1} != 0

	sub.l	d0,d1
	move.l	d1,NBS_DATACOUNT(a0)
	add.l	NBS_DATAOFFSET(a0),d0
	move.l	d0,NBS_DATAOFFSET(a0)

	move.l	NB_LIST+MLH_HEAD(a1),a1
	exg.l	a0,a1

TNBFCommon
	sub.l	#MLH_SIZE,sp
	NEWLIST	sp
	move.l	sp,d0
	bsr.w	MoveSubList
	move.l	sp,a0
	bsr.w	FreeSegmentsCommon
	add.l	#MLH_SIZE,sp
	bra.s	TNBGood

TNBNeg
	neg.l	d0
	move.l	NB_LIST+MLH_TAILPRED(a1),a0
TNBNLoop
	move.l	NBS_DATACOUNT(a0),d1
	cmp.l	d1,d0			; find segment of interest
	bcs.s	TNBNLDone
	sub.l	d1,d0
	move.l	NBS_NODE+MLN_PRED(a0),a0
	tst.l	NBS_NODE+MLN_PRED(a0)
	beq.s	TNBLCommon

	bra.s	TNBNLoop

TNBNLDone
; assert:
;	segment{a0} != 0
;	count{d0} < NBS_DATACOUNT{d1}
;	count{d0} >= 0
;	netbuff{a1} != 0

	sub.l	d0,d1
	move.l	d1,NBS_DATACOUNT(a0)

	lea	NB_LIST+MLH_TAIL(a1),a1
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	bra.s	TNBFCommon

TNBLCommon
	tst.l	d0
	beq.s	TNBAll
	rts

TNBAll
	move.l	a1,a0
	move.l	d0,NB_COUNT(a0)
	bsr.w	FreeSegments
	moveq.l	#0,d0
	rts


******* netbuff.library/CopyToNetBuff() ******************************
*
*   NAME
*	CopyToNetBuff -- Replace data in a NetBuff.
*
*   SYNOPSIS
*	error = CopyToNetBuff( Netbuff, Offset, Data, Count )
*	D0                     A0       D0      A1    D1
*
*	LONG CopyToNetBuff( struct NetBuff *, LONG, UBYTE *, ULONG );
*
*   FUNCTION
*	This function replaces existing data in Netbuff with 'Count'
*	bytes from those pointed to by Data. If Offset is
*	non-negative, then the replacement starts 'Offset' bytes from
*	the start of Netbuff. If Offset is negative, then the
*	replacement starts 'abs(Offset)' from the end of Netbuff.
*
*	This function will not change the amount of data that Netbuff
*	contains; it will only replace parts of it.
*
*   INPUTS
*	Netbuff         Pointer to NetBuff structure to copy data to.
*	Offset          Offset into NetBuff to place data.
*	Data            Pointer to data to copy.
*	Count           Number of bytes of data to copy.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function may be called from interrupts.
*
*	This function might be used within a network device driver to
*	fill a NetBuff from bytes taken from the hardware. In this
*	case, CopyToNetBuff()s would be preceded possibly by a call to
*	ReadyNetBuff().
*
*   SEE ALSO
*	netbuff.library/CopyFromNetBuff(),
*	netbuff.library/ReadyNetBuff()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*	Relies on CopyBytes() returning with A0 and A1 incremented by
*	Count bytes.
*
*	Special code can make small negative offsets (most negative
*	offsets are expected to be small) much faster for multisegment
*	NetBuffs.
*
*
*   REGISTER USAGE
*
* D0 --	offset	copycnt	error
* D1 --	count	scratch	?????
* D2 --	?????	togo	restored
* A0 --	netbuff	copysrc	?????
* A1 --	data	copydst	?????
* A2 --	?????	segment	restored
* A5 --	?????	cpsetup	restored
* A6 --	library	library	library
*

CopyToNetBuff:
*!* debug
*	ERRMSG	"CopyToNetBuff"
*!* debug
	movem.l	a2/a5/d2,-(sp)
	lea.l	CTNBCopySetup,a5
	bra.s	CopyXNetBuffCommon
	; rtf


CTNBCopySetup
	move.l	NBS_BUFFER(a2),a1	; these 3 lines are the only difference
	adda.l	NBS_DATAOFFSET(a2),a1	;   between CTNB and CFNB
	adda.l	d0,a1
	rts


******* netbuff.library/CopyFromNetBuff() ****************************
*
*   NAME
*	CopyFromNetBuff -- Copy data from a NetBuff to memory.
*
*   SYNOPSIS
*	error = CopyFromNetBuff( Netbuff, Offset, Data, Count )
*	D0                       A0       D0      A1    D1
*
*	LONG CopyFromNetBuff( struct NetBuff *, LONG, UBYTE *, ULONG );
*
*   FUNCTION
*	This function copies 'Count' bytes from a NetBuff to memory
*	pointed to by Data. If Offset is non-negative, then the start
*	of the data to copy is 'Offset' bytes from the start of the
*	NetBuff. If Offset is negative, then the start of the data to
*	copy is 'abs(Offset)' bytes from the end of the NetBuff.
*
*   INPUTS
*	NetBuff         Pointer to NetBuff structure with source data.
*	Offset          Offset into Netbuff where data starts.
*	Data            Pointer to area to store data in.
*	Count           Number of bytes to copy.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function may be called from interrupts.
*
*	This function might be used by a network device driver to
*	extract bytes from a NetBuff to fill hardware.
*
*	This function might be used within a protocol stack to extract
*	data structures from a NetBuff into local memory.
*
*   SEE ALSO
*	netbuff.library/CopyToNetBuff()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*	Relies on CopyBytes() returning with A0 and A1 incremented by
*	Count bytes.
*
*	Special code can make small negative offsets (most negative
*	offsets are expected to be small) much faster for multisegment
*	NetBuffs.
*
*
*   REGISTER USAGE
*
* D0 --	offset	copycnt	error
* D1 --	count	scratch	?????
* D2 --	?????	togo	restored
* A0 --	netbuff	copysrc	?????
* A1 --	data	copydst	?????
* A2 --	?????	segment	restored
* A5 --	?????	cpsetup	restored
* A6 --	library	library	library
*

CopyFromNetBuff:
*!* debug
*	ERRMSG	"CopyFromNetBuff"
*!* debug
	movem.l	a2/a5/d2,-(sp)
	lea.l	CFNBCopySetup,a5
	bra.s	CopyXNetBuffCommon
	; rtf


CFNBCopySetup
	move.l	NBS_BUFFER(a2),a0	; these 3 lines are the only difference
	adda.l	NBS_DATAOFFSET(a2),a0	;   between CTNB and CFNB
	adda.l	d0,a0
	rts


CopyXNetBuffCommon
*!* debug
*	ERRMSG	"CopyXNetBuffCommon"
*!* debug
	move.l	d1,d2

CxNBCNonEmpty1
	tst.l	d2			; Count of zero is trivial
	beq.w	CxNBCGood

CxNBCNonEmpty2
	cmp.l	NB_LIST+MLH_TAILPRED(a0),a0		; no segments is trivial
	beq.w	CxNBCBad

CxNBCMakePos
	tst.l	d0			; make offset positive
	bpl.s	CxNBCInside1
	add.l	NB_COUNT(a0),d0

CxNBCInside1
	cmp.l	NB_COUNT(a0),d0		; offset outside is trivial
	bcc.w	CxNBCBad

CxNBCInside2
	add.l	d0,d1
	cmp.l	NB_COUNT(a0),d1		; offset+count outside is trivial
	bhi.w	CxNBCBad

	move.l	NB_LIST+MLH_HEAD(a0),a2
	move.l	a1,a0
CxNBCLoop
	move.l	NBS_DATACOUNT(a2),d1
	cmp.l	d1,d0
	bcs.s	CxNBCLDone
	sub.l	d1,d0
	move.l	NBS_NODE+MLN_SUCC(a2),a2
	tst.l	NBS_NODE+MLN_SUCC(a2)
	bne.s	CxNBCLoop

CxNBCBad
	movem.l	(sp)+,a2/a5/d2
	moveq.l	#1,d0
	rts

CxNBCLDone
; assert:
;	a2 != 0
;	offset{d0} < NBS_DATACOUNT{d1}
;	offset{d0} >= 0
;	togo{d2} > 0
;	datacount{d1} > 0
;	togo{d2} > 0

	jsr	(a5)			; setup a0/a1 for copy

; d0 = MIN((datacount{d1} - offset{d0}), togo{d2})
	sub.l	d0,d1
	exg.l	d0,d1
	cmp.l	d0,d2
	bcc.s	CxNBCGotCount

	move.l	d2,d0

CxNBCGotCount
	sub.l	d0,d2
	bsr.w	CopyBytes

	tst.l	d2
	beq.s	CxNBCGood

	move.l	NBS_NODE+MLN_SUCC(a2),a2
	tst.l	NBS_NODE+MLN_SUCC(a2)
	beq.s	CxNBCBad

	moveq.l	#0,d0
	bra.s	CxNBCLoop

CxNBCGood
	movem.l	(sp)+,a2/a5/d2
	moveq.l	#0,d0
	rts


******* netbuff.library/CopyNetBuff() ********************************
*
*   NAME
*	CopyNetBuff -- Make a copy of a NetBuff.
*
*   SYNOPSIS
*	error = CopyNetBuff( Netbuff0, Netbuff1 )
*	D0                   A0        A1
*
*	LONG CopyNetBuff( struct NetBuff *, struct NetBuff * );
*
*   FUNCTION
*	This function makes NetBuff1 a logical clone of Netbuff0.
*	Netbuff0 will be unmodified and Netbuff1 will be optimal.
*
*	All data in Netbuff1 will be lost when this function is
*	called.
*
*   INPUTS
*	Netbuff0        Pointer to source NetBuff structure.
*	Netbuff1        Pointer to destination NetBuff structure.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function may be called from interrupts.
*
*	Only the first NetBuffSegment in NetBuff1 is guaranteed to
*	still be in NetBuff1 when this function returns.
*
*   SEE ALSO
*	netbuff.library/CopyFromNetBuff(),
*	netbuff.library/CopyToNetBuff()
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
* D0 --	?????	cpcount	error
* D1 --	?????	scratch	?????
* D2 --	?????	srccnt	restored
* D3 --	?????	dstcnt	restored
* A0 --	netbuf0	copysrc	?????
* A1 --	netbuf1	copydst	?????
* A2 --	?????	srcseg	restored
* A3 --	?????	dstseg	restored
* A6 --	library	library	library
*

CopyNetBuff:
*!* debug
*	ERRMSG	"CopyNetBuff"
*!* debug
	movem.l	a2-a3/d2-d3,-(sp)
	move.l	a0,a2
	move.l	a1,a3
	move.l	a1,a0
	move.l	NB_COUNT(a2),d0
	bsr.w	ReadyNetBuff
;	bsr.w	DeferedFree		; preserves registers!
	tst.l	d0
	bne.s	CNBBad

	moveq.l	#0,d2
	moveq.l	#0,d3
	bra.s	CNBLSetup

CNBLoop
	move.l	d2,d0
	cmp.l	d2,d3
	bcc.s	CNBLd2small

	move.l	d3,d0

CNBLd2small
	sub.l	d0,d2
	sub.l	d0,d3
	bsr.w	CopyBytes

CNBLSetup
CNBLSetupdst
	tst.l	d3
	bne.s	CNBLSetupsrc

CNBLNextdst
	move.l	NBS_NODE+MLN_SUCC(a3),a3
	tst.l	NBS_NODE+MLN_SUCC(a3)
	beq.s	CNBGood

	move.l	NBS_BUFFER(a3),a1
	move.l	NBS_DATACOUNT(a3),d3
	beq.s	CNBLNextdst

	adda.l	NBS_DATAOFFSET(a3),a1

CNBLSetupsrc
	tst.l	d2
	bne.s	CNBLoop

CNBLNextsrc
	move.l	NBS_NODE+MLN_SUCC(a2),a2
	tst.l	NBS_NODE+MLN_SUCC(a2)
	beq.s	CNBBad

	move.l	NBS_BUFFER(a2),a0
	move.l	NBS_DATACOUNT(a2),d2
	beq.s	CNBLNextsrc

	adda.l	NBS_DATAOFFSET(a2),a0
	bra.s	CNBLoop

CNBGood
	movem.l	(sp)+,a2-a3/d2-d3
	moveq.l	#0,d0
	rts

CNBBad
	movem.l	(sp)+,a2-a3/d2-d3
	moveq.l	#1,d0
	rts


******* netbuff.library/CompactNetBuff() *****************************
*
*   NAME
*	CompactNetBuff -- Optimize a NetBuff.
*
*   SYNOPSIS
*	error = CompactNetBuff( Netbuff )
*	D0                      A0
*
*	LONG CompactNetBuff( struct NetBuff * );
*
*   FUNCTION
*	This function optimizes a NetBuff by moving the data to fill
*	each needed NetBuffSegment as much as possible and return
*	unused NetBuffSegments to the free pool. This function will
*	not modify NetBuffSegments of physical size zero.
*
*   INPUTS
*	Netbuff         Pointer to NetBuff structure to optimize.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function consumes time.
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
* D0 --	?????	?????	error
* D1 --	?????	?????	?????
* A0 --	netbuff	?????	?????
* A1 --	?????	?????	?????
* A6 --	library	?????	library
*

CompactNetBuff:
*!* debug
	ERRMSG	"CompactNetBuff"
*!* debug
	movem.l	a2,-(sp)
	sub.l	#MLH_SIZE,sp
	move.l	NB_LIST+MLH_HEAD(a0),a2
	NEWLIST	sp

ONBOLoop
;	a2 first segment not yet optimized

	tst.l	NBS_NODE+MLN_SUCC(a2)
	beq.s	ONBOLDone

	move.l	a2,a1
	tst.l	NBS_PHYSICALSIZE(a2)
	bne.s	ONBILoop

ONBILoopPSZ
;	a1 first segment to check

	tst.l	NBS_NODE+MLN_SUCC(a1)
	beq.s	ONBILDonePSZ

	tst.l	NBS_PHYSICALSIZE(a1)
	bne.s	ONBILDonePSZ

	move.l	NBS_NODE+MLN_SUCC(a1),a1
	bra.s	ONBILoop

ONBILDonePSZ
;	a2 first segment to optimize
;	a1 segment after last segment to optimize

	move.l	a2,a0
	move.l	a1,a2
	move.l	sp,d0
	bsr.w	MoveSubList
	move.l	sp,a0
	bsr.w	ONBSimplePSZ
	move.l	NBS_NODE+MLN_PRED(a2),a0
	move.l	sp,a1
	bsr.w	InsertList
	bra.s	ONBOLoop

ONBILoop
;	a1 first segment to check

	tst.l	NBS_NODE+MLN_SUCC(a1)
	beq.s	ONBILDone

	tst.l	NBS_PHYSICALSIZE(a1)
	beq.s	ONBILDone

	move.l	NBS_NODE+MLN_SUCC(a1),a1
	bra.s	ONBILoop

ONBILDone
;	a2 first segment to optimize
;	a1 segment after last segment to optimize

	move.l	a2,a0
	move.l	a1,a2
	move.l	sp,d0
	bsr.w	MoveSubList
	move.l	sp,a0
	bsr.s	ONBSimple
	move.l	NBS_NODE+MLN_PRED(a2),a0
	move.l	sp,a1
	bsr.w	InsertList
	bra.s	ONBOLoop

ONBOLDone
	add.l	#MLH_SIZE,sp
	movem.l	(sp)+,a2

	bsr.w	DeferedFree
	moveq.l	#0,d0
	rts


ONBSimple
*!* debug
	ERRMSG	"ONBSimple"
*!* debug
;	a0 - list of segments; no segment has PhysicalSize = 0

	movem.l	d2-d3/a2-a4,-(sp)
	move.l	a0,a4
	cmp.l	MLH_TAILPRED(a4),a4
	beq.w	ONBSDone

	move.l	MLH_HEAD(a4),a2
	move.l	NBS_BUFFER(a2),a0
	add.l	NBS_DATAOFFSET(a2),a0
	move.l	NBS_DATACOUNT(a2),d2
	move.l	MLH_HEAD(a4),a3
	move.l	NBS_BUFFER(a3),a1
	move.l	NBS_PHYSICALSIZE(a3),d3
	move.l	#0,NBS_DATACOUNT(a3)
	move.l	#0,NBS_DATAOFFSET(a3)

ONBSLoop
;	d0 - copy count
;	d1 - ?????
;	d2 - bytes left to copy in source segment
;	d3 - room left in dest segment
;	a0 - copy source
;	a1 - copy dest
;	a2 - source segment
;	a3 - dest segment
;	a4 - list

	move.l	d2,d0
	cmp.l	d3,d0
	bcs.s	ONBSSmall
	move.l	d3,d0
ONBSSmall
	sub.l	d0,d2
	sub.l	d0,d3
	add.l	d0,NBS_DATACOUNT(a3)
	bsr.w	CopyBytes

ONBSSetup
	tst.l	d2
	bne.s	ONBSCheckDest

	move.l	NBS_NODE+MLN_SUCC(a2),a2
	tst.l	NBS_NODE+MLN_SUCC(a2)
	beq.s	ONBSCleanup

	move.l	NBS_BUFFER(a2),a0
	add.l	NBS_DATAOFFSET(a2),a0
	move.l	NBS_DATACOUNT(a2),d2
	bra.s	ONBSSetup

ONBSCheckDest
	tst.l	d3
	bne.s	ONBSLoopEnd

	move.l	NBS_NODE+MLN_SUCC(a3),a3
	move.l	NBS_BUFFER(a3),a1
	move.l	NBS_PHYSICALSIZE(a3),d3
	move.l	#0,NBS_DATACOUNT(a3)
	move.l	#0,NBS_DATAOFFSET(a3)

ONBSLoopEnd
	bra.s	ONBSLoop

ONBSCleanup
;	a3 is last segment wanted
;	a4 is list

	sub.l	#MLH_SIZE,sp
	NEWLIST	sp
	move.l	sp,d0
	move.l	NBS_NODE+MLN_SUCC(a3),a0
	lea	NB_LIST+MLH_TAIL(a4),a1
	bsr.w	MoveSubList

	move.l	sp,a0
	bsr.w	FreeSegmentsCommon
	add.l	#MLH_SIZE,sp

ONBSDone
	movem.l	(sp)+,d2-d3/a2-a4
	rts


ONBSimplePSZ
*!* debug
	ERRMSG	"ONBSimplePSZ"
*!* debug
;	a0 - list of segments; all segments have PhysicalSize = 0

	movem.l	a2,-(sp)
	move.l	MLH_HEAD(a0),a2

ONBSLoopPSZ
	move.l	a2,a0
	move.l	NBS_NODE+MLN_SUCC(a0),a2
	tst.l	NBS_NODE+MLN_SUCC(a2)
	beq.s	ONBSDonePSZ

	move.l	NBS_BUFFER(a0),d0
	add.l	NBS_DATAOFFSET(a0),d0
	add.l	NBS_DATACOUNT(a0),d0

	move.l	NBS_BUFFER(a2),d1
	add.l	NBS_DATAOFFSET(a2),d1

	cmp.l	d0,d1
	bne.s	ONBSLoopPSZ

	move.l	NBS_DATACOUNT(a2),d0
	add.l	d0,NBS_DATACOUNT(a0)

	move.l	a2,a1
	move.l	a0,a2
	move.l	a1,d0
	REMOVE				; saves d0
	move.l	d0,a1
	move.l	#NBS_SIZE,d0
	LINKSYS	FreeMem
	bra.s	ONBSLoopPSZ

ONBSDonePSZ
	movem.l	(sp)+,a2
	rts


******* netbuff.library/ReadyNetBuff() *******************************
*
*   NAME
*	ReadyNetBuff -- Ready a NetBuff for copying to.
*
*   SYNOPSIS
*	error = ReadyNetBuff( Netbuff, Count )
*	D0                    A0       D0
*
*	LONG ReadyNetBuff( struct NetBuff *, ULONG );
*
*   FUNCTION
*	This function sets the amount of data refered to by a NetBuff,
*	and the associated NetBuffSegments, to Count bytes in
*	preparation for a CopyToNetBuff() operation. It does not
*	initialize the data in the NetBuff.
*
*   INPUTS
*	Netbuff         Pointer to NetBuff structure to initialize.
*	Count           The number of bytes of data that the NetBuff
*	                    is to refer to.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function may be called from interrupts.
*
*	This function will attempt to allocate NetBuffSegments as
*	needed to make room for Count bytes available.
*
*	Unneeded NetBuffSegments will be returned to the system free
*	pool.
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*	Un-needed segments are freed. The only other option is to zero
*	the datacount field in the unused segments.
*
*	If insufficient room, leaves NB_COUNT set to room available.
*
*
*   REGISTER USAGE
*
* D0 --	count	scratch	error
* D1 --	?????	scratch	?????
* D2 --	?????	segsize	restored
* A0 --	netbuff	segment	?????
* A1 --	?????	scratch	?????
* A2 --	?????	netbuff	restored
* A6 --	library	library	library
*

ReadyNetBuff:
*!* debug
*	ERRMSG	"ReadyNetBuff"
*!* debug
	movem.l	a2-a3/d2,-(sp)
	move.l	a0,a2
	move.l	d0,d2
	move.l	d2,NB_COUNT(a2)		; optimistic

	move.l	NB_LIST+MLH_HEAD(a0),a0
	tst.l	NBS_NODE+MLN_SUCC(a0)
	bne.w	RNBLoop

	tst.l	d2
	bne.s	RNBNeedSeg

	movem.l	(sp)+,a2-a3/d2
	moveq.l	#0,d0
	rts

RNBNeedSeg
	lea	NB_LIST(a2),a0
	bsr.w	AllocSegmentsCommon
	lea	NB_LIST(a2),a0
	cmp.l	MLH_TAILPRED(a0),a0
	beq.w	RNBBad

	move.l	MLH_HEAD(a0),a0

RNBLoop
	tst.l	d2
	beq.s	RNBDelete

	move.l	NBS_PHYSICALSIZE(a0),d1
	bne.s	RNBLRealSeg

RNBLZeroSeg
	move.l	NBS_DATACOUNT(a0),d1
	bra.s	RNBLCommon

RNBLRealSeg
	move.l	#0,NBS_DATAOFFSET(a0)

RNBLCommon
	cmp.l	d1,d2
	bls.s	RNBLDone
	sub.l	d1,d2
	move.l	d1,NBS_DATACOUNT(a0)
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	tst.l	NBS_NODE+MLN_SUCC(a0)
	bne.s	RNBLoop

RNBAddSeg
;	d2 = needed
;	d2 != 0

	move.l	NB_LIST+MLH_TAILPRED(a2),-(sp)
	sub.l	#MLH_SIZE,sp
	move.l	sp,a0
	NEWLIST	a0
	move.l	d2,d0
	bsr.w	AllocSegmentsCommon
	move.l	sp,a1
	cmp.l	MLH_TAILPRED(a1),a1
	beq.s	RNBBadAdd

	move.l	NB_LIST+MLH_TAILPRED(a2),a0
	bsr.w	InsertList
	add.l	#MLH_SIZE,sp
	move.l	(sp)+,a0
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	bra.s	RNBLoop

RNBLDone
; assert:
;	segment{a0} != 0
;	count{d2} <= size{d1}
;	count{d2} >= 0

; notes
;	a0 is last segment needed
;	a2 is the NetBuff

	move.l	d2,NBS_DATACOUNT(a0)
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	tst.l	NBS_NODE+MLN_SUCC(a0)
	beq.s	RNBGood

RNBDelete
;	delete segments from here on
;	segment{a0} != 0

	sub.l	#MLH_SIZE,sp
	NEWLIST	sp
	move.l	sp,d0
	lea	NB_LIST+MLH_TAIL(a2),a1
	bsr.w	MoveSubList
	move.l	sp,a0
	bsr.w	FreeSegmentsCommon
	add.l	#MLH_SIZE,sp

RNBGood
	movem.l	(sp)+,a2-a3/d2
	moveq.l	#0,d0
	rts

RNBBadAdd
	add.l	#MLH_SIZE,sp
	move.l	(sp)+,a0
	
RNBBad
	move.l	NB_COUNT(a2),d0
	sub.l	d2,d0
	move.l	d0,NB_COUNT(a2)
	movem.l	(sp)+,a2-a3/d2
	moveq.l	#1,d0
	rts


******* netbuff.library/IsContiguous() *******************************
*
*   NAME
*	IsContiguous -- Checks if data in in contiguous memory.
*
*   SYNOPSIS
*	result = IsContiguous( Netbuff, Offset, Count )
*	D0                     A0       D0      D1
*
*	LONG IsContiguous( struct NetBuff *, LONG, ULONG );
*
*   FUNCTION
*	This function indicates whether or not Count bytes of data
*	starting at Offset in NetBuff are in contiguous bytes. If
*	Offset is non-negative, then the start of the data to check is
*	'Offset' bytes from the start of the NetBuff. If Offset is
*	negative, then the start of the data to check is 'abs(Offset)'
*	bytes from the end of the NetBuff.
*
*   INPUTS
*	Netbuff         Pointer to NetBuff to check.
*	Offset          Offset into NetBuff where to check.
*	Count           Number of bytes that should be contiguous.
*
*   RESULTS
*	result          Zero if non-contiguous; non-zero otherwise.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*	Special code can make small negative offsets (most negative
*	offsets are expected to be small) much faster for multisegment
*	NetBuffs.
*
*
*   REGISTER USAGE
*
* D0 --	offset	offset	result
* D1 --	count	count	?????
* A0 --	netbuff	segment	?????
* A6 --	library	library	library
*

IsContiguous:
*!* debug
*	ERRMSG	"IsContiguous"
*!* debug
	bsr.w	DeferedFree		; preserves registers!

ICNonEmpty1
;	lea	MB_LIST(a0),a1
;	cmp.l	MLH_TAILPRED(a1),a1
	cmp.l	NB_LIST+MLH_TAILPRED(a0),a0	; if empty
	beq.w	ICNonCon

ICNonEmpty2
	tst.l	d1			; Count of zero is trivial
	beq.w	ICNonCon

ICMakePos
	tst.l	d0			; make offset positive
	bpl.s	ICInside
	add.l	NB_COUNT(a0),d0

ICInside
	cmp.l	NB_COUNT(a0),d0		; offset outside is trivial
	bcc.w	ICNonCon

	move.l	NB_LIST+MLH_HEAD(a0),a0
ICLoop
	cmp.l	NBS_DATACOUNT(a0),d0	; find segment of interest
	bcs.s	ICLDone
	sub.l	NBS_DATACOUNT(a0),d0
	move.l	NBS_NODE+MLN_SUCC(a0),a0
	tst.l	NBS_NODE+MLN_SUCC(a0)
	bne.s	ICLoop

ICNonCon
	moveq.l	#0,d0
	rts

ICLDone
; assert:
;	a0 != 0
;	offset{d0} < NBS_DATACOUNT
;	offset{d0} >= 0

	add.l	d1,d0
	cmp.l	NBS_DATACOUNT(a0),d0	; if offset{d0}+count{d1} <= datacount
	bhi.s	ICNonCon			;   branch

ICIsCon
	moveq.l	#1,d0
	rts


******* netbuff.library/NetBuffAppend() ******************************
*
*   NAME
*	NetBuffAppend -- Append one NetBuff to then end of another.
*
*   SYNOPSIS
*	error = NetBuffAppend( Netbuff0, Netbuff1 )
*	D0                     A0        A1
*
*	LONG NetBuffAppend( struct NetBuff *, struct NetBuff * );
*
*   FUNCTION
*	This function appends the contents of Netbuff1 to the end of
*	Netbuff0.
*
*   INPUTS
*	Netbuff0        NetBuff to be appended to.
*	Netbuff1        NetBuff to append.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*
*   SEE ALSO
*	netbuff.library/PrependNetBuff(),
*	netbuff.library/SplitNetBuff()
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
* D0 --	?????	?????	error
* D1 --	?????	?????	?????
* A0 --	netbuf0	?????	?????
* A1 --	netbuf1	?????	?????
* A6 --	library	library	library
*

NetBuffAppend:
*!* debug
*	ERRMSG	"NetBuffAppend"
*!* debug
	move.l	NB_COUNT(a1),d0
	add.l	d0,NB_COUNT(a0)
	move.l	#0,NB_COUNT(a1)

;	lea	NB_LIST(a0),a0
;	lea	NB_LIST(a1),a1

	move.l	MLH_TAILPRED(a0),a0
	bra.s	PNBAInsertList


******* netbuff.library/PrependNetBuff() *****************************
*
*   NAME
*	PrependNetBuff -- Prepend one NetBuff to the front of another.
*
*   SYNOPSIS
*	error = PrependNetBuff( Netbuff0, Netbuff1 )
*	D0                      A0        A1
*
*	LONG PrependNetBuff( struct NetBuff *, struct NetBuff * );
*
*   FUNCTION
*	This function prepends the contents of Netbuff1 to the front
*	of Netbuff0.
*
*   INPUTS
*	Netbuff0        NetBuff to be prepended to.
*	Netbuff1        NetBuff to prepend.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*
*   SEE ALSO
*	netbuff.library/NetBuffAppend(),
*	netbuff.library/SplitNetBuff()
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
* D0 --	?????	?????	error
* D1 --	?????	?????	?????
* A0 --	netbuf0	?????	?????
* A1 --	netbuf1	?????	?????
* A6 --	library	library	library
*

PrependNetBuff:
*!* debug
*	ERRMSG	"PrependNetBuff"
*!* debug
	move.l	NB_COUNT(a1),d0
	add.l	d0,NB_COUNT(a0)
	move.l	#0,NB_COUNT(a1)

;	lea	NB_LIST(a0),a0
;	lea	NB_LIST(a1),a1
;	lea	MLH_HEAD(a0),a0

PNBAInsertList
	bsr.w	InsertList
	bsr.w	DeferedFree		; preserves registers!
	moveq.l	#0,d0
	rts


******* netbuff.library/ReclaimSegments() ****************************
*
*   NAME
*	ReclaimSegments -- Free all but first NetBuffSegment.
*
*   SYNOPSIS
*	error = ReclaimSegments( Netbuff )
*	D0                       A0
*
*	LONG ReclaimSegments( struct NetBuff * );
*
*   FUNCTION
*	This function frees all non-zero PhysicalSize NetBuffSegments
*	except the first NetBuffSegment of the NetBuff.
*
*   INPUTS
*	Netbuff         Pointer to NetBuff to reclaim segments from.
*
*   RESULTS
*	error           Zero if successful; non-zero otherwise.
*
*   NOTES
*	This function may be called from interrupts.
*
*	The intended use for this function is to encourage drivers or
*	protocol stacks to free NetBuffSegments from NetBuffs in read
*	queues.
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
* D0 --	?????	scratch	error
* D1 --	?????	scratch	?????
* D2 --	?????	scratch	restored
* A0 --	netbuff	segment	?????
* A1 --	?????	scratch	?????
* A2 --	?????	nextseg	restored
* A6 --	library	library	library
*

ReclaimSegments:
*!* debug
*	ERRMSG	"ReclaimSegments"
*!* debug
	movem.l	a2/d2,-(sp)
	move.l	#0,NB_COUNT(a0)

	move.l	NB_LIST+MLH_HEAD(a0),a1
	tst.l	NBS_NODE+MLN_SUCC(a1)
	beq.w	RSGood

	tst.l	NBS_PHYSICALSIZE(a1)
	beq.s	RSNext

	move.l	#0,NBS_DATACOUNT(a1)

RSNext
	move.l	NBS_NODE+MLN_SUCC(a1),a1
	tst.l	NBS_NODE+MLN_SUCC(a1)
	beq.s	RSGood

	sub.l	#MLH_SIZE,sp
	NEWLIST	sp

RSLoop
	move.l	NBS_NODE+MLN_SUCC(a1),a2
	tst.l	NBS_PHYSICALSIZE(a1)
	beq.s	RSLCommon

RSLRealSeg
	move.l	a1,d2
	REMOVE
	move.l	sp,a0
	move.l	d2,a1
	ADDHEAD

RSLCommon
	move.l	a2,a1
	tst.l	NBS_NODE+MLN_SUCC(a1)
	bne.s	RSLoop

RSLDone
	move.l	sp,a0
	bsr.w	FreeSegmentsCommon
	add.l	#MLH_SIZE,sp

RSGood
	movem.l	(sp)+,a2/d2
	moveq.l	#0,d0
	rts


	END
