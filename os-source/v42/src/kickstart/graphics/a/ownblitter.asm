*******************************************************************************
*
*	$Id: OwnBlitter.asm,v 42.0 93/06/16 11:13:32 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/ports.i'
    include 'exec/interrupts.i'
    include 'exec/libraries.i'
	include	'exec/ables.i'
    include 'graphics/gfx.i'
    include 'graphics/gfxbase.i'

    include 'hardware/blit.i'
    include '/a/submacs.i'
    include 'hardware/intbits.i'
    include 'hardware/dmabits.i'
    include '/sane_names.i'

	section	graphics
    xref    _intena
    xref    _intreq
    xref    _dmaconr
******* graphics.library/OwnBlitter ********************************************
*
*   NAME
*       OwnBlitter -- get the blitter for private usage
*
*   SYNOPSIS
*       OwnBlitter()
*
*	void OwnBlitter( void );
*
*   FUNCTION
*	If blitter is available return immediately with the blitter
*	locked for your exclusive use. If the blitter is not available
*	put task to sleep. It will be awakened as soon as the blitter
*	is available. When the task first owns the blitter the blitter
*	may still be finishing up a blit for the previous owner. You
*	must do a WaitBlit before actually using the blitter registers.
*
*	Calls to OwnBlitter() do not nest. If a task that owns the
*	blitter calls OwnBlitter() again, a lockup will result. 
*	(Same situation if the task calls a system function
*	that tries to own the blitter).
*
*   INPUTS
*	NONE
*
*   RETURNS
*	NONE
*
*   SEE ALSO
*	DisownBlitter() WaitBlit()
***********************************************************

    xdef    _OwnBlitter
	xdef	OwnBlitmacroentry
    xref    _cownblitter

	xref	_LVOFindTask
	xref	_LVOSetSignal
	xref	_LVOSignal
	xref	_LVOAddTail
	xref	_LVOWait
	xref	_intena

BLITTER_SIGNAL equ 4

_OwnBlitter:
* enter with a6->GfxBase
    addq.w  #1,gb_BlitLock(a6)
    if =
		rts
    endif
OwnBlitmacroentry:
	movem.l d0/d1/a0/a1/a5-a6,-(sp)
*    jsr _cownblitter
	sub.w	#LN_SIZE+2,sp
	lea	gb_BlitWaitQ(a6),a5
	move.l	gb_ExecBase(a6),a6
	move.l	ThisTask(a6),LN_NAME(sp)
	moveq	#0,d0
	moveq	#1<<BLITTER_SIGNAL,d1
	jsr	_LVOSetSignal(a6)
	lea	(gb_Flags+1)-gb_BlitWaitQ(a5),a0
	moveq	#BLITMSG_FAULTn,d0
	move.l	sp,a1	; for ADDTAIL below
	DISABLE
	bclr.b	d0,(a0)	; test and clear
	bne.s	OBme1
	move.l	a5,a0
	ADDTAIL
	ENABLE
	moveq	#1<<BLITTER_SIGNAL,d0
	jsr	_LVOWait(a6)
	add.w	#LN_SIZE+2,sp
	movem.l (sp)+,d0/d1/a0/a1/a5-a6
	rts
OBme1:
	ENABLE
	add.w	#LN_SIZE+2,sp
	movem.l (sp)+,d0/d1/a0/a1/a5-a6
	rts

******* graphics.library/DisownBlitter **********************************
*
*   NAME
*       DisownBlitter -- return blitter to free state.
*
*
*   SYNOPSIS
*       DisownBlitter()
*
*	void DisownBlitter( void );
*
*   FUNCTION
*	Free blitter up for use by other blitter users.
*
*   INPUTS
*
*   RETURNS
*
*   SEE ALSO
*       OwnBlitter()
*
*
***********************************************************
* called like disownblitter(GB)
    xdef    _DisownBlitter
    xdef    _disownnodec
    xdef    DisownBlitmacroen   * used if already decremented
    xref    _cdisownblitter
_DisownBlitter:
	subq.w  #1,gb_BlitLock(a6)
_disownnodec:
	blt.s	rts1
DisownBlitmacroen:
	movem.l	d0/d1/a0/a1/a5/a6,-(sp)
	move.l	gb_ExecBase(a6),a5
	lea	gb_BlitWaitQ(a6),a0	* for REMHEAD below
	DISABLE	a5,NOFETCH
	tst.l	gb_blthd(a6)		* any blits queued?
	beq.s	no_qb1
enablit:
	tst.b   _dmaconr 		* bart - 03.08.90 - if bad agnus !!!
	btst    #DMAB_BLTDONE-8,_dmaconr
	bne.s	no_cause_int
	move.w  #BITSET+INTF_BLIT,_intreq
no_cause_int:
	move.w  #BITSET+INTF_BLIT,_intena
	or.w    #QBOWNER,gb_Flags(a6)
	ENABLE	a5,NOFETCH
	movem.l	(sp)+,d0/d1/a0/a1/a5/a6
rts1:	rts
no_qb1:
    MOVE.L  (A0),A1
    MOVE.L  (A1),D0
    BEQ.S   set_fault
    MOVE.L  D0,(A0)
    MOVE.L  LN_NAME(a1),a6
    MOVE.L  D0,A1
    MOVE.L  A0,LN_PRED(A1)
	move.l	a6,a1
	move.l	a5,a6
	move.l	#1<<BLITTER_SIGNAL,d0
	jsr	_LVOSignal(a6)
	ENABLE
	movem.l	(sp)+,d0/d1/a0/a1/a5/a6
	rts
set_fault:
	bset.b	#BLITMSG_FAULTn,gb_Flags+1(a6)
	ENABLE	a5,NOFETCH
	movem.l	(sp)+,d0/d1/a0/a1/a5/a6
	rts

	end
