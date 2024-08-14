/******************************************************************************
*
*	$Id: bltbitmask.c,v 42.0 93/06/16 11:16:15 chrisg Exp $
*
*****************************************************************************/

/****** graphics.library/BltMaskBitMapRastPort *********************************
*
*   NAME
*	BltMaskBitMapRastPort -- blit from source bitmap to destination rastport
*	with masking of source image.
*
*   SYNOPSIS
*	BltMaskBitMapRastPort
*	    (srcbm, srcx, srcy, destrp, destX, destY, sizeX, sizeY,
*	     A0     D0    D1    A1      D2     D3     D4     D5
*	     minterm, bltmask)
*	     D6       A2
*
*	void BltMaskBitMapRastPort
*	     (struct BitMap *, WORD, WORD, struct RastPort *, WORD, WORD,
*	      WORD, WORD, UBYTE, APTR);
*
*   FUNCTION
*	Blits from source bitmap to position specified in destination rastport
*	using bltmask to determine where source overlays destination, and
*	minterm to determine whether to copy the source image "as is" or
*	to "invert" the sense of the source image when copying. In either
*	case, blit only occurs where the mask is non-zero.
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
*	minterm - either (ABC|ABNC|ANBC) if copy source and blit thru mask
*	          or     (ANBC)          if invert source and blit thru mask
*	bltmask - pointer to the single bit-plane mask, which must be the
*	          same size and dimensions as the planes of the
*	          source bitmap.
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	BltBitMapRastPort() graphics/gfx.h graphics/rastport.h
*
******************************************************************************/


#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <utility/hooks.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include <pragmas/layers_pragmas.h>
#include "/gfxpragmas.h"

#include "/macros.h"
#include "c.protos"

struct MaskHook
{
    struct Hook hook;
    int	       *args;
};

void __asm mask_hook(register __a0 struct MaskHook *hook,register __a2 struct RastPort *object,register __a1 int *message)
{
    struct BitMap    *srcbm   =  (struct BitMap *)(*(hook->args+0));
    WORD 	      srcx    =  (WORD)		  (*(hook->args+1));
    WORD 	      srcy    =  (WORD)		  (*(hook->args+2));
    WORD 	      oldx    =  (WORD)		  (*(hook->args+4));
    WORD 	      oldy    =  (WORD)		  (*(hook->args+5));
    UWORD	      minterm =  (UWORD)	  (*(hook->args+8));
    UWORD	     *bltmask =  (UWORD *)	  (*(hook->args+9));

    struct Layer     *layer   = *(struct Layer **)   (message+0);
    struct Rectangle *rect    =  (struct Rectangle *)(message+1);
    WORD	      xoff    = *(LONG *)	     (message+3);
    WORD	      yoff    = *(LONG *)	     (message+4);
    struct RastPort  *dst     =  (object);
    WORD 	      dstx    =  (rect->MinX);
    WORD 	      dsty    =  (rect->MinY);
    WORD 	      xsize   =  (rect->MaxX - rect->MinX + 1);
    WORD 	      ysize   =  (rect->MaxY - rect->MinY + 1);


    /* call old routine for now to perform actual work */

    if (bltmask)
    {
	int i;
	UWORD *TempA;
	struct BitMap tempbm,*tbm;
	UWORD   using_tmpras;

        if(layer)
	{
	    /* adjust source offsets for cliprects */

	    srcx  += (xoff - oldx);
	    srcy  += (yoff - oldy);
	}
	using_tmpras = getmaxbytesperrow(dst,&TempA);
	tempbm = *srcbm;
	tempbm.pad=0;	/* defeat interleaved source */
	tbm = &tempbm;

	for (i=0; i<srcbm->Depth; i++)
	{
	    tbm->Planes[i] = bltmask;	
	}

	gfx_BltBitMap(srcbm,srcx,srcy,dst->BitMap,dstx,dsty,xsize,ysize,(ABC|ANBNC),dst->Mask,TempA);

	gfx_BltBitMap(tbm,srcx,srcy,dst->BitMap,dstx,dsty,xsize,ysize,minterm,dst->Mask,TempA);

	gfx_BltBitMap(srcbm,srcx,srcy,dst->BitMap,dstx,dsty,xsize,ysize,(ABC|ANBNC),dst->Mask,TempA);

	if (!using_tmpras)
	{
	    waitblitdone();
	    freemustmem(TempA,MAXBYTESPERROW);
	}
    
    }

}

extern int hookEntry();

bltmaskrastport(srcbm,srcx,srcy,dst,dstx,dsty,xsize,ysize,minterm,bltmask)
struct BitMap *srcbm;
WORD srcx, srcy;
struct RastPort *dst;
WORD dstx, dsty, xsize, ysize;
UWORD minterm;
UWORD *bltmask;
{
    struct MaskHook  mh;
    struct Rectangle rect;

    /* initialize hook */

    mh.hook.h_Entry 	= hookEntry;
    mh.hook.h_SubEntry   = mask_hook;
    mh.hook.h_Data	= GBASE;
    mh.args 		= &srcbm;

    /* initialize rect */

    rect.MinX = rect.MaxX =  dstx; rect.MaxX += (xsize - 1);
    rect.MinY = rect.MaxY =  dsty; rect.MaxY += (ysize - 1);

    /* intersect rect with cliprects and call hook */

	DoHookClipRects(&mh,dst,&rect);

}

