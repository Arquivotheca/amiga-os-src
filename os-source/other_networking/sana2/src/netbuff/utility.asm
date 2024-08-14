**********************************************************************
*
* Utility.asm
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

	XDEF	DeferedFree
	XDEF	CopyBytes
	XDEF	InsertList
	XDEF	MoveSubList

	XSYS	FreeMem
	XSYS	CopyMem


*****i* netbuff.library/DeferedFree() ********************************
*
*   NAME
*	DeferedFree -- FreeMem NetBuffSegment structures.
*
*   SYNOPSIS
*	DeferedFree(  )
*
*	void DeferedFree( void );
*
*   FUNCTION
*	This function FreeMem()s the NetBuffSegment structures for
*	NetBuffSegments of physical size zero that were passed to
*	IntFreeSegment().
*
*   NOTES
*
*   SEE ALSO
*	netbuff.library/IntFreeSegment()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*	Must restore ALL registers.
*
*
*   REGISTER USAGE
*
* D0 --	?????	scratch	restored
* D1 --	?????	scratch	restored
* D2 --	?????	segment	restored
* A0 --	?????	list	restored
* A1 --	?????	node	restored
* A6 --	library	library	library
*

DeferedFree:
*!* debug
*	ERRMSG	"DeferedFree"
*!* debug
	movem.l	a0-a1/d0-d2,-(sp)
	NB_DISABLE	a0,NBL_SYSLIB(a6)
	move.l	NBL_DEFERED(a6),d2
	move.l	#0,NBL_DEFERED(a6)
	NB_ENABLE	a0,NBL_SYSLIB(a6)

DFLoop
	tst.l	d2
	beq.s	DFDone
	move.l	d2,a1
	move.l	NBS_PHYSICALSIZE(a1),d0
	beq.s	DFLNoBuff

	move.l	NBS_BUFFER(a1),a1
	LINKSYS	FreeMem
	move.l	d2,a1

DFLNoBuff
	move.l	NBS_NODE+MLN_SUCC(a1),d2
	move.l	#NBS_SIZE,d0
	LINKSYS	FreeMem
	bra.s	DFLoop

DFDone
	movem.l	(sp)+,a0-a1/d0-d2
	rts


*****i* netbuff.library/CopyBytes() **********************************
*
*   NAME
*	CopyBytes -- Copy bytes from source to destination.
*
*   SYNOPSIS
*	CopyBytes( Src, Dst, Count )
*	           A0   A1   D0
*
*	void CopyBytes( void *, void *, ULONG );
*
*   FUNCTION
*	This function copies Count bytes from Src to Dst.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*	Significant speedups are possible in a number special cases.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*	CopyToNetBuff(), CopyFromNetBuff() and CopyNetBuff() all need
*	CopyBytes() to return with A0 and A1 incremented by Count
*	bytes.
*
*	Significant speedups are worth it here.
*
*
*   REGISTER USAGE
*
* D0 --	count	countl	?????
* D1 --	?????	counth	?????
* A0 --	src	src	?????
* A1 --	dst	dst	?????
* A6 --	library	library	library
*

CopyBytes:
*!* debug
*	ERRMSG	"CopyBytes"
*!* debug
	movem.l	a0-a1/d0,-(sp)
	LINKSYS	CopyMem
	movem.l	(sp)+,a0-a1/d0
	adda.l	d0,a0
	adda.l	d0,a1
	rts


*****i* netbuff.library/InsertList() *********************************
*
*   NAME
*	InsertList -- Insert a list into another.
*
*   SYNOPSIS
*	InsertList( Node, List )
*	            A0    A1
*
*	void InsertList( struct Node *, struct List * );
*
*   FUNCTION
*	This function inserts the nodes of a list List into another
*	list after node Node.
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
*
*   REGISTER USAGE
*
* D0 --	?????	?????	?????
* D1 --	?????	?????	?????
* A0 --	node	?????	?????
* A1 --	list	list	?????
* A6 --	library	library	library
*

InsertList:
*!* debug
	ERRMSG	"InsertList"
*!* debug
	move.l	a0,d0
	move.l	MLH_HEAD(a1),a0
	lea	MLH_TAIL(a1),a1
;	bra.s	MoveSubList
;	rft


*****i* netbuff.library/MoveSubList() ********************************
*
*   NAME
*	MoveSubList -- Move part of a list to another list.
*
*   SYNOPSIS
*	MoveSubList( First, Past, After )
*	             A0     A1    D0
*
*	void MoveSubList( struct Node *, struct Node *, struct Node * );
*
*   FUNCTION
*	This function move the nodes between First and Past, including
*	node First but not node Past to after node After. After may be
*	a list different from the source list.
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
*
*   REGISTER USAGE
*
* D0 --	after	scratch	?????
* D1 --	?????	scratch	?????
* A0 --	first	scratch	?????
* A1 --	past	scratch	?????
* A2 --	?????	scratch	restored
* A6 --	library	library	library
*

MoveSubList:
*!* debug
	ERRMSG	"MoveSubList"
*!* debug
	cmp.l	a0,a1
	beq.s	MSLDone

	move.l	a2,-(sp)
	move.l	MLN_PRED(a0),a2
	move.l	MLN_PRED(a1),d1
	move.l	a2,MLN_PRED(a1)
	move.l	a1,MLN_SUCC(a2)
	exg.l	d1,a1
	exg.l	d0,a2
	move.l	MLN_SUCC(a2),d0
	move.l	a2,MLN_PRED(a0)
	move.l	a0,MLN_SUCC(a2)
	exg.l	d0,a2
	move.l	a1,MLN_PRED(a2)
	move.l	a2,MLN_SUCC(a1)
	move.l	(sp)+,a2

MSLDone
	rts


	END
