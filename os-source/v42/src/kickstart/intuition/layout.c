/*** layout.c **************************************************************
 *
 *  Layout routines for system requesters.
 *
 *  $Id: layout.c,v 40.0 94/02/15 17:35:04 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include <exec/memory.h>
#include "ezreq.h"
#include "classusr.h"
#include "gadgetclass.h"

/*****************************************************************************/

#define D(x)	;
#define DLAYOUT(x)	;

/*****************************************************************************/

/* cheezy gadgets
 * could easily toss out two box images, to get nicer border
 *
 * just return 1 gadget (linked in), built with perhaps
 * a multi-part IntuiText */
struct Gadget *createITGadget (struct Screen *screen, struct IntuiText *gtext, struct Remember **remkey, struct Gadget *prevgad, ULONG id, ULONG frame)
{
    extern UBYTE *FrButtonClassName;
    struct DrawInfo *drinfo;
    struct Gadget *g;

    D (printf ("createITG enter, gtext %lx\n", gtext));

    if (!screen)
	return (NULL);
    drinfo = GetScreenDrawInfo (screen);

    if (g = (struct Gadget *) NewObject (NULL, FrButtonClassName,
					 GA_INTUITEXT, gtext,
					 GA_IMAGE, frame,
					 GA_RELVERIFY, TRUE,
					 GA_ID, id,
					 GA_PREVIOUS, prevgad,
					 GA_DRAWINFO, drinfo,
					 TAG_END))
    {
#define SAFEEXTRAGWIDTH	( imax( screen->RastPort.Font->tf_XSize, 20) )
#define EXTRAGWIDTH     ( screen->RastPort.Font->tf_XSize )
#define EXTRAGHEIGHT	(screen->RastPort.Font->tf_YSize - \
			  screen->RastPort.Font->tf_Baseline)

	SetGadgetAttrs (g, NULL, NULL,
			GA_WIDTH, g->Width + SAFEEXTRAGWIDTH,
			GA_HEIGHT, g->Height + EXTRAGHEIGHT,
			GA_DRAWINFO, drinfo,
			TAG_END);
    }

    D (printf ("cITG returning %lx\n", g));

    FreeScreenDrawInfo (screen, drinfo);
    return (g);
}

/*****************************************************************************/

static LONG divvyUp (LONG bins, LONG balls, UWORD *errorp)
{
    LONG count = 0;

    D (printf ("divvy bins %ld balls %ld error %ld ... ", bins, balls, *errorp));
    while (*errorp < balls)
    {
	*errorp += bins;
	count++;
    }
    *errorp -= balls;

    D (printf (" count %ld\n", count));
    return (count);
}

/*****************************************************************************/

/* spread out a top-aligned row of gadgets to totalwidth
 * assumes that you've used glistExtentBox to get natural
 * width.  Keep an eye on SPACING1 between gadgets.
 * Screw that: res-sensitive spacing is passed in. */
void spreadLayoutGadgets (struct Gadget *g, LONG gadcount, struct IBox *box, LONG totalwidth, LONG gadspace)
{
    LONG xtraspace;		/* extra space to distribute	*/
    LONG numgad = gadcount;
    LONG left = box->Left;
    UWORD dverror;

    D (printf ("sLG: gadcount %lx, total width %lx\n", gadcount, totalwidth));

    if (numgad > 1)
    {
	/* (extra) space to be distributed between gadgets */
	xtraspace = totalwidth - box->Width;	/* xtra	*/
	D (printf ("xtraspace: %lx\n", xtraspace));

	/* standard init for error is 'space count - 1' */
	dverror = (gadcount - 1) - 1;

	while (g && numgad--)
	{
	    g->LeftEdge = left;
	    g->TopEdge = box->Top;
	    left += g->Width + gadspace + divvyUp (gadcount - 1, xtraspace, &dverror);

	    g = g->NextGadget;
	}
    }
    else if (gadcount == 1)
    {
	/* center single gadget */
	g->TopEdge = box->Top;
	g->LeftEdge = box->Left + (totalwidth - g->Width) / 2;
    }
}

/*****************************************************************************/

/* returns gadget count */
LONG glistExtentBox (struct Gadget *g, struct IBox *box, LONG gadspace)
{
    LONG count;

    D (printf ("glistEB. g: %lx, box %lx\n", g, box));

    box->Left = box->Top = 0;

    /* accumulate these	*/
    box->Width = box->Height = 0;

    count = 0;			/* count 'em, too	*/
    while (g)
    {
	count++;
	box->Width += g->Width;
	box->Height = imax (box->Height, g->Height);

	g = g->NextGadget;
    }
    box->Width += (count - 1) * gadspace;	/* add in space between	*/

    return (count);
}

/*****************************************************************************/

/* gets TextExtent around a whole list of IntuiTexts's,
 * and optionally lays them out in the process, left-justified.
 * Note that returned TextExtent.te_Width/Height are just the
 * width/height of the accumulated te_Extent rectangle, which
 * will be fine. */
LONG ITextLayout (struct RastPort *rp, struct IntuiText *it, struct IBox *box, BOOL do_layout, LONG xoffset, LONG yoffset)
{
    struct TextExtent localte;	/* extent of a single itext item	*/
    struct TextExtent textent;	/* extent of grand answer	*/

    /* Peter 24-Jan-91: Initialize box to NULL, in case there is no IntuiText.  */
    clearWords ((UWORD *)box, 4);

    if (it == NULL)
    {
	D (printf ("ITextLayout bailing early!!!\n"));
	return (FALSE);
    }

    /* start us off	*/
    if (do_layout)
    {
	it->LeftEdge = xoffset;
	it->TopEdge = yoffset;
    }

    /* initialize summary text extent with this first one	*/
    D (printf ("ITextLayout, do_layout: %lx, offsets %lx/%lx\n",
	       do_layout, xoffset, yoffset));
    RastPortITextExtent (rp, it, &textent);
    it = it->NextText;
    D (dumpRect ("ITL initial", &textent.te_Extent));

    /* second and subsequent itexts	*/
    while (it)
    {
	if (do_layout)
	{
	    /* put next IText just below the accumulated extent box	*/
	    it->LeftEdge = xoffset;

	    /* regarding leading: using the RastPort's (default)
	     * font to do spacing is OK since we only automatically
	     * layout ezreq's, which are all default font.
	     */
	    it->TopEdge = textent.te_Extent.MaxY +
		(rp->TxHeight - rp->TxBaseline);
	}

	RastPortITextExtent (rp, it, &localte);

	/* grow the accumulated extent rectangle	*/
	rectHull (&textent.te_Extent, &localte.te_Extent);

	it = it->NextText;
    }

    rectToBox (&textent.te_Extent, box);
    return (TRUE);
}
