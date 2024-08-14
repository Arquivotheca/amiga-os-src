/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: gel.c,v 42.1 93/07/20 13:39:24 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	gel.c,v $
 * Revision 42.1  93/07/20  13:39:24  chrisg
 * umuls gone.
 * 
 * Revision 42.0  93/06/16  11:19:03  chrisg
 * initial
 * 
 * Revision 42.0  1993/06/01  07:20:29  chrisg
 * *** empty log message ***
 *
*   Revision 39.3  92/05/05  12:20:33  chrisg
*   FETCHGBASE, regargs, etc.
*   
*   Revision 39.2  92/02/21  13:08:22  bart
*   moved collType to avoid forward reference
*   
*   Revision 39.1  92/02/21  12:57:26  bart
*   re-ordered functions for proto making from scracth
*   to avoid forward references
*   
*   Revision 39.0  91/08/21  17:34:00  chrisg
*   Bumped
*   
*   Revision 37.6  91/05/14  16:10:58  chrisg
*   made work without glue routines
*   
*   Revision 37.5  91/05/13  14:36:16  chrisg
*   removed compiler warnings for lattice
*   made use prototypes
*   got rid of interface code.. made routines take
*   arguments in registers.
*   
*   Revision 37.4  91/05/02  16:45:53  chrisg
*    killed "../" for lattice
*   
*   Revision 37.3  91/02/15  11:00:47  spence
*   Removed Bart's BORDERHIT fix (broke people :-( ).
*   
*   Revision 37.2  91/02/12  15:49:01  spence
*   autodoc
*   
*   Revision 37.1  91/01/23  13:11:59  bart
*   correct BORDERHIT processing (1 << BORDERHIT)
*   
*   Revision 37.0  91/01/07  15:22:10  spence
*   initial switchover from V36
*   
*   Revision 33.11  90/07/27  16:37:14  bart
*   id
*   
*   Revision 33.10  90/03/28  09:26:40  bart
*   *** empty log message ***
*   
*   Revision 33.9  90/03/19  13:20:43  bart
*   fix bug B6600
*   
*   Revision 33.8  89/12/04  14:09:14  bart
*   *** empty log message ***
*   
*   Revision 33.7  89/12/04  14:03:01  bart
*   *** empty log message ***
*   
*   Revision 33.6  89/12/04  13:32:01  bart
*   dont collide vsprite if bob and bobsaway
*   
*   Revision 33.5  88/11/16  09:59:52  bart
*   *** empty log message ***
*   
*   Revision 33.4  88/11/16  09:57:58  bart
*   *** empty log message ***
*   
*   Revision 33.3  88/11/16  09:54:20  bart
*   big blits
*   
*   Revision 33.2  86/07/01  11:30:02  bart
*   documentation update to DoCollision : removed BUGS comment
*   
*   Revision 33.1  86/06/17  10:17:15  bart
*   fixes collision bug #B2897
*   boffset now set correctly
*   
*   Revision 33.0  86/05/17  15:22:58  bart
*   added to rcs for updating
*   
*
******************************************************************************/


/*** gel.c *******************************************************************
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
 * 08-02-85		-Dale-		removed V28 node stuff for V29
 *
 ****************************************************************************/

#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/gels.h>
#include <graphics/rastport.h>
#include <graphics/clip.h>
#include <graphics/collide.h>
#include <graphics/gels.h>
#include "gelsinternal.h"
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <hardware/dmabits.h>
#include "/sane_names.h"
#include "/macros.h"
#include "gels.protos"


#define umuls(a,b) (((UWORD) a)*((UWORD) b))

#define BLITTERSIGNAL 4


/*#define DEBUG 1*/

/* #define SQUEEZE */

/* ************************************************************************ */

int __regargs bndryHit(struct VSprite *s,struct GelsInfo *gi)
/* returns 4 bit mask bit 0 = top (TOPHIT)
 *                    bit 1 = bottom (BOTTOMHIT)
 *                    bit 2 = left (LEFTHIT)
 *                    bit 3 = right (RIGHTHIT)
 */
{
    WORD offWidth, i;
    ULONG m;
    LONG *bptr;
    WORD hit_val = 0;
    int j,k;

#ifdef DEBUG
if (Debug & GELBUG) printf("bndryHit (%lx, %lx, %lx)\n", s, gi );
#endif

    /*  hit_val = 0; */

    /* test top collision */
    if (s->Y < gi->topmost)
    {
	UWORD *bptr;
	/* int j,k; */

	/* have to test if any bits in collision mask are set */
	i = gi->topmost - s->Y + 1;	/* how deep into collision mask */
	k = smuls(s->Width,i+1);	/* number of words to check */
	bptr = s->CollMask;

	for (j = 0; j < k ; j++)
	if ( *bptr++ )
	{
	    hit_val = TOPHIT;
	    break;
	}

	/*printf("s->Y=%lx topmost = %lx width=%lx collmask=%lx hitval=%lx\n",
	            s->Y,gi->topmost,s->Width,s->CollMask,hit_val);*/
	/*Debug();*/

#ifdef DEBUG
if (Debug & GELBUG)
printf("   TOP y=%ld h=%ld tm=%ld\n",s->Y,s->Height,gi->topmost);
#endif

    }

    /* test bottom collision */
    if (s->Y + s->Height - 1 > gi->bottommost)
    {
	UWORD *bptr;
	/* int j,k; */

	/* have to test if any bits in collision mask are set */
	i = gi->bottommost - s->Y + 1;

	/* how deep into collision mask */
	k = smuls(s->Width,i);	/* offset into CollMask */
	bptr = s->CollMask + k;

	/* how many words left to check ? */
	k = smuls(s->Width,s->Height - i);
	for (j = 0; j < k ; j++)
	if ( *bptr++ )
	{
	    hit_val |= BOTTOMHIT;
	    break;
	}

#ifdef DEBUG
if (Debug & GELBUG)
printf("   BOTTOM y=%ld h=%ld bm=%ld\n",s->Y,s->Height,gi->bottommost);
#endif

    }

    /* test left collision */
    if ((offWidth = gi->leftmost - s->X) > 0)
    {
	if (offWidth > (s->Width << 4)) offWidth = (s->Width << 4);

#ifdef DEBUG
if(Debug & GELBUG)
printf("   LEFT x=%ld w=%ld lm=%ld\n",s->X,s->Width,gi->leftmost);
#endif
        bptr = (LONG *) s->BorderLine;
        do
            {
	    /* m = *bptr; */ /* bart - see below */
	    m = *bptr++;
            if (offWidth < 32) m >>= (32 - offWidth);
#ifdef DEBUG
if (Debug & GELBUG)
printf("  m=%lx bptr=%lx offWidth=%ld\n",m,bptr,offWidth);
#endif
            if (m)
                {
                hit_val |= LEFTHIT;
                break;
                }
            /* bptr++;*/ /* bart - see above */
            offWidth -= 32;
            }
        while (offWidth > 0);
    }

    /* test right collision */
    if ((offWidth = (s->X + (s->Width << 4) - 1) - gi->rightmost) > 0)
    {
	UWORD *bptr,m;

	if (offWidth > (s->Width << 4)) offWidth = (s->Width << 4);

#ifdef DEBUG
if(Debug & GELBUG)
printf("  RIGHT x=%ld w=%ld rm=%ld\n",s->X,s->Width,gi->rightmost);
#endif

        bptr = s->BorderLine + s->Width - 1;	/* point to end of Line */
        do
	{
	    /* m = *bptr; */ /* bart - see below */
	    m = *bptr--;

#ifdef DEBUG
if (Debug & GELBUG)
printf("  m=%lx bptr=%lx offWidth=%ld\n",m,bptr,offWidth);
#endif

            if (offWidth < 16) m <<= (16 - offWidth);
            if (m)
	    {
                hit_val |= RIGHTHIT;
                break;
	    }
            /* bptr--; */ /* bart - see above */
            offWidth -= 16;
	}
        while (offWidth > 0);
    }

    return((int) hit_val);
}



#ifdef OLDCHECK

/* mag added ownblitter, disownblitter, gfxBase ptr, increased code size */

BOOL __regargs checkColl(struct VSprite *a,struct VSprite *b)
{
    struct AmigaRegs *io ={(BYTE *)0x0dff000};
    struct gelRect gRa;
    SHORT fwm, lwm; 
    SHORT aoffset,boffset,blitHeight,blitWidth,blitsize;
    SHORT amodulus, bmodulus, aymax, bymax, axmax, bxmax, wcls, wcrs;
    USHORT bzero;
    SHORT shifta,shiftb;
    SHORT OMaxX,OMaxY;

#ifdef DEBUG
if (Debug & GELBUG)
    printf("checkColl(a=%lx b=%lx )\n", a, b);
#endif


    OMaxY = aymax = (a->Y) + (a->Height) - 1;
    OMaxX = axmax = (a->X) + (a->Width<<4) - 1;

    bymax = (b->Y) + (b->Height) - 1;
    bxmax = (b->X) + (b->Width<<4) - 1;

    gRa.rRealWW = a->Width;

    gRa.rX = (a->X < b->X)? b->X : a->X; /* MAX(rX,left) */
    OMaxX = (OMaxX > bxmax) ? bxmax : OMaxX; /* MIN(OMaxX,right) */
    gRa.rW = OMaxX - (gRa.rX) + 1;

    gRa.rY = (a->Y < b->Y)? b->Y: a->Y; /* MAX(rY, top */
    OMaxY = (OMaxY > bymax)? bymax: OMaxY; /* MIN(OMaxY,bottom) */
    gRa.rH = OMaxY - (gRa.rY) + 1;

    shifta = ((b->X) - (a->X)) & 0x0f;

  /* word of 'a' containing left side of 'b' */
    wcls = ((gRa.rX) - (a->X))>>4;

    if (bxmax >= axmax) shiftb = 0;
    else shiftb = (axmax - bxmax) & 0x0f;

    wcrs = (((gRa.rX) + (gRa.rW) - 1) - (a->X))>>4;

  /* rightmost word of 'a' containing ANY of 'b' */
    blitWidth = wcrs - wcls + 1;

    fwm = ((USHORT)(~0))>>shifta;

    if (blitWidth == 1)
	{
	fwm = fwm & ((~0)<<shiftb);
	lwm = fwm;
	}
    else
	lwm = ((~0)<<shiftb);

    amodulus = ((a->Width) - blitWidth)<<1;
    bmodulus = ((b->Width) - blitWidth)<<1;

    aoffset =((umuls(((b->Y > a->Y)? (b->Y - a->Y): 0), a->Width)) + wcls)<<1;

	/* bart - 06.17.86 */
    /* boffset =(umuls(((a->Y > b->Y)? (a->Y - b->Y): 0), b->Width))<<1; */
    boffset =
		((umuls(( (a->Y > b->Y) ? (a->Y - b->Y) : 0), b->Width)) + 
		(((gRa.rX) - (b->X)) >>4) ) <<1;


    blitHeight = gRa.rH;

    blitsize = (blitHeight<<6) + blitWidth; /*???*/

/*???    WaitBlit();*/
    OwnBlitter();
    waitblitdone();

#ifdef DEBUG
if (Debug & GELBUG)
{
printf("io->bltcon0 = %lx\n", 0x0CC0);
printf("io->bltcon1 = %lx\n", shifta << 12);
printf("fwmask = %lx\n", fwm);
printf("lwmask = %lx\n",  lwm);
printf("io->bltpta = %lx\n",  (BYTE *)(&a->CollMask[0]) + aoffset);
printf("io->bltptb = %lx\n",  (BYTE *)(&b->CollMask[0]) + boffset);
printf("io->bltptc = %lx\n",  0);
printf("io->bltptd = %lx\n",  0);
printf("io->bltmda = %lx\n",  amodulus);
printf("io->bltmdb = %lx\n",  bmodulus);
printf("io->bltmdc = %lx\n",  0);
printf("io->bltmdd = %lx\n",  0);
printf("io->bltsize = %lx\n",  blitsize);
Debug();
}
#endif

    io->bltcon0 = SRCA|SRCB | ABC|ABNC;
    io->bltcon1 = shifta << 12;
    io->fwmask = fwm;
    io->lwmask = lwm;
    /* pointer into object a */
    io->bltpta = (BYTE *)(&a->CollMask[0]) + aoffset;
    /* pointer to object b */
    io->bltptb = (BYTE *)(&b->CollMask[0]) + boffset;
    io->bltptc = 0;
    io->bltptd = 0;
    io->bltmda = amodulus;
    io->bltmdb = bmodulus;
    io->bltmdc = 0;
    io->bltmdd = 0;
    /* big blits */
    {

	if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
	{
	    io->bltsizv = (blitHeight & 0x7fff);
	    io->bltsizh = (blitWidth  & 0x07ff);
	}
	else
	{
	    io->bltsize = blitsize;
	}
    }

/*???    WaitBlit();*/

    waitblitdone();

    bzero = io->dmaconr & DMAF_BLTNZERO;

    DisownBlitter();

    if (bzero) return(FALSE);
    else return(TRUE);
}

#else

/* bart reduced variables and sped things up 03.14.86 */

BOOL __regargs checkColl(a, b )
struct VSprite *a,*b;
{
    extern struct AmigaRegs custom;
    struct gelRect gRa;
    SHORT fwm, lwm; 
    SHORT aoffset,boffset,blitWidth;
    SHORT bymax, bxmax, wcls;
    /* SHORT wcrs; */
    USHORT bzero;
    SHORT shifta,shiftb;
    SHORT OMaxX,OMaxY;

#ifdef DEBUG
if (Debug & GELBUG)
    printf("checkColl(a=%lx b=%lx )\n", a, b);
#endif


    OMaxY = (a->Y) + (a->Height) - 1;
    OMaxX = (a->X) + (a->Width<<4) - 1;

    bymax = (b->Y) + (b->Height) - 1;
    bxmax = (b->X) + (b->Width<<4) - 1;

    /* bart - speedup */
    /* if (bxmax >= OMaxX ) shiftb = 0; else shiftb = (OMaxX - bxmax) & 0x0f; */

    if ((shiftb = OMaxX - bxmax) < 0) shiftb = 0;
    else shiftb &= 0x0f;

    /* gRa.rRealWW = a->Width;*/
    /*shifta = ((b->X) - (a->X)) & 0x0f;*/
    /*gRa.rX = (a->X < b->X)? b->X : a->X;*/ /* MAX(rX,left) */

    if((shifta = (b->X) - (a->X)) > 0) gRa.rX = b->X; else gRa.rX = a->X;
    shifta &= 0x0f;

    /* end bart - speedup */

    OMaxX = (OMaxX > bxmax) ? bxmax : OMaxX; /* MIN(OMaxX,right) */
    gRa.rW = OMaxX - (gRa.rX) + 1;

    gRa.rY = (a->Y < b->Y)? b->Y: a->Y; /* MAX(rY, top */
    OMaxY = (OMaxY > bymax)? bymax: OMaxY; /* MIN(OMaxY,bottom) */
    gRa.rH = OMaxY - (gRa.rY) + 1;

  /* word of 'a' containing left side of 'b' */

    wcls = ((gRa.rX) - (a->X))>>4;

  /*  wcrs = (((gRa.rX) + (gRa.rW) - 1) - (a->X))>>4; */

  /* rightmost word of 'a' containing ANY of 'b' */

  /* blitWidth = wcrs - wcls + 1; */ /* wcrs not used enough, removed - bart */

     blitWidth = ((((gRa.rX) + (gRa.rW) - 1) - (a->X)) >> 4) - wcls + 1;

    fwm = ((USHORT)(~0))>>shifta;

    if (blitWidth == 1)
	{
	fwm = fwm & ((~0)<<shiftb);
	lwm = fwm;
	}
    else
	lwm = ((~0)<<shiftb);

    /* bart - see below */
    /* amodulus = (((a->Width) - blitWidth)<<1); */
    /* bmodulus = (((b->Width) - blitWidth)<<1); */
    /* blitsize = ((gRa.rH<<6) + blitWidth);*/

    aoffset =((umuls(((b->Y > a->Y)? (b->Y - a->Y): 0), a->Width)) + wcls)<<1;

	/* bart - 06.17.86 */
    /* boffset =(umuls(((a->Y > b->Y)? (a->Y - b->Y): 0), b->Width))<<1; */
    boffset =
		((umuls(( (a->Y > b->Y) ? (a->Y - b->Y) : 0), b->Width)) + 
		(((gRa.rX) - (b->X)) >>4) ) <<1;

    OwnBlitter();

    waitblitdone();

#ifdef DEBUG
if (Debug & GELBUG)
{
printf("custom.bltcon0 = %lx\n", 0x0CC0);
printf("custom.bltcon1 = %lx\n", shifta << 12);
printf("custom.fwmask = %lx\n", fwm);
printf("custom.lwmask = %lx\n",  lwm);
printf("custom.bltpta = %lx\n",  (BYTE *)(&a->CollMask[0]) + aoffset);
printf("custom.bltptb = %lx\n",  (BYTE *)(&b->CollMask[0]) + boffset);
printf("custom.bltptc = %lx\n",  0);
printf("custom.bltptd = %lx\n",  0);
/* printf("custom.bltmda = %lx\n",  amodulus); */
/* printf("custom.bltmdb = %lx\n",  bmodulus); */
printf("custom.bltmdc = %lx\n",  0);
printf("custom.bltmdd = %lx\n",  0);
/* printf("custom.bltsize = %lx\n",  blitsize); */
Debug();
}
#endif

    custom.bltcon0 = SRCA|SRCB | ABC|ABNC;
    custom.bltcon1 = shifta << 12;
    custom.fwmask = fwm;
    custom.lwmask = lwm;

    /* pointer into object a */

    custom.bltpta = (BYTE *)(&a->CollMask[0]) + aoffset;

    /* pointer to object b */

    custom.bltptb = (BYTE *)(&b->CollMask[0]) + boffset;
    custom.bltptc = 0;
    custom.bltptd = 0;

    /* amodulus = (((a->Width) - blitWidth)<<1); */ /* bart */
    /* bmodulus = (((b->Width) - blitWidth)<<1); */ /* bart */

    custom.bltmda = (((a->Width) - blitWidth)<<1);
    custom.bltmdb = (((b->Width) - blitWidth)<<1);

    custom.bltmdc = 0;
    custom.bltmdd = 0;

    /* blitsize = ((gRa.rH<<6) + blitWidth);*/ /* bart */

    /* big blits */
    {

	if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
	{
	    custom.bltsizv = (gRa.rH    & 0x7fff);
	    custom.bltsizh = (blitWidth & 0x07ff);
	}
	else
	{
	    custom.bltsize = ((gRa.rH<<6) + blitWidth);
	}
    }

    waitblitdone();

    /* bzero = custom.dmaconr & DMAF_BLTNZERO; */

    bzero = custom.dmaconr & DMAF_BLTNZERO;

    DisownBlitter();

    if (bzero) return(FALSE);
    else return(TRUE);
}

#endif

__regargs collType(struct VSprite *s, struct VSprite *t)
/* if the hit masks coincide and there is a collision,
 *  return the bit position of the lowest-most set bit of the hit masks
 *  else return 0
 */
{
    WORD flag, pos;

#ifdef DEBUG
if (Debug & GELBUG) printf("CollType(%lx, %lx, %lx)\n", s, t );
#endif

/* assumes that s is to the left of and/or above t */
    if ( (flag = (t->HitMask & -2) & (s->MeMask & -2)) != 0 )
        if (checkColl(s, t ))
	{
            pos = 0;
            do
                pos++;
            while (((flag >>= 1) & 0x01) == 0);
            return((int) pos);
	}

    return(0);
}


void __regargs downCheck(struct VSprite *s,      /* look down in list for collisions */
					struct GelsInfo *gi,
					struct VSprite *real)   /* apply result to real vsprite */
{

    WORD stop, sx, sw;
    WORD type, diff;
    struct VSprite *t;

    /* look forward */
    stop = s->Y + s->Height;
    sx = s->X;
    sw = s->Width << 4;
    for(t = s->NextVSprite; t->Y < stop; t = t->NextVSprite)
    {
        /* diff gets [+ or -] number of overlap columns */
        if ((diff = t->X - sx) < 0) /* t's X is left of s's */
	{
            if (-diff >= (t->Width << 4)) continue; /* if >= then no overlap */
	}
        else /* s's X is left of or equal to t's */
	if (diff >= sw) continue; /* if >= then no overlap */

        /* else they overlap along both axes, so test masks for collision */
        if ((type = collType(s, t )) != 0)
	{
	    if((s->VSBob && ((s->VSBob)->Flags & BOBSAWAY))
	    || (t->VSBob && ((t->VSBob)->Flags & BOBSAWAY)));
            else (*gi->collHandler->collPtrs[type])(real, t);
	}
    }
}

BOOL __regargs setRects(WORD high, WORD wide, struct gelRect *src,
						struct gelRect *dest, WORD dispx, WORD dispy,
						int RHeight, int RWidth )
/* arguments:
 *  high, wide = Height and Width for rectangularization
 *  src, dest = gelRect types
 *  dispx, dispy = starting display location
 *
 * sets:
 *  src->rY = starting line of image
 *  src->rX = starting column of image
 *  src->rH = number of image lines onscreen
 *  src->rW = number of image columns onscreen
 *  dest->rY = true starting line of display
 *  dest->rX = true starting column of display
 *
 * returns:
 *  0 = VSprite completely offscreen
 *  1 = VSprite at least partially onscreen
 */
{
    WORD ssx, ssy, h, w;
    WORD scx, scy;

    wide <<= 4;

/* clip top of raster */
    if (dispy < 0) ssy = 0;
    else ssy = dispy;
    scy = ssy - dispy;  /* scy = number of lines clipped from top */
    h = high - scy; /* h = number of lines onscreen */
/* clip bottom of raster */
    if (h + ssy > RHeight) h = RHeight - ssy;

    dest->rY = ssy;
    src->rH = h;
    src->rY = scy;

/* clip left */
    if (dispx < 0) ssx = 0;
    else ssx = dispx;
    scx = ssx - dispx;
    w = wide - scx;
/* clip right */
    if (w + ssx > RWidth) w = RWidth - ssx;

    dest->rX = ssx;
    src->rW = w;
    src->rX = scx;
 
    if (h <= 0 || w <= 0)
        {
#ifdef DEBUG
if (Debug & GELBUG) printf("setRects VSprite totally clipped\n");
#endif
        return(0);
        }
    else
        {
        return(1);
        }
}

#ifndef SQUEEZE

void __regargs sortVSprite(struct VSprite *s)
{
    struct VSprite *t;
    LONG q;

    q = *(LONG*)(&s->Y);  /* Here's why the y,x ordering.  Combined sort  */
    if (q < ( *(LONG *)(&s->PrevVSprite->Y))) 
    {   
	/* VSprite moves up or left */
	/* go backwards through list to put VSprite in new place */

        t = s->PrevVSprite;
        do t = t->PrevVSprite; while (q < (*(LONG*)(&t->Y)));
        /* extract s from list */
        s->NextVSprite->PrevVSprite = s->PrevVSprite;
        s->PrevVSprite->NextVSprite = s->NextVSprite;
        /* reinsert into list */
        t->NextVSprite->PrevVSprite = s;
        s->PrevVSprite = t;
        s->NextVSprite = t->NextVSprite;
        t->NextVSprite = s;
    }
    else if (q > (*(LONG*)(&s->NextVSprite->Y)))
	{
	    /* VSprite moves down or right */
	    t = s->NextVSprite;
	    do t = t->NextVSprite; while (q > (*(LONG*)(&t->Y)));
	    /* extract s from list */
	    s->PrevVSprite->NextVSprite = s->NextVSprite;
	    s->NextVSprite->PrevVSprite = s->PrevVSprite;
	    /* reinsert into list */
	    t->PrevVSprite->NextVSprite = s;
	    s->NextVSprite = t;
	    s->PrevVSprite = t->PrevVSprite;
	    t->PrevVSprite = s;
	}

    /* else object didn't change position */
}

#endif

/* ************************************************************************ */

/****** graphics.library/AddVSprite *******************************************
*
*   NAME
*	AddVSprite -- Add a VSprite to the current gel list.
*
*   SYNOPSIS
*	AddVSprite(vs, rp)
*	           A0  A1
*
*	void AddVSprite(struct VSprite *, struct RastPort *);
*
*   FUNCTION
*	Sets up the system VSprite flags
*	Links this VSprite into the current gel list using its Y,X
*
*   INPUTS
*	vs = pointer to the VSprite structure to be added to the gel list
*	rp = pointer to a RastPort structure
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	InitGels()  graphics/rastport.h  graphics/gels.h
*
*****************************************************************************/

void __asm AddVSprite(register __a0 struct VSprite *VS,
		      register __a1 struct RastPort *RPort)
{
    struct VSprite *p, *q;
    LONG t;

    VS->Flags &= SUSERFLAGS;

#ifdef DEBUG
if (Debug & GELBUG) printf("_AddVSprite(%lx, %lx, %lx)\n", VS, RPort );
#endif

    t = *(LONG *)(&VS->Y);   /* hold (Y, X) position */
    q = RPort->GelsInfo->gelHead;
    p = q->NextVSprite;

#ifdef DEBUG
if (Debug & GELBUG) printf("  initial q=%lx p=%lx t=%lx\n", q, p, t);
#endif

/* insert the new VSprite into the list based on its
*   vertical position with (0, 0) being above (0, 1)
*/

    while (*(LONG*)(&p->Y) < t)
    {
        q = p;
        p = p->NextVSprite;

#ifdef DEBUG
if (Debug & GELBUG) printf("  q=%lx p=%lx\n", q, p);
#endif

    }
    VS->NextVSprite = q->NextVSprite;
    q->NextVSprite->PrevVSprite = VS;
    q->NextVSprite = VS;
    VS->PrevVSprite = q;
    *(LONG*)(&VS->OldY) = t;
}

/****** graphics.library/AddBob ***********************************************
*
*   NAME
*	AddBob -- Adds a Bob to current gel list.
*
*   SYNOPSIS
*	AddBob(Bob, rp)
*	       A0   A1
*
*	void AddBob(struct Bob *, struct RastPort *);
*
*   FUNCTION
*	Sets up the system Bob flags, then links this gel into the list
*	via AddVSprite.
*
*   INPUTS
*	Bob = pointer to the Bob structure to be added to the gel list
*	rp  = pointer to a RastPort structure
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	InitGels()  AddVSprite()  graphics/gels.h  graphics/rastport.h
*
*****************************************************************************/
void __asm AddBob(register __a0 struct Bob *b, register __a1 struct RastPort *RPort )
{

#ifdef DEBUG
if (Debug & GELBUG) printf("_AddBob(%lx, %lx, %lx)\n", b, RPort );
#endif

    b->Flags &= BUSERFLAGS;
    AddVSprite(b->BobVSprite, RPort );
}


/****** graphics.library/DoCollision ******************************************
*
*   NAME
*	DoCollision -- Test every gel in gel list for collisions.
*
*   SYNOPSIS
*	DoCollision(rp)
*	            A1
*
*	void DoCollision(struct RastPort *);
*
*   FUNCTION
*	Tests each gel in gel list for boundary and gel-to-gel collisions.
*	On detecting one of these collisions, the appropriate collision-
*	handling routine is called. See the documentation for a thorough
*	description of which collision routine is called. This routine
*	expects to find the gel list correctly sorted in Y,X order.
*	The system routine SortGList performs this function for the user.
*
*   INPUTS
*	rp = pointer to a RastPort
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	InitGels()  SortGList()  graphics/gels.h  graphics/gels.h
*
*****************************************************************************/
void __asm DoCollision(register __a1 struct RastPort *RPort )   /* check for all collisions in list */
{
    struct GelsInfo *gi;
    struct VSprite *s, *end;
    BOOL bflag;
    WORD b;
    struct Layer *layer = RPort->Layer;

    if( layer ) 
    {
	LOCKLAYER( layer );
    }

    gi = RPort->GelsInfo;
    s = gi->gelHead;
    end = gi->gelTail;
    bflag = (LONG)(gi->collHandler->collPtrs[BORDERHIT]) != 0;
    while ((s = s->NextVSprite) != end)
    {
	struct VSprite tmp_sprite, *t=s;

	if((layer) && (t->Flags & VSPRITE)) 
	{
	    tmp_sprite = *s;
	    t = &tmp_sprite;
	    t->X -= (layer->bounds.MinX - layer->Scroll_X);
	    t->Y -= (layer->bounds.MinY - layer->Scroll_Y);
	}

        if (bflag) if (t->HitMask | BORDERHIT) 
	{
	    if (b = bndryHit(t, gi ))
	    {
		if(s->VSBob && ((s->VSBob)->Flags & BOBSAWAY));
		else (*gi->collHandler->collPtrs[BORDERHIT])(s, b);
	    }
	}
        downCheck(t, gi, s );
    }

    if( layer ) 
    {
	UNLOCKLAYER( layer );
    }
}


/****** graphics.library/InitGels ***********************************************
*
*   NAME
*	InitGels -- initialize a gel list; must be called before using gels.
*
*   SYNOPSIS
*	InitGels(head, tail, GInfo)
*	         A0    A1    A2
*
*	void InitGels(struct VSprite *, struct VSprite *, struct GelsInfo *);
*
*   FUNCTION
*	Assigns the VSprites as the head and tail of the gel list in GfxBase.
*	Links these two gels together as the keystones of the list.
*	If the collHandler vector points to some memory array, sets
*	the BORDERHIT vector to NULL.
*
*   INPUTS
*	head  = pointer to the VSprite structure to be used as the gel list head
*	tail  = pointer to the VSprite structure to be used as the gel list tail
*	GInfo = pointer to the GelsInfo structure to be initialized
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	graphics/gels.h  graphics/rastport.h
*
*******************************************************************************/
void __asm InitGels(register __a0 struct VSprite *sHead,
					register __a1 struct VSprite *sTail,
					register __a2 struct GelsInfo *GInfo)

{   /* soft VSprite data structure init */

#ifdef DEBUG
if (Debug & GELBUG) printf("_InitGels(%lx, %lx, %lx, %lx)\n", sHead, sTail, GInfo );
#endif

    GInfo->gelHead = sHead;
    GInfo->gelTail = sTail;

    sHead->NextVSprite = sTail;
    sHead->ClearPath = sTail;
    sHead->PrevVSprite = sTail->NextVSprite = 0;
    sTail->PrevVSprite = sHead;

    sTail->Y = 32767;
    sTail->OldY = 32767;
    sHead->Y = -32767;
    sHead->OldY = -32767;
    sTail->X = 32767;
    sTail->OldX = 32767;
    sHead->X = -32767;
    sHead->OldX = -32767;

/* initialize border collision response vector to NULL */
    if (GInfo->collHandler)
        GInfo->collHandler->collPtrs[BORDERHIT] = 0;
}

/****** graphics.library/InitMasks *********************************************
*
*   NAME
*	InitMasks -- Initialize the BorderLine and CollMask masks of a VSprite.
*
*   SYNOPSIS
*	InitMasks(vs)
*	          A0
*
*	void InitMasks(struct VSprite *);
*
*   FUNCTION
*	Creates the appropriate BorderLine and CollMask masks of the VSprite.
*	Correctly detects if the VSprite is actually a Bob definition, handles
*	the image data accordingly.
*
*   INPUTS
*	vs = pointer to the VSprite structure
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	InitGels()  graphics/gels.h
*
*****************************************************************************/

void __asm InitMasks(register __a0 struct VSprite *s)
{
    WORD i, j;
    WORD *sptr, *sptr2;
    WORD size;

    /* get word count per Planes */
    size = umuls(s->Height, s->Width);

    /* create the one-dimensional boundry-collision mask */

	/* bart - 03.12.86 */
	/* combined clear op with loop below */

	/* sptr2 = s->BorderLine;*/	/* point to area reserved for mask */
	/*for (i=1; i <= s->Width; i++)  *sptr2++ = 0;*/ /* clear borderline */

	/* end bart - 03.12.86 */

	sptr2 = s->BorderLine;	/* point to area reserved for mask */
	for (i=1; i <= s->Width; i++)
	{
	    /* bart - 03.12.86 */
	    /* combined clear op with this loop right here */
	    *sptr2 = 0; 
	    /* end bart - 03.12.86 */

	    sptr = (WORD *) ((BYTE *)(s->ImageData) + ((i-1)<<1));
	    /* sptr = top of image plus appropriate word offset  */
	    for (j=1; j <= s->Height; j++)
	    {	
		*sptr2 |= *sptr;
		sptr += s->Width;  /* next word is 1 Width away */
	    }
	    sptr2++;  /* next word of mask  */
	}


    /* create the two-dimensional shadow mask */

    if (s->Flags & VSPRITE) /* is this a VSprite? */
    {
        sptr = s->ImageData;
        sptr2 = s->CollMask;
	for (j = size; j; j--) *sptr2++ = *sptr++ | *sptr++;
    }
    else /* it's a Bob */
    {
        sptr = s->ImageData;
        sptr2 = s->CollMask;
        for (j = size; j; j--) *sptr2++ = *sptr++;

        for (i = s->Depth - 1; i; i--)  /* for the subsequent Depths */
	{
            sptr2 = s->CollMask;
            for (j = size; j; j--) *sptr2++ |= *sptr++;
	}
    }
}

/****** graphics.library/RemVSprite ******************************************
*
*   NAME
*	RemVSprite -- Remove a VSprite from the current gel list.
*
*   SYNOPSIS
*	RemVSprite(vs)
*	           A0
*
*	void RemVSprite(struct VSprite *);
*
*   FUNCTION
*	Unlinks the VSprite from the current gel list.
*
*   INPUTS
*	vs = pointer to the VSprite structure to be removed from the gel list
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	InitGels()  RemIBob()  graphics/gels.h
*
*****************************************************************************/

void __asm RemVSprite(register __a0 struct VSprite *s)
{
    s->PrevVSprite->NextVSprite = s->NextVSprite;
    s->NextVSprite->PrevVSprite = s->PrevVSprite;
}

/****** graphics.library/RemBob **********************************************
*
*   NAME
*	RemBob -- Macro to remove a Bob from the gel list.
*
*   SYNOPSIS
*	RemBob(bob)
*
*	RemBob(struct Bob *);
*
*   FUNCTION
*	Marks a Bob as no-longer-required.  The gels internal code then
*	removes the Bob from the list of active gels the next time
*	DrawGList is executed. This is implemented as a macro.
*	If the user is double-buffering the Bob, it could take two
*	calls to DrawGList before the Bob actually disappears from
*	the RastPort.
*
*   INPUTS
*	Bob = pointer to the Bob to be removed
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	RemIBob()  DrawGList()  graphics/gels.h  graphics/gfxmacros.h
*
*****************************************************************************/

/****** graphics.library/RemIBob **********************************************
*
*   NAME
*	RemIBob -- Immediately remove a Bob from the gel list and the RastPort.
*
*   SYNOPSIS
*	RemIBob(bob, rp, vp)
*	        A0   A1  A2
*
*	void RemIBob(struct Bob *, struct RastPort *, struct ViewPort *);
*
*   FUNCTION
*	Removes a Bob immediately by uncoupling it from the gel list and
*	erases it from the RastPort.
*
*   INPUTS
*	bob = pointer to the Bob to be removed
*	rp  = pointer to the RastPort if the Bob is to be erased
*	vp  = pointer to the ViewPort for beam-synchronizing
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	InitGels()  RemVSprite()  graphics/gels.h
*
*****************************************************************************/
void __regargs _RemIBob(struct Bob *b, struct RastPort *RPort, struct ViewPort *VPort)
{
    struct VSprite *bs;

    bs = b->BobVSprite;
    if (bs->Flags & BACKSAVED) clrBob(bs, RPort, VPort );
    RemVSprite(bs);
    b->Flags |= BOBNIX;
}

/****** graphics.library/SetCollision *****************************************
*
*   NAME
*	SetCollision -- Set a pointer to a user collision routine.
*
*   SYNOPSIS
*	SetCollision(num, routine, GInfo)
*	             D0   A0       A1
*
*	void SetCollision(ULONG, VOID (*)(), struct GelsInfo *);
*
*   FUNCTION
*	Sets a specified entry (num) in the user's collision vectors table
*	equal to the address of the specified collision routine.
*
*   INPUTS
*	num     = collision vector number
*	routine = pointer to the user's collision routine
*	GInfo   = pointer to a GelsInfo structure
*
*   RESULT
*
*   BUGS
* 
*   SEE ALSO
*	InitGels()  graphics/gels.h  graphics/rastport.h
*
*****************************************************************************/

void __asm SetCollision(register __d0 int h,register __a0 int (*p)(),
						register __a1 struct GelsInfo *GInfo)
{
    GInfo->collHandler->collPtrs[h] = p;
}

/****** graphics.library/SortGList *********************************************
*
*   NAME
*	SortGList -- Sort the current gel list, ordering its y,x coordinates.
*
*   SYNOPSIS
*	SortGList(rp)
*	          A1
*
*	void SortGList(struct RastPort *);
*
*   FUNCTION
*	Sorts the current gel list according to the gels' y,x coordinates.
*	This sorting is essential before calls to DrawGList or DoCollision.
*
*   INPUTS
*	rp = pointer to the RastPort structure containing the GelsInfo
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	InitGels()  DoCollision()  DrawGList()  graphics/rastport.h
*
*****************************************************************************/
void __asm SortGList(register __a1 struct RastPort *RPort)
{
    struct VSprite *s, *q, *send;
    /* struct VSprite *t; */

#ifdef DEBUG
	printf("RPOrt=%lx\n",RPort);
#endif

    s = RPort->GelsInfo->gelHead;
    q = s->NextVSprite;
    send = RPort->GelsInfo->gelTail;
    /* skip the head VSprite */
    while (q != send)
    {
        s = q;
        q = q->NextVSprite;
        sortVSprite(s);
    }
}

/* bart - 09.24.85 user call to RemIBob() vectored through here 	   */
/* this routine calls _RemIBob and then cleans up blissObjs left on queue  */

void __asm RemIBob(register __a0 struct Bob *b,
				register __a1 struct RastPort *RPort,
				register __a2 struct ViewPort *VPort)
{
    /* struct VSprite *bs; */
    struct GelsInfo *gi;

    gi = RPort->GelsInfo;

    /* initialize list of allocated blissObj's to zero */
    gi->firstBlissObj = 0;
    gi->lastBlissObj = 0;

    /* call c routine to remove the bob immediately */
    _RemIBob(b, RPort, VPort );

    /* now clean up */
    byebyeblissobjs(gi);

    /* done, so return to userproc */

}

/* end bart - 09.24.85 							*/

