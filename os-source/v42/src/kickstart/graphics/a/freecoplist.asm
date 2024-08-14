*******************************************************************************
*
*	$Id: FreeCopList.asm,v 42.0 93/06/16 11:13:30 chrisg Exp $
*
*******************************************************************************


	section	graphics
    xdef    _FreeCopList
    xref    _freecoplist
******* graphics.library/FreeCopList *******************************************
*
*   NAME
*	FreeCopList -- deallocate intermediate copper list
*
*   SYNOPSIS
*       FreeCopList(coplist)
*		      a0
*
*	void FreeCopList( struct CopList *);
*
*   FUNCTION
*	Deallocate all memory associated with this copper list.
*
*   INPUTS
*       coplist	- pointer to structure CopList
*
*   RESULTS
*	memory returned to memory manager
*
*   BUGS
*
*   SEE ALSO
*	graphics/copper.h
*
*******************************************************************************
_FreeCopList:
*               current routine calls a C subroutine to do the work
	move.l	a0,-(sp)	* pointer to coplist
    jsr     _freecoplist
    lea     4(sp),sp   * reset stack
    rts

	end
