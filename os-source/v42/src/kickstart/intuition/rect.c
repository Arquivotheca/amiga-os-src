/*** rect.c *************************************************************
 *
 *  File rect.c: intuition rectangle and box utilities
 *
 *  $Id: rect.c,v 40.0 94/02/15 17:30:04 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#define DEBUG 0

/*****************************************************************************/

#define D(x)	;

/*****************************************************************************/

#include "intuall.h"
#include <graphics/regions.h>
#include <graphics/gfxmacros.h>

/*****************************************************************************/

struct Rectangle nullrect =
{0, 0, 0, 0};

/*****************************************************************************/

void eraseRect (struct RastPort *rp, struct Rectangle *r)
{
    EraseRect (rp, r->MinX, r->MinY, r->MaxX, r->MaxY);
}

/*****************************************************************************/

/* jimm: 4/27/90: patternpen != ~0 means to use patternfill */
void fillAround (rp, yesrect, norect, pen, patternpen)
struct RastPort *rp;
struct Rectangle *yesrect;	/* fill this rectangle ...	*/
struct Rectangle *norect;	/* ... except here		*/
UBYTE pen;
UBYTE patternpen;
{
    struct Region *rgn;
    struct RegionRectangle *rr;
    struct Point ulc;		/* region rects are offset by bounds */
    extern UWORD CPattern[];

    /* start with old rectangle */
    if (!(rgn = (struct Region *) NewRegion ()))
	return;

    OrRectRegion (rgn, yesrect);

    /* oh, yeah, I can use ClearRectRegion() */
    if (norect)
	ClearRectRegion (rgn, norect);

    /* erase remnants */
    ulc = upperLeft (&rgn->bounds);	/* note: a macro	*/

    for (rr = rgn->RegionRectangle; rr != NULL; rr = rr->Next)
    {
	offsetRect (&rr->bounds, ulc.X, ulc.Y);

	if (patternpen != ~0)
	{
	    BlastPattern (rp,
			  rr->bounds.MinX,
			  rr->bounds.MinY,
			  rr->bounds.MaxX,
			  rr->bounds.MaxY,
			  CPattern, 1,
			  patternpen, pen, JAM2);
	}
	else
	{
	    rp->Mask = (UBYTE) ~ 0;
	    BNDRYOFF (rp);	/* 3/88: hoo boy	*/
	    SetABPenDrMd (rp, pen, 0, JAM2);
	    SafeRectFill (rp,
			  rr->bounds.MinX,
			  rr->bounds.MinY,
			  rr->bounds.MaxX,
			  rr->bounds.MaxY);
	}
    }

    DisposeRegion (rgn);
}

/*****************************************************************************/

/* used by prop gadgets	*/
/* jimm: 4/27/90: patternpen != ~0 means to use patternfill */
void eraseRemnants (rp, oldbox, newbox, offset, pen, patternpen)
struct RastPort *rp;
struct IBox *oldbox;
struct IBox *newbox;
struct Point offset;
UBYTE pen;
UBYTE patternpen;
{
    struct Rectangle oldrect;
    struct Rectangle newrect;

    /* convert boxes to rectangles in layer coordinates */

    boxToRect (oldbox, &oldrect);
    offsetRect (&oldrect, offset.X, offset.Y);
    boxToRect (newbox, &newrect);
    offsetRect (&newrect, offset.X, offset.Y);

    fillAround (rp, &oldrect, &newrect, pen, patternpen);
}

/*****************************************************************************/

/* until all callers of func are on the structured train */
void boxModernize (void (*func)(struct RastPort *,LONG,LONG,LONG,LONG), struct RastPort *rp, struct IBox *box)
{
    (*func) (rp, box->Left, box->Top, box->Width, box->Height);
}
