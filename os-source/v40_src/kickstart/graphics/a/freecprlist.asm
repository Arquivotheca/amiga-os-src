*******************************************************************************
*
*	$Id: FreeCprList.asm,v 39.0 91/08/21 17:25:20 chrisg Exp $
*
*******************************************************************************


	section	graphics
    xdef    _FreeCprList
    xref    _freecprlist
******* graphics.library/FreeCprList *******************************************
*
*   NAME
*       FreeCprList -- deallocate hardware copper list
*
*   SYNOPSIS
*       FreeCprList(cprlist)
*		      a0
*
*	void FreeCprList(struct cprlist *);
*
*   FUNCTION
*       return cprlist to free memory pool
*
*   INPUTS
*       cprlist - pointer to cprlist structure
*
*   RESULTS
*	memory returned and made available to other tasks
*
*   BUGS
*
*   SEE ALSO
*	graphics/copper.h
*
*******************************************************************************/
_FreeCprList:
*               current routine calls a C subroutine to do the work
	move.l	a0,-(sp)	* pointer to coplist
    jsr     _freecprlist
    lea     4(sp),sp   * reset stack
    rts

	end
