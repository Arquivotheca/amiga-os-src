/*** alert.c *****************************************************************
 *
 *  alert support routines
 *
 *  $Id: alert.c,v 40.0 94/02/15 17:44:54 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1985, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

#define D(x)	;

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef GRAPHICS_SPRITE_H
#include <graphics/sprite.h>
#endif

#ifndef GRAPHICS_GFXMACROS_H
#include <graphics/gfxmacros.h>
#endif

#ifndef HARDWARE_CUSTOM_H
#include <hardware/custom.h>
#endif

#ifndef HARDWARE_DMABITS_H
#include <hardware/dmabits.h>
#endif

/*****************************************************************************/

extern struct Custom volatile custom;

/*****************************************************************************/

/* This routine replaces the ReadLeft/ReadRight and thus
 * reduces code and also makes for easier testing in a number
 * of places */
ULONG CheckButtons(void);

/*****************************************************************************/

/* New:  The timeout value no longer has a true un-timed value.
 * However, the timeout is now based on frame rate of the machine.
 * That is, a 60fps display will timeout one tick in 1/60 second.
 * This means that a value of 360 is 6 seconds of alert.
 * It also means that ~0 is 828.5 days and thus we don't need to
 * check for ~0 as being a non-timeout.
 *
 * Since we are now frame rate based, we can flash about right.
 * The define below defines the number of frames before we flash
 * and (with the current code) needs to be a power of 2. */
#define	FLASH_RATE	64

/*****************************************************************************/

/*** intuition.library/DisplayAlert ***/
/* Changed to ULONG since WORD bools are a problem */
ULONG timedDisplayAlert (ULONG alertnum, UBYTE *string, LONG height, ULONG timeout)
{
    extern struct IFontPrefs topazprefs;
    struct BitMap *BitMap;
    struct TextFont *fp;

    /* Big Data on the stack */
    struct View View;
    struct ViewPort AViewPort;
    struct RasInfo RasterInfo;
    struct RastPort RasterPort;

#define ViewPtr	(&View)

    ULONG type;
    LONG x, y, dheight;
    LONG width;
    ULONG i;
    UBYTE *workstring;
    ULONG BoolReturn = FALSE;
    struct IntuitionBase *IBase = fetchIBase();

    struct BitMap *AllocBitMap();

    /* for alert type of:
     *	Daisy Alerts are hanlded strictly by the Exec
     * 	DeadEnd, create a new ViewPort just large enough to hold
     * 		the Alert, create the copperlist for this
     * 		ViewPort, create the View, add the copperlist to
     * 		the View, and that's it
     *	Recovery, create a new ViewPort just large enough to hold
     *		the Alert, move all other Screens down by that
     *		much, and add that ViewPort at the top.
     */


    type = alertnum & ALERT_TYPE;

    D( printf("Alert number %lx, type %lx\n", alertnum, type ) );
    /* alert is standard HIRES display width	*/
    width = 640;	/* standard my ass	*/

    /* jimm: 6/1/90: single-thread alerts, and keep
     * input from running through during alerts
     */
    ObtainSemaphore( &IBase->ISemaphore[ ALERTLOCK ] );

    /*
     * Check if we are to (if timeout is 0, we do not run)
     */
    if (timeout)
    {
        /*
         * if there isn't enough memory for a recoverable alert,
         * we just tell the caller that the user said "FALSE"
         */

        /* try to allocate the memory for the structures and the display */
        if (BitMap=AllocBitMap(width,height,1,BMF_CLEAR|BMF_DISPLAYABLE,NULL))
        {
            /* now, fill out these structures with the appropriate data */
            RasterInfo.BitMap = BitMap;
            RasterInfo.RxOffset = RasterInfo.RyOffset = 0;
            RasterInfo.Next = 0;

            InitRastPort(&RasterPort);
            RasterPort.BitMap = BitMap;

            /* ViewPtr = &View; (ViewPtr is now a macro) */
            InitView(ViewPtr);
            ViewPtr->ViewPort = &AViewPort;

            InitVPort(&AViewPort);
#if 0	/* InitVPort() zeroed these for me */
            AViewPort.DxOffset = AViewPort.DyOffset = 0;
#endif
            AViewPort.DWidth = width;
            AViewPort.DHeight = height;
            AViewPort.Modes = HIRES | SPRITES;
            AViewPort.RasInfo = &RasterInfo;
	    /* Peter 6-May-92: Amazing.  This viewport never had
	     * a colormap.  SetRGB4() does weird things in absence of
	     * a colormap.  Under V37 and prior, it actually modified
	     * the copper lists even though there was no colormap.
	     * It's possible this was the source of the "recoverable
	     * alerts in red" bug of V37, the kicker being that the
	     * graphics default colors are red on black.
	     *
	     * According to Chris, it will be difficult to replicate
	     * the copper-list poking under V39, so I'd better go with
	     * the colormap.
	     *
	     * Now I don't care about failure to allocate the ColorMap,
	     * since we'd then just drop back to the old way, which
	     * under V37 mostly worked, and under V39 gives the gfx
	     * default colors, which happen to be red on black.
	     */
            AViewPort.ColorMap = (struct ColorMap *)GetColorMap( 2 );
            MakeVPort(ViewPtr, &AViewPort);

            /* draw the text and graphics to the display memory */
            dheight = height - 1;
            drawBox(&RasterPort, 0, 0,  width, dheight, 1, JAM1, 6, 3);
            fp = ISetFont( &RasterPort, &topazprefs.ifp_TABuff.tab_TAttr );
            do
                {
                x = (*string++) << 8;
                x |= (*string++);
                y = *string++;
                Move(&RasterPort, x, y);
                i = 0;

                workstring = string;
                while (*string++) i++;
                Text(&RasterPort, workstring, i);
                /* the char at string is fetched, string is bumped, char is tested */
                /* if the char string points at now is NULL too, we're done */
                }
            while (*string++);

            ICloseFont( fp );

            /* display it */
            MrgCop( ViewPtr );
            LoadView( ViewPtr );
            WaitTOF();
            ON_DISPLAY;

            SetRGB4(&AViewPort, 0, 1, 1, 1);

            if (type == DEADEND_ALERT)
            {
                D( printf("DisplayAlert: red, type %lx, num %lx\n", type, alertnum) );
                SetRGB4( &AViewPort, 1, 15, 2, 0 );       /** red **/
            }
            else
            {
                D( printf("DisplayAlert: amber, type %lx, num %lx\n", type, alertnum) );
                SetRGB4( &AViewPort, 1, 15, 10, 2 );      /** amber **/
            }

            /* turn off the sprite without nasty DMA smear.
             * ZZZ: might need to (selectively) do more to
             * prevent smear of attached sprite 1.
             */

	    /*
	     * Note!  Is this safe with the new sprite stuff?
	     */
	    ChangeExtSpriteA (NULL, (struct ExtSprite *) IBase->SimpleSprite, (struct ExtSprite *) IBase->BlankSprite, TAG_DONE );

            WaitTOF(); OFF_SPRITE;

            /* Now, wait until both buttons are up! */
            while (CheckButtons()) ;

	    /*
	     * Now wait for mouse buttons or timeout before returning...
	     */
	    while (timeout)
	    {
		/*
		 * Wait for a frame to finish before we loop
		 */
		WaitTOF();

		/*
		 * Count our timeout
		 */
		timeout--;

		/*
		 * Flash every FLASH_RATE frames...
		 */
		if (!(timeout & (FLASH_RATE-1)))
		{
		    drawBox(&RasterPort,0,0,width,dheight,1,COMPLEMENT,6,3);
		}

		/*
		 * Check for any buttons...
		 */
		if (i=CheckButtons())
		{
			BoolReturn=i-1;	/* Slime... Set FALSE for RIGHT button */
			timeout=0;
		}
	    }

            /* OK, got a mouse button, wait until button up... */
            while (CheckButtons()) ;

            FreeVPortCopLists(&AViewPort);

            RethinkDisplay();           /* back to ViewLord     */
	    /* RethinkDisplay() does a WaitTOF() */
            FreeCprList (ViewPtr->LOFCprList); /* get rid of mine      */

	    /* Get rid of the colormap, if any */
	    FreeColorMap (AViewPort.ColorMap);
            FreeBitMap (BitMap);
	}
	else if (type == DEADEND_ALERT)
	{
	    /* If this was dead-end, we really should crash now
	     * since we did not get the memory to display it!
	     * EXEC will take care of that! */
	    Alert(alertnum);    /* exec will surely restart */
	}

    }
    ReleaseSemaphore (&IBase->ISemaphore[ALERTLOCK]);

    return (BoolReturn);
}
