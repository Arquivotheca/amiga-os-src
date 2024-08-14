/******************************************************************************
*
*	$Id: copylayer.c,v 38.2 91/08/02 10:19:21 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>

#include <hardware/blit.h>

#include "macros.h"

#include "layersbase.h"

/* procedure to copy all raster bits from src Layer to dst Layer */
void copylayer(struct Layer *src,struct Layer *dst)
{
short width,height;

	width = 1 + SHORTMIN(src->bounds.MaxX-src->bounds.MinX,
					dst->bounds.MaxX-dst->bounds.MinX);
	height= 1 + SHORTMIN(src->bounds.MaxY-src->bounds.MinY,
					dst->bounds.MaxY-dst->bounds.MinY);
	LayerBlit(src,src->Scroll_X,src->Scroll_Y,
		dst,0,0,
		width,height,NANBC|NABC|ABNC|ABC);
}
