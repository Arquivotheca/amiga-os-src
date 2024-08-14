/******************************************************************************
*
*	$Id: copyrect.c,v 38.4 92/01/10 18:21:02 mks Exp $
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
#include <hardware/blit.h>

#include "layersbase.h"

void __stdargs __asm copyrect(register __a0 struct ClipRect *src,register __a1 struct ClipRect *dst)
{
	struct BitMap *sbm,*dbm;
	sbm = src->BitMap;
	dbm = dst->BitMap;
	if ((sbm != 0) && (dbm != 0)) if (rectXrect(&src->bounds,&dst->bounds))
	{
	struct Rectangle r;
	short srcX,dstX;

		intersect(&src->bounds,&dst->bounds,&r);
		srcX = r.MinX - (src->bounds.MinX & 0xfff0);
		dstX = r.MinX - (dst->bounds.MinX & 0xfff0);
		BltBitMap(sbm,	srcX,
				r.MinY-src->bounds.MinY,
				dbm,
				dstX,
				r.MinY-dst->bounds.MinY,
				r.MaxX-r.MinX+1,
				r.MaxY-r.MinY+1,
				NANBC|NABC|ABNC|ABC,
				-1,
				NULL);
	}
}
