/******************************************************************************
*
*	$Id: blitplate2.c,v 42.0 93/06/16 11:15:51 chrisg Exp $
*
******************************************************************************/

/* areafill code */
#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include "/macros.h"
#include "c.protos"

/*#define DEBUG*/

clipbltpattern(w,ras,rect,fillbytes)
register struct RastPort *w;
char *ras;
int fillbytes;
struct Rectangle rect;
{
    struct Layer *cw;
    struct ClipRect *cr;
	struct Rectangle trex;
    SHORT   rx,ry;
    UBYTE   done;

    /* blit ras to (xl,yl)  thru ClipRects */
    /* similar to cdraw */

    cw = w->Layer;
	LOCKLAYER(cw);
    cr = cw->ClipRect;

    rx = cw->bounds.MinX - cw->Scroll_X;
    ry = cw->bounds.MinY - cw->Scroll_Y;
	trex = rect;
	trex.MinX += rx;
	trex.MinY += ry;
	trex.MaxX += rx;
	trex.MaxY += ry;

    done = FALSE;
    while (cr)
    {
		if (done = bp_cr(w,ras,cr,trex,
	    	fillbytes,ry,rx))    cr = 0;
		else cr = cr->Next;
    }
    if ( (done == FALSE) && (cw->SuperBitMap != 0))
    {
#ifdef STUFF
	struct Rectangle tmprect;
	tmprect.MinX = xl;
	tmprect.MinY = yl;
	tmprect.MaxX = maxx;
	tmprect.MaxY = maxy;
	if (!rectXrect(&cw->bounds,&tmprect))
#endif
#ifdef DEBUG
	printf("in first part of super bitmap\n");
#endif
	{
	    struct BitMap *bm;
	    bm = w->BitMap;
	    w->BitMap = cw->SuperBitMap;
#ifdef DEBUG
	    printf("call blitplate for super bitmap\n");
#endif
	    oldbltpattern(w,ras,0,PASSRECT(rect),
				fillbytes,0,0,0,0);
#ifdef DEBUG
	    printf("return from super blitplate\n");
#ifdef DEBUGDEBUG
	    Debug();
#endif
#endif
	    w->BitMap = bm;
	}
    }
#ifdef DEBUG
    printf("return from clipblit\n");
#endif
	UNLOCKLAYER(cw);
}

void __regargs fix_rect_cr(struct Rectangle *rect,struct ClipRect *cr)
{
		rect->MinX -= (cr->bounds.MinX&(~15));
		rect->MaxX -= (cr->bounds.MinX&(~15));
		rect->MinY -= cr->bounds.MinY;
		rect->MaxY -= cr->bounds.MinY;
}

bp_cr(w,ras,cr,rect,fillbytes,ry,rx)
struct ClipRect *cr;
struct RastPort *w;
char *ras;
struct Rectangle rect;
int fillbytes;
short ry,rx;
{
#ifdef DEBUG
    printf("bp_cr(%lx,%lx,%lx,%lx)\n",w,ras,cr);
#ifdef DEBUGDEBUG
	Debug();
#endif
#endif
    /*if (ACCEPT(cr,cx,cy,tx,ty))*/
	if (obscured(&cr->bounds,&rect))
    {
	if (cr->lobs == 0)
	{
#ifdef DEBUG
    printf("call blitplate to nil lobs\n");
#endif
		/* this blit goes to the screen */
	    oldbltpattern(w,ras,0,PASSRECT(rect),
				fillbytes,-ry,rx,0,0);
	}
	else
	{
	    /* blit into offscreen buffer */
	    /* this code has not been tested */
	    /* and doesn't always work the last time checked DL */
	    struct BitMap *bm;
#ifdef DEBUG
    printf("call blitplate to off screen buffer\n");
#endif
	    bm = w->BitMap;
	    if (w->BitMap = cr->BitMap)
	    {
		fix_rect_cr(&rect,cr);
		oldbltpattern(w,ras,0,PASSRECT(rect),
					fillbytes,cr->bounds.MinY-ry,
					rx,0,0);
	    }
	    w->BitMap = bm;
	}
	return(TRUE);
    }
    /*else if (!REJECT(cr,cx,cy,tx,ty))*/
    else if (rectXrect(&cr->bounds,&rect))
    {
		/* do a partial blit */
		if ( (cr->lobs == 0) || (cr->BitMap != 0) )
			partialbltpattern(w,ras,
					PASSRECT(rect),
					fillbytes,cr,ry,rx);
    }
    return(FALSE);
}

partialbltpattern(w,ras,rect,fillbytes,cr,ry,rx)
register struct RastPort *w;
char *ras;
int fillbytes;
struct Rectangle rect;
struct ClipRect *cr;
int ry,rx;
{
#ifdef DEBUG
    printf("partialblitplate(%lx:%d,%d,%d,%d)\n",ras,rect);
#endif
    if (ras == 0)
    {
		/* blit without a real a source */
		intersect(&rect,&cr->bounds,&rect);
		if (cr->lobs == 0)
		{
	    	oldbltpattern(w,ras,0,PASSRECT(rect),
							fillbytes,-ry,rx,0,0);
		}
		else
		{
	    	struct BitMap *bm;
	    	/* blit into offscreen buffer */
#ifdef DEBUG
    		printf("call blitplate to off screen buffer\n");
#endif
	    	bm = w->BitMap;
	    	w->BitMap = cr->BitMap;
		fix_rect_cr(&rect,cr);
	    	oldbltpattern(w,ras,0,PASSRECT(rect),
							fillbytes,cr->bounds.MinY-ry,rx,0,0);
	    	w->BitMap = bm;
		}
    }
    else
    {
		short dx,dy;
		int leftright;
		struct ClipRect *tmpclip;
		leftright = ( rect.MinX < cr->bounds.MinX) || (rect.MaxX > cr->bounds.MaxX);
		dx = rect.MinX;
		dy = rect.MinY;
/*#define SQUEEZE*/
#ifdef SQUEEZE
		intersect(&rect,&cr->bounds,&rect);
#else
		rect.MinY = MAX(rect.MinY,cr->bounds.MinY);
		rect.MinX = MAX(rect.MinX,cr->bounds.MinX);
		rect.MaxX = MIN(rect.MaxX,cr->bounds.MaxX);
		rect.MaxY = MIN(rect.MaxY,cr->bounds.MaxY);
#endif
		/*dx = (cx&(~15)) - (dx&(~15));*/
		dx = rect.MinX - dx;
		dy = rect.MinY - dy;

		if (cr->lobs == 0)
		{
			/*cx &= ~15;*/
/*#define GREATDEBUG*/
#ifdef GREATDEBUG
	printf("oldbltpattern(%lx,%lx,%lx,c=%ld,%ld,t=%ld,%ld,f=%ld,r=%ld,%ld,d=%ld,%ld)\n",
						w,ras,cr,cx,cy,tx,ty,fillbytes,-ry,rx,dx,dy);
#endif
			if (leftright)	tmpclip = cr;
			else			tmpclip = 0;
#ifdef DEBUG
			printf("tmpclip=%lx\n",tmpclip);
#endif
	     	oldbltpattern(w,ras,tmpclip,PASSRECT(rect),
							fillbytes,-ry,rx,dx,dy);
		}
		else
		{
	    	struct BitMap *bm;
#ifdef DEBUG
	    	printf("BLIT to offscreen buffer\n");
#endif
			/*return;*/		/* kluge test for now */
	    	bm = w->BitMap;
	    	w->BitMap = cr->BitMap;
		fix_rect_cr(&rect,cr);
	    	oldbltpattern(w,ras,cr,PASSRECT(rect),
			fillbytes,cr->bounds.MinY-ry,rx,dx,dy);
	    	w->BitMap = bm;
		}
    }
}
