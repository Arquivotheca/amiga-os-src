/******************************************************************************
*
*	$Id: copy_sbitmap.c,v 39.3 92/10/23 11:38:56 chrisg Exp $
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
#include "/gfxpragmas.h"

/*#define DEBUG*/

/****** graphics.library/CopySBitMap ******************************************
*
*   NAME
*       CopySBitMap --	Syncronize Layer window with contents of
*						Super BitMap
*
*   SYNOPSIS
*       CopySBitMap( layer )
*                     a0
*
*	void CopySBitMap(struct Layer *);
*
*   FUNCTION
*	This is the inverse of SyncSBitMap.
*       Copy all bits from SuperBitMap to Layer bounds.
*	This is used for those functions that do not
*	want to deal with the ClipRect structures but do want
*	to be able to work with a SuperBitMap Layer.
*
*   INPUTS
*	layer - pointer to a SuperBitMap Layer
*	    The Layer must already be locked by the caller.
*
*   BUGS
*
*   SEE ALSO
*	LockLayerRom() SyncSBitMap()
*
******************************************************************************/

/* copy raster bits from super bitmap to Layer cliprects */
/* no tmpras needed for these BltRasters */
void __asm CopySBitMap(register __a0 struct Layer *l)
{
	struct BitMap *bm;
	bm = l->SuperBitMap;
	l->SuperBitMap = 0;
	clipbltrastport(bm,0,0,
					l->rp,0,0,
					bm->BytesPerRow<<3,bm->Rows,
					NANBC|NABC|ABNC|ABC,0);
	l->SuperBitMap = bm;
}
