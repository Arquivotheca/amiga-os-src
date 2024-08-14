/******************************************************************************
*
*	$Id: deletelayer.c,v 38.12 91/11/19 12:54:59 mks Exp $
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
#include <graphics/gfxbase.h>

#include "layersbase.h"

/****** layers.library/DeleteLayer *********************************************
*
*    NAME
*	DeleteLayer -- delete layer from layer list.
*
*    SYNOPSIS
*	result = DeleteLayer( dummy, l )
*	d0                    a0,    a1
*
*	LONG DeleteLayer( LONG, struct Layer *);
*
*    FUNCTION
*	Remove this layer from the list of layers.  Release memory
*	associated with it.  Restore other layers that may have been
*	obscured by it.  Trigger refresh in those that may need it.
*	If this is a superbitmap layer make sure SuperBitMap is current.
*	The SuperBitMap is not removed from the system but is available
*	for program use even though the rest of the layer information has
*	been deallocated.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a layer
*
*    RESULTS
*	result - TRUE if this layer successfully deleted from the system
*	         FALSE if layer not deleted. (probably out of memory )
*
*    BUGS
*
*    SEE ALSO
*	graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

BOOL __stdargs __asm deletelayer(register __a1 struct Layer *l)
{
	struct LayerInfo *li = l->LayerInfo;
	if (fatten_lock(li))
	{
		deletelayer2(l, 0);
		unlock_thin(li);
		return(TRUE);
	}
	return(FALSE);
}

/*
 * New DeleteLayer2 routine that uses the new movelayerinfrontof()
 * routine to do a "delete in place" without all of the code...
 */
void __stdargs __asm deletelayer2(register __a0 struct Layer *l,register __d0 long error)
{
struct Layer_Info *li=l->LayerInfo;
struct ClipRect *cr;

	/*
	 * The layer is going away so make its backfill hook that
	 * of the blank area...
	 */
	l->BackFill=li->BlankHook;

	if (!error) DisposeRegion(l->DamageList);

	if (l->Flags & LAYERSUPER)
	{
		/* free off super save cliprects */
		Freecrlist(l->SuperSaveClipRects);
		SyncSBitMap(l);
		/* now free rest of SuperClipRects */
		Freecrlist(l->SuperClipRect);
		l->SuperBitMap = 0;
		l->SuperClipRect = 0;
	}

	/*
	 * We need to free all of the obscured bitmaps...
	 * This is to make things faster for tomiddle, etc.
	 */
	if (!error) Freecrlist(l->ClipRect);
	l->ClipRect=NULL;

	/*
	 * now convert to LAYERSIMPLE before doing
	 * the SetRast() and the movelayerinfrontof().
	 * This will make sure that our display is
	 */
	l->Flags &= ~(LAYERSMART|LAYERSUPER);
	l->Flags |= LAYERSIMPLE;

	/*
	 * Unobscure all of the layers below me.  (We move to the back
	 * in the very simple operation here...)
	 *
	 * First, we strip the cliprects since they just get in the way
	 * and make things slower (and we needed to do this anyway)
	 */
	strip_onscreen_cliprects(li);
	movelayerinfrontof(l,NULL,FALSE);
	if (!error) Freecrlist(l->ClipRect);
	l->ClipRect=NULL;

	/*
	 * Ok, now clear the layer's bits that are left
	 * and flush these cliprects and restore the old...
	 * We need to save off the old FreeClipRects
	 * in order to not trash the other layers...
	 */
	cr=li->FreeClipRects;
	li->FreeClipRects=NULL;
	genrect(l);
	FullBackFill(l);
	Freecrlist(l->ClipRect);
	Freecrlist(li->FreeClipRects);
	li->FreeClipRects=cr;

	/*
	 * We are now in the back, so remove us from the layer
	 * list and remove our lock from the lock list...
	 */
	unlinklayer(l);
	Remove(&(l->Lock));

	/*
	 * If we are not in ABORT state, lets free all of the
	 * memory this layer was using...
	 */
	if (!error)
	{
		FreeMem(l->rp,sizeof(struct RastPort));
		FreeMem(l,sizeof(struct Layer));

		/*
		 * On screen cliprect are already gone from above.
		 * So, now optimize the cliprects.
		 */
		optimizeClipRects(li);
	}
	else WaitBlit(); /* Need to make sure to wait in error state... */

	/*
	 * generate new onscreen cliprects for our use...
	 */
	gen_onscreen_cliprects(li);
}
