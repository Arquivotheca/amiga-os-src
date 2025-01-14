/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: gel3.c,v 42.0 93/06/16 11:19:09 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	gel3.c,v $
 * Revision 42.0  93/06/16  11:19:09  chrisg
 * initial
 * 
 * Revision 42.0  1993/06/01  07:20:43  chrisg
 * *** empty log message ***
 *
*   Revision 39.2  92/05/05  12:21:23  chrisg
*   FETCHGBASE, regargs, etc.
*   
*   Revision 39.1  92/02/21  19:50:50  bart
*   process interleaved bitmaps for V39
*   
*   Revision 39.0  91/08/21  17:34:30  chrisg
*   Bumped
*   
*   Revision 37.3  91/05/13  14:37:21  chrisg
*   removed compiler warnings for lattice
*   made use prototypes
*   got rid of interface code.. made routines take
*   arguments in registers.
*   
*   Revision 37.2  91/05/02  16:54:45  chrisg
*   killed ".." for lattice
*   
*   Revision 37.1  91/04/19  14:10:19  chrisg
*     Removed blitter completion routine downcoded in asgel3.
*   
*   Revision 37.0  91/01/07  15:22:28  spence
*   initial switchover from V36
*   
*   Revision 33.8  90/12/06  13:50:11  bart
*   removed gfxallocmem.c and implemented better lo-mem error recovery -- bart
*   
*   Revision 33.7  90/07/27  16:38:28  bart
*   id
*   
*   Revision 33.6  90/03/28  09:26:56  bart
*   *** empty log message ***
*   
*   Revision 33.5  88/11/16  13:09:33  bart
*   *** empty log message ***
*   
*   Revision 33.4  88/11/16  09:48:20  bart
*   *** empty log message ***
*   
*   Revision 33.3  88/11/16  09:12:21  bart
*   big blits
*   
*   Revision 33.2  88/08/21  15:46:07  bart
*   resetRects -- process bobs which are temporarily beyond the pale... bart
*   
*   Revision 33.1  88/06/20  18:07:48  bart
*   use ../macros.h
*   
*   Revision 33.0  86/05/17  15:23:51  bart
*   added to rcs for updating
*   
*
******************************************************************************/

/*** gel3.c ******************************************************************
 *
 *  File
 *
 *  Confidential Information: Amiga Computer, Inc.
 *  Copyright (c) Amiga Computer, Inc.
 *
 *                  Modification History
 *  date    :   author :    Comments
 *  -------     ------      ---------------------------------------
 *  9-28-84     -=RJ=-      added this header file
 *                          commented out all EXEC14 stuff
 *
 ****************************************************************************/


#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/gels.h>
#include "gelsinternal.h"
#include "/macros.h"
#include <hardware/custom.h>
#include "gels.protos"

#define BLITTERSIGNAL	4

extern struct Custom custom;

/*???#define DEBUG 1*/

/* #define OLDBLISSLICER */

#define blitsize(h,w) ((h << HSIZEBITS) | (w >> 4))
extern qBlissIt(),BlsDn();

#define rastoff(r) ((umuls(r->rY , r->rRealWW)) + (r->rX >> 4))

/* #define rectXrect_macro(a,b) ( ((b)->MaxX < (a)->MinX) && ((b)->MinX > (a)->MaxX) && ((b)->MaxY < (a)->MinY) && ((b)->MinY > (a)->MaxY) )  */

/* ************************************************************************ */
#define SQUEEZE
#ifndef SQUEEZE

QBlissIt(bo, io)
register struct blissObj *bo;
register struct AmigaRegs *io;
{
    register WORD index, type;
    BYTE *cd;

#ifdef DEBUG
if (bo->deBug & GEL3BUG) printf("qB:");
#endif

    type = bo->blitType;

    if ((index = bo->blissIndex) == 0)
        {
#ifdef DEBUG
if (bo->deBug & GEL3BUG)
{
printf(
"blitCnt=%ld beamSync=%ld blitSize=%lx pPick=%lx pOnOff=%lx\n",bo->blitCnt,bo->beamSync,0xFFFF&bo->blitSize,bo->pPick,bo->pOnOff); 
printf(
" blissIndex=%ld fwm=%lx lwm=%lx pbcn0=%lx minterm=%lx bcn1=%lx\n",
bo->blissIndex,0xFFFF&bo->fwm,0xFFFF&bo->lwm,0xFFFF&bo->pbcn0,0xFFFF&bo->minterm,0xFFFF&bo->bcn1);
printf(
" bmdsrc=%ld bmddst=%ld blitType=%ld *asrc=%lx writeMask=%lx\n",
bo->bmdsrc,bo->bmddst,bo->blitType,bo->asrc,bo->writeMask);
}
#endif

        io->fwmask = bo->fwm;
        io->lwmask = bo->lwm;

        io->bltmda = TOBB(bo->bmdsrc);
        io->bltmdb = TOBB(bo->bmdsrc);
        io->bltmdc = TOBB(bo->bmddst);
        io->bltmdd = TOBB(bo->bmddst);
        }

    io->bltpta = (UBYTE *)TOBB(bo->asrc);
    io->adata = 0xFFFF;
    cd = bo->destPtr[index];

    if (bo->writeMask & 0x1)
        {
        if ((type == B2BOBBER) || ((bo->pPick & 0x1) != 0))
            {
            io->bltcon0 = bo->pbcn0 | bo->minterm;

            io->bltptb = (UBYTE *)TOBB(bo->srcPtr[bo->srcIndex]);
            io->bltptc = (UBYTE *)TOBB(cd);
            io->bltptd = (UBYTE *)TOBB(cd);
            }
        else
            {
            if (bo->pOnOff & 0x1)
                io->bltcon0 = bo->pbcn0 | FILLSHADOW;
            else
                io->bltcon0 = bo->pbcn0 | CLEARSHADOW;
            io->bltptc = (UBYTE *)TOBB(cd);
            io->bltptd = (UBYTE *)TOBB(cd);
            }

	    io->bltcon1 = bo->bcn1;

	    if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
	    {
		io->bltsizv = (bo->blitSizV & 0x7fff);
		io->bltsizh = (bo->blitSizH & 0x07ff);
	    }
	    else
	    {
	        io->bltsize = bo->blitSize;
	    }

        } /* end writeMask test */

    bo->pPick >>= 1;
    bo->pOnOff >>= 1;
    bo->writeMask >>= 1;
    bo->blissIndex++;
    bo->srcIndex++;

    if (--bo->blitCnt)
        return (TRUE);
    else
        {
#ifdef DEBUG
if (bo->deBug & GEL3BUG) printf("\n");
#endif
        return(FALSE);
        }
}

#endif


SHORT resetRects(struct gelRect *src, struct gelRect *dest,
							struct ClipRect *CR)
/* this does a cross-rectangle adjustment of the src and dest gelRects based
 *  on their intersection (if any) with the ClipRect
 *  if new white space is created on the bob shadow (width is adjusted
 *  downward), returns the total amount of white space at the right 
 *  edge of the shadow
 */
/* This routine is used by newgenblit, modify it and you may screw it up */
/* however newgenblit doesn't use the returned value for anything - bart */
{
    WORD right;
    /* WORD bottom; */ /* bart - removed bottom */
    WORD newRight;
    WORD dstx = dest->rX;
    WORD dsty = dest->rY;
    /* extra for those bobs which are temporarily beyond the pale... bart */
    WORD xtra = ((src->rRealWW << 4) - (src->rX + src->rW)); 

/*???printf("SRC:");reportRect(src);*/
/*???printf("DEST:");reportRect(dest);*/

    right = dstx + src->rW - 1;
    /* bottom = dsty + src->rH - 1; */

    dest->rX = MAX(dstx, CR->bounds.MinX);
    dest->rY = MAX(dsty, CR->bounds.MinY);

    newRight = MIN(right, CR->bounds.MaxX);

    /* eliminated newbottom : see below... bart */
    /*newBottom = MIN(bottom, CR->bounds.MaxY);*/

    src->rX += (dest->rX - dstx);
    src->rY += (dest->rY - dsty);
    src->rW = newRight - dest->rX + 1;
    src->rH = (MIN((dsty + src->rH - 1),CR->bounds.MaxY)) - dest->rY + 1;

/*???printf("SRC:");reportRect(src);*/
/*???printf("DEST:");reportRect(dest);*/

    return(right - newRight + xtra);
}

#ifdef DEBUG
/*???*/
reportRect(rect)
struct gelRect *rect;
{
printf("&rect=%lx x=%ld y=%ld w=%ld h=%ld WW=%ld addr=%lx\n",
&rect, rect->rX, rect->rY, rect->rW, rect->rH, rect->rRealWW, rect->rAddr);
#endif

#ifdef OLDBLISSLICER

blisslicer(src, dest, RPort, s, mint, type, beamsync )
struct gelRect *src, *dest;
struct VSprite *s;
struct RastPort *RPort;
SHORT mint,type,beamsync;
{
    struct Layer *CW;
    register struct ClipRect *CR;
    struct gelRect worksrc, workdest;
    struct Rectangle blissRect;
    struct BitMap *BMap;
    USHORT Mask;
    SHORT  CWOffX, CWOffY, clippedwidth;
    /* SHORT  newBSync; */ /* bart - use real beamsync */
    struct GelsInfo *GI;

#ifdef DEBUG
if (Debug & GEL3BUG) printf(
"blisslicer(src=%lx dest=%lx RPort=%lx s=%lx mint=%lx type=%lx beamsync=%lx )\n",
src, dest, RPort, s, mint, type, beamsync );
#endif


    /* layered rastport ? */

    if ((CW = RPort->Layer) == 0)
    {

	/* there's no Layer, this is a simple blit to a RastPort */
	blisser(RPort->GelsInfo,src, dest, RPort->BitMap, s, RPort->Mask, mint, type,
		beamsync, 0 );
	return;
    }

	/* save time in loops */

	Mask = RPort->Mask;
	GI = RPort->GelsInfo;

	GI->Flags = RPort->Flags & DBUFFER;	/* copy DBUFFER flag */

	/* do the ClipRectangling Dance */

	CWOffX = CW->bounds.MinX - CW->Scroll_X;
	CWOffY = CW->bounds.MinY - CW->Scroll_Y;
	dest->rX += CWOffX;
	dest->rY += CWOffY;
	blissRect.MinX = dest->rX;
	blissRect.MinY = dest->rY;
	blissRect.MaxX = dest->rX + src->rW - 1;
	blissRect.MaxY = dest->rY + src->rH - 1;

	for(CR = CW->ClipRect; CR ; CR = CR->Next)
	{
	    /* look into calling blisser for each ClipRect */
	    if (rectXrect(&blissRect,&CR->bounds))
	    {
		/* ok, these two intersect, so reset the gelRects and bliss it */
		worksrc = *src;
		workdest = *dest;
		clippedwidth = resetRects(&worksrc, &workdest, CR);

		/* if this layer is obscured, fetch the alternate BitMap */
		if (CR->lobs)
		{
		    if(BMap = CR->BitMap) /* set zero if this is SIMPLE_REFRESH! */
		    {
			/* adjust dest gelRect x,y coordinates for special BitMap */
			workdest.rX -= (CR->bounds.MinX & ~0xF);
			workdest.rY -= CR->bounds.MinY;

			/* replace V37 bitmaps with V39 bitmaps -- bart */
			/* workdest.rRealWW = CR->BitMap->BytesPerRow >> 1; */

			workdest.rRealWW = ( gfx_GetBitMapAttr(CR->BitMap,BMA_WIDTH) >> 4 );

		    }
		    else
		    {
			continue;
		    }
		}
		else
		{
		    BMap = RPort->BitMap;
		}

		/* use real beamsync */
		/* newBSync = beamsync; */

		/* if it's a valid BitMap, go bliss it */

		if (BMap)
		    blisser(GI, &worksrc, &workdest, BMap, s, Mask,
		    mint, type, beamsync, clippedwidth );
	    }
	}

	/* now bliss super-cliprects, if any */

	if (CW->SuperBitMap)
	{

	    dest->rX -= CWOffX;
	    dest->rY -= CWOffY;
	    blissRect.MinX = dest->rX;
	    blissRect.MinY = dest->rY;
	    blissRect.MaxX = dest->rX + src->rW - 1;
	    blissRect.MaxY = dest->rY + src->rH - 1;

	    for(CR = CW->SuperClipRect; CR; CR = CR->Next)
	    {
		if (rectXrect(&blissRect,&CR->bounds))
		{
		    worksrc = *src;
		    workdest = *dest;

			/* convert from V37 to V39 bitmaps */
		    /* workdest.rRealWW = CW->SuperBitMap->BytesPerRow >> 1; */
			workdest.rRealWW = ( gfx_GetBitMapAttr(CR->SuperBitMap,BMA_WIDTH) >> 4 );

		    clippedwidth = resetRects(&worksrc, &workdest, CR);
		    /* newBSync = beamsync;	*/ /* use real beamsync */

		    blisser(GI, &worksrc, &workdest, CW->SuperBitMap, s, Mask, mint, type, beamsync, clippedwidth );
		}

	    }

	}
}

#endif


#ifdef OLDBLISSER

blisser(gi,src, dest, bm, s, mask, mint, type, beamsync, clippedwidth )
struct GelsInfo *gi;
struct gelRect *src, *dest;
struct BitMap *bm;
struct VSprite *s;
SHORT mask, mint, type, beamsync, clippedwidth;
{
    WORD shift;
    WORD sBitSize, dBitSize;
    WORD srcX, workW, workH, destX;
    WORD i, t, j;
    LONG srcOff, destOff, srcIncr; /* bart - 04.03.86 was WORD */
    WORD clipword, clipmask;
    WORD *shadowptr, *wordptr;
    struct blissObj *bo;
    struct gelRect *tg;
    /* struct Bob *thisBob; */
    LONG transfer;

#ifdef DEBUG
if (Debug & GEL3BUG)
{
printf("SRC:");reportRect(src);
printf("DEST:");reportRect(dest);
}
#endif

    if (type == B2SWAP)
    {
	transfer = *(LONG *)(&src->rW);
        *(LONG *)(&dest->rW) = transfer;
        tg = dest;
        dest = src;
        src = tg;
    }

    /* thisBob = s->VSBob; */

    if ((bo = (struct blissObj *)GfxAllocMem(sizeof(struct blissObj),MEMF_PUBLIC)) == 0)
    {
#ifdef MEMDEBUG
        printf("\nblisser:ERROR GETTING MEMORY PANIC PANIC\n");
#endif
	return (FALSE);
    }

    bo->whoSentMe = (struct Task *)FindTask(0);
    bo->blitStat = CLEANME; /* clean the memory allocation After last blit */
    bo->cleanUp = BlsDn;
    bo->blitCnt = bm->Depth;
    bo->blitRoutine = qBlissIt;
    bo->pPick = s->PlanePick; 
    bo->pOnOff = s->PlaneOnOff;
    bo->blissIndex = 0;
    bo->srcIndex = 0;
    bo->minterm = mint;
    bo->blitType = type;
    bo->beamSync = beamsync;
    bo->writeMask = mask;
    bo->deBug = FALSE;
    bo->shadow = NULL;	/* temporarily set to none */

    shadowptr = s->VSBob->ImageShadow;

/* set up srca mask and adata */

    srcX = src->rX;
    workW = src->rW;
    workH = src->rH - 1;
    destX = dest->rX;
    srcIncr = umuls(s->Width, s->Height) << 1;

/* shift = delta from src to dest.  if physical shift wants to be to the right
 *  subtract 16 to make negative and start destination address one to left
 */
    shift = (destX & 0xF) - (srcX & 0xF);

/* sBitSize = number of (physical words per row of source) * 16 */
    sBitSize = (((srcX + workW - 1) >> 4) - (srcX >> 4) + 1) << 4;
/* dBitSize = number of (physical words per row of destination) * 16 */
    dBitSize = (((destX + workW - 1) >> 4) - (destX >> 4) + 1) << 4;

    if (shift >= 0) /* shift right or no shift */
        /* dest word size is >= source, so 
         *  if there's no shadow (SRCA) use dst size, mask at dst (no AShift)
         *  else use shadow, AShift, if dst size > then lwmask = 0
         */
        {
        if (mint & SRCA)
            {
            bo->pbcn0 = ((shift & 0xF) << ASHIFTSHIFT);
            bo->fwm = 0xFFFF >> (srcX & 0xF);
            if (sBitSize < dBitSize)
		{
		bo->lwm = 0;
		/* did we clip some of the width of the shadow mask?
		 *  if so, we need to create a clipped shadow mask!
		 */
		if (clippedwidth)

/* ************************************************************************ */
{
    bo->shadowSize = i = umuls(s->Width, s->Height) << 1;
    if ( (wordptr = (WORD *) GfxAllocMem(i, MEMF_CHIP)) == 0)
    {
#ifdef MEMDEBUG
        printf("\nblisser:ERROR GETTING MEMORY PANIC PANIC\n");
#endif
		return (FALSE);
    }
    bo->shadow = wordptr;
    clipword = s->Width - 1 - ((clippedwidth - 1) >> 4);
    clipmask = 0xFFFF << (((clippedwidth - 1) & 0xF) + 1);
#ifdef DEBUG
if (Debug & 0x1)
printf("CLIPSHADOW:cwidth=%ld word=%ld mask=%lx size=%ld ptr=%lx\n",
	clippedwidth, clipword, clipmask, bo->shadowSize, wordptr);
#endif

    /* copy shadowmask to new memory */

    for (i = 0; i < s->Height; i++)
    {
	for (j = 0; j < clipword; j++)
	    *wordptr++ = *shadowptr++;

	*wordptr++ = (*shadowptr++) & clipmask;

	for (j = clipword + 1; j < s->Width; j++)
	{
	    *wordptr++ = 0;
	    shadowptr++;
	}
    }
    shadowptr = bo->shadow;
}
/* ************************************************************************ */

		}
            else
                if (i = (srcX + workW) & 0xF)
                    bo->lwm = 0xFFFF << (16 - i);
                else bo->lwm = 0xFFFF;
            }
        else /* no SRCA so simple rectangle cut:  mask destination */
            {
            bo->pbcn0 = 0;
            bo->fwm = 0xFFFF >> (destX & 0xF);
            if (i = (destX + workW) & 0xF)
                bo->lwm = 0xFFFF << (16 - i);
            else bo->lwm = 0xFFFF;
            }
        bo->bcn1 = ((shift & 0xF) << BSHIFTSHIFT);
        sBitSize = dBitSize;
        /* in this case, srcOff and destOff point to the END of both source
	 *  and destination.  The blitter has an idiosyncrosy that
         *  (unfortunately) requires blitting in reverse for some
         *  applications.  Note the negative modulus at the end!
	 */
        srcOff = (rastoff(src) + umuls(workH , src->rRealWW)) << 1;
        destOff = (rastoff(dest) + umuls(workH , dest->rRealWW)) << 1;
        }
    else
        /*****  the following sagacious comment is a TRUE-ISM !!!  *******/
        /* source word size is >= dest, so blit src size and mask at src */
        {
        shift = -shift;
        bo->pbcn0 = ((shift & 0xF) << ASHIFTSHIFT);
        bo->bcn1 = ((shift & 0xF) << BSHIFTSHIFT) | BLITREVERSE;
        bo->lwm = 0xFFFF >> (srcX & 0xF);
        if (i = (srcX + workW) & 0xF)
            bo->fwm = 0xFFFF << (16 - i);
        else bo->fwm = 0xFFFF;

        srcOff = (rastoff(src) + ((sBitSize >> 4) - 1)) << 1;
        destOff = (rastoff(dest) + ((sBitSize >> 4) - 1)) << 1;
        }

#ifdef DEBUG
if(Debug & GEL3BUG)
printf(" srcOff=%ld srcIncr=%ld destOff=%ld\n",srcOff,srcIncr,destOff);
#endif
    t = 0;
    if (type == B2SWAP)
        /* we're saving the background into the buffer */
        {
        bo->asrc = shadowptr + (destOff >> 1);
/*        for (i = 0; i < rP->Depth; i++)  */
        for (i = 0; i < bm->Depth; i++)
            {
            bo->srcPtr[i] = bm->Planes[i] + srcOff;
            bo->destPtr[i] = dest->rAddr + destOff + t;
            t += srcIncr;

#ifdef DEBUG
if(Debug & GEL3BUG)
printf("bo->srcPtr[i]=%lx bo->destPtr[i]=%lx bo->asrc=%lx\n",bo->srcPtr[i],bo->destPtr[i],bo->asrc);
#endif
            }
        }
    else
        {
        bo->asrc = shadowptr + (srcOff >> 1);
        for (i = 0; i < bm->Depth; i++)
            {
            bo->srcPtr[i] = src->rAddr + srcOff + t;
            t += srcIncr;
            bo->destPtr[i] = bm->Planes[i] + destOff;
#ifdef DEBUG
if(Debug & GEL3BUG)
printf("bo->srcPtr[i]=%lx bo->destPtr[i]=%lx bo->asrc=%lx\n",bo->srcPtr[i],bo->destPtr[i],bo->asrc);
#endif
            }
        }

    bo->blitSize = blitsize(src->rH, sBitSize);
    /* "big blits" -- bart */
    bo->blitSizV = src->rH;
    bo->blitSizH = (sBitSize >> 4);

    bo->bmdsrc = (-(src->rRealWW + (sBitSize >> 4))) << 1;
    bo->bmddst = (-(dest->rRealWW + (sBitSize >> 4))) << 1;

    /* add this blissObj to gelsinfo list */
    {
	struct blissObj *botemp;

        /* get the last */

	if (botemp = gi->lastBlissObj) botemp->Next = bo;
	else
	{
	    /* stash the first */
	    gi->firstBlissObj = bo;
	}

	gi->lastBlissObj = bo;	/* stash the last */
	bo->Next = 0;
    }

    if (gi->Flags & DBUFFER)	QBLIT(bo);
	else	QBSBLIT(bo);
}

#endif

