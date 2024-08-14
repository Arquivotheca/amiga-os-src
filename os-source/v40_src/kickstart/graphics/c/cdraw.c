/******************************************************************************
*
*	$Id: cdraw.c,v 39.1 92/10/06 13:50:26 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include "/macros.h"
#include "c.protos"

/*#define DEBUG*/
#include "cdrawinfo.h"

#define DOWNCODE

void cdraw(rp,cp,to,cdi)
struct RastPort *rp;
Point to;
Point cp;
struct cdraw_info   *cdi;
{
    struct Layer *cw;
    struct ClipRect *cr;
	struct BitMap *bm;

    /* check for trivial accept/reject */
#ifdef DEBUG
    printf("CDRAW(%d,%d,%lx,rp=%lx)",to,cdi,rp);
#ifdef DEBUGDEBUG
	Debug();*/
#endif
#endif
    cw = rp->Layer;
	cr = cw->SuperClipRect;
	bm = rp->BitMap;
	rp->BitMap = cw->SuperBitMap;
	while (cr)
	{
#ifdef DEBUG
	    printf("call cr_draw cr=%lx with super bitmap\n",cr);
#ifdef DEBUGDEBUG
		Debug();
#endif
#endif
	    if (cr_cdraw(rp,cr,cdi,cp,to)) break;
	    else cr = cr->Next;
	}
	rp->BitMap = bm;
    /*rp->Layer = cw;*/
#ifdef DEBUG
	printf("exit cdraw\n");
#endif
}

#ifndef DOWNCODE
c_cr_cdraw(rp,cr,cdi,old,t)
struct RastPort *rp;
struct ClipRect *cr;
Point old,t;
struct cdraw_info *cdi;
{
#ifdef DEBUG
	printf("checking cr = %lx\n",cr);
#ifdef DEBUGDEBUG
	Debug();
#endif
#endif
	cdi->code1 = getcode(cr,old.x,old.y);
	cdi->code2 = getcode(cr,t.x,t.y);
	if ((cdi->code1 | cdi->code2) == 0) /* trivial accept */
	{
	    /* trivial accept this vector for this cliprect */
	    /* under standard assumptions, this vector will not appear */
	    /* in any other ClipRect, we can return immediately */
#ifdef DEBUG
	    printf("accept and draw (%d,%d)->(%d,%d)\n",old,t);
#ifdef DEBUGDEBUG
		Debug();
#endif
#endif
	    /* can assume that no other draw has occured for this vector */
	    if ( cr->lobs == 0)
	    {   /* directly on screen */
		rp->cp_x = old.x;
		rp->cp_y = old.y;
#ifdef DEBUG
	printf("call baredraw(%lx,%d,%d)\n",rp,t);
#endif
		baredraw(rp,t.x,t.y);
	    }
	    else
	    {
		if (cr->BitMap)
		{
		    int mx;
		    struct BitMap *bm;
		    /* swap in new plane descriptor */
		    bm = rp->BitMap;    /* saveoriginal */
		    rp->BitMap = cr->BitMap;
		    mx = cr->bounds.MinX & (~15);
		    rp->cp_x = old.x-mx;
		    rp->cp_y = old.y-cr->bounds.MinY;
		    baredraw(rp,t.x-mx,t.y-cr->bounds.MinY);
		    rp->BitMap = bm;
		}
	    }
#ifdef DEBUG
	printf("returnTRUE\n");
#endif
	    return(TRUE);
	}
	else
	if ( (cdi->code1 & cdi->code2) == 0)
	{
	    if ( (cr->lobs == 0) || (cr->BitMap != 0) )
		{
#ifdef DEBUG
	printf("call clipdraw(%d,%d,%d,%d)\n",old,t);
#endif
		clipdraw(rp,cr,cdi,old.x,old.y,t.x,t.y);
		}
	}
#ifdef DEBUG
	printf("returnFALSE\n");
#endif
	return(FALSE);
}

#endif
