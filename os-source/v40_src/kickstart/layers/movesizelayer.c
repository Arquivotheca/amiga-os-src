
/******************************************************************************
*
*	$Id: movesizelayer.c,v 38.6 92/07/01 19:47:20 mks Exp $
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

#include "macros.h"

#include "layersbase.h"

void swizzlelayer(struct Layer *tl)
{
	struct Layer *l = tl->back;
	struct ClipRect *t;

	/* first swap front/back pointers */
	if (tl->LayerInfo->top_layer == tl)	tl->LayerInfo->top_layer = l;
	else		tl->front->back = l;
	if (l->back)	l->back->front = tl;
	tl->back = l->back;
	l->front = tl->front;
	tl->front = l;
	l->back = tl;

	/* now swap cliprect pointers */
	t = tl->ClipRect;
	tl->ClipRect = l->ClipRect;
	l->ClipRect = t;

	/* must fix lobs of all back layers */
	{
		struct Layer *p;
		for ( p=tl; p ; p = p->back)
		{
			for ( t = p->ClipRect; t ; t = t->Next)
				if (t->lobs == l)	t->lobs = tl;
				else if (t->lobs == tl)	t->lobs = l;
		}
	}

	/* swap bounds */
	{
		struct Rectangle t;
		t = l->bounds;
		l->bounds = tl->bounds;
		tl->bounds = t;
	}

	/* swap width and height */
	{
		USHORT t;
		t = l->Width;
		l->Width = tl->Width;
		tl->Width = t;
		t = l->Height;
		l->Height = tl->Height;
		tl->Height = t;
	}

	/* do not touch Lock stuff, roms are using it too */

	/* merge damagelists together */
	OrRegionRegion(tl->DamageList,l->DamageList);
	l->Flags |= tl->Flags & (LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2);
}

/****** layers.library/MoveSizeLayer ***************************************
*
*    NAME                                                              (V36)
*	MoveSizeLayer -- Position/Size layer
*
*    SYNOPSIS
*	result = MoveSizeLayer( layer, dx, dy, dw, dh )
*	d0                      a0     d0  d1  d2  d3
*
*	LONG MoveSizeLayer( struct Layer *, LONG, LONG, LONG, LONG);
*
*    FUNCTION
*	Change upperleft and lower right position of Layer.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a nonbackdrop layer
*	dx,dy - change upper left corner by (dx,dy)
*	dw,dy - change size by (dw,dh)
*
*    RETURNS
*	result - TRUE if operation successful
*	         FALSE if failed (due to out of memory)
*	         FALSE if failed (due to illegal layer->bounds)
*
*    BUGS
*
*    SEE ALSO
*	graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

long __stdargs __asm movesizelayer(register __a0 struct Layer *l,register __a1 struct Rectangle *r)
{
struct Layer_Info *wi=l->LayerInfo;
struct Layer *tmplayer;
ULONG flags;
long status = FALSE;
short move_change;
short size_change;
short dx;
short dy;

	dx = r->MaxX;
	dy = r->MaxY;

	move_change = r->MinX || r->MinY;
	size_change = dx || dy;

	if ( (move_change | size_change) == 0)	return TRUE;

	/* convert delta to absolute */
	r->MaxX += l->bounds.MaxX + r->MinX;
	r->MaxY += l->bounds.MaxY + r->MinY;

	r->MinX += l->bounds.MinX;
	r->MinY += l->bounds.MinY;

	/* dont turn inside out */
	if(( r->MaxX < r->MinX ) || ( r->MaxY < r->MinY )) return(FALSE);

	if (!fatten_lock(wi))	return(FALSE);
	flags = LAYERBACKDROP | (l->Flags&(LAYERSMART|LAYERSIMPLE));

	if (tmplayer = (struct Layer *)createbehindlayer(wi,l->rp->BitMap,*r,flags,NULL,(struct Hook *)1))
	{
	    if (!(l->Flags & LAYERBACKDROP)) tmplayer->Flags &= ~LAYERBACKDROP;

	    /* put new layer infront of old layer */
		if (movelayerinfrontof(tmplayer,l,TRUE))
		{
			regen_display(wi);

			tmplayer->BackFill = l->BackFill;

			/* bart's optimisations for the most common case */

			if(l->Flags & LAYERSIMPLE)
			{
			    /* start damage with a clean slate */
			    ClearRegion(tmplayer->DamageList);

			    /* preserve as many bits as possible */
			    if ((move_change) && (rectXrect(&(l->bounds),&(tmplayer->bounds))))
			    {
				WORD sx = l->bounds.MinX-l->Scroll_X-tmplayer->bounds.MinX;
			        WORD sy = l->bounds.MinY-l->Scroll_Y-tmplayer->bounds.MinY;

				WORD xmax =
				 SHORTMIN(((sx > 0) ? sx : 0)+l->bounds.MaxX-l->bounds.MinX,
				 		tmplayer->bounds.MaxX-tmplayer->bounds.MinX);

				WORD ymax =
				 SHORTMIN(((sy > 0) ? sy : 0)+l->bounds.MaxY-l->bounds.MinY,
				 		tmplayer->bounds.MaxY-tmplayer->bounds.MinY);

				struct Region *damage = tmplayer->DamageList;

				tmplayer->DamageList = NULL;
				ScrollRaster(tmplayer->rp,sx,sy,0,0,xmax,ymax);
				tmplayer->DamageList = damage;
			   }
		        }

			if (l->Flags & LAYERSUPER)
			{
				if (size_change)	SyncSBitMap(l);
				else	copylayer(l,tmplayer);
			}
			else
			{
				tmplayer->Flags &= ~(LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2);
				copylayer(l,tmplayer);
			}


			/* make certain that newly revealed bits */
			/* are filled with correct backfill pattern */

			if( l->Flags & (LAYERSIMPLE|LAYERSMART) )
				{
				if (size_change)
					{
					struct Rectangle damage;

					damage.MaxX = tmplayer->Width - 1;
					damage.MaxY = tmplayer->Height - 1;

					if (dx > 0)
						{
						damage.MinY = 0;
						damage.MinX = damage.MaxX - dx + 1;
						OrRectRegion(tmplayer->DamageList,&damage);
						tmplayer->Flags |= (LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2);
						}

					if (dy > 0)
						{
						damage.MinX = 0;
						damage.MinY = damage.MaxY - dy + 1;
						OrRectRegion(tmplayer->DamageList,&damage);
						tmplayer->Flags |= (LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2);
						}

					if ( (dx < 0) || (dy < 0) )
						{
						/* undo any damage that may go away now */
						damage.MinX = 0;
						damage.MinY = 0;
						/* at most the damage should be */
						/* constrained to the actual layer size */
						AndRectRegion(tmplayer->DamageList,&damage);
						}
					}
				}


			/* backfill damage from scroll, size, and copy  */

			tmplayer->Flags |= LAYERUPDATING;
			if(internal_beginupdate(tmplayer))
			{
			    FullBackFill(tmplayer);
			}
			internal_endupdate(tmplayer);
			tmplayer->Flags &= ~LAYERUPDATING;

			swizzlelayer(tmplayer);
			deletelayer2(tmplayer, 0);
			if (l->Flags & LAYERSUPER)
			{
				if (size_change)
				{
					CopySBitMap(l);
					gen_sbitmap_cr(l);
				}
			}
			/* we want to be able to delete tmplayer */
			status = TRUE;
		}
		else	deletelayer2(tmplayer, 0);
	}
    unlock_thin(wi);
	return (status);
}

/****** layers.library/SizeLayer ***********************************************
*
*    NAME
*	SizeLayer -- Change the size of this nonbackdrop layer.
*
*    SYNOPSIS
*	result = SizeLayer( dummy, l, dx, dy )
*	d0                  a0     a1 d0  d1
*
*	LONG SizeLayer( LONG, struct Layer *, LONG, LONG);
*
*    FUNCTION
*	Change the size of this layer by (dx,dy). The lower right hand
*	corner is extended to make room for the larger layer.
*	If there is SuperBitMap for this layer then copy pixels into
*	or out of the layer depending on whether the layer increases or
*	decreases in size.  Collect damage list for those layers that may
*	need to be refreshed if damage occurred.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a nonbackdrop layer
*	dx - delta to add to current x size
*	dy - delta to add to current y size
*
*    RESULTS
*	result - TRUE if operation successful
*	         FALSE if failed (out of memory)
*
*    BUGS
*
*    SEE ALSO
*	graphics/layers.h, graphics/clip.h
*
*******************************************************************************/

/****** layers.library/MoveLayer ***********************************************
*
*    NAME
*	MoveLayer -- Move layer to new position in BitMap.
*
*    SYNOPSIS
*	result = MoveLayer( dummy, l, dx, dy )
*	d0                  a0     a1 d0  d1
*
*	LONG MoveLayer( LONG, struct Layer *, LONG, LONG);
*
*    FUNCTION
*	Move this layer to new position in shared BitMap.
*	If any refresh layers become revealed, collect damage and
*	set REFRESH bit in layer Flags.
*
*    INPUTS
*	dummy - unused
*	l - pointer to a nonbackdrop layer
*	dx - delta to add to current x position
*	dy - delta to add to current y position
*
*    RETURNS
*	result - TRUE if operation successful
*	         FALSE if failed (out of memory)
*
*    BUGS
*	May not handle (dx,dy) which attempts to move the layer outside the
*	layer's RastPort->BitMap bounds .
*
*    SEE ALSO
*	graphics/layers.h, graphics/clip.h
*
*******************************************************************************/
