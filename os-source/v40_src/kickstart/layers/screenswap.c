/******************************************************************************
*
*	$Id: screenswap.c,v 38.4 91/11/15 20:20:16 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include <hardware/dmabits.h>

#include "layersbase.h"

/****** layers.library/SwapBitsRastPortClipRect ********************************
*
*    NAME
*	SwapBitsRastPortClipRect -- Swap bits between common bitmap
*	                            and obscured ClipRect
*
*    SYNOPSIS
*	SwapBitsRastPortClipRect( rp, cr )
*	                          a0  a1
*
*	void SwapBitsRastPortClipRect( struct RastPort *, struct ClipRect *);
*
*    FUNCTION
*	Support routine useful for those that need to do some
*	operations not done by the layer library.  Allows programmer
*	to swap the contents of a small BitMap with a subsection of
*	the display. This is accomplished without using extra memory.
*	The bits in the display RastPort are exchanged with the
*	bits in the ClipRect's BitMap.
*
*	Note: the ClipRect structures which the layer library allocates are
*	actually a little bigger than those described in the graphics/clip.h
*	include file.  So be warned that it is not a good idea to have
*	instances of cliprects in your code.
*
*    INPUTS
*	rp - pointer to rastport
*	cr - pointer to cliprect to swap bits with
*
*    NOTE
*	Because the blit operation started by this function is done asynchronously,
*	it is imperative that a WaitBlit() be performed before releasing or using
*	the processor to modify any of the associated structures.
*
*    BUGS
*
*    SEE ALSO
*	graphics/clip.h, graphics/rastport.h, graphics/clip.h
*
*******************************************************************************/

void newscreentocr(struct RastPort *rp,struct ClipRect *cr,long minterm)
{
	BltBitMap(rp->BitMap,cr->bounds.MinX,cr->bounds.MinY,
			cr->BitMap,cr->bounds.MinX&15,0,
			cr->bounds.MaxX-cr->bounds.MinX+1,
			cr->bounds.MaxY-cr->bounds.MinY+1,
			minterm,-1,0);
}

void newcrtoscreen(struct RastPort *rp,struct ClipRect *cr,long minterm)
{
	BltBitMap(cr->BitMap,cr->bounds.MinX&15,0,
			rp->BitMap,cr->bounds.MinX,cr->bounds.MinY,
			cr->bounds.MaxX-cr->bounds.MinX+1,
			cr->bounds.MaxY-cr->bounds.MinY+1,
			minterm,-1,0);
}

/* similar to screenswap but only one way */
void screentocr(struct RastPort *rp,struct ClipRect *cr)
{
	newscreentocr(rp,cr,NANBC|NABC|ABNC|ABC);
}

void __stdargs __asm crtoscreen(register __a0 struct RastPort *rp,register __a1 struct ClipRect *cr)
{
	newcrtoscreen(rp,cr,NANBC|NABC|ABNC|ABC);
}

void __stdargs __asm screenswap(register __a0 struct RastPort *rp,register __a1 struct ClipRect *cr)
{
	newscreentocr(rp,cr,NANBC|NABC|ANBC|ABNC);
	newcrtoscreen(rp,cr,NANBC|NABC|ANBC|ABNC);
	newscreentocr(rp,cr,NANBC|NABC|ANBC|ABNC);
}
