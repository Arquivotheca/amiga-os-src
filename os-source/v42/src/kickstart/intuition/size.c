/*** size.c **************************************************************
 *
 * intuition.c support routines
 *
 *  $Id: size.c,v 40.0 94/02/15 17:56:37 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"
#include "gadgetclass.h"
#include <graphics/regions.h>

/*****************************************************************************/

#define D(x)	;

/*****************************************************************************/

/* right/left borders: or damage, clear, delta, or damage */
static void borderDamage (struct Window *w, LONG dx, LONG dy)
{
    struct Layer *blayer;	/* layer border is in */
    struct Rectangle newrect, oldrect;
    LONG i;
    struct RastPort *rp;
    LONG right, bottom, right_left, bottom_top, top_bottom;

    blayer = (struct Layer *) getGimmeLayer (w);

    /* get salient parameters */
    /* Peter 15-Feb-93: top_bottom has to be one less than BorderTop
     * because we may need to refresh the shadow line under the drag
     * bar if this window has no depth gadget
     */
    right_left = w->Width - w->BorderRight;
    bottom_top = w->Height - w->BorderBottom;
    top_bottom = w->BorderTop - 1;
    right = w->Width - 1;
    bottom = w->Height - 1;

    /* will only be rendering into border layer */
    rp = obtainRP (&w->WScreen->RastPort, blayer);

    /* two passes: one each for right/bottom borders */
    for (i = 0; i < 2; i++)
    {
	/* newrect and oldrect are (almost) the new and old
	 * border rectangles.  We don't care about changes in
	 * height for the bottom border or width for the right
	 * border, since all affected pixels are either just
	 * entering or just leaving the layer.
	 */
	newrect.MaxY = bottom;

	if (i)			/* Bottom border */
	{
	    newrect.MinX = 0;
	    newrect.MaxX = right_left;
	    newrect.MinY = bottom_top;

	    oldrect = newrect;
	    oldrect.MinY += dy;
	    oldrect.MaxY += dy;
	}
	else
	{			/* Right border */
	    newrect.MinX = right_left;
	    newrect.MaxX = right;
	    newrect.MinY = top_bottom;

	    oldrect = newrect;
	    oldrect.MinX += dx;
	    oldrect.MaxX += dx;
	}

	/* jimm: 6/22/86: no border => no damage, no clear */
	if (nonDegenerate (&newrect))
	{
	    OrRectRegion (blayer->DamageList, &newrect);
	    OrRectRegion (blayer->DamageList, &oldrect);
	    /* Here's the key part to making this all look good:
	     * We know we're going to rerender every pixel which
	     * is in the border, so we only want to erase those
	     * pixels which are no longer in the border.  First,
	     * see the note above about ignoring height effects
	     * on the right border and width on the bottom.
	     * The only remaining pixels to be cleared are those
	     * left behind when the window grows.
	     */
	    if (i)		/* Bottom border */
	    {
		oldrect.MaxY = imin (oldrect.MaxY, newrect.MinY - 1);
	    }
	    else
		/* Right border */
	    {
		oldrect.MaxX = imin (oldrect.MaxX, newrect.MinX - 1);
	    }
	    if (nonDegenerate (&oldrect))
	    {
		eraseRect (rp, &oldrect);
	    }
	}
    }				/* end damage handling of right and bottom borders */

    blayer->Flags |= (LAYERREFRESH | LAYERI_NOTIFYREFRESH | LAYERI_GADGETREFRESH);

    freeRP (rp);
}

/*****************************************************************************/

/* or damage, clear, delta (relativity), or damage */
static void gadgetDamage (struct Window *w, LONG dx, LONG dy)
{
    struct RastPort *rp;
    struct Layer *layer;
    USHORT flagsmask = 0;	/* which dimensions change determine which
				 * gadget Flags are important */
    struct GListEnv genv;
    struct Gadget *g;
    USHORT gflags;
    struct Rectangle rect;
    struct IBox gadgetbox;
    struct Rectangle winrect;

    /* get layer dimensions and stuff like that */
    if (!gListDomain (w->FirstGadget, w, &genv))
	goto OUT;

    /* get locks all the way down */
    lockGDomain (&genv);	/* lock linfo, gadgets, appropriate layers */

    /* already have layers lock.
     * Now I'll be able to do just a quick swap of layer pointers
     * in the rastport.
     */
    rp = obtainRP (genv.ge_GInfo.gi_RastPort, genv.ge_Layer);

    /* which relativity types need to be changed */
    if (dx)
	flagsmask |= (GFLG_RELSPECIAL | GFLG_RELRIGHT | GFLG_RELWIDTH);
    if (dy)
	flagsmask |= (GFLG_RELSPECIAL | GFLG_RELBOTTOM | GFLG_RELHEIGHT);

    /* ZZZ: would like to do all this in subroutine */

    for (g = w->FirstGadget; g; g = g->NextGadget)
    {
	if (((gflags = g->Flags) & flagsmask)	/* sensitive to change */
	    ||			/* jimm: 4/7/86: all rel gadg's must be included */
	    (g->Activation & (GACT_LEFTBORDER | GACT_TOPBORDER)))
	{
	    /* get the select box to add to the damage list */
	    gadgetInfo (g, &genv);

	    /* set up proper layer for gadget */
	    rp->Layer = layer = genv.ge_GInfo.gi_Layer;

	    /* The window is already in its new size, so we have
	     * to factor out dx and dy to get a GadgetInfo that
	     * describes how things used to be before the resizing
	     * happened:
	     */
	    genv.ge_GInfo.gi_Domain.Width += dx;
	    genv.ge_GInfo.gi_Domain.Height += dy;
	    gadgetBoundingBox (g, &genv.ge_GInfo, &gadgetbox);
	    /* convert from 'width' to 'right' */
	    boxToRect (&gadgetbox, &rect);

	    if (nonDegenerate (&rect))
	    {
		/* Now these are the old gadget bounds, which
		 * have to be erased, as well as added to
		 * the damage region, so that other rendering
		 * that might be blown away by the erasing can
		 * get repaired.
		 */
		OrRectRegion (layer->DamageList, &rect);

		/* Here's a cool optimization:  we don't always need
		 * to erase the old gadget's bounds.  Those pixels
		 * could be:
		 *
		 * - outside the new window's extent (never needs drawing)
		 * - in new old window's border (no point in erasing,
		 *   since we'll be drawing border color into those pixels)
		 * - in the new window's interior (needs erasing)
		 *
		 * So we limit the rectangle to be erased to the inner
		 * rectangle of the window!
		 */
		windowInnerBox (w, (struct IBox *) &winrect);
		boxToRect ((struct IBox *) &winrect, &winrect);
		limitRect (&rect, &winrect);
		if (nonDegenerate (&rect))
		{
		    eraseRect (rp, &rect);
		}
	    }

	    /* Now we restore the true window dimensions, so
	     * the GadgetInfo describes the window in its
	     * new size:
	     */
	    genv.ge_GInfo.gi_Domain.Width -= dx;
	    genv.ge_GInfo.gi_Domain.Height -= dy;

	    /* We send a GM_LAYOUT method to the gadget, expecting
	     * it to fill in its bounds field.  For gadgets that
	     * don't handle this method, nothing happens.  But for
	     * regular GREL_ gadgets, regular relativity is handled
	     * inside gadgetBoundingBox().
	     */
	    callHook (gadgetHook (g), g, GM_LAYOUT, &genv.ge_GInfo, 0);

	    gadgetBoundingBox (g, &genv.ge_GInfo, &gadgetbox);
	    /* convert from 'width' to 'right' */
	    boxToRect (&gadgetbox, &rect);

	    if (nonDegenerate (&rect))
	    {
		/* Since the window is already in the new size,
		 * these are the new bounds of the gadget.
		 * Add them to the damage region so the gadget
		 * will appear in its new place.
		 */
		OrRectRegion (layer->DamageList, &rect);
	    }

	    /* better to be redundant than to ask gimmelayer to refresh
	     * unnecessarily
	     */
	    layer->Flags |= (LAYERREFRESH | LAYERI_NOTIFYREFRESH | LAYERI_GADGETREFRESH);
	}
    }

    rp->Layer = genv.ge_Layer;	/* restore original layer in there */
    freeRP (rp);

    unlockGDomain (&genv);
  OUT:
    return;
}

/*****************************************************************************/

/* sizeDamage() is called after window resize.  It's expected
 * to erase old right/left border, put old/new border areas into
 * damage list, prepare for updating of relative gadgets,
 * esp. depth arrangers.  Set LAYERREFRESH for BorderPatrol().
 *
 * Note well: the window is already in its new size at this point;
 * dx and dy are the deltas from the old size.
 */
void sizeDamage (struct Window *w, LONG dx, LONG dy)
{
    D (printf ("sizeDamage, window %lx, dx/dy %ld/%ld\n", w, dx, dy));
    if (!(dx || dy))
    {
	D (printf ("sizeDamage, not changing\n"));
	return;
    }

    /* do it for all */
    borderDamage (w, dx, dy);
    gadgetDamage (w, dx, dy);
}
