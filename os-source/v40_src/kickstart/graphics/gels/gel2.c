/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: gel2.c,v 39.3 92/08/17 13:46:03 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	gel2.c,v $
*   Revision 39.3  92/08/17  13:46:03  chrisg
*   fix wait pos for scandoubled screens.
*   
*   Revision 39.2  92/05/05  12:21:08  chrisg
*   FETCHGBASE, regargs, etc.
*   
*   Revision 39.1  92/02/21  19:50:19  bart
*   process interleaved bitmaps for V39
*   
*   Revision 39.0  91/08/21  17:34:17  chrisg
*   Bumped
*   
*   Revision 37.5  91/05/22  12:15:54  chrisg
*     Fixed bug introduced during native build--changed UCopperListInit back to a cinit.
*   this bug killed gels.
*   
*   Revision 37.4  91/05/15  13:35:35  chrisg
*    added c_protos.h to include and changed ucinit to ucopperlistinit
*   
*   Revision 37.3  91/05/13  14:36:54  chrisg
*   removed compiler warnings for lattice
*   made use prototypes
*   got rid of interface code.. made routines take
*   arguments in registers.
*   
*   Revision 37.2  91/05/02  16:50:33  chrisg
*   killed ".." for lattice
*   
*   Revision 37.1  91/02/12  15:49:31  spence
*   autodocs
*   
*   Revision 37.0  91/01/07  15:22:20  spence
*   initial switchover from V36
*   
*   Revision 33.5  90/12/06  13:50:05  bart
*   removed gfxallocmem.c and implemented better lo-mem error recovery -- bart
*   
*   Revision 33.4  90/07/27  16:37:52  bart
*   id
*   
*   Revision 33.3  90/03/28  09:26:49  bart
*   *** empty log message ***
*   
*   Revision 33.2  89/07/24  16:22:33  bart
*   better beamsyncs
*   
*   Revision 33.1  88/01/25  10:14:34  bart
*   removed temporary patch
*   
*   Revision 33.0  86/05/17  15:23:28  bart
*   added to rcs for updating
*   
*
******************************************************************************/


/*** gel2.c *******************************************************************
*
*   File
*
*   Confidential Information: Amiga Computer, Inc.
*   Copyright (c) Amiga Computer, Inc.
*
*		   Modification History
*   date    :   author :    Comments
*  -------      ------      ---------------------------------------
*   9-28-84     -=RJ=-      added this header file
*			   commented out all EXEC14 stuff
*  10-23-84     Dale	start conversion for GFX21
*
*****************************************************************************/


#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/gels.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/copper.h>
#include <graphics/clip.h>
#include "/macros.h"
#include "/c/tune.h"
#include "gelsinternal.h"
#include <hardware/custom.h>

#include "gels.protos"
#include "/c/c.protos"
#include "/gfxpragmas.h"

#define BLITTERSIGNAL	4
#define USE_CEND
#define BARTDEBUG
#define GEL2BUG		4

extern struct Custom custom;

/*#define DEBUG 1*/


/* ************************************************************************ */

void __regargs enqSprite(WORD pick, struct VSprite *s, struct gelRect *src,
			struct gelRect *dest, struct CopList **sprcl,
			struct CopList **clrcl, struct RastPort *RPort,
			struct ViewPort *VPort)
{
    struct GelsInfo *gi;
    WORD t, tx, ty;
    WORD *nextline;
    struct CopList *cl; /* We only operate on one list at a time */
    struct CopIns *c;
    WORD *image;

#ifdef DEBUG
if (Debug & GEL2BUG)printf("enq: %ld %lx %lx %lx \n",pick,s,src,dest);
#endif

    gi = RPort->GelsInfo;
    cl = *sprcl;
    c = cl->CopPtr;

/* get address of nextline array */
    nextline = gi->nextLine + pick;

/* get physical address of start of image */
    image = s->ImageData + umuls(s->Depth, src->rY);

/* wait until the next available line */
    /* we assume that a copper list exists! and there is space for
	one more instruction After a CBUMP */
    CWAIT(c, *nextline, 0);
    CBUMP(sprcl);
    CMOVE(c,custom.sprpt[pick],TOBB((WORD)(((LONG)image) >> 16)));
    /*CMOVE(c,sprpt[pick],TOBB((WORD)(image) >> 16));*/
    CBUMP(sprcl);
    CMOVE(c, custom.sprpt[pick], TOBB((WORD)(image)));
    c->u3.u4.u1.DestAddr += 2;
    CBUMP(sprcl);

    /* we assume the multiple viewport are similar here */
    tx = GBASE->ActiView->DxOffset;/* let hardware do clipping */

    /* might be a nice future enhancement to support 70ns sprite positioning */
    if (SUPERHIRES & new_mode(VPort)) tx += (s->X>>2); /* new mode for 1.4   */
    else if (HIRES & new_mode(VPort)) tx += (s->X>>1); /* hardware is lores  */
    else tx += s->X;

    /* following line may be bogus if interlaced */
    ty = dest->rY + GBASE->ActiView->DyOffset;
    t = ty + src->rH;

    /* send start position */
    CMOVE(c, custom.spr[pick].pos,(ty << 8) | ((tx >> 1) & 0xFF));
    CBUMP(sprcl);
    CMOVE(c, custom.spr[pick].ctl,(t << 8) | (0x4 & (ty >> 6))
	    | (0x2 & (t >> 7)) | (0x1 & tx));
    CBUMP(sprcl);

    cl = *sprcl;
    cl->CopPtr = c;

    *nextline = dest->rY + src->rH + 2;
/*???    *nextline = dest->rY + src->rH + 3;*/

/* has it got a color address? */
    if (s->SprColors)
    {
/*???tried putting this in queueSprite*/
/*???	*(gi->lastColor + pick) = s->SprColors;*/
	*(gi->lastColor + pick) = s->SprColors;

	/* wait until just Before VSprite to set color registers */
	cl = *clrcl;
	c = cl->CopPtr;
	CWAIT(c, dest->rY - 1, 0);
	CBUMP(clrcl);

	t = 17 + ((pick & 0x6) << 1);
	CMOVE(c,custom.color[t],s->SprColors[0]);
	CBUMP(clrcl);
	CMOVE(c,custom.color[++t],s->SprColors[1]);
	CBUMP(clrcl);
	CMOVE(c,custom.color[++t],s->SprColors[2]);
	CBUMP(clrcl);
	cl = *clrcl;
	cl->CopPtr = c;
    }
}


BOOL __regargs queueSprite(struct VSprite *s, struct CopList **sprcl,
				struct CopList **clrcl,
				 struct RastPort *RPort,struct ViewPort *VPort)

{
    struct GelsInfo *gi;
    /* WORD i, h, w, spritenum, othernum; */ /* bart - removed h,w */
    WORD i, spritenum, othernum;
    WORD *nextline;
    WORD **lastcolor;
    WORD *scolor;
    BOOL found, first, second;
    BYTE reserved;
    WORD ystart;
    struct gelRect sGR, dGR;

    /* h = RPort->BitMap->Rows; */
    /* w = (RPort->BitMap->BytesPerRow << 3); */

    gi = RPort->GelsInfo;
    if (!setRects(s->Height, s->Width, &sGR, &dGR, s->X, s->Y, 
	RPort->BitMap->Rows, 
	/* convert from V37 to V39 bitmaps -- bart */
	/* (RPort->BitMap->BytesPerRow << 3) */
	gfx_GetBitMapAttr(RPort->BitMap,BMA_WIDTH) ) )
    {
	s->Flags |= GELGONE;
	return(FALSE);
    }

    s->Flags &= ~GELGONE;

    ystart = dGR.rY;
    found = FALSE;
    nextline = gi->nextLine;
    lastcolor = gi->lastColor;
    scolor = s->SprColors;

    /* search for an available VSprite processor using this algorithm:
     *	- search for any half-taken sprite pairs.  for each found:
     *		- if this VSprite has no color preference, or
     *		- if the half-pair has no color preference, or
     *		- if the color preferences are equal, then
     *		- use this sprite (could be better, would take extra loops)
     * - if no placed yet, search for each unused pair, fit it there, set the
     *   color for that pair
     */

    /* do not use sprites being used by simple sprite machine */

    reserved = gi->sprRsrvd  & (~GBASE->SpriteReserved);
    for(i = 0; i < 8; i += 2)
    {
	first = (nextline[i] < ystart) && (reserved & 1);
	second = (nextline[i + 1] < ystart) && (reserved & 2);
	if ((first && !second) || (!first && second))
	{
	    if (first)
	    {
		spritenum = i;
		othernum = i + 1;
	    }
	    else
	    {
		spritenum = i + 1; 
		othernum = i;
	    }
	    if ((scolor == NULL)
		|| (lastcolor[othernum] == NULL)
		|| (lastcolor[othernum] == scolor))
	    {

#ifdef DEBUG
if (Debug & GEL2BUG)
    printf("halfpair: num=%ld other=%ld scolor=%lx lc=%lx olc=%lx\n",
	spritenum, othernum, scolor, lastcolor[spritenum], lastcolor[othernum]
	); 
#endif
		found = TRUE; 
		lastcolor[spritenum] = scolor;
		enqSprite(spritenum, s, &sGR, &dGR, sprcl, clrcl, 
			RPort, VPort ); 
		goto SPRITEFOUND;
	    }
	}

	reserved >>= 2;
    }

    /* no half-pairs found, so try for any full pairs now */

    reserved = gi->sprRsrvd  & (~GBASE->SpriteReserved);
    for(i = 0; i < 8; i += 2)
	{
	first = (nextline[i] < ystart) && (reserved & 1);
	second = (nextline[i + 1] < ystart) && (reserved & 2);
	if (first && second)
	    {
#ifdef DEBUG
if (Debug & GEL2BUG)
    printf("fullpair: num=%ld scolor=%lx\n", i, scolor);
#endif
	    found = TRUE; 
	    lastcolor[i] = scolor;
	    enqSprite(i, s, &sGR, &dGR, sprcl, clrcl, RPort, VPort ); 
	    goto SPRITEFOUND;
	    }
	reserved >>= 2;
	}

/* Oh well, no sprites available! */
#ifdef DEBUG
    if (Debug & GEL2BUG) printf("   VSprite not queued\n");
#endif

    if (s->Flags & MUSTDRAW)
	{
	s->Flags |= VSOVERFLOW;
/*??? if !found & MUSTDRAW then try draw, set VSOVERFLOW TRUE if no getmem */
	}
    else s->Flags |= VSOVERFLOW;
    return(FALSE);

SPRITEFOUND:
    return(TRUE);
}

int __regargs new_bsync(WORD sync, struct ViewPort *vp)
{
    ULONG modes = new_mode(vp);
    WORD  viewoffset;

    viewoffset = GBASE->ActiView->DyOffset;

    if (modes & DOUBLESCAN)
	sync<<=1;

    sync += (viewoffset + (vp->DyOffset - vp->RasInfo->RyOffset));


    if (modes & LACE)
    {
	sync += viewoffset;
	sync >>= 1;
    }

    return((int) sync);
}

/* ************************************************************************ */

void __regargs clrBob(struct VSprite *s, struct RastPort *RPort, struct ViewPort *VPort)
/* expects that caller has already checked that BACKSAVED is set */
{
    struct Bob *bptr;
    struct gelRect src, dest;
    register struct VSprite *t;
    WORD bsync;
    WORD w; /* bart - added w */
    struct VSprite *endVSprite;
    WORD sx, sy, bottomedge, rightedge;
    WORD sh,sw; /* bart - added sh, sw */
    WORD tx, ty;
    BOOL onscreen;

    /* struct GfxBase *GB;

    FETCHGBASE; */

	/* w = RPort->BitMap->BytesPerRow << 3; */      /* replaced old V37 bitmap */
    w = gfx_GetBitMapAttr(RPort->BitMap,BMA_WIDTH); /* new V39 bitmaps -- bart */ 

#ifdef DEBUG
    if (Debug & GEL2BUG) printf("clrBob s=%lx x=%ld y=%ld cP=%lx\n", s, s->OldX, s->OldY, s->ClearPath);
#endif

    bptr = s->VSBob;
    endVSprite = RPort->GelsInfo->gelTail;

    /* make sure all VSprites that may overlap it are cleared first */

    sx = s->OldX;  sy = s->OldY;
    sh = s->Height; sw = s->Width; /* bart - added sh, sw */

    /* rightedge = sx + (s->Width << 4) - 1;  bottomedge = sy + s->Height - 1;*/
    /* bart - use sw, sh */
    rightedge = sx + (sw << 4) - 1;  bottomedge = sy + sh - 1;

    for(t = s->ClearPath; t != endVSprite; t = t->ClearPath)
	if (t->Flags & BACKSAVED)
	{
	    /* test for x-axis overlap */
	    tx = t->OldX;
	    if ((sx <= (tx + (t->Width << 4) - 1)) && (tx <= rightedge))
	    {
		/* OK, so now test for y-axis overlap */
		ty = t->OldY;
		if ((sy <= (ty + t->Height - 1)) && (ty <= bottomedge))
		    /* OK, so go erase it */
		    clrBob(t, RPort, VPort );
	    }
	}

/* If this Bob's background wasn't preserved, return */
    if ((s->Flags & BACKSAVED) == 0) return;
    s->Flags &= ~BACKSAVED;

/* Test if we want to skip erasing this Bob and return if so */
    if (bptr->Flags & SAVEBOB) return;

    src.rAddr = (void *) bptr->SaveBuffer;

    /* onscreen = setRects(s->Height, s->Width, &src, &dest, s->OldX, s->OldY,
	    RPort->BitMap->Rows , w ); */
    /* bart - use sh, sw, sx, sy, w */ 
    onscreen = setRects(sh, sw, &src, &dest, sx, sy, RPort->BitMap->Rows , w );

    if (onscreen)
	{
	    struct Layer *layer;

	    /* bsync = s->OldY + s->Height + GBASE->ActiView->DyOffset
		    + VPort->DyOffset; */ /* bart - use sy, sh  */

	    /* bsync = sy + sh + GBASE->ActiView->DyOffset
		    + VPort->DyOffset; */

	    bsync = sy+sh;

	    if (layer = RPort->Layer)	bsync += layer->bounds.MinY
						  - layer->Scroll_Y;

	    if((bsync = new_bsync(bsync,VPort)) < 0xc) bsync = 0xc; /* min */

	    /* src.rRealWW = s->Width; */ /* bart - use sw */
	    src.rRealWW = sw;

	    /* dest.rRealWW = RPort->BitMap->BytesPerRow >> 1; */
	    /* bart - use w */
	    /* dest.rRealWW = w >> 4; */
		/* bart - back to physical for V39 */
	    dest.rRealWW = RPort->BitMap->BytesPerRow >> 1; 

	    blisslicer(&src,&dest,RPort, s, BLOCKCUT, B2NORM, bsync );
	}

}


void __regargs drawBob(struct VSprite *s, struct VSprite **sLastDrawn,
			struct RastPort *RPort, struct ViewPort *VPort)
/* bliss the Bob into the raster */
{
    struct GelsInfo *gi;
    struct gelRect src, dest;
    register struct VSprite *t;
    struct VSprite *endVSprite;
    WORD onscreen;
    WORD h, w; /* bart - added h, w */
    struct Bob *b;
    WORD bsync;
    WORD *flagptr;
    WORD sx, sy, bottomedge, rightedge;
    WORD sh,sw; /* bart - added sh, sw */
    WORD tx, ty;
    struct Layer *layer;

    /* struct GfxBase *GB;

    FETCHGBASE; */

    h = RPort->BitMap->Rows;
	/* w = RPort->BitMap->BytesPerRow << 3; */      /* replaced old V37 bitmap */
    w = gfx_GetBitMapAttr(RPort->BitMap,BMA_WIDTH); /* new V39 bitmaps -- bart */ 

    b = s->VSBob;

#ifdef DEBUG
    if (Debug & GEL2BUG) printf("drawBob b=%lx s=%lx x=%ld y=%ld ox=%ld oy=%ld\n", b, s, s->X, s->Y, s->OldX, s->OldY);
#endif

    if (b->After) if ((b->After->Flags & BDRAWN) == 0)
    {
	b->Flags |= BWAITING;
	return;
    }

    gi = RPort->GelsInfo;
    endVSprite = gi->gelTail;
    flagptr = &s->Flags;

    sx = s->X;  sy = s->Y; /* bart - moved here from below */
    sh = s->Height;  sw = s->Width; /* bart - added sh, sw */

    /* src.rRealWW = s->Width; */ /* bart - use sw */
    src.rRealWW = sw;

    /* dest.rRealWW = RPort->BitMap->BytesPerRow >> 1; */ /* bart - use w */
    /* dest.rRealWW = w >> 4; */
	/* bart - back to physical for V39 */
    dest.rRealWW = RPort->BitMap->BytesPerRow >> 1; 

    /* bsync = s->Y + s->Height + GBASE->ActiView->DyOffset + VPort->DyOffset; */
    /* bart - use sh */
    /* bsync = s->Y + sh + GBASE->ActiView->DyOffset + VPort->DyOffset; */

    bsync = sy + sh;

    if (layer = RPort->Layer)   bsync += layer->bounds.MinY  
					  - layer->Scroll_Y;

    if((bsync = new_bsync(bsync,VPort)) < 0xc) bsync = 0xc; /* min */

    /* bart - use sh, sw, sx, sy, h, w */
    /* onscreen = setRects(s->Height, s->Width, &src, &dest, s->X, s->Y,
	    h , w ); */

    onscreen = setRects(sh, sw, &src, &dest, sx, sy,
	    h , w ); 

    if (onscreen)
	s->Flags &= ~GELGONE;
    else
	s->Flags |= GELGONE;
	
    /* sx = s->X;  sy = s->Y; */ /* bart - set above */

    /* bart - use sw, sh */
    /* rightedge = sx + (s->Width << 4) - 1;  bottomedge = sy + s->Height - 1;*/
    /* bart - only set if needed in loop below - 04.01.86 */
    /* rightedge = sx + (sw<< 4) - 1;  bottomedge = sy + sh- 1; */

    if (*flagptr & SAVEBACK)
    {
	/* bart - only set if needed in loop - 04.01.86 */
	rightedge = sx + (sw<< 4) - 1;  bottomedge = sy + sh- 1;

	if (*flagptr & BACKSAVED) clrBob(s, RPort, VPort );
	for(t = gi->gelHead->NextVSprite; t != endVSprite; t = t->NextVSprite)
	    if (t->Flags & BACKSAVED)
	    {
		/* test for x-axis overlap */
		tx = t->OldX;
		if (sx <= (tx + (t->Width << 4) - 1) && tx <= rightedge)
		{
		    /* OK, so now test for y-axis overlap */
		    ty = t->OldY;
		    if (sy <= (ty + t->Height - 1) && ty <= bottomedge)
			/* OK, so go erase it */
			clrBob(t, RPort, VPort );
		}
	    }

	/* save away the raster */

	if (onscreen)
	{
	    /* src.rAddr = s->VSBob->SaveBuffer; */ /* bart - use b */
	    src.rAddr = (void *) b->SaveBuffer;

	    blisslicer(&src, &dest, RPort, s, BLOCKCUT, B2SWAP, bsync );
	    *flagptr |= BOBUPDATE;

	    /* bart - reset src, dest rect if bliss occurred - 04.01.86 */ 
	    onscreen = setRects(sh, sw, &src, &dest, sx, sy,
		    h , w ); 
	}
    }

    /* bart - use sh, sw, sx, sy, h, w */
    /* onscreen = setRects(s->Height, s->Width, &src, &dest, s->X, s->Y,
	    h, w); */

    /* bart - reset src, dest rect only if bliss occurred - 04.01.86 */ 
    /* onscreen = setRects(sh, sw, &src, &dest, sx, sy,
	    h, w); */
    /* bart - see above */

    if (onscreen)
    {
	*sLastDrawn = (*sLastDrawn)->DrawPath = s;
	src.rAddr = (void *) s->ImageData;
	if (*flagptr & OVERLAY)
	    blisslicer(&src, &dest, RPort, s, SHADOWCUT,
		    B2BOBBER, bsync );
	else 
	    blisslicer(&src, &dest, RPort, s, BLOCKCUT,
		    B2BOBBER, bsync );
    }

    b->Flags = (b->Flags & ~BWAITING) | BDRAWN;

    if (b->Before) if (b->Before->Flags & BWAITING)
	drawBob(b->Before->BobVSprite, sLastDrawn, RPort, VPort );

}

/* ************************************************************************* */

/****** graphics.library/DrawGList *********************************************
*
*   NAME
*	DrawGList -- Process the gel list, queueing VSprites, drawing Bobs.
*
*   SYNOPSIS
*	DrawGList(rp, vp)
*	          A1  A0
*
*	void DrawGList(struct RastPort *, struct ViewPort *);
*
*   FUNCTION
*	Performs one pass of the current gel list.
*	   - If nextLine and lastColor are defined, these are
*	     initialized for each gel.
*          - If it's a VSprite, build it into the copper list.
*          - If it's a Bob, draw it into the current raster.
*          - Copy the save values into the "old" variables,
*	     double-buffering if required.
*
*   INPUTS
*	rp = pointer to the RastPort where Bobs will be drawn
*	vp = pointer to the ViewPort for which VSprites will be created
*
*   RESULT
*
*   BUGS
*	MUSTDRAW isn't implemented yet.
*
*   SEE ALSO
*	InitGels()  graphics/gels.h graphics/rastport.h  graphics/view.h
*
*****************************************************************************/
void __asm DrawGList(register __a1 struct RastPort *RPort,
				register __a0 struct ViewPort *VPort )
{
    register struct VSprite *s;
    struct VSprite *endVSprite;
    WORD i;
    struct VSprite *sLastDrawn; /* pointer to last VSprite drawn */
    BYTE *cptr;
    struct Bob *bptr;
    WORD bf;
    struct DBufPacket *dptr;
    struct CopList *isprcl = 0; /* bart - 10.15.85 */
    struct CopList *iclrcl = 0; /* bart - 10.15.85 */
    struct CopList *sprcl = 0;
    struct CopList *clrcl = 0;
#ifdef COPINIT
#ifndef USE_OLD_LISTS
    struct CopList *tmpsprcl = 0;
    struct CopList *tmpclrcl = 0;
    struct CopIns *c;
#endif
#endif
    struct GelsInfo *gi;
    struct Layer *layer;

    /* bart - 08.21.85 - temporary patch */
    /* bart - 10.07.85 - removed temporary patch */
    /* bart - 11.24.85 - restored temporary patch */
    /* bart - 02.26.86 - removed temporary patch... see QBlit.asm 1.06.86  */
    /* bart - 02.27.86 - restored temporary patch */
    /* bart - 01.25.88 - removed temporary patch */
    /* Forbid(); */ /* bart - 01.25.88 */

    if (layer = RPort->Layer) LOCKLAYER(layer);

    gi = RPort->GelsInfo;

    /* initialize list of allocated blissObj's to zero */

    gi->firstBlissObj = gi->lastBlissObj=  0;

#ifdef DEBUG
    if (Debug & GEL2BUG)
	{
	printf("_DrawGList(%lx, %lx, %lx)", RPort, VPort );
	printf("  VPort=%lx ",VPort);
	}
#endif

/* initialize copper VSprite instruction list */
#define COPINIT
#ifdef COPINIT

#define USE_OLD_LISTS
#ifndef USE_OLD_LISTS
	/* allocate new lists */
	isprcl = sprcl = (struct CopList *)copinit(sprcl,SPRINS);
	if ((sprcl == 0) || (sprcl->MaxCount == 0)) goto AWFUL_GOTO_EXIT;
	iclrcl = clrcl = (struct CopList *)copinit(clrcl,CLRINS);
	if ((clrcl == 0) || (clrcl->MaxCount == 0)) goto AWFUL_GOTO_EXIT;
#else
	/* use existing lists */
	isprcl = sprcl = (struct CopList *)copinit(VPort->SprIns,SPRINS);
	if ((sprcl == 0) || (sprcl->MaxCount == 0)) goto AWFUL_GOTO_EXIT;
	iclrcl = clrcl = (struct CopList *)copinit(VPort->ClrIns,CLRINS);
	if ((clrcl == 0) || (clrcl->MaxCount == 0)) goto AWFUL_GOTO_EXIT;
#endif

#else
    if ((sprcl = VPort->SprIns) == 0)   /* main header already allocated ? */
	{   /* not already set up */
	if ((sprcl = (struct CopList *)GfxAllocMem(sizeof(struct CopList),
		MEMF_PUBLIC)) == 0)
	    {
#ifdef MEMDEBUG
	    printf("no mem for SprInsCopList Head\n");
#endif
			goto AWFUL_GOTO_EXIT;
	    }
	VPort->SprIns = sprcl;
	sprcl->Next = 0;
	if ((sprcl->CopIns = (struct CopIns *)
		GfxAllocMem(sizeof (struct CopIns) * SPRINS, MEMF_PUBLIC)) == 0)
	    {
#ifdef MEMDEBUG
	    printf("no mem for spr ins list\n");
#endif
			goto AWFUL_GOTO_EXIT;
	    }
	sprcl->MaxCount = SPRINS;   /* must leave space for indirect jmp */
	}
    sprcl->Count = 0;
    sprcl->CopPtr = sprcl->CopIns;

    if ((clrcl = VPort->ClrIns) == 0)   /* main header already allocated ? */
	{   /* not already set up */
	if ((clrcl = (struct CopList *)GfxAllocMem(sizeof(struct CopList),MEMF_PUBLIC)) == 0)
	{
#ifdef MEMDEBUG
	    printf("no mem for ClrInsCopList Head\n");
#endif
		goto AWFUL_GOTO_EXIT;
	}

	VPort->ClrIns = clrcl;
	clrcl->Next = 0;
	if ((clrcl->CopIns = (struct CopIns *)GfxAllocMem(sizeof (struct CopIns) * CLRINS,MEMF_PUBLIC)) == 0)
	{
#ifdef MEMDEBUG
	    printf("no mem for clrclr ins list\n");
#endif
		goto AWFUL_GOTO_EXIT;
	}
	clrcl->MaxCount = CLRINS;   /* must leave space for indirect jmp */
	}
    clrcl->Count = 0;
    clrcl->CopPtr = clrcl->CopIns;
#endif

/* the hardware waits till row 20 Before loading the
*   first set of VSprite parameters
*/
    {
	WORD *lptr;
	WORD **cpptr;

	if (lptr = gi->nextLine)
	    for (i = 0; i < 8; i++) *(lptr + i) = -1;
	if (cpptr = gi->lastColor)
	    for (i = 0; i < 8; i++) *(cpptr + i) = 0;
    }

    GBASE->VBlank = 1;

    s = gi->gelHead;
    sLastDrawn = s;
    endVSprite = gi->gelTail;
    while ((s = s->NextVSprite) != endVSprite)
    {

	/* Forbid(); */ /* bart - 03.19.86 */

	if (s->Flags & VSPRITE) queueSprite(s, &sprcl, &clrcl, RPort, VPort );
	else
	{
	    bptr = s->VSBob;
	    bf = bptr->Flags = (bptr->Flags & ~BWAITING) & ~BDRAWN;
	    if (bf & BOBSAWAY)
	    {
		if (bptr->DBuffer)
		{
		    if (bf & OUTSTEP) /* cleared other buffer already */
			_RemIBob(bptr, RPort, VPort );
		    else /* OUTSTEP not set yet, both buffers still active */
		    {
			if (s->Flags & BACKSAVED)
			    clrBob(s, RPort, VPort );
			bptr->Flags |= OUTSTEP;
		    }
		}
		else
		    _RemIBob(bptr, RPort, VPort );
	    }
	    else drawBob(s, &sLastDrawn, RPort, VPort );
	}

	/* Permit(); */ /* bart - 03.19.86 */
    }

    sLastDrawn->DrawPath = gi->gelTail;
    s = gi->gelHead;
    endVSprite = gi->gelTail;
    while ((s = s->NextVSprite) != endVSprite)
    {
	/* Forbid(); */ /* bart - 03.19.86 */

	if (s->Flags & BACKSAVED) clrBob(s, RPort, VPort );

	/* Permit(); */ /* bart - 03.19.86 */

	if (s->Flags & BOBUPDATE)
	    s->Flags = s->Flags & ~BOBUPDATE | BACKSAVED;
	if (s->VSBob)
	{
	    bptr = s->VSBob;
	    bptr->Flags = (bptr->Flags & ~BWAITING) & ~BDRAWN;
	    if (dptr = s->VSBob->DBuffer)
	    {
		s->OldX = dptr->BufX;
		dptr->BufX = s->X;
		s->OldY = dptr->BufY;
		dptr->BufY = s->Y;

		s->ClearPath = dptr->BufPath;
		dptr->BufPath = s->DrawPath;

		cptr = (void *) dptr->BufBuffer;
		dptr->BufBuffer = bptr->SaveBuffer;
		bptr->SaveBuffer = (void *) cptr;

		i = s->Flags;
		if (bptr->Flags & SAVEPRESERVE) s->Flags |= BACKSAVED;
		else s->Flags &= ~BACKSAVED;

		if (i & BACKSAVED) bptr->Flags |= SAVEPRESERVE;
		else bptr->Flags &= ~SAVEPRESERVE;
	    }
	    else
	    {
		s->OldX = s->X;
		s->OldY = s->Y;
		s->ClearPath = s->DrawPath;
	    }
	}
    }

    /* close up VSprite instruction list */

#ifdef USE_CEND
	CEND(sprcl->CopPtr);
	CEND(clrcl->CopPtr);
#else
    CWAIT(sprcl->CopPtr,263,255);       /* no need for a BUMP */
    CWAIT(clrcl->CopPtr,263,255);       /* " */
#endif

#ifdef COPINIT

#ifndef USE_OLD_LISTS

    /* link in new VSprite instruction lists */

        tmpsprcl = VPort->SprIns;
        tmpclrcl = VPort->ClrIns;

	VPort->SprIns = isprcl;
	VPort->ClrIns = iclrcl;

    /* free old VSprite instruction lists */ 

	freecoplist(tmpsprcl);
	freecoplist(tmpclrcl);

#else

    /* link in new VSprite instruction lists */

	VPort->SprIns = isprcl;
	VPort->ClrIns = iclrcl;

#endif

#endif

	/* This routine will free memory as it is allocated by blisser */
	/* it frees blissObjs that have clipped shadows by waiting for */
	/* the next blissObj to complete and then freeing the shadow */

	byebyeblissobjs(gi);	/* bart - 09.24.85 */

AWFUL_GOTO_EXIT: /* I'm not really doing this.  You don't see me */

    if (layer) UNLOCKLAYER(layer);

    /* bart - 08.21.85 - temporary patch */
    /* bart - 10.07.85 - removed temporary patch */
    /* bart - 11.24.85 - restored temporary patch */
    /* bart - 02.26.86 - removed temporary patch... see QBlit.asm 1.06.86  */
    /* bart - 02.27.86 - restored temporary patch */
    /* bart - 01.25.88 - removed temporary patch */
    /* Permit(); */ /* bart - 01.25.88 */

}

