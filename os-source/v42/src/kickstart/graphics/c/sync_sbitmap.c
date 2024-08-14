/******************************************************************************
*
*	$Id: sync_sbitmap.c,v 42.0 93/06/16 11:15:57 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include "c.protos"

/*#define DEBUG*/
/* this could eventually become a simple call to ClipBlt */
/* but only when clipblit supports superbitmaps I think */

/****** graphics.library/SyncSBitMap *******************************************
*
*   NAME
*       SyncSBitMap --	Syncronize Super BitMap with whatever is
*			in the standard Layer bounds.
*
*   SYNOPSIS
*       SyncSBitMap( layer )
*                      a0
*
*	void SyncSBitMap( struct Layer * );
*
*   FUNCTION
*       Copy all bits from ClipRects in Layer into Super BitMap
*	BitMap.  This is used for those functions that do not
*	want to deal with the ClipRect structures but do want
*	to be able to work with a SuperBitMap Layer.
*
*   INPUTS
*	layer - pointer to a Layer that has a SuperBitMap
*		The Layer should already be locked by the caller.
*
*   RESULT
*	After calling this function, the programmer can manipulate
*	the bits in the superbitmap associated with the layer.
*	Afterwards, the programmer should call CopySBitMap to
*	copy the bits back into the onscreen layer.
*
*   BUGS
*
*   SEE ALSO
*	CopySBitMap() graphics/clip.h
*
******************************************************************************/

/* no tmpras needed */
void __asm SyncSBitMap(register __a0 struct Layer *l)
{
	struct RastPort rp;
	struct BitMap *bm;

	shortinitrast(&rp);
	bm = rp.BitMap = l->SuperBitMap;
	l->SuperBitMap = 0;

	bltrastport(l->rp,0,0,&rp,0,0,bm->BytesPerRow<<3,bm->Rows,
				NANBC|NABC|ABNC|ABC);
	l->SuperBitMap = bm;
}
