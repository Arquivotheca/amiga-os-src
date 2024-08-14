/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: blisser.c,v 37.4 91/05/14 11:23:39 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	blisser.c,v $
*   Revision 37.4  91/05/14  11:23:39  chrisg
*   changed to include gfxbase.h and macros.h for exec pragmas
*   
*   Revision 37.3  91/05/13  14:35:40  chrisg
*   removed compiler warnings for lattice
*   made use prototypes
*   got rid of interface code.. made routines take
*   arguments in registers.
*   
*   Revision 37.2  91/05/02  17:00:10  chrisg
*    added #include <hardware/custom.h>
*   
*   Revision 37.1  91/05/02  16:58:37  chrisg
*   killed ".." for lattice
*   
*   Revision 37.0  91/01/07  15:22:01  spence
*   initial switchover from V36
*   
*   Revision 33.6  90/12/06  13:49:53  bart
*   removed gfxallocmem.c and implemented better lo-mem error recovery -- bart
*   
*   Revision 33.5  90/07/27  16:35:58  bart
*   id
*   
*   Revision 33.4  90/03/28  09:27:22  bart
*   *** empty log message ***
*   
*   Revision 33.3  88/11/16  13:09:39  bart
*   *** empty log message ***
*   
*   Revision 33.2  88/11/16  09:06:26  bart
*   big blits
*   
*   Revision 33.1  88/06/20  18:08:43  bart
*   use ../macros.h
*   
*   Revision 33.0  86/05/17  15:22:24  bart
*   added to rcs for updating
*   
*
******************************************************************************/

/*** blisser.c ******************************************************************
 *
 *  File
 *
 *  Confidential Information: Amiga Computer, Inc.
 *  Copyright (c) Amiga Computer, Inc.
 *
 *                  Modification History
 *  date    :   author :    Comments
 *  -------     ------      ---------------------------------------
 *  9-28-84     -=RJ=-      originally in file gel3.c
 *  3-26-86      bart       block allocation/deallocation of blissobjs
 *
 ****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/gels.h>
#include <graphics/gfxbase.h>
#include "gelsinternal.h"
#include "/macros.h"
#include <hardware/custom.h>
#include "gels.protos"

#define BLITTERSIGNAL   4

extern struct Custom custom;

/*???#define DEBUG 1*/

/* #define OLDBLISSOBJ */

/* #define BARTDEBUG */

#ifdef BARTDEBUG
#define printf kprintf
#endif

#define blitsize(h,w) ((h << HSIZEBITS) | (w >> 4))
extern qBlissIt(),BlsDn();

#define rastoff(r) ((umuls(r->rY , r->rRealWW)) + (r->rX >> 4))

/* #define rectXrect_macro(a,b) ( ((b)->MaxX < (a)->MinX) && ((b)->MinX > (a)->MaxX) && ((b)->MaxY < (a)->MinY) && ((b)->MinY > (a)->MaxY) )  */

/* ************************************************************************ */

#ifndef OLDBLISSOBJ

struct blissObj *allocblissobj(gi)
struct GelsInfo *gi;
{
    struct blissBlock **bi;
    struct blissObj *bo = NULL;
    UBYTE i;

#ifdef BARTDEBUG
    printf("allocblissobj: entering...\n");
#endif

    /* start at head of list */

    bi = (struct blissBlock **) &gi->firstBlissObj;

    /* get blissblock which is not fully allocated yet */

    while ( (*bi) && ((*bi)->CurCount == (*bi)->MaxCount) )
    {
	bi = &((*bi)->NextBlock);
    }

    /* need another block ? */

    if(!(*bi))
    {
	if ((*bi = (struct blissBlock *)AllocMem(sizeof(struct blissBlock),MEMF_PUBLIC)) == 0)
	{

#ifdef BARTDEBUG
    printf("allocblissobj: failed to allocate blissblock...\n");
    printf("allocblissobj: bo = %lx...\n",bo);
    printf("allocblissobj: exiting...\n");
#endif
	    return (FALSE);
	}
	else
	{

#ifdef BARTDEBUG
    printf("allocblissobj: allocated blissblock = %lx...\n",*bi);
#endif
	    (*bi)->CurCount = 0;
	    (*bi)->MaxCount = BLISS_BLOCK_MAXCOUNT;
	    (*bi)->NextBlock = NULL;
	}
    }

    /* got a blissblock to allocate a blissobject from */

    i = (*bi)->CurCount++;	 /* increment the count of blissobjs */
				 /* currently allocated from this block */

    bo =  &((*bi)->blissObj[i]); /* get pointer to the allocated blissobject */

#ifdef BARTDEBUG
    printf("allocblissobj: bo =  &((*bi)->blissObj[%lx]) = %lx...\n",i,bo);
    printf("allocblissobj: exiting...\n");
#endif

    return(bo);
}

void freelastblissobj(gi)
struct GelsInfo *gi;
{
    struct blissBlock **bi;

    /* start at head of list */

    bi = (struct blissBlock **) &gi->firstBlissObj;

    /* find blissblock which is contains last blissobj allocated */

    while ( (*bi) && ((*bi)->CurCount == (*bi)->MaxCount) )
    {
	if( (*bi)->NextBlock ) bi = &((*bi)->NextBlock);
    }

    /* "free" the most recent blissobj allocated */

    --(*bi)->CurCount;
}

#endif

#ifndef OLDBLISSOBJ

void byebyeblissobjs(gi)
struct GelsInfo *gi;
{
    struct blissBlock **bi;
    struct blissBlock *bfree;
    UBYTE i;
    struct blissObj *bo, *bn;
    struct blissObj *bodelay;

    /* This routine will free memory as it is allocated by blisser */
    /* it frees blissObjs that have clipped shadows by waiting for */
    /* the next blissObj to complete and then freeing the shadow */
    /* modified on 03.26.86 to deallocate block structures blissobjs - bart */

#ifdef BARTDEBUG
    printf("byebyeblissobjs: entering...\n");
#endif

    bodelay = NULL;		/* used to delay freeing of clipped blissObjs */

    bi = (struct blissBlock **) &gi->firstBlissObj;    /* get pointer to head of block list */

#ifdef BARTDEBUG
    printf("byebyeblissobjs: bi = &gi->firstBlissObj = %lx...\n",bi);
#endif

    while(*bi)
    {
#ifdef BARTDEBUG
    printf("byebyeblissobjs: *bi = %lx...\n",*bi);
#endif
	for(i = 0; i < (*bi)->CurCount; i++)
	{
	    bo = &((*bi)->blissObj[i]);

#ifdef BARTDEBUG
    printf("byebyeblissobjs:  bo = &((*bi)->blissObj[%lx])= %lx...\n",i,bo);
#endif

	    while (bo->blitCnt) Wait(1<<BLITTERSIGNAL);   

	    if (bodelay)
	    {
		/* free off clipped shadow */
		waitblitdone();

#ifdef BARTDEBUG
    printf("byebyeblissobjs: bodelay = %lx...\n",bodelay);
    printf("byebyeblissobjs: FreeMem(bodelay->shadow = %lx,bodelay->shadowSize = %lx)...\n",bodelay->shadow,bodelay->shadowSize);
#endif

		FreeMem(bodelay->shadow,bodelay->shadowSize);
		bodelay = NULL;
	    }

	    if (bo->shadow) bodelay = bo;
	}

	bfree = *bi;			/* get pointer to current block */

#ifdef BARTDEBUG
    printf("byebyeblissobjs:  bfree = *bi = %lx...\n",bfree);
#endif

	*bi = (*bi)->NextBlock; 	/* de-link current block */

#ifdef BARTDEBUG
    printf("byebyeblissobjs:  *bi = (*bi)->NextBlock = %lx...\n",*bi);
#endif

	if(!(bodelay))
	{			
	    /* ok to free block */
	    waitblitdone();
#ifdef BARTDEBUG
    printf("byebyeblissobjs: FreeMem(bfree = %lx, sizeof(struct blissBlock)) = %lx...\n",bfree, sizeof(struct blissBlock));
#endif
	    FreeMem(bfree, sizeof(struct blissBlock));
	}
	else
	{
	    /* wait to free the block */

	    if( (*bi) && ((*bi)->CurCount) )
	    {

		bn = (void *) *bi;

#ifdef BARTDEBUG
    printf("byebyeblissobjs: bn = *bi = %lx...\n",bn);
#endif
		while (bn->blitCnt) Wait(1<<BLITTERSIGNAL);
	    }

	    waitblitdone();

#ifdef BARTDEBUG
    printf("byebyeblissobjs: bodelay = %lx...\n",bodelay);
    printf("byebyeblissobjs: FreeMem(bodelay->shadow = %lx,bodelay->shadowSize = %lx)...\n",bodelay->shadow,bodelay->shadowSize);
#endif
	    FreeMem(bodelay->shadow,bodelay->shadowSize);

#ifdef BARTDEBUG
    printf("byebyeblissobjs: FreeMem(bfree = %lx, sizeof(struct blissBlock)) = %lx...\n",bfree, sizeof(struct blissBlock));
#endif
	    FreeMem(bfree, sizeof(struct blissBlock));
	    bodelay = NULL;
	}

    }

#ifdef BARTDEBUG
    printf("byebyeblissobjs: exiting...\n");
#endif

}

#else

byebyeblissobjs(gi)
struct GelsInfo *gi;
{
    struct blissObj *bo,*botemp,*bodelay;

    /* This routine will free memory as it is allocated by blisser */
    /* it frees blissObjs that have clipped shadows by waiting for */
    /* the next blissObj to complete and then freeing the shadow */

    bodelay = 0;		/* used to delay freeing of clipped blissObjs */
    bo = gi->firstBlissObj;    /* get pointer to first if any */
    while (bo)
    {
	while (bo->blitCnt) Wait(1<<BLITTERSIGNAL);	/* bart - 03.19.86 */
	/* if (bo->blitCnt) Wait(1<<BLITTERSIGNAL);*/	/* wait till told */
	/* else */
	{
	    botemp = bo;
	    bo = bo->Next;
	    if (bodelay)
	    {	/* free off blissobj with clipped shadow */
		FreeMem(bodelay->shadow,bodelay->shadowSize);
		FreeMem(bodelay,sizeof(struct blissObj));
		bodelay = 0;
	    }
	    if (botemp->shadow)	bodelay = botemp;
	    else			
	    {
		/* have to wait till blitter is not using it */
		waitblitdone();

		FreeMem(botemp,sizeof(struct blissObj));
	    }
	}
    }
    if (bodelay)	/* do we still have one hanging around ? */
    {
	waitblitdone();	/* have to wait till blitter is not using it */
	FreeMem(bodelay->shadow,bodelay->shadowSize);
	FreeMem(bodelay,sizeof(struct blissObj));
    }
}

#endif

#define DOWNBLISSER

#ifndef  DOWNBLISSER

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

#ifndef OLDBLISSOBJ

    if((bo = allocblissobj(gi)) == 0)
    {

#ifdef MEMDEBUG
        printf("\nblisser:ERROR GETTING MEMORY PANIC PANIC\n");
#endif
	return (FALSE);
    }

#else

    if ((bo = (struct blissObj *)GfxAllocMem(sizeof(struct blissObj),MEMF_PUBLIC)) == 0)
    {
#ifdef MEMDEBUG
        printf("\nblisser:ERROR GETTING MEMORY PANIC PANIC\n");
#endif
	return (FALSE);
    }

#endif

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

#ifndef OLDBLISSOBJ

    /* add this blissObj to gelsinfo list */
    {
	struct blissObj *botemp;

	/* none after this one */

	bo->Next = 0;

        /* get the last */

	if (botemp = gi->lastBlissObj) botemp->Next = bo;

	gi->lastBlissObj = bo;	/* stash the last */
    }

#else

    /* add this blissObj to gelsinfo list */
    {
	struct blissObj *botemp;

	/* none after this one */

	bo->Next = 0;

        /* get the last */

	if (botemp = gi->lastBlissObj) botemp->Next = bo;
	else
	{
	    /* stash the first */
	    gi->firstBlissObj = bo;
	}

	gi->lastBlissObj = bo;	/* stash the last */
    }

#endif

    if (gi->Flags & DBUFFER)	QBLIT(bo);
	else	QBSBLIT(bo);
}

#endif

