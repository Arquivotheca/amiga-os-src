/******************************************************************************
*
*	$Id: moreregions.c,v 38.4 91/10/04 15:50:34 bart Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>

#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/regions.h>

#include "layersbase.h"

struct Region *ClipRectsToRegion(struct Layer *l,struct ClipRect *crf,long type)
{
struct ClipRect *cr;
struct Region *nrgn;
struct Rectangle rect;

	if (nrgn = NewRegion())
		for (cr = crf; cr ; cr = cr->Next)
			if( (type) ? (cr->lobs && cr->BitMap) : (!cr->lobs) )
			{
				rect.MinX = cr->bounds.MinX - l->bounds.MinX;
				rect.MaxX = cr->bounds.MaxX - l->bounds.MinX;
				rect.MinY = cr->bounds.MinY - l->bounds.MinY;
				rect.MaxY = cr->bounds.MaxY - l->bounds.MinY;

				OrRectRegion(nrgn,&rect);
			}
	return(nrgn);
}
