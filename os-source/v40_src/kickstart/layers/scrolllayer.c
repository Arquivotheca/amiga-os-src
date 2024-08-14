/******************************************************************************
*
*	$Id: scrolllayer.c,v 38.4 92/07/01 19:47:43 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>

#include "layersbase.h"

/****** layers.library/ScrollLayer *********************************************
*
*    NAME
*	ScrollLayer -- Scroll around in a superbitmap, translate coordinates
*                      in non-superbitmap layer.
*
*    SYNOPSIS
*	ScrollLayer( dummy, l, dx, dy )
*	             a0     a1 d0  d1
*
*	void ScrollLayer( LONG, struct Layer *, LONG, LONG);
*
*    FUNCTION
*	For a SuperBitMap Layer:
*	Update the SuperBitMap from the layer display, then copy bits
*	between Layer and SuperBitMap to reposition layer over different
*	portion of SuperBitMap.
*	For nonSuperBitMap layers, all (x,y) pairs are adjusted by
*	the scroll(x,y) value in the layer.  To cause (0,0) to actually
*	be drawn at (3,10) use ScrollLayer(-3,-10). This can be useful
*	along with InstallClipRegion to simulate Intuition GZZWindows
*	without the overhead of an extra layer.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a layer
*	dx - delta to add to current x scroll value
*	dy - delta to add to current y scroll value
*
*    BUGS
*	May not handle (dx,dy) which attempts to move the layer outside the
*	layer's SuperBitMap bounds.
*
*    SEE ALSO
*	graphics/layers.h
*
*******************************************************************************/

void __stdargs __asm scrolllayer(register __a1 struct Layer *l,register __d0 long dx,register __d1 long dy)
{
	/* transfer all raster from l to nl */

	LockLayerRom(l);

	if (l->SuperBitMap)
	{
		SyncSBitMap(l); /* copy stuff from screen to superbitmap */
	}

	l->Scroll_X += dx;
	l->Scroll_Y += dy;

	if (l->SuperBitMap)
	{
		CopySBitMap(l); /* copy stuff from superbitmap to screen */
		gen_sbitmap_cr(l);
	}

	UnlockLayerRom(l);
}
