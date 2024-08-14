
/*** wbench.c ************************************************************** 
 * 
 *  The routines for dealing with the Workbench Screen 
 * 
 *  $Id: wbench.c,v 38.17 93/01/12 16:22:11 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc. 
 *  Copyright (C) 1985, Commodore-Amiga, Inc. 
 *  All Rights Reserved 
 * 
 ***************************************************************************/ 
 
#include "intuall.h" 
#include "preferences.h"
#include <exec/alerts.h>
#include <exec/memory.h>
#include <exec/initializers.h>

/* is there enough screen buffer to hold 'field'?	*/
#define ENUFSCREEN( field ) ( ((ULONG) OFFSET(Screen, field) )+ \
	    sizeof(((struct Screen *)0)->field) )


#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

/** #define USELACEWB 1 **/

#define D(x)		;	/* debug 	*/
#define DGSD(x)		;	/* debug GetScreenData	 	*/
#define DOWB(x)		;	/* debug OpenWorkbench() 	*/
#define DOWB3(x)	;	/* debug OpenWorkbench() 	*/
#define DOWB2(x)	;	/* debug OpenWorkbench() 	*/
#define DCOMMWB(x)	;	/* debug communicateWBench() 	*/
#define OWBALERT	1	/* pull alert if openwb fails */
 
/* Workbench Public Screen Name: */
UBYTE WBPUBNAME[] = "Workbench";

/* ======================================================================= */ 

/*
 * this nasty little function is trickier than it seems.
 * not only do you have to protect its walk down the
 * list, but when you call it unprotected you have to be sure
 * that you don't care if the WB screen it finds goes away
 * before you use it.
 *
 * ZZZ: probably should get IBASELOCK right here, right now.
 */
 
struct Screen *findWorkBench() 
{ 
    struct Screen *sc = NULL; 
    struct IntuitionBase *IBase = fetchIBase();

    DOWB(printf("findWorkBench\n"));

    /* jimm: added LOCKIBASE: 6/6/90	*/
    LOCKIBASE();

    for ( sc = IBase->FirstScreen; sc; sc = sc->NextScreen )
    { 
	DOWB(printf("screentype: %lx\n", sc->Flags & SCREENTYPE ));
	if ((sc->Flags & SCREENTYPE) == WBENCHSCREEN) 
	{
	    break;
	}
    } 

    UNLOCKIBASE();
    return( sc ); 
} 

/*
 * returns with psn_VisitorCount++
 */
struct Screen *
openwbscreen()
{
    struct Screen *wbscreen;
    struct IntuitionBase *IBase = fetchIBase();
    struct Screen *openScreenOnlyTags();

    wbscreen = openScreenOnlyTags(
	SA_LikeWorkbench, LIKEWB_WORKBENCH,
	SA_Type, WBENCHSCREEN,
	SA_Title, IBase->WBScreenTitle,	/* "Workbench Screen" */
	SA_PubName, (ULONG)WBPUBNAME,	/* "Workbench" */
	TAG_DONE );


    if ( wbscreen )
    {
	D( printf("openwbscreen ok.\n") );
	/* put a visitor lock on it right away	*/
	bumpPSNVisitor( XSC(wbscreen)->PSNode );
	PubScreenStatus( wbscreen, 0 );	/* make it public	*/

	pokeWorkbench();	/* only poke Dave when screen opens	*/
    }

#if OWBALERT
/* DEBUG version: openwb should be allowed to fail, except at init */
    else
    {
	DOWB( printf("openwbscreen failing miserably!!\n") );
	DOWB( Alert( AN_OpenScreen ) );
    }
#endif

    return( wbscreen ); 
}

/*** intuition.library/WBenchToFront ***/
/* Downcoded into intuitionface.asm */

/*** intuition.library/WBenchToBack ***/
/* Downcoded into intuitionface.asm */

 
 
 /*** intuition.library/OpenWorkBench ***/


/* NB: OpenWorkBench() calls here through LVO
 *
 *  This routine boils down to this:
 * "openSysScreen() with no visitor lock hanging."
 */
struct Screen *openWorkBench() 
{ 
    struct Screen	*openSysScreen();
    struct Screen *screen;

    /* Can't skip this operation just because the WB
     * screen is already there, since any other window
     * opening might have gotten it open. 
     *
     * HAH! Fixed: openSysScreen now tells wb.library
     *
     * Jeezus: no protection?
     * OK, I put IBASELOCK in oSS
     *
     * jimm: jan 1990
     */
    DOWB3(printf("OpenWB\n"));
    if ( screen =  openSysScreen(WBENCHSCREEN) )	/* returns VC++ */
    { 
	DOWB3( printf("WB screen found or opened OK\n") );

	decPSNVisitor( XSC(screen)->PSNode );	/* I leave no visitor lock */
    } 
    return(screen); 
} 

/* asynchronous WBENCHOPEN message to workbench.library (WBPort).
 * Needs to be asynchronous so that we won't deadlock when
 * we openwb in response to the workbench.library task calling 
 * (for openwindow, lockpubscreen, ... )
 */
pokeWorkbench()
{
    struct IntuitionBase *IBase = fetchIBase();
    struct IntuiMessage 	*wbmsg;
     
    Forbid();	/* prevent AlohaWorkBench	*/
    if ( IBase->WBPort && 
	(wbmsg = (struct IntuiMessage *)AllocMem(
	    sizeof(struct IIntuiMessage), MEMF_PUBLIC | MEMF_CLEAR)))
    {
	initWBMessage( wbmsg, &IBase->WBReplyPort, WBENCHOPEN );
	PutMsg( IBase->WBPort, wbmsg );
    }
    Permit();
}
/*** intuition.library/GetScreenData ***/

BOOL GetScreenData(buffer, size, type, customsc )
struct Screen *buffer;
UWORD size;
UWORD type;
struct Screen *customsc;
{
    struct Screen *sc = NULL;
    struct Screen *windowPubScreen();
    BOOL	locked_pubscreen = TRUE;

DGSD( printf("GSD buffer %lx, size 0x%lx, address of modes %lx limitaddr %lx\n",
	buffer, size, &buffer->ViewPort.Modes, 
	(ULONG) buffer + (ULONG) size ) );

    DGSD( printf("offset of viewport.modes %lx\n",
	OFFSET(Screen, ViewPort.Modes) ) );


    DGSD( printf("ENUFSCREEN for vpmodes 0x%lx, width %lx, height %lx\n",
	ENUFSCREEN( ViewPort.Modes ), ENUFSCREEN( Width ),
	ENUFSCREEN( Height ) ) );

    if (type == CUSTOMSCREEN)
    {
	sc =  customsc;
	locked_pubscreen = FALSE;
    }
    else if ( (type == WBENCHSCREEN ) &&
	    TESTFLAG( fetchIBase()->PubScreenFlags, SHANGHAI ) )
    {
	sc = windowPubScreen( NULL, 0 );	/* default	*/
	/* returns VC++	*/
    }
    else	/* non-shanghai'd WBENCHSCREEN and "new" sysscreens */
    {
    	sc = openSysScreen( type );	/*returns VC++ */
    }

    if (sc != NULL) 
    {
	/* copy 'size' bytes */
	CopyMem(sc, buffer, MIN(sizeof (struct Screen), size) );
	if ( locked_pubscreen ) decPSNVisitor( XSC(sc)->PSNode );

	/* "fake out" people who rely on WBenchScreen to get
	 * the text overscan (morerows) dimensions
	 */
	if ( type == WBENCHSCREEN )
	{
	    struct Rectangle	oscanrect;
	    ULONG		displayID;

	    /* We used to lie about the display ID, based on
	     * the IS_LACE and IS_PANELLED property of the WB screen's
	     * real mode.  Now, for V39, we lie based on the height
	     * of the display clip.  This is so 640x400 double-NTSC
	     * and double-PAL modes pretend to be interlaced.
	     */
	    displayID = HIRES_KEY;
	    if ( rectHeight( &XSC(sc)->DClip ) >= 400 )
	    {
		displayID = HIRESLACE_KEY;
	    }

	    /* now lie about screen mode and dimensions */
	    /* jimm: 6/5/90: should be changing the destination
	     * 'buffer' not the source 'sc'. ;^)
	     */

	    if ( size >= ENUFSCREEN( ViewPort.Modes ) )
	    {
		DGSD( printf("copying vpmodes\n"));
		buffer->ViewPort.Modes = displayID | SPRITES;
	    }
	    DGSD( printf("not enough space for vpmodes\n"));
	    /*** jimm: 6/11/90: Hedley mode doesn't lie about
	     *** dimensions.
	     ***/
	    if ( ( XSC(sc)->NaturalDisplayID & MONITOR_ID_MASK )
		!= A2024_MONITOR_ID )
	    {
		QueryOverscan( displayID, &oscanrect, OSCAN_TEXT );
		if ( size >= ENUFSCREEN( Width ) )
		{
		    buffer->Width =  imin(rectWidth( &oscanrect ), sc->Width );
		}
		if ( size >= ENUFSCREEN( Height ) )
		{
		    buffer->Height = 
			imin(rectHeight( &oscanrect ), sc->Height );
		}
	    }
	}
	return (TRUE);
    }

    return (FALSE);
}
 
 
 /*** intuition.library/CloseWorkBench ***/

#if 0

Moved to screens.h

BOOL CloseWorkBench() 
{ 
    DOWB2( printf("===\nCloseWorkBench enter\n") );
    return ( doISM( itCLOSESCREEN, NULL, NULL, 0 ) );
} 
 
AlohaWorkbench(wbport) 
struct MsgPort *wbport; 
/* "aloha" in hawaiian means both hello and goodbye */ 
{ 
    fetchIBase()->WBPort = wbport; 
} 
 
#endif
