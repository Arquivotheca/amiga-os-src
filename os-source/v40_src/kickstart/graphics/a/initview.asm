*******************************************************************************
*
*	$Id: InitView.asm,v 39.0 91/08/21 17:26:18 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'                  * Data type definitions
	include 'graphics/view.i'               * View and ViewPort structures

	section	graphics
	xdef    _InitView

	xref    ClearMem

	PAGE
******* graphics.library/InitView *******************************************
* 
*   NAME   
*   InitView - Initialize View structure.
* 
*   SYNOPSIS
*	InitView( view )
*		   a1
*
*	void InitView( struct View * );
* 
*   FUNCTION
*	Initialize View structure to default values.
*
*   INPUTS
*	view - pointer to a View structure
*
*   RESULT
*	View structure set to all 0's. (1.0,1.1.1.2)
*	Then values are put in DxOffset,DyOffset to properly position
*	default display about .5 inches from top and left on monitor.
*	InitView pays no attention to previous contents of view.
* 
*   BUGS
*
*   SEE ALSO
* 	MakeVPort graphics/view.h
* 
******************************************************************************

_InitView:
	move.l  a1,a0
	moveq   #v_SIZEOF,d0                * d0 = number of bytes to clear

	bsr     ClearMem
	move.w  #$2c,v_DyOffset(a1)
	move.w  #$81,v_DxOffset(a1)

	rts                                 * Exit to caller


	end                                 * End of initvp.asm
