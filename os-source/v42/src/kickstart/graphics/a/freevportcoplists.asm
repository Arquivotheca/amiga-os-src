*******************************************************************************
*
*	$Id: FreeVPortCopLists.asm,v 42.0 93/06/16 11:13:25 chrisg Exp $
*
*******************************************************************************


	section	graphics
    xdef    _FreeVPortCopLists
    xref    _freevportcoplists
******* graphics.library/FreeVPortCopLists *************************************
*
*   NAME
*       FreeVPortCopLists -- deallocate all intermediate copper lists and
*       their headers from a viewport
*
*   SYNOPSIS
*       FreeVPortCopLists(vp)
*                         a0
*
*	void FreeVPortCopLists(struct ViewPort *);
*
*   FUNCTION
*       Search display, color, sprite, and user copper
*       lists and call FreeMem() to deallocate them from memory
*
*   INPUTS
*       vp - pointer to ViewPort structure
*
*   RESULTS
*       The memory allocated to the various copper lists will be returned
*	to the system's free memory pool, and the following fields in
*	the viewport structure will be set to NULL:
*		
*		DspIns, Sprins, ClrIns, UCopIns
*
*   BUGS
*       none known
*
*   SEE ALSO
*	graphics/view.h
*
*******************************************************************************
_FreeVPortCopLists:
*               current routine calls a C subroutine to do the work
	move.l	a0,-(sp)	* pointer to viewport
    jsr     _freevportcoplists
    lea     4(sp),sp   * reset stack
    rts

	end
