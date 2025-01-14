/******************************************************************************
*
*	$Id: bltrastport.c,v 39.8 93/05/07 13:41:56 chrisg Exp $
*
*****************************************************************************/

/*#define DEBUG*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include <graphics/layers.h>
#include <pragmas/layers_pragmas.h>
#include "/gfxpragmas.h"
#include "/macros.h"
#include "c.protos"
#include "/gfxbase.h"

BYTE *AllocRaster();

UWORD * __regargs getTempA(rp)
struct RastPort *rp;
{
    if (rp->TmpRas == 0)    return(FALSE);
    if (rp->TmpRas->Size < MAXBYTESPERROW) return(FALSE);
    return ((UWORD *)rp->TmpRas->RasPtr);
}

intersect(a,b,c)
struct Rectangle *a,*b;
struct Rectangle *c;
{
	c->MinX = MAX(a->MinX,b->MinX);
	c->MinY = MAX(a->MinY,b->MinY);
	c->MaxX = MIN(a->MaxX,b->MaxX);
	c->MaxY = MIN(a->MaxY,b->MaxY);
}

static int __regargs testintersect(struct Rectangle *a,struct Rectangle *b,struct Rectangle *c)
{
	if ( (a->MinX > b->MaxX) ||
		 (b->MinX > a->MaxX) ||
		 (a->MinY > b->MaxY) ||
		 (b->MinY > a->MaxY) )	return(FALSE);	/* no intersection */
	intersect(a,b,c);
	return(TRUE);
}

/****** graphics.library/BltBitMapRastPort *************************************
*
*   NAME
*	BltBitMapRastPort -- Blit from source bitmap to destination rastport.
*
*   SYNOPSIS
*	error = BltBitMapRastPort
*	        (srcbm, srcx, srcy, destrp, destX, destY, sizeX, sizeY, minterm)
*	 D0      A0     D0    D1    A1      D2     D3     D4     D5     D6
*
*	BOOL BltBitMapRastPort
*	     (struct BitMap *, WORD, WORD, struct RastPort *, WORD, WORD,
*	      WORD, WORD, UBYTE);
*
*   FUNCTION
*	Blits from source bitmap to position specified in destination rastport
*	using minterm.
*
*   INPUTS
*	srcbm   - a pointer to the source bitmap
*	srcx    - x offset into source bitmap
*	srcy    - y offset into source bitmap
*	destrp  - a pointer to the destination rastport
*	destX   - x offset into dest rastport
*	destY   - y offset into dest rastport
*	sizeX   - width of blit in pixels
*	sizeY   - height of blit in rows 
*	minterm - minterm to use for this blit
*
*   RESULT
*	TRUE
*
*   BUGS
*
*   SEE ALSO
*	BltMaskBitMapRastPort() graphics/gfx.h graphics/rastport.h
*
******************************************************************************/

bltbitmaprastport(srcbm,srcx, srcy, dst, dstx, dsty, xsize, ysize, minterm)
struct BitMap *srcbm;
SHORT srcx, srcy;
struct RastPort *dst;
SHORT dstx, dsty, xsize, ysize;
USHORT minterm;
{
	UWORD *TempA;
	struct Layer *l;
	UWORD	using_tmpras;

	if ( l = dst->Layer ) LOCKLAYER( l );

	using_tmpras = getmaxbytesperrow(dst,&TempA);

	clipbltrastport(srcbm,srcx,srcy,dst,dstx,dsty,xsize,ysize,minterm,TempA);

	if (!using_tmpras)
	{
		waitblitdone();
		freemustmem(TempA,MAXBYTESPERROW);
	}

	if (l)	UNLOCKLAYER( l );
	return(TRUE);
}

struct BltHook { struct Hook hook; int *args; };


void hookEntry();

void __asm blit_hook(register __a0 struct BltHook *hook, register __a2 struct RastPort *object, register __a1 int *message)
{
	struct BitMap    *srcbm   =  (struct BitMap *)(*(hook->args+0));
    WORD 	      srcx    =  (WORD)		  (*(hook->args+1));
    WORD 	      srcy    =  (WORD)		  (*(hook->args+2));
    UWORD	      minterm =  (UWORD)	  (*(hook->args+8));
    UWORD	     *tempA =  (UWORD *)	  (*(hook->args+9));

    struct Rectangle *rect    =  (struct Rectangle *)(message+1);
    WORD 	      xsize   =  (rect->MaxX - rect->MinX + 1);
    WORD 	      ysize   =  (rect->MaxY - rect->MinY + 1);


	if(message[0])
	{
	    /* adjust source offsets for cliprects */
	    srcx  += *((LONG *)(message+3))-hook->args[4];
	    srcy  += *((LONG *)(message+4))-hook->args[5];
	}
	gfx_BltBitMap(srcbm,srcx,srcy,object->BitMap,rect->MinX,rect->MinY,xsize,ysize,minterm,object->Mask,tempA);
}


clipbltrastport(srcbm,srcx, srcy, dst, dstx, dsty, xsize, ysize, minterm, TempA)
struct BitMap *srcbm;
SHORT srcx, srcy;
struct RastPort *dst;
SHORT dstx, dsty, xsize, ysize;
USHORT minterm;
UWORD *TempA;
{
	struct BltHook bh;
	struct Rectangle rect;

	bh.hook.h_Entry=hookEntry;
	bh.hook.h_SubEntry=blit_hook;
	bh.hook.h_Data=GBASE;
	bh.args=&srcbm;
    rect.MinX = rect.MaxX =  dstx; rect.MaxX += (xsize - 1);
    rect.MinY = rect.MaxY =  dsty; rect.MaxY += (ysize - 1);
	DoHookClipRects(&bh,dst,&rect);
}



bltrastport(src, srcx, srcy, dst, dstx, dsty, xsize, ysize, minterm)
struct RastPort *src,*dst;
SHORT srcx, srcy;
SHORT dstx, dsty, xsize, ysize;
USHORT minterm;
{
    struct Layer *layer;
	UWORD *TempA;
	UWORD	using_tmpras;

    /* can we use TmpRas? */
    if (TempA = getTempA(dst)) using_tmpras = TRUE;
    else
		using_tmpras = getmaxbytesperrow(src,&TempA);

	if (layer = src->Layer)
	{
    	struct ClipRect *CR;
		struct Rectangle srcrect;
		struct Rectangle src_workrect;
		struct BitMap *BMap;
		int dx,dy;
		int w,h;

		/* basic source rectangle */
		srcrect.MinX = srcx;
		srcrect.MinY = srcy;
		srcrect.MaxX = srcx + xsize - 1;
		srcrect.MaxY = srcy + ysize - 1;

		if (src == dst)
		{
			/* are the source and destination overlapping? */
			src_workrect.MinX = dstx;
			src_workrect.MinY = dsty;
			src_workrect.MaxX = dstx + xsize -1;
			src_workrect.MaxY = dsty + ysize -1;
			if (rectXrect(&srcrect,&src_workrect))
			{
#ifdef DEBUG
				printf("calling crsort(%lx,%lx,%lx)\n",layer,srcx-dstx,srcy-dsty);
#endif

					SortLayerCR(layer,srcx-dstx,srcy-dsty);
			}
		}
		/* offset by position of layer and scroll value */
		dx = layer->bounds.MinX - layer->Scroll_X;
		dy = layer->bounds.MinY - layer->Scroll_Y;
		srcrect.MinX += dx;
		srcrect.MaxX += dx;
		srcrect.MinY += dy;
		srcrect.MaxY += dy;

    	/* now, loop on all ClipRects */
    	for (CR = layer->ClipRect; CR ; CR = CR->Next)
		{
			if (testintersect(&srcrect,&CR->bounds,&src_workrect))
			{
				w = src_workrect.MaxX - src_workrect.MinX + 1;
				h = src_workrect.MaxY - src_workrect.MinY + 1;
				dx = src_workrect.MinX - srcrect.MinX;
				dy = src_workrect.MinY - srcrect.MinY;
	    		/* if this layer is obscured, fetch the alternate BitMap */
	    		if (CR->lobs)
				{
					BMap = CR->BitMap; /* set zero if this is SIMPLE_REFRESH! */
					/* adjust regions */
					src_workrect.MinX -= (CR->bounds.MinX & ~0xF);
					src_workrect.MinY -= CR->bounds.MinY;
				}
	    		else BMap = src->BitMap;
	    		/* if it's a valid BitMap, go bliss it */
	    		if (BMap)
					clipbltrastport(BMap, src_workrect.MinX, src_workrect.MinY,
							dst, dstx + dx, dsty + dy, 
							w, h, minterm, TempA);
	    	}
		}
		if (BMap = layer->SuperBitMap)
		{
			srcrect.MinX = srcx;
			srcrect.MinY = srcy;
			srcrect.MaxX = srcx + xsize - 1;
			srcrect.MaxY = srcy + ysize - 1;
			for (CR = layer->SuperClipRect; CR ; CR = CR->Next)
			{
				if (testintersect(&srcrect,&CR->bounds,&src_workrect))
	    		{
					w = src_workrect.MaxX - src_workrect.MinX + 1;
					h = src_workrect.MaxY - src_workrect.MinY + 1;
					dx = src_workrect.MinX - srcrect.MinX;
					dy = src_workrect.MinY - srcrect.MinY;
					clipbltrastport(BMap, src_workrect.MinX, src_workrect.MinY,
								dst, dstx + dx, dsty + dy, 
								w, h, minterm, TempA);
				}
			}
		}
	}
	else	clipbltrastport(src->BitMap,srcx,srcy, dst, dstx, dsty,
						 xsize, ysize,
						minterm, TempA);
    if (!using_tmpras)
	{
		waitblitdone();
		freemustmem(TempA,MAXBYTESPERROW);
	}
	return(TRUE);
}

/****** graphics.library/ClipBlit **********************************************
*
*   NAME
*	ClipBlit  --  Calls BltBitMap() after accounting for windows
*
*   SYNOPSIS
*	ClipBlit(Src, SrcX, SrcY, Dest, DestX, DestY, XSize, YSize, Minterm)
*	         A0   D0    D1    A1    D2     D3     D4     D5     D6
*
*	void ClipBlit
*	     (struct RastPort *, WORD, WORD, struct RastPort *, WORD, WORD,
*	      WORD, WORD, UBYTE);
*
*   FUNCTION
*	Performs the same function as BltBitMap(), except that it
*	takes into account the Layers and ClipRects of the layer library,
*	all of which are (and should be) transparent to you.  So, whereas
*	BltBitMap() requires pointers to BitMaps, ClipBlit requires pointers to
*	the RastPorts that contain the Bitmaps, Layers, etcetera.
*
*	If you are going to blit blocks of data around via the RastPort of your
*	Intuition Window, you must call this routine (rather than BltBitMap()).
*
*	Either the Src RastPort, the Dest RastPort, both, or neither, can have
*	Layers. This routine takes care of all cases.
*
*	See BltBitMap() for a thorough explanation.
*
*   INPUTS
*	Src          = pointer to the RastPort of the source for your blit
*	SrcX, SrcY   = the topleft offset into Src for your data
*	Dest         = pointer to the RastPort to receive the blitted data
*	DestX, DestY = the topleft offset into the destination RastPort
*	XSize        = the width of the blit (must be ta least 1)
*	YSize        = the height of the blit (must be at least 1)
*	Minterm      = the boolean blitter function, where SRCB is associated
*	               with the Src RastPort and SRCC goes to the Dest RastPort
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	BltBitMap();
*
*******************************************************************************/

external_bltrastport(src, srcx, srcy, dst, dstx, dsty, xsize, ysize, minterm)
struct RastPort *src,*dst;
SHORT srcx, srcy;
SHORT dstx, dsty, xsize, ysize;
USHORT minterm;
{
	struct Layer *s,*d;
	int qwe = 0;

	s = src->Layer;
	d = dst->Layer;
	if (  (s) && (d)  && (s != d) )
	{
		if (s->LayerInfo == d->LayerInfo)
		{
			qwe = TRUE;
			LOCKLAYERINFO(s->LayerInfo);
		}
	}
	if ( s ) LOCKLAYER(s);
	if ( d ) LOCKLAYER(d);

	if (qwe)	UNLOCKLAYERINFO(s->LayerInfo);

	/* special kluge for same src/dst and super */
	if (src == dst)
	{
		if ( s )
		{
			if (s->SuperBitMap)
			{
				struct BitMap *bm;
				bm = src->BitMap;
				/* have to kluge it up */
				SyncSBitMap(s);
				src->Layer = 0;		/* cheat */
				src->BitMap = s->SuperBitMap;
	qwe = bltrastport(src,srcx, srcy, dst, dstx, dsty, xsize, ysize, minterm);
				src->Layer = s;
				src->BitMap = bm;
				CopySBitMap(s);
				goto lcl_exit;
			}
		}
	}
	qwe = bltrastport(src,srcx, srcy, dst, dstx, dsty, xsize, ysize, minterm);
lcl_exit: ;
	if ( s ) UNLOCKLAYER(s);
	if ( d ) UNLOCKLAYER(d);
	return (qwe);
}
