*******************************************************************************
*
*	$Id: initrastport.asm,v 39.3 93/05/06 11:58:59 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'                      * Data type definitions
	include '/macros.i'
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include 'exec/ports.i'
	include 'graphics/gfxbase.i'
	include 'graphics/rastport.i'           * RastPort/AreaInfo structures
	include 'submacs.i'                     * Subroutine/function macros

	section	graphics
	xdef    shortinitrast               * used by local gfx code
	xdef    _InitRastPort               * vector code

	xref    GenMinTerms                     * Define external symbols used

	EXTERN_FUNC SetFont

******* graphics.library/InitRastPort ******************************************
*
*   NAME
*	InitRastPort -- Initialize raster port structure
*
*   SYNOPSIS
*   	InitRastPort( rp )
*		      a1
*
*	void InitRastPort(struct RastPort *rp);
*
*   FUNCTION
*       Initialize a RastPort structure to standard values.
*
*   INPUTS
*	rp	= pointer to a RastPort structure.
*
*   RESULT
*	all entries in RastPort get zeroed out, with the following exceptions:
*
*	    Mask, FgPen, AOLPen, and LinePtrn are set to -1.
*	    The DrawMode is set to JAM2
*	    The font is set to the standard system font
*
*   NOTES
*	The struct Rastport describes a control structure
*       for a write-able raster. The RastPort structure
*       describes how a complete single playfield display
*       will be written into. A RastPort structure is
*       referenced whenever any drawing or filling
*       operations are to be performed on a section of
*       memory.
*
*       The section of memory which is being used in this
*       way may or may not be presently a part of the
*       current actual onscreen display memory. The name
*       of the actual memory section which is linked to
*       the RastPort is referred to here as a "raster" or
*       as a bitmap.
*
*       NOTE: Calling the routine InitRastPort only
*       establishes various defaults. It does NOT
*       establish where, in memory, the rasters are
*       located. To do graphics with this RastPort the user
*	must set up the BitMap pointer in the RastPort.
*
*   BUGS
*
*   SEE ALSO
*   	graphics/rastport.h
*
******************************************************************************



    PAGE
*                               INITIALIZE RASTER PORT STRUCTURE
    xref ClearMem
shortinitrast:
_shortinitrast::
; void __asm shortinitrast(register __a1 struct RastPort *rp);

    move.l  a1,a0               * a0 = pointer to start of init area
    move.l  #rp_SIZEOF,d0       * d0 = count of bytes to clear
    bsr     ClearMem

    moveq   #-1,d0
    move.b  d0,rp_Mask(a1)              * r->Mask = -1;
    move.b  d0,rp_FgPen(a1)             * r->FgPen = -1;
    move.b  d0,rp_AOLPen(a1)            * r->AOLPen = -1;
    move    d0,rp_LinePtrn(a1)          * r->LinePtrn = -1;
    move.b  #RP_JAM2,rp_DrawMode(a1)    * r->DrawMode = 1;

	bra		GenMinTerms

*                           CALLS TO INIT RASTER PORT STRUCTURE, ETC.
_InitRastPort:
    bsr     shortinitrast

    move.l  gb_DefaultFont(a6),a0
    JUMPGFX SetFont         * SETFONT(GB, r, &setFontDesc);

    end                                 * End of _InitRast
