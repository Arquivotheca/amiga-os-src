*******************************************************************************
*
*	$Id: InitTmpRas.asm,v 42.0 93/06/16 11:12:57 chrisg Exp $
*
*******************************************************************************


    include 'exec/types.i'
    include 'graphics/rastport.i'

	section	graphics
    xdef    _InitTmpRas
******* graphics.library/InitTmpRas *****************************************
*
*   NAME
*	InitTmpRas -- Initialize area of local memory for usage by
*			areafill, floodfill, text.
*
*   SYNOPSIS
*   	InitTmpRas(tmpras, buffer, size)
*              	    a0	     a1     d0
*
*	void InitTmpRas( struct TmpRas *, void *, ULONG );
*
*   FUNCTION
*	The area of memory pointed to by buffer is set up to be used
*	by RastPort routines that may need to get some memory for
*	intermediate operations in preparation to putting the graphics
*	into the final BitMap.
*	Tmpras is used to control the usage of buffer.
*
*   INPUTS
*	tmpras - pointer to a TmpRas structure to be linked into
*		a RastPort
*	buffer - pointer to a contiguous piece of chip memory.
*	size - size in bytes of buffer
*
*   RESULT
*	makes buffer available for users of RastPort
*
*   BUGS
*	Would be nice if RastPorts could share one TmpRas.
*
*   SEE ALSO
*	AreaEnd() Flood() Text() graphics/rastport.h
*
*********************************************************************
_InitTmpRas:
    move.l  a1,tr_RasPtr(a0)
    move.l  d0,tr_Size(a0)
    move.l  a0,d0           return(tmpras *)
    rts

	end
