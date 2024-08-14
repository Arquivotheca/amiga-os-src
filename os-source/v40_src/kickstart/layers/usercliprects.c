/******************************************************************************
*
*	$Id: usercliprects.c,v 38.2 91/08/02 10:23:00 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <graphics/regions.h>

#include "layersbase.h"

void pushusercliprects(struct Layer_Info *li)
{
struct Layer *l;

	for (l = li->top_layer; l ; l = l->back)
	{
		l->saveClipRects = 0;
		if (l->ClipRegion)	/* is there a ClipRegion? */
		{	/* swap em back */
			/* cw->ClipRects has the programmers clip region */
			/* must transfer contents to global ClipRects */
			internal_endupdate(l);
		}
	}
	/* cliprects have all been syncronized now */
}

void popusercliprects(struct Layer_Info *li)
{
struct Layer *l;

	for (l = li->top_layer; l ; l = l->back)
	{
		if (l->ClipRegion)
		{
			/* have to do an effective 'BeginUpdate' */
			internal_beginupdate(l);
		}
	}
}


/****** layers.library/InstallClipRegion ***************************************
*
*    NAME
*	InstallClipRegion -- Install clip region in layer
*
*    SYNOPSIS
*	oldclipregion = InstallClipRegion( l,  region )
*	d0                                 a0  a1
*
*	struct Region *InstallClipRegion( struct Layer *, struct Region *);
*
*    FUNCTION
*	Installs a transparent Clip region in the layer. All
*	subsequent graphics calls will be clipped to this region.
*	You MUST remember to call InstallClipRegion(l,NULL) before
*	calling DeleteLayer(l) or the Intuition function CloseWindow()
* 	if you have installed a non-NULL ClipRegion in l.
*
*    INPUTS
*	l - pointer to a layer
*	region - pointer to a region
*
*    RESULTS
*	oldclipregion - The pointer to the previous ClipRegion that
*	    was installed. Returns NULL if no previous ClipRegion installed.
*
*	    Note: If the system runs out of memory while computing the
*	        resulting ClipRects the LAYERS_CLIPRECTS_LOST bit will
*	        be set in l->Flags.
*
*    BUGS
*	If the system runs out of memory during normal layer operations,
*	the ClipRect list may get swept away and not restored.
*	As soon as there is enough memory and the layer library
*	gets called again the ClipRect list will be rebuilt.
*
*    SEE ALSO
*	BeginUpdate EndUpdate,
*	graphics/layers.h, graphics/clip.h, graphics/regions.h
*
*******************************************************************************/

struct Region * __stdargs __asm installclipregion(register __a0 struct Layer *l,register __a1 struct Region *r)
{
	struct Region *oldregion;
	LockLayerRom(l);

	/* is there a current clipregion being used? */
	if (oldregion = l->ClipRegion)
		internal_endupdate(l);	/* remove it */
	l->ClipRegion = r;	/* install new one */
	if (r)	internal_beginupdate(l);

	UnlockLayerRom(l);
	return(oldregion);
}
