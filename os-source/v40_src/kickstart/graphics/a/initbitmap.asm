*******************************************************************************
*
*	$Id: initbitmap.asm,v 37.2 91/11/15 11:00:45 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'                  * Data type definitions
	include 'graphics/gfx.i'                    * BitMap structure

	section	graphics
	xdef    _InitBitMap     * Define function entries as public symbols

******* graphics.library/InitBitMap ********************************************
* 
*   NAME   
* 
*   	InitBitMap -- Initialize bit map structure with input values.
* 
*   SYNOPSIS
*	InitBitMap( bm, depth, width, height )
*		    a0   d0     d1      d2
*
*	void InitBitMap( struct BitMap *, BYTE, UWORD, UWORD );
*	
*   FUNCTION
*	Initialize various elements in the BitMap structure to
*	correctly reflect depth, width, and height.
*	Must be used before use of BitMap in other graphics calls.
*	The Planes[8] are not initialized and need to be set up
*	by the caller.  The Planes table was put at the end of the
*	structure so that it may be truncated to conserve space,
*	as well as extended. All routines that use BitMap should
*	only depend on existence of depth number of bitplanes.
*	The Flagsh and pad fields are reserved for future use and
*	should not be used by application programs.
*
*   INPUTS
*	bm - pointer to a BitMap structure (gfx.h)
*	depth - number of bitplanes that this bitmap will have
*	width - number of bits (columns) wide for this BitMap
*	height- number of bits (rows) tall for this BitMap
*
*   BUGS
* 
*   SEE ALSO
*	graphics/gfx.h
* 
******************************************************************************
	PAGE
*                               INITIALIZE BitMap STRUCTURE
_InitBitMap:
	clr.b	bm_Flags(a0)
	clr.w	bm_Pad(a0)
	move    d2,bm_Rows(a0)              * bm->Rows = height;

	move.b  d0,bm_Depth(a0)             * bm->Depth = depth;

	add.w   #15,d1
	asr     #4,d1
	asl     #1,d1
	move    d1,bm_BytesPerRow(a0)       * bm->BytesPerRow = ((width+15)>>4)<1;

	rts                                 * Exit to caller

	end                                 * End of InitBitMap
