*******************************************************************************
*
*	$Id: QBSBlit.asm,v 42.0 93/06/16 11:13:19 chrisg Exp $
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
    xref    _ciab
    xref    _intreq
    xref    _intreqr
    xref    _intena
    xref    _intenar
    xref    _custom
    xref    _vposr
    xref    _dmaconr

    xref    disownblitter
    xref    start_timer

    xref    _Forbid
    xref    _Permit

    xref    _LVOAbleICR
    xref    waitblitdone
	xref	_LVOPermit

    xdef    _QBSBlit
    xref    _Debug
	xref	_QBlit
	xref	qbs_entry

******* graphics.library/QBSBlit **********************************************
*
*   NAME
*
*	QBSBlit -- Synchronize the blitter request with the video beam.
*
*   SYNOPSIS
*
*	QBSBlit( bsp )
*		 a1
*
*	void QBSBlit( struct bltnode * );
*
*   FUNCTION
*	Call a user routine for use of the blitter, enqueued separately from
*       the QBlit queue.  Calls the user routine contained in the blit
*       structure when the video beam is located at a specified position
*       onscreen.   Useful when you are trying to blit into a visible part
*       of the screen and wish to perform the data move while the beam is
*       not trying to display that same area.  (prevents showing part of
*       an old display and part of a new display simultaneously).  Blitter
*       requests on the QBSBlit queue take precedence over those on the
*       regular blitter queue. The beam position is specified the blitnode.
*
*   INPUTS
*	bsp - pointer to a blit structure.  See description in the
*             Graphics Support section of the manual for more info.
*
*   RESULT
*       User routine is called when the QBSBlit queue reaches this
*       request AND the video beam is in the specified position.
*       If there are lots of blits going on and the video beam
*       has wrapped around back to the top it will call all the
*       remaining bltnodes as fast as it can to try and catch up.
*
*   NOTES
*       QBlit(), and QBSBlit() have been rewritten for V39.  Queued
*       blits are now handled in FIFO order.  Tasks trying to
*       OwnBlitter() are now given a fair share of the total
*       blitter time available.  QBSBlit() are no longer queued
*       separately from nodes added by QBlit().  This fixes the
*       ordering dependencies listed under BUGS in prior autodoc
*       notes. 
*
*   BUGS
*
*   SEE ALSO
*	QBlit() hardware/blit.h
*
******************************************************************************
_QBSBlit:

	;-- This code is similar to QBlit().  The only real difference
	;-- is we try to honor the request for beam syncing... if
	;-- can do so before the next VBLANK


	move.w	bn_beamsync(a1),d0

	;-- The argument could be made that we should check the current
	;-- monitor's MaxDisplayRow value ... wait ...
	;
	;-- This is probably not what we want to check at all!  But this
	;-- is the value that the old QBSBlit() code used for beam position
	;-- clipping, so I do the same, but in any case...
	;
	;-- It could be argued that we really need to check this in FORBID,
	;-- but the answer to that is, why bother?  It doesn't really
	;-- matter that we clip now, and the active monitor changes before
	;-- we queue the blit, or after.   We could just as easily end
	;-- up checking inside of FORBID or DISABLE, and the current monitor
	;-- could switch after the blit is queued, but before it is actually
	;-- started (waiting for the beam position to trigger the timer).
	;
	;-- Since it doesn't matter ... regardless, we will fix ourselves
	;-- because at the next VBLANK will immediately satisfy any blitnode
	;-- waiting for the beamsynced timer to alarm.


	move.w	gb_MaxDisplayRow(a6),d1		;atomic read of value (may change)
	cmp.w	d1,d0
	bls.s	qbs_validbeam			;unsigned; also clips negative #'s
						;for compatability with old code
	move.w	d1,d0

qbs_validbeam:

	bset	#15,d0				;mark as beamsync if possible
	move.w	d0,bn_beamsync(a1)		;cache clipped value

	;-- Note that beam position 0 is really silly, but allowed.  This
	;-- new code does not use gb_BeamSync for anything anymore.  It use
	;-- to use it as a flag.  If someone had specified 0 for
	;-- beamsync value, the system was hosed.  Probably noone did,
	;-- but this isn't a bug anymore.  If the current beam position is
	;-- past the requested position, then the beam synced blit is
	;-- simply started immediately.

	bra	qbs_entry

	END

