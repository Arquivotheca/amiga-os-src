 TTL '$Id: copymem.asm,v 39.0 91/10/15 08:26:34 mks Exp $'

*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		  Memory Copying		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,90 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
* Source Control:
*
*	$Id: copymem.asm,v 39.0 91/10/15 08:26:34 mks Exp $
*
*	$Log:	copymem.asm,v $
* Revision 39.0  91/10/15  08:26:34  mks
* V39 Exec initial checkin
* 
**********************************************************************


	NOLIST
	INCLUDE "assembly.i"
	LIST

	XDEF	CopyMem
	XDEF	CopyMemQuick



*****o* exec.library/CopyMem **************************************************
*
*   NAME
*	CopyMem - general purpose memory copy routine
*
*   SYNOPSIS
*	CopyMem( source, dest, size )
*		 A0	 A1    D0
*
*   FUNCTION
*	CopyMem is a general purpose, fast memory copy routine.  It
*	can deal with arbitrary lengths, with its pointers on arbitrary
*	alignments.  It attempts to optimize larger copies with more
*	efficient copies, it uses byte copies for small moves or when
*	pointers are misaligned.
*
*	This routine may be implemented in a processor dependend fashion.
*
*   INPUTS
*	source - a pointer to the source data region
*	dest - a pointer to the destination data region
*	size - the size (in bytes) of the memory area
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	CopyMemQuick
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*/

*****o* exec.library/CopyMemQuick *********************************************
*
*   NAME
*	CopyMemQuick - optimized memory copy routine
*
*   SYNOPSIS
*	CopyMemQuick( source, dest, size )
*		      A0      A1    D0
*
*   FUNCTION
*	CopyMemQuick is a highly optimized memory copy routine, with
*	restrictions on the size and alignment of its arguments.
*	Both the source and destination pointers must be longword
*	aligned.  In addition, the size must be an integral number
*	of longwords (e.g. the size must be evenly divisible by
*	four).
*
*	This routine may be implemented in a processor dependend fashion.
*
*   INPUTS
*	source - a pointer to the source data region, long aligned
*	dest - a pointer to the destination data region, long aligned
*	size - the size (in bytes) of the memory area
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	CopyMem
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*/

CopyMemQuick:	; ( a0: source, a1: dest, d0: numBytes )
	moveq	#0,d1
	bra.s	bc_copyquick


CopyMem:	; ( a0: source, a1: dest, d0: numBytes )

	;------ see if it is too small to even bother with
	moveq	#12,d1
	cmp.l	d1,d0
	bcs.s	bc_bytes	; blo.s

	;------ check out the allignment.  There are four cases:
	;------   1. properly aligned (use block copy)
	;------ 	a. big blocks (use movem's)
	;------ 	b. little blocks (use dbra's)
	;------   2. both on odd byte boundaries (copy one byte, then blocks)
	;------   3. on different boundaries (use byte copy)

	;------ see if a0 is aligned
	move.l	a0,d1
	btst	#0,d1
	beq.s	bc_alignA1

	;------ copy one bytes
	move.b	(a0)+,(a1)+
	subq.l	#1,d0

bc_alignA1:
	;------ see if a1 is aligned.  if not, go to byte copy
	move.l	a1,d1
	btst	#0,d1
	bne.s	bc_bytes


	;------ we know we have a word aligned block.  do a block copy
	move.l	d0,d1
	and.w	#3,d1

bc_copyquick:
	move.l	d1,-(sp)                ; save residual byte count


	moveq	#120,d1
	cmp.l	d1,d0
	bcs.s	bc_longcopy		; blo.s

MYREGS		REG	d1/d2/d3/d4/d5/d6/d7/a2/a3/a4/a5/a6
SAVEREGS	REG	d2/d3/d4/d5/d6/d7/a2/a3/a4/a5/a6

	;------ copy big stuff by movem.l copies
	movem.l SAVEREGS,-(sp)

bc_bigcopy:
	movem.l (a0)+,MYREGS
	movem.l MYREGS,(a1)
	moveq	#48,d1			; # of bytes we just moved
	add.l	d1,a1			; bump dest pointer
	sub.l	d1,d0			; decrement count
	cmp.l	d1,d0			; see if we can do it again
	bcc.s	bc_bigcopy		; bhs

	movem.l (sp)+,SAVEREGS

bc_longcopy:
	lsr.l	#2,d0			; turn into longword count
	beq.s	bc_longcopy_end

	subq.l	#1,d0			; dbra madness
	move.l	d0,d1			; get high word accessible
	swap	d0

1$:	move.l	(a0)+,(a1)+
	dbra	d1,1$
	dbra	d0,1$

	;------ now deal with the residual count
bc_longcopy_end:
	move.l	(sp)+,d1
	beq.s	bc_end

	;------ jump right into the byte copy routine
	moveq	#0,d0
	bra.s	bc_bytesentry


bc_bytes:
	;------ do byte by byte copy
	move.w	d0,d1		; least significant word
	swap	d0		; most significant word
	bra.s	bc_bytesentry

bc_bytesloop:
	move.b	(a0)+,(a1)+
bc_bytesentry:
	dbra	d1,bc_bytesloop
	dbra	d0,bc_bytesloop

bc_end:
	rts

	END
