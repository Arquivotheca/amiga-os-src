/******************************************************************************
*
*	$Id: clipdraw.c,v 42.0 93/06/16 11:15:44 chrisg Exp $
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
/*#define SASDEBUG*/
#define CEIL

#ifdef ABS
#undef ABS
static ABS(x)
int x;
{
    if (x<0)    return(-x);
    return(x);
}
#endif

#include "cdrawinfo.h"

void clipdraw(rp,cr,cdi,x0,y0,x1,y1)
struct RastPort *rp;
struct ClipRect *cr;
struct cdraw_info *cdi;
LONG x0,y0,x1,y1;
{
    LONG cx0,cy0,cx1,cy1;
    LONG top,bot;
    LONG xnew,ynew;
    /* draw a possibly truncated vector into this cliprect */
    LONG dot_count;
    LONG Y,X;
    LONG e;

    cx0 = x0;
    cx1 = x1;
    cy0 = y0;
    cy1 = y1;


    if (lineclip(cr,cdi,&cx0,&cy0,&cx1,&cy1))
    {
	struct cdraw_info cdi2;

	/* bart - 09.18.85 double verify clip */

	cx0 = x0;
	cx1 = x1;
	cy0 = y0;
	cy1 = y1;

	cdi2 = *cdi;    /* copy all fields */
	
	cdi2.code1 = cdi->code2; /* swap codes */
	cdi2.code2 = cdi->code1;

	if (lineclip(cr,&cdi2,&cx1,&cy1,&cx0,&cy0)) return;
    }

#ifdef DEBUG
    printf("clipdraw(%lx,%lx,%ld,%ld,%ld,%ld)\n",rp,cr,x0,y0,x1,y1);
    printf("afterclip con1=%ld ! %ld,%ld,%ld,%ld)\n",cdi->con1,cx0,cy0,cx1,cy1);
    
    /*Debug();*/
#endif

    bot = cdi->absdy<<1;
    X = 0;
    if (cdi->code1 != 0)
    {
	/* the line starts out side of ClipRect and proceeds into the */
	/* the ClipRect */
	/* calculate the first dot it would turn on and the associated */
	/* error term */
#ifdef DEBUG
	printf("CODE1 = %lx\n",cdi->code1);
#endif
	if (cdi->xmajor)
	{
	    Y = ABS(cy0-y0);
	    ynew = cy0;
	    X = cx0 - x0;
	}
	else
	{
	    Y = ABS(cx0-x0);
	    xnew = cx0;
	    X = cy0 - y0;
	}

	if (bot)
	switch (cdi->con1)
	{
	    case OCTANT1:
	    case OCTANT8:
		    if (cx0 != cr->bounds.MinX)
		    {   /* enters from bottom */
			top = IMulS(cdi->absdx<<1,Y) - cdi->absdx;
			X =  IDivS_ceiling(top,bot);
		    }
		    break;
	    case OCTANT2:
	    case OCTANT3:
		    if (cy0 != cr->bounds.MinY)
		    {
			top = IMulS(cdi->absdx<<1,Y) - cdi->absdx;
			X = IDivS_ceiling(top,bot);
		    }
		    break;
	    case OCTANT4:
	    case OCTANT5:
		    if (cx0 != cr->bounds.MaxX)  
		    {
			top = IMulS(cdi->absdx<<1,Y) - cdi->absdx;   
			X =  -IDivS_ceiling(top,bot);
		    } 
		    break;
	    case OCTANT6:
	    case OCTANT7:
		    if (cy0 != cr->bounds.MaxY)
		    {
			top = IMulS(cdi->absdx<<1,Y) - cdi->absdx;
			X =  -IDivS_ceiling(top,bot);
		    }   
		    break;
	}

	if (cdi->xmajor) xnew = x0 + X;
	else            ynew = y0 + X;

	/* clip ?new to borders if necessary */
	xnew = MAX(xnew,cr->bounds.MinX);
	xnew = MIN(xnew,cr->bounds.MaxX);
	ynew = MAX(ynew,cr->bounds.MinY);
	ynew = MIN(ynew,cr->bounds.MaxY);

	/* bart - 10.04.85 */

	/* if (cdi->xmajor)    X = ABS(X) - ABS( xnew - (x0+X)); */
	/* else                X = ABS(X) - ABS( ynew - (y0+X)); */

	if (cdi->xmajor)   X = ABS((X) + ( xnew - (x0+X)));
	else               X = ABS((X) + ( ynew - (y0+X)));

	/* end bart - 10.04.85 */

#ifdef DEBUG
	printf("X=%ld y=%ld\n",X,Y);
#endif

	e = IMulS(X+1,bot) - IMulS(1+(Y<<1),cdi->absdx);

    }
    else 
    {
	e = bot - cdi->absdx;
	xnew = x0;
	ynew = y0;
    }
    if (cdi->xmajor)     dot_count = ABS(xnew - cx1);
    else                 dot_count = ABS(ynew - cy1);

    /* check for ending condition */
    if (cdi->code2 != 0)
    {
#ifdef DEBUG
	printf("CODE2 = %lx\n",cdi->code2);
#endif
	/* starting point of this vector lies inside of this ClipRect */
	/* the endpoint, therefore must lie outside this ClipRect */
	/* all we have to do is do a standard line draw, but give it */
	/* a smaller dot count so that it doesn't extend out beyond this */
	/* cliprect */
	/* for now initialize dot count to 0 */
	/* which actually means 1 */
	if (cdi->xmajor)
	    Y = ABS(cy1 - y0);
	else
	    Y = ABS(cx1 - x0);
	if (bot)
	switch (cdi->con1)
	{
	    case OCTANT1:   /* check if intersection hit right or top edge */
	    case OCTANT8:
		    if (cx1 != cr->bounds.MaxX) dot_count = -1;
		    break;
	    case OCTANT2:
	    case OCTANT3:
		    if (cy1 != cr->bounds.MaxY) dot_count = -1;
		    break;
	    case OCTANT4:
	    case OCTANT5:
		    if (cx1 != cr->bounds.MinX) dot_count = -1;
		    break;
	    case OCTANT6:
	    case OCTANT7:
		    if (cy1 != cr->bounds.MinY) dot_count = -1;
		    break;
	}
	if (dot_count == -1)
	{
	    top = IMulS(cdi->absdx,1+(Y<<1));
	    dot_count = IDivS_ceiling(top,bot) - 1 - X;
	    /* have to clip dotcount to edge maybe */
	    {
		int xend,yend;
#ifdef DEBUG
    printf("clip dotcount\n");
#endif
		switch (cdi->con1)
		{
		    case OCTANT1:
		    case OCTANT8:
			xend = xnew + dot_count;
			break;
		    case OCTANT2:
		    case OCTANT3:
			yend = ynew + dot_count;
			break;
		    case OCTANT4:
		    case OCTANT5:
			xend = xnew - dot_count;
			break;
		    case OCTANT6:
		    case OCTANT7:
			yend = ynew - dot_count;
			break;
		}
		if (cdi->xmajor)
		{
		    int newxend;
		    newxend = MIN(xend,cr->bounds.MaxX);
		    newxend = MAX(newxend,cr->bounds.MinX);
		    dot_count -= ABS(newxend - xend);
		}
		else
		{
		    int newyend;
		    newyend = MIN(yend,cr->bounds.MaxY);
		    newyend = MAX(newyend,cr->bounds.MinY);
#ifdef DEBUG
    printf("yend=%ld newyend=%ld cr=%lx\n",yend,newyend,cr);
#endif
		    dot_count -= ABS(newyend - yend);
		}
	    }
	}
    }
    /* now draw the line into the buffer.
     *
	 * this has been reworked for the case of line-patterns.
	 * Previously, it ignored the facts that:
	 * 1) Line segment coordinates were recalculated to (0,0) for
	 * off screen cliprects. This produced erroneous pattern-start values.
	 * 2) We need to take into account the Layer offset in the rastport.
	 * 		- spence Dec  7 1990
	*/
	{
		WORD fudge=0;
		WORD lbmx, lbmy;
		struct BitMap *bm;
		lbmx = rp->Layer->bounds.MinX;
		lbmy = rp->Layer->bounds.MinY;

		if (cr->lobs == 0)
		{
			if (cdi->xmajor)
			{
				fudge = -(lbmx);
			}
			else
			{
				fudge = -(lbmy);
			}
		}
		else	/* draw into off-screen buffer */
		{
			if (~rp->LinePtrn)	/* only do this heavy maths if there is a pattern */
			{
				if (cdi->xmajor)
				{
					fudge = (cr->bounds.MinX & ~15);
					if (xnew < rp->cp_x)
						fudge += (lbmx + ((rp->cp_x - cr->bounds.MaxX)<<1));
            		else
    					fudge -= lbmx;
				}
				else
				{
					fudge = cr->bounds.MinY;
					if (ynew < rp->cp_y)
						fudge += lbmy + ((rp->cp_y - cr->bounds.MaxY)<<1);
					else
						fudge -= lbmy;
				}		/* spence - Dec  5 1990 */
			}
			xnew -= (cr->bounds.MinX&(~15));
			ynew -= cr->bounds.MinY;
			bm = rp->BitMap;
			rp->BitMap = cr->BitMap;
		}
		draw_vect(rp,cdi,xnew,ynew,dot_count,e,fudge);
		if (cr->lobs)
			rp->BitMap = bm;
	}
}

