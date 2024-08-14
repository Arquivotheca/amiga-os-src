*******************************************************************************
*
*	$Id: InitVPort.asm,v 42.0 93/06/16 11:13:09 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'                  * Data type definitions
	include 'graphics/view.i'               * View and ViewPort structures

	section	graphics
	xdef    _InitVPort      * Define public entry pointsA

	xref    ClearMem

	PAGE

******* graphics.library/InitVPort *********************************************
* 
*   NAME   
*	InitVPort - Initialize ViewPort structure.
* 
*   SYNOPSIS
*	InitVPort( vp )
*		   a0
*
*	void InitViewPort( struct ViewPort * );
* 
*   FUNCTION
*	Initialize ViewPort structure to default values.
*
*   INPUTS
*	vp - pointer to a ViewPort structure
*
*   RESULT
*	ViewPort structure set to all 0's. (1.0,1.1)
*       New field added SpritePriorities, initialized to 0x24 (1.2)
* 
*   BUGS
*
*   SEE ALSO
*	MakeVPort() graphics/view.h
* 
******************************************************************************

	PAGE
*                               INIT ViewPort STRUCTURE
_InitVPort:
	move.l	a0,a1		* clearmem does not trash a1
	moveq   #vp_SIZEOF,d0               * d0 = number of bytes to clear
	bsr     ClearMem
	move.b	#$24,vp_SpritePriorities(a1)
	rts

	end
