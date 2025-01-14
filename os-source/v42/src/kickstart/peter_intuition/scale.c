/*** scale.c **************************************************************
 *
 * coordinate system scaling/transformations
 *
 *  $Id: scale.c,v 38.2 93/01/12 16:20:47 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 ****************************************************************************/

#include "intuall.h"

#define D(x)	;
#define DZ(x)	;
#define DSPRITE(x)	;

/*
 * convert a point in screen coordinates to Mouse coordinates
 * !!!! jimm: 2/19/90: EVERY use of these needs to be checked
 * against overlap of signed range.  I'm going to limit it down
 * a little more so that you can get away with adding or subtracting
 * a little (such as INTERSCREENGAP) without blowing out.
 */
screenToMouse( pt, screen )
struct Point	*pt;
struct Screen	*screen;
{
#if 1

#define TOO_BIG		 (30000)
#define TOO_SMALL 	(-30000)

    /* max/min are 32-bit signed functions	*/
    pt->X =
	imax( imin( pt->X * XSC(screen)->ScaleFactor.X, TOO_BIG ), TOO_SMALL );
    pt->Y =
	imax( imin( pt->Y * XSC(screen)->ScaleFactor.Y, TOO_BIG ), TOO_SMALL );

#else
    pt->X *= XSC(screen)->ScaleFactor.X;
    pt->Y *= XSC(screen)->ScaleFactor.Y;
#endif
}

/* convert a LongPoint in screen coordinates to Mouse coordinates
 * unlike screenToMouse() above, we don't worry about overflow
 * since we have signed LONGs here, instead of signed WORDs.
 */
screenToLongMouse( pt, screen )
struct LongPoint *pt;
struct Screen *screen;
{
    pt->LX *= XSC(screen)->ScaleFactor.X;
    pt->LY *= XSC(screen)->ScaleFactor.Y;
}

/*
 * convert a point in mouse coordinates to screen-resolution coordinates
 * NOTE: performs scaleRatio() step in mouse-to-screen mapping
 */
longmouseToScreen( pt, screen )
struct LongPoint *pt;
struct Screen	*screen;
{
    DZ( if (!(XSC(screen)->ScaleFactor.X  && XSC(screen)->ScaleFactor.Y ) )
	{
	    printf("mouseToScreen divide by zero! %lx\n", screen );
	    dumpPt("scale factor", XSC(screen)->ScaleFactor );
	} )

    pt->LX /= XSC(screen)->ScaleFactor.X;
    pt->LY /= XSC(screen)->ScaleFactor.Y;
}

scaleConversion( pt, res1, res2 )
struct Point	*pt;
struct Point	res1;
struct Point	res2;
{
    LONG	tmp;

    tmp = pt->X * res1.X;
    pt->X = tmp / res2.X;

    tmp = pt->Y * res1.Y;
    pt->Y = tmp / res2.Y;
}

#if 0	/* using viewport sprite positioning	*/
/*
 * convert point in mouse coord space to lores coord
 */
mouseToSprite( pt )
struct Point *pt;
{
    struct IntuitionBase *IBase = fetchIBase();
    struct Point	mousescale;

    mousescale.X = (2 * MOUSESCALEX );	/* ZZZ: is this always right? */
    mousescale.Y = ( IBase->SpriteYFactor );

    DSPRITE( dumpPt( "mouse to sprite in", *pt ) );

    /* from monitor to base coordinate system (handled in spriteY?)	*/
    /* mspcScaleConversion( NULL, IBase->ActiveMonitorSpec, pt ); */

    DSPRITE( dumpPt( "after mspecScale", *pt ) );

    scaleFactorDivide( pt, &mousescale );

    DSPRITE( IBase->debugpt = *pt );	/* DEBUG */
    DSPRITE( dumpPt( "mouse to sprite out", *pt ) );
}
#endif

/*
 * convert screen resolution coords to Mouse coordinates
 */
scaleScreenMouse(rect, screen)
struct Rectangle *rect;
struct Screen	*screen;
{
    screenToMouse((struct Point *) &rect->MinX, screen);
    screenToMouse((struct Point *) &rect->MaxX, screen);
}

