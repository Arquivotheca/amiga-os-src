/******************************************************************************
*
*	$Id: localclipblit.c,v 38.7 92/04/14 16:38:49 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>

#include "macros.h"

#include "layersbase.h"

void LayerBlit(struct Layer *src,SHORT srcx,SHORT srcy, struct Layer *dst,SHORT dstx,SHORT dsty,SHORT xsize,SHORT ysize,USHORT mode)
{
struct ClipRect *CR;
struct Rectangle srcrect;
struct Rectangle src_workrect;
struct BitMap *BMap;

	/* basic source rectangle */
	/* offset by position of layer and scroll value */

	srcrect.MinX = srcrect.MaxX = srcx + src->bounds.MinX - src->Scroll_X;
	srcrect.MinY = srcrect.MaxY = srcy + src->bounds.MinY - src->Scroll_Y;
	srcrect.MaxX += xsize - 1;
	srcrect.MaxY += ysize - 1;

	/* now, loop on all ClipRects */
	CR = src->ClipRect;
	while (CR)
	{
	    int dx,dy;
	    int w,h;
	    if (rectXrect(&srcrect, &CR->bounds))
	    {
			/* break down source and get its bitmap */
			/* calculate intersection of 2 rectangles */
			intersect(&srcrect,&CR->bounds,&src_workrect);
			w = src_workrect.MaxX - src_workrect.MinX + 1;
			h = src_workrect.MaxY - src_workrect.MinY + 1;
			dx = src_workrect.MinX - srcrect.MinX;
			dy = src_workrect.MinY - srcrect.MinY;
	    	/* if this layer is obscured, fetch the alternate BitMap */
	    	if(CR->lobs)
			{
			    BMap = CR->BitMap; /* set zero if this is SIMPLE_REFRESH! */
			    /* adjust regions */
			    /* But first do the damage thing */
			    if (BMap == 0)
			    {
				struct Rectangle damnrect;

				damnrect.MaxX =  damnrect.MinX = dstx + dx;
				damnrect.MaxY =  damnrect.MinY = dsty + dy;
				damnrect.MaxX += (w - 1);
				damnrect.MaxY += (h - 1);

				if(CR->Flags & CR_NEEDS_NO_LAYERBLIT_DAMAGE)
				{
				    CR->Flags &= ~CR_NEEDS_NO_LAYERBLIT_DAMAGE;
				}
				else
				{
					OrRectRegion(dst->DamageList,&damnrect);
					dst->Flags |= LAYERREFRESH|LAYERIREFRESH|LAYERIREFRESH2;
				}

			    }
			    src_workrect.MinX -= (CR->bounds.MinX & ~0xF);
			    src_workrect.MinY -= CR->bounds.MinY;
			}
	    	else BMap = src->rp->BitMap;
	    	/* if it's a valid BitMap, go bliss it */
	    	if (BMap)
			{
				BltBitMapRastPort(  BMap, src_workrect.MinX, src_workrect.MinY,
								    dst->rp, dstx + dx, dsty + dy,
								    w, h, mode);
			}
	    }
	    CR = CR->Next;
	}
}
