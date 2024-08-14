/******************************************************************************
*
*	$Id: update.c,v 38.5 92/07/01 19:47:06 mks Exp $
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


long internal_beginupdate(struct Layer *cw)
{
struct LayerInfo *li;
struct Region *clipregion;
struct Region *tmpregion = 0;	/* this will be disposed */
long out_of_memory = FALSE;
struct Region *ScreenRegion = NULL;
struct Region *ObscuredRegion = NULL;

	li = cw->LayerInfo;
	cw->_cliprects = cw->ClipRect;	/* store for here for now */
	cw->ClipRect = 0;
	clipregion = cw->ClipRegion;

	/* attempt to avoid creating a new region */
	if (cw->Flags & LAYERUPDATING)
	{
		if (clipregion)
		{
			if ( (!(tmpregion = NewRegion()))
			||   (!OrRegionRegion(cw->DamageList,tmpregion))
			||   (!AndRegionRegion(clipregion,tmpregion)))
			{
				out_of_memory = TRUE;
				goto abortupdate;
			}
			clipregion = tmpregion;
		}
		else
		{
			clipregion = cw->DamageList;
		}
	}

	if (clipregion && clipregion->RegionRectangle)
	{
		/* convert region stuff to cliprect list */
		struct RegionRectangle *rr;
		struct Region *region;
		struct ClipRect *cr;
		int dx,dy;

		if ( (!(ScreenRegion = ClipRectsToRegion(cw,cw->_cliprects,0)))
		||   (!(ObscuredRegion = ClipRectsToRegion(cw,cw->_cliprects,1)))
		||   (!(AndRegionRegion(clipregion,ScreenRegion)))
		||   (!(AndRegionRegion(clipregion,ObscuredRegion))) )
		{
			out_of_memory = TRUE;
			goto abortupdate;
		}
		/* always results in smaller rectangles */

		if (ScreenRegion->RegionRectangle)	/* anything left? */
		{
			region = ScreenRegion;
			dx = cw->bounds.MinX + region->bounds.MinX;
			dy = cw->bounds.MinY + region->bounds.MinY;

			for (rr = region->RegionRectangle; rr ; rr = rr->Next )
			{
				if (!(cr = AllocCR(0)))
				{
					out_of_memory = TRUE;
					goto abortupdate;
				}

				cr->bounds.MinX = rr->bounds.MinX + dx;
				cr->bounds.MinY = rr->bounds.MinY + dy;
				cr->bounds.MaxX = rr->bounds.MaxX + dx;
				cr->bounds.MaxY = rr->bounds.MaxY + dy;

				linkcr(cw,cr);
			}
		}

		if (ObscuredRegion->RegionRectangle)
		{
		struct ClipRect *crt;

			region = ObscuredRegion;
			dx = cw->bounds.MinX + region->bounds.MinX;
			dy = cw->bounds.MinY + region->bounds.MinY;
			cw->LayerInfo = 0;

			for (rr = region->RegionRectangle; rr ; rr = rr->Next )
			{
				if (!(cr = AllocCR(0)))
				{
					out_of_memory = TRUE;
					goto abortupdate;
				}

				cr->bounds.MinX = rr->bounds.MinX + dx;
				cr->bounds.MinY = rr->bounds.MinY + dy;
				cr->bounds.MaxX = rr->bounds.MaxX + dx;
				cr->bounds.MaxY = rr->bounds.MaxY + dy;

				cr->lobs = (struct Layer *)1;	/* make it non zero */

				/* don't worry about clearing for now */
				if (!get_concealed_rasters(cw,cr))
				{
					cw->LayerInfo = li;
					Freecr(cr);
					out_of_memory = TRUE;
					goto abortupdate;
				}

				/* transfer contents of real bitmap to temp bitmap */
				for (crt = cw->_cliprects; crt ; crt = crt->Next)
				{
					if ( (crt->lobs) && (obscured(&crt->bounds,&cr->bounds)))
					{
						/* transfer bits from crt to cr */
						copyrect(crt,cr);
						break;
					}
				}

				linkcr(cw,cr);
			}

			cw->LayerInfo = li;
			out_of_memory = FALSE;
		}
abortupdate:
		if (ScreenRegion)
		{
			DisposeRegion(ScreenRegion);
		}
		if (ObscuredRegion)
		{
			DisposeRegion(ObscuredRegion);
		}
	}

	if (tmpregion)
	{
		DisposeRegion(tmpregion);
	}

	if (out_of_memory)
	{
		cw->Flags |= LAYER_CLIPRECTS_LOST;
	}

	return(!out_of_memory);
}

void internal_endupdate(struct Layer *cw)
{
	/* transfer contents of temporary ClipRects to real ClipRects */
	struct ClipRect *cr,*crp;
	for(cr = cw->ClipRect; cr ; cr = crp)	/* for all sub cliprects */
	{
		crp = cr->Next;
		if (cr->lobs)
		{
			struct ClipRect *crt;
			/* transfer contents into the real obscured cliprect */
			for (crt = cw->_cliprects; crt ; crt = crt->Next)
				if ( (crt->lobs) && (obscured(&crt->bounds,&cr->bounds)) )
				{
					/* transfer bits from cr to crt */
					copyrect(cr,crt);
					break;
				}
		}
	}
	/* Freecrlist() does a WaitBlit if needed */
	Freecrlist(cw->ClipRect);	/* free all at once */
	cw->ClipRect = cw->_cliprects;
	cw->_cliprects = 0;
}

/****** layers.library/BeginUpdate *********************************************
*
*    NAME
*	BeginUpdate -- Prepare to repair damaged layer.
*
*    SYNOPSIS
*	result = BeginUpdate( l )
*	d0                    a0
*
*	LONG BeginUpdate( struct Layer *);
*
*    FUNCTION
*	Convert damage list to ClipRect list and swap in for
*	programmer to redraw through. This routine simulates
*	the ROM library environment. The idea is to only render in the
*	"damaged" areas, saving time over redrawing all of the layer.
*	The layer is locked against changes made by the layer library.
*
*    INPUTS
*	l - pointer to a layer
*
*    RESULTS
*	result - TRUE if damage list converted to ClipRect list successfully.
*	         FALSE if list conversion aborted. (probably out of memory)
*
*    BUGS
*	If BeginUpdate returns FALSE, programmer must abort the attempt to
*	refresh this layer and instead call EndUpdate( l, FALSE ) to restore
*	original ClipRect and damage list.
*
*    SEE ALSO
*	EndUpdate, graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

long __stdargs __asm beginupdate(register __a0 struct Layer *cw)
{
	/* begin update takes the current damage list */
	/* if there is a ClipRegion, it ands it with the */
	/* ClipRegion, the result is the new cliprect list */
long status;

	LockLayerRom(cw);
	if (cw->ClipRegion)
	{
		internal_endupdate(cw);	/* transfer current clipregion */
	}
	cw->Flags |= LAYERUPDATING;
	status = internal_beginupdate(cw);	/* install new cliprects */
	return(status);
}

/****** layers.library/EndUpdate ***********************************************
*
*    NAME
*	EndUpdate -- remove damage list and restore state of layer to normal.
*
*    SYNOPSIS
*	EndUpdate( l, flag )
*	           a0  d0
*
*	void EndUpdate( struct Layer *, UWORD);
*
*    FUNCTION
*	After the programmer has redrawn his picture he calls this
*	routine to restore the ClipRects to point to his standard
*	layer tiling. The layer is then unlocked for access by the
*	layer library.
*
*	Note: use flag = FALSE if you are only making a partial update.
*	    You may use the other region functions (graphics functions such as
*	    OrRectRegion, AndRectRegion, and XorRectRegion ) to clip adjust
*	    the DamageList to reflect a partial update.
*
*    INPUTS
*	l - pointer to a layer
*	flag - use TRUE if update was completed. The damage list is cleared.
*	       use FALSE if update not complete. The damage list is retained.
*
*    EXAMPLE
*
*	-- begin update for first part of two-part refresh --
*	BeginUpdate(my_layer);
*
*	-- do some refresh, but not all --
*	my_partial_refresh_routine(my_layer);
*
*	-- end update, false (not completely done refreshing yet) --
*	EndUpdate(my_layer, FALSE);
*
*	-- begin update for last part of refresh --
*	BeginUpdate(my_layer);
*
*	-- do rest of refresh --
*	my_complete_refresh_routine(my_layer);
*
*	-- end update, true (completely done refreshing now) --
*	EndUpdate(my_layer, TRUE);
*
*    BUGS
*
*    SEE ALSO
*	BeginUpdate, graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

void __stdargs __asm endupdate(register __a0 struct Layer *cw,register __d0 UWORD flag)
{
	if (flag)	ClearRegion(cw->DamageList);
	internal_endupdate(cw);
	cw->Flags &= ~LAYERUPDATING;
	if (cw->ClipRegion)
	{
		/* restore to old ClipRegion */
		internal_beginupdate(cw);
	}
	UnlockLayerRom(cw);
}
