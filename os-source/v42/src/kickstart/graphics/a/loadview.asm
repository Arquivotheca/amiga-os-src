*******************************************************************************
*
*	$Id: LoadView.asm,v 42.0 93/06/16 11:13:17 chrisg Exp $
*
*******************************************************************************

	section	graphics

	include '/monitor.i'
	include '/view.i'
	include '/macros.i'

	xdef    _LoadView   
	xref    _loadview
	xref	_LVOGfxLookUp

******* graphics.library/LoadView *********************************************
*
*   NAME
*       LoadView -- Use a (possibly freshly created) coprocessor instruction
*                   list to create the current display.
*
*   SYNOPSIS
*       LoadView( View )
*                  A1
*
*	void LoadView( struct View * );
*
*   FUNCTION
*	Install a new view to be displayed during the next display
*	refresh pass.
*       Coprocessor instruction list has been created by
*       InitVPort(), MakeVPort(), and MrgCop().
*
*   INPUTS
*       View - a pointer to the View structure which contains the
*       pointer to the constructed coprocessor instructions list, or NULL.
*
*   RESULT
*	If the View pointer is non-NULL, the new View is displayed, 
*	according to your instructions.  The vertical blank routine 
*	will pick this pointer up and direct the copper to start 
*	displaying this View.
*
*	If the View pointer is NULL, no View is displayed. 
*
*   NOTE
*	Even though a LoadView(NULL) is performed, display DMA will still be 
*	active.  Sprites will continue to be displayed after a LoadView(NULL)
*	unless an OFF_SPRITE is subsequently performed. 
*
*   BUGS
*
*   SEE ALSO
*       InitVPort() MakeVPort() MrgCop() intuition/RethinkDisplay()
*	graphics/view.h
*
******************************************************************************

_LoadView:
* From V39, LoadView() is vectored through the MonitorSpec. Use _loadview as
* a default.

	move.l	a1,-(sp)		; push view pointer on stack
	beq.s	lv_default			; LoadViewDefault

	move.l	a1,a0
	jsr	_LVOGfxLookUp(a6)
	tst.l	d0
	beq.s	lv_default
	move.l	d0,a0			; ViewExtra
	move.l	ve_Monitor(a0),d0
	beq.s	lv_default
	move.l	d0,a0
	move.l	ms_LoadView(a0),a0	; address to call
	move.l	(sp),a1			; get view address
lv_doit:
	jsr	(a0)
	addq.l	#4,sp
	rts
lv_default:
	lea	_loadview,a0
	bra.s	lv_doit

	end
