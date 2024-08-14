*******************************************************************************
*
*	$Id: qblit.asm,v 39.4 92/08/11 13:58:58 chrisg Exp $
*
*******************************************************************************

    
    include 'exec/types.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/ports.i'
    include 'exec/interrupts.i'
    include 'exec/libraries.i'
    include	'exec/ables.i'

    include '/gfx.i'
    include '/gfxbase.i'
    include 'hardware/blit.i'
    include 'hardware/intbits.i'
    include 'hardware/dmabits.i'
    include 'hardware/custom.i'
    include '/view.i'


	section	graphics
    xref    _intreq
    xref    _intreqr
    xref    _intena
    xref    _intenar
    xref    _custom
    xref    _dmaconr

	xref	_LVOPermit

	IFNE	bn_n
	FAIL	"bn_n NOT ZERO; recode!"
	ENDC

    xdef    _QBlit
    xdef    qbs_entry

******* graphics.library/QBlit ************************************************
*
*   NAME
*
*	QBlit -- Queue up a request for blitter usage
*
*   SYNOPSIS
*	QBlit( bp )
*	       a1
*
*	void QBlit( struct bltnode * );
*
*   FUNCTION
*	Link a request for the use of the blitter to the end of the
*       current blitter queue.  The pointer bp points to a blit structure
*       containing, among other things, the link information, and the
*       address of your routine which is to be called when the blitter
*       queue finally gets around to this specific request.  When your
*       routine is called, you are in control of the blitter ... it is
*       not busy with anyone else's requests.  This means that you can
*       directly specify the register contents and start the blitter.
*       See the description of the blit structure and the uses of QBlit
*       in the section titled Graphics Support in the OS Kernel Manual.
*       Your code must be written to run either in supervisor or user
*       mode on the 68000.
*
*   INPUTS
*	bp - pointer to a blit structure
*
*   RESULT
*	Your routine is called when the blitter is ready for you.
*	In general requests for blitter usage through this channel are
*	put in front of those who use the blitter via OwnBlitter and
*	DisownBlitter. However for small blits there is more overhead
*	using the queuer than Own/Disown Blitter.
*
*   NOTES
*	Code which uses QBlit(), or QBSBlit() should make use of
*	the pointer to a cleanup routine in the bltnode structure.
*	The cleanup routine may be called on the context of an
*	interrupt, therefore the routine may set a flag, and signal
*	a task, but it may not call FreeMem() directly.  Use of
*	the cleanup routine is the only safe way to signal that
*	your bltnode has completed.
*
*   BUGS
*	QBlit(), and QBSBlit() have been rewritten for V39 due to
*	various long standing bugs in earlier versions of this code.
*
*   SEE ALSO
*	QBSBlit() hardware/blit.h
*
******************************************************************************

_QBlit:

	;-- Indicate that beam sync'ed blits not requested.  We will
	;-- use the upper bit of the bn_beamsync WORD to indicate this,
	;-- thereby trying not to munge the actual beamsync value provided.
	;-- This bit must be TRUE, or else we interpret this to mean that
	;-- beamsync value is to be ignored.  An example of this is
	;-- QBSBlit(); the flag is cleared for each node at VBLANK time
	;-- meaning we should not bother trying to beamsync the node any
	;-- longer because VBLANK has occured sometime between the call
	;-- to QBSBlit(), and the time the node is handled.  As it turns
	;-- out, the old code had special case handling for negative
	;-- beamsync values, and clipped those to mean MaxDisplayRows.
	;-- So the 15 bit number is not a new limit.

		bclr	#7,bn_beamsync(a1)

	;-- An entry point for QBSBlit(); do not munge bn_beamsync high bit

qbs_entry:

	;-- clear pointer to next blit node

		clr.l	(a1)
		move.l	a2,d1
		lea	_custom,a2

	;-- protect QBlit list (DISABLE does all we need; no need for
	;-- for FORBID).

		move.l	gb_ExecBase(a6),a0
		DISABLE	A0,NOFETCH


	;-- If something already on QBlit list, then just add to list,
	;-- and bail out.

		tst.l	gb_blthd(a6)
		bne.s	qb_inuse

	;-- This node is now head, and tail of QBlit list

		move.l	a1,gb_blthd(a6)		;AddHead

	;-- Try to own the blitter immediately

		addq.w	#1,gb_BlitLock(a6)	;one lock for all qblits
		bne.s	qb_enable

	;-- mark that QBlit owns the blitter.

		or.w	#QBOWNER,gb_Flags(a6)

	;-- clear any pending interrupt requests and let the current blit
	;-- cause one (or we'll cause one below).
	;-- Enable blitter interrupts; let the next blitter interrupt
	;-- trigger our blitter interrupt code when the current
	;-- blit (if any) is done, or as a result of this code
	;-- causing a blitter interrupt.

		move.w	#INTF_BLIT,intreq(a2)
		move.w	#BITSET+INTF_BLIT,intena(a2)

	;-- make sure we get a blitter interrupt to wake up the
	;-- blitter interrupt code, which will actually deal with this
	;-- node

		tst.b	dmaconr(a2)		; pretest (for bad agnus)


		btst	#DMAB_BLTDONE-8,dmaconr(a2)
		bne.s	qb_enable

		move.w	#BITSET!INTF_BLIT,intreq(a2)

qb_enable:
		move.l	a1,gb_blttl(a6)		;This new node is now tail of list
		ENABLE	A0,NOFETCH
		move.l	d1,a2
		rts

	;-----------------------------------------------------------
	;-- The QBlit list is active, so simply add to qblit list.
	;-- We know that we have already incremented the BlitLock
	;-- count because some blit node had to go on this list
	;-- when the list was empty.
	;-----------------------------------------------------------
  

qb_inuse:
		move.l	gb_blttl(a6),a0		;link at tail
		move.l	a1,(a0)

		move.l	gb_ExecBase(a6),a0
		bra.s	qb_enable

		END
