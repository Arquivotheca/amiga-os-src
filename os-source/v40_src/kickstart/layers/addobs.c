/******************************************************************************
*
*	$Id: addobs.c,v 38.4 91/10/04 15:48:24 bart Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>

#include "layersbase.h"

void addit(struct ClipRect *op,struct Rectangle *argr,struct Layer *lp)
{
struct ClipRect *newop = AllocCR(lp->LayerInfo);
struct ClipRect *cr;

	newop->bounds = *argr;

	if (op->BitMap)
	{
		get_concealed_rasters(lp,newop);
		copyrect(op,newop);
	}

	newop->lobs = op->lobs;

	cr = lp->ClipRect;
	while (cr->Next)    cr = cr->Next;
	newop->Next = 0;
	cr->Next = newop;
}

BOOL addobs(struct ClipRect *op,struct Rectangle *argr,struct Rectangle *newr,struct Layer *lp)
{
	struct Rectangle r;

    r = *argr;

    if(rectXrect(&r,newr))
    {
		struct Rectangle temp;
		if (r.MinY < newr->MinY)
		{
		    temp = r;
		    r.MinY = temp.MaxY = newr->MinY;
		    temp.MaxY -= 1;
		    addit(op,&temp,lp);
		}
		if (r.MaxY > newr->MaxY)
		{
		    temp = r;
		    r.MaxY =  temp.MinY = newr->MaxY;
			temp.MinY += 1;
		    addit(op,&temp,lp);
		}
		if (r.MinX < newr->MinX)
		{
		    temp = r;
		    r.MinX =  temp.MaxX = newr->MinX;
		    temp.MaxX -= 1;
		    addit(op,&temp,lp);
		}
		if (r.MaxX > newr->MaxX)
		{
		    temp = r;
		    r.MaxX = temp.MinX = newr->MaxX;
		    temp.MinX += 1;
		    addit(op,&temp,lp);
		}

		/* r is now inside rectangle of new layer */

		if ( (*(ULONG *)(&r.MinX) == *(ULONG *)(&argr->MinX)) &&
			 (*(ULONG *)(&r.MaxX) == *(ULONG *)(&argr->MaxX)) )
		{
		    addrect(&r,op->lobs);
		    return(FALSE); /* if this happens no addits were called */
		}

		addrect(&r,op->lobs);
    }

    addit(op,&r,lp);

    return(TRUE);
}
