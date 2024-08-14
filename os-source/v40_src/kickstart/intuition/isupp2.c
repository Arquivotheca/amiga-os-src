

/*** isupp2.c *************************************************************
 *
 *  File isupp2.c	- recent intuition support routines
 *	NEVER called by anything except Input Handler Intuition().
 *
 *  $Id: isupp2.c,v 38.5 92/08/02 12:43:02 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

#define D(x)	;

stretchIFrame(delta)
struct Point delta;
{
    struct IBox *b = &(fetchIBase()->IFrame);

    b->Width += delta.X;
    b->Height += delta.Y;
}

dragIFrame(delta)
struct Point delta;
{
    struct IBox *b = &(fetchIBase()->IFrame);

    b->Left	+= delta.X;
    b->Top	+= delta.Y;
}

BOOL
isTick(ie)
struct InputEvent *ie;
{
    return ( ie->ie_Class == IECLASS_TIMER );
}

BOOL
isSelectup(ie)
struct InputEvent *ie;
{
    return ((ie->ie_Class == IECLASS_RAWMOUSE) && (ie->ie_Code == SELECTUP));
}

/* Function to draw one of the edges of the window size/drag frame.
 * oldrect and newrect are pointers to rectangles describing the
 * old frame position and the new one, respectively.  Either pointer
 * can be NULL, in which case that rectangle is not drawn (this
 * is useful for getting the frame on or off the screen).
 * 
 * The idea here is to avoid too many dynamic uglies.  We achieve that
 * in a few ways.  First, we erase the old before drawing the new, and
 * second, we do each edge of both rectangles consecutively, so the new
 * rendering happens very soon after the erasing.
 *
 * New to V39, we always use single-pixel thick horizontal lines,
 * and two-pixel thick vertical lines.  We use a RectFill() for
 * the vertical line because it's faster.
 *
 * Note that under V37, Intuition used to verify that the coordinates
 * were clipped against the RastPort extremities.  The test was broken
 * for interleaved bitmaps, since it depended on BytesPerRow.  We use
 * the ClipLayer->rp and SafeRectFill() instead.
 */
drawFrames( RP, oldrect, newrect )
struct RastPort *RP;
struct Rectangle *oldrect, *newrect;
{
    struct Rectangle *rect;
    int edge, pass;

    for ( edge = FRAMEEDGE_TOP; edge <= FRAMEEDGE_RIGHT; edge++ )
    {
	for ( pass = 0; pass < 2; pass++ )
	{
	    rect = oldrect;
	    if ( pass )
	    {
		rect = newrect;
	    }

	    if ( rect )
	    {
		LONG minx, miny, maxx, maxy;
		minx = rect->MinX;
		miny = rect->MinY;
		maxx = rect->MaxX;
		maxy = rect->MaxY;

		if ( edge & FRAMEEDGE_VERTICAL )
		{
		    /* So we don't render the corners twice */
		    miny++;
		    maxy--;

		    if ( edge == FRAMEEDGE_LEFT )
		    {
			/* Take the left edge of the rectangle, and grow it one
			 * pixel to the right.
			 */
			maxx = minx+1;
		    }
		    else /* ( edge == FRAMEEDGE_RIGHT ) */
		    {
			/* Take the right edge of the rectangle, and grow it one
			 * pixel to the left.
			 */
			minx = maxx-1;
		    }
		}
		else
		{
		    if ( edge == FRAMEEDGE_TOP )
		    {
			/* It's the top edge */
			maxy = miny;
		    }
		    else
		    {
			/* It's the bottom edge */
			miny = maxy;
		    }
		}
		SafeRectFill( RP, minx, miny, maxx, maxy );
	    }
	}
    }
}
