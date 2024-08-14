/******************************************************************************
*
*	$Id: newgenblit.c,v 42.0 93/06/16 11:16:09 chrisg Exp $
*
******************************************************************************/

/*???#define DEBUG 1*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <graphics/clip.h>
#include <graphics/gels.h>
#include <hardware/blit.h>
#include "/gels/gelsinternal.h"
#include "/macros.h"
#include "c.protos"

/* = declarations ======================================================== */

/*******************************************************************************
*
*NAME
*   newGenBlit  --  generates one plane of a floodfill bltmask
*                   after accounting for Windows
*
*SYNOPSIS
*   newGenBlit(Src, SrcX, SrcY, Dest, DestX, DestY, XSize, YSize, Mode,Planecount);
*
*FUNCTION
*   Performs the same function as genblit() , except that it
*   takes into account the Layers and ClipRects of the layer library,
*   and only deals with one bitplane at a time.
*
*   If you are going to generate a bltmask for the RastPort of your
*   Intuition Window, you can call this routine to account for both
*   onscreen and obsured cliprects.
*
*INPUTS
*   Src = pointer to the RastPort of the source for your blit
*   SrcX, SrcY = the topleft offset into Src for your data
*   Dest = pointer to the RastPort to receive the blitted data
*   DestX, DestY = the topleft offset into the destination RastPort
*   XSize = the width of the blit
*   YSize = the height of the blit
*   Mode = the boolean blitter function, where SRCB is associated with the
*          Src RastPort and SRCC goes to the Dest RastPort
*   Planecount = which plane of the source bitmap we are currently
*          looking at ( necessary for obscured cliprects)
*
*RESULT
*   None
*
*BUGS
*   None
*
*SEE ALSO
*   None
*****************************************************************************/
newGenBlit(src, srcx, srcy, dest, destx, desty, xsize, ysize, mode,planecount)
struct RastPort *src;
SHORT srcx, srcy;
struct RastPort *dest;
SHORT destx, desty, xsize, ysize;
USHORT mode;
SHORT planecount;
{
    struct Layer *layer;
    struct ClipRect *CR;
    struct gelRect srcRect, destRect, worksrc, workdest;
    struct Rectangle blissRect;
    struct BitMap CopyObsBMap;
    struct BitMap *SaveBMap, *BMap;
    SHORT layerOffX, layerOffY;
    SHORT destxoff, destyoff;
	USHORT *TempA;

#ifdef DEBUG
printf("\nnewGenBlit(src=%lx srcx=%ld srcy=%ld dest=%lx destx=%ld desty=%ld xsize=%ld ysize=%ld mode=%lx)\n",
		src, srcx, srcy, dest, destx, desty, xsize, ysize, mode);
#endif

    /* use floodrast->BitMap->Planes[1] as TmpRas */

    TempA = dest->BitMap->Planes[1];

    if ((layer = src->Layer) == 0)
	{
	/* there's no Layer, this is a simple blit from a RastPort */
	clipbltrastport(src->BitMap, srcx, srcy, dest, destx, desty, xsize, ysize, mode, TempA);
	return;
	}
/*???printf("CB:");*/
    /* else do the ClipRectangling Dance through the source Layer */
	/* but first make sure no one changes the cliprects while I'm */
	/* dancing through the list of ClipRects */
	LOCKLAYER(layer);

    /* the offsets are for the true offsets into full-Screen BitMaps.  the
     * Scroll factors are added for SUPER_BITMAPping
     */
    layerOffX = layer->bounds.MinX - layer->Scroll_X;
    layerOffY = layer->bounds.MinY - layer->Scroll_Y;

    /* if there's offsets to be had for the destination, get them */
    if (dest->Layer)
	{
	destxoff = dest->Layer->bounds.MinX - dest->Layer->Scroll_X;
	destyoff = dest->Layer->bounds.MinY - dest->Layer->Scroll_Y;
	}
    else
	{
	destxoff = 0;
	destyoff = 0;
	}

    srcRect.rW = xsize;	/* width and height start out as requested */
    srcRect.rH = ysize;
    destRect.rW = xsize;	/* width and height start out as requested */
    destRect.rH = ysize;
    srcRect.rX = srcx + layerOffX;  /* src x and y are set to true offsets */
    srcRect.rY = srcy + layerOffY;
    destRect.rX = destx + destxoff;	/* dest x and y are as requested */
    destRect.rY = desty + destyoff;

    /* set the blissRect to describe the entire Rectangle of the src, as
     * already offset into the BitMap
     */
    blissRect.MinX = srcRect.rX;
    blissRect.MinY = srcRect.rY;
    blissRect.MaxX = srcRect.rX + xsize - 1;
    blissRect.MaxY = srcRect.rY + ysize - 1;

    SaveBMap = src->BitMap; /* save this to be restored later */

    /* now, loop on all ClipRects */
    CR = layer->ClipRect;
    while (CR)
	{
	/* look into calling blisser for each ClipRect */
	if (rectXrect(&blissRect, &CR->bounds))
	    {
	    /* ok, these two intersect, so reset the gelRects and bliss it */
	    worksrc = srcRect;
	    workdest = destRect;
	    resetRects(&workdest, &worksrc, CR);
/*???printf("   src:");reportRect(&worksrc);*/
/*???printf("   dest:");reportRect(&workdest);*/
	    /* if this layer is obscured, fetch the alternate BitMap */
	    if (CR->lobs)
	    {
		if(BMap = CR->BitMap) /* set zero if this is SIMPLE_REFRESH! */
		{
		    /* fool routine into blitting from the proper single plane */
		    CopyObsBMap = *(CR->BitMap);	/*copy all fields */
		    CopyObsBMap.Depth = 1;
		    CopyObsBMap.Planes[0] = CR->BitMap->Planes[planecount];
		    BMap = &CopyObsBMap;
		}
		/* adjust dest gelRect x,y coordinates for special BitMap */
		worksrc.rX -= (CR->bounds.MinX & ~0xF);
/*???		worksrc.rX = CR->bounds.MinX & 0xF;*/
		worksrc.rY -= CR->bounds.MinY;
/*???		worksrc.rY = 0;*/
/*???printf("  LOBS:src:");reportRect(&worksrc);*/
	    }
	    else BMap = SaveBMap;
	    /* if it's a valid BitMap, go bliss it */
	    if (src->BitMap = BMap)
		clipbltrastport(src->BitMap, worksrc.rX, worksrc.rY,
			dest, workdest.rX, workdest.rY, 
			workdest.rW, workdest.rH, mode, TempA);
	    }
	CR = CR->Next;
	}

    src->BitMap = SaveBMap;
    UNLOCKLAYER(layer);
    return(TRUE);
}


