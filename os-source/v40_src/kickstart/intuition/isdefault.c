/*** isdefault.c *************************************************************
 *
 *  isdefault.c -- Intuition State Machine default token handling
 *
 *  $Id: isdefault.c,v 38.19 93/01/14 14:22:11 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


/* This optimization depends on GfxBase maintaining a VBlank counter.
 * the idea is that in place of WaitTOF()/FreeXXX(), we send ourselves
 * a token (reminder) to FreeXXX after the VBlank counter has had
 * a chance to change, thus taking a WaitTOF() clean out of RethinkDisplay()
 * and setMousePointer().
 */
#define WAITTOF_OPTIMIZE	0

#include "intuall.h"
#include "classusr.h"

#define DERROR(x)	;
#define D(x)		;
#define DZOOM(x)	;
#define DV(x)		;
#define DCWB(x)		;
#define DSTUB(x)	;
#define DST(x)		;	/* show title	*/
#define DR(x)		;	/* refresh	*/

/* "not timer" macro	*/
#define NT(it) (it->it_Command != itTIMER  )

#define ITWINDOW	((struct Window *)object1)
#define ITSCREEN	((struct Screen *)object1)

doDefault()
{
    struct IntuitionBase	*IBase = fetchIBase();
    CPTR			object1, object2;
    ULONG			subcommand;
    ULONG			*IT_error;

    D( if NT(IT) printf("dD ") );

    /* This saves space */
    object1 = IT->it_Object1;
    object2 = IT->it_Object2;
    subcommand = IT->it_SubCommand;
    IT_error = &IT->it_Error;

    switch ( IT->it_Command )
    {
    case itMENUDOWN:	/* skip */
    case itMENUUP:	/* skip */
    case itSELECTDOWN:	/* skip */
    case itSELECTUP:	/* skip */
    case itRAWKEY:	/* skip */
    case itMETADRAG:	/* skip */
	return;		/* skip	*/

    case itACTIVATEWIN:	/* fix window borders, then skip */
	fixWindowBorders();
	return;

    case itACTIVATEGAD:	/* skip, return failure */
	*IT_error = 1;	/* default is to fail the command */
	return;

    case itMOUSEMOVE:	/* scroll, update mouse, updateMousePointer() */
	/* NOTE: some of the default processing (everything
	 * except autoScroll()) is duplicated in sscreendrag.c
	 */
	/* scaled versions of movement are in it_LongMouse.LX/LY */
	IBase->LongMouse.LX += IT->it_LongMouse.LX;
	IBase->LongMouse.LY += IT->it_LongMouse.LY;
	autoScroll( IBase->ActiveScreen );
	updateMousePointer(NULL);
	/* IDCMP message is state specific	*/
	return;

    case itTIMER:	/* doTiming(), tick active window */
	/* 
	 * go time out blinking screens
	 */
	doTiming();

	{
	    struct Window *AWindow = IBase->ActiveWindow;

	    /* tick active window */
	    if ( AWindow &&
		TESTFLAG( AWindow->IDCMPFlags, INTUITICKS ) &&
		! TESTFLAG( AWindow->Flags, WINDOWTICKED ) )
	    {
		activeEvent( IECLASS_TIMER, NULL );
		SETFLAG( AWindow->Flags, WINDOWTICKED );
	    }
	}
	return;

    case itDISKCHANGE:	/* Broadcast diskchange */
	/* it looks like you get both messages even if you ask for just one */
	sendAllAll( (ULONG) object1, (DISKINSERTED | DISKREMOVED));
	return;

    case itSETREQ:	/* Attempt to post requester */
	/* note no 'distant' version: always send REQSET to
	 * window that requester is appearing in.
	 */
	if ( IRequest( (struct Requester *)object2, ITWINDOW, FALSE ) ) {
	    *IT_error = 0;
	}
	else {
	    *IT_error = 1;	/** failed **/
	}
	return;

    case itCLEARREQ:	/* Take down requester */
	IEndRequest( ITWINDOW, (struct Requester *)object2 );
	return;
	
    case itCLOSEWIN:	/* Delink, delete layer, BorderPatrol() */
	/* delink from screen window list */
	LOCKIBASE();
	if (ITWINDOW->Parent)
	    ITWINDOW->Parent->Descendant = ITWINDOW->Descendant;
	if (ITWINDOW->Descendant)
	    ITWINDOW->Descendant->Parent = ITWINDOW->Parent;

	/* delink from screen */
	delinkThing( ITWINDOW, &ITWINDOW->WScreen->FirstWindow );
	UNLOCKIBASE();

	/* good bye layers:
	 * we do this within the state machine so that 
	 * I don't get screwed up by a button click in a
	 * layer for a window in process of being deleted
	 */
	LockLayerInfo( &ITWINDOW->WScreen->LayerInfo );
	if ( ITWINDOW->Flags & GIMMEZEROZERO )
	{
	    DeleteLayer( NULL, ITWINDOW->BorderRPort->Layer );
	}
	DeleteLayer( NULL, ITWINDOW->WLayer );
	DR( printf("have deleted layer\n"));
	DR( waitFrames( 30 ));

	/* NOTE WELL: be SURE this isn't called for the
	 * active window unless you are prepared to deal
	 * with that explicitly (by call to startIdleWindow)
	 */

	BorderPatrol( ITWINDOW->WScreen );
	UnlockLayerInfo( &ITWINDOW->WScreen->LayerInfo );
	return;

    case itVERIFY:	/* skip (extraneous if we get here) */
	DV( kquery("extraneous verify return!!" ) );
	return;		/* skip	*/
	
    case itREMOVEGAD:	/* Remove the gadget(s) */
	/* know that active gadget is not contained, else would
	 * have been deferred by sGadget
	 */
	D( printf("itREMOVEGAD, do it\n") );
	*IT_error = delinkGadgetList( (struct Gadget *)object1,
		(struct Gadget *)object2, subcommand );
	/* gadget/firstgadget/numgad */

	return;

    case itSETMENU:	/* Do it */
	realSetMenuStrip( ITWINDOW, (struct Menu *)object2, subcommand );
	return;

    case itCLEARMENU:	/* Do it */
	ITWINDOW->MenuStrip = NULL;
	return;

    case itZOOMWIN:	/* Do it */
	DZOOM( if ( subcommand ) printf("BAD DEFAULT ZOOM TOKEN!\n"); )
	IZoomWindow( ITWINDOW, FALSE );
	return;

    case itCHANGEWIN:	/* Do it */
	/* dumpBox("dD: itCHANGEWIN box", &IT->it_Box ); */
	IChangeWindow( ITWINDOW, &IT->it_Box, subcommand );
	return;

    case itDEPTHWIN:	/* Do it */
	IWindowDepth( ITWINDOW, subcommand, (struct Window *)object2 );
	return;

    case itMOVESCREEN:	/* Do it */
	IPreMoveScreen( ITSCREEN, (struct Rect *)object2, subcommand );
	return;

    case itDEPTHSCREEN:	/* Do it */
	IScreenDepth( ITSCREEN, subcommand );
	return;

    case itSHOWTITLE:	/* Do it */
	DST( printf("itSHOWTITLE, state: %lx\n", IBase->CurrentState ) );
	IShowTitle( ITSCREEN, subcommand );
	return;

    case itONOFFMENU:	/* Do it */
	IOnOffMenu( ITWINDOW, (ULONG)object2, subcommand );
	return;

    case itOPENSCREEN:	/* Do it */
	D( printf("default:: itOPENSCREEN, sc %lx\n", ITSCREEN) );

	*IT_error = IOpenScreen( ITSCREEN, (struct TagItem *)object2 );
	D( printf("default:: itOS returned %lx\n", *IT_error) );
	return;

    case itOPENWIN:	/* Do it */
	D( printf("itOPENWIN\n") );
	*IT_error = IOpenWindow( ITWINDOW, (struct BitMap *)object2,
			    subcommand );
	return;

    case itCLOSESCREEN:	/* Do it */
	DCWB( printf("closescreen default\n") );
	*IT_error = ICloseScreen( ITSCREEN );
	return;

    /* WHY is this thing around?  Why, just to single-thread
     * ADDEVENT processing.
     */
    case itNEWPREFS:	/* Broadcast NEWPREFS */
	sendAllAll( IECLASS_NEWPREFS, NEWPREFS );
	return;

    case itMODIFYIDCMP:	/* Do it */
	IModifyIDCMP( ITWINDOW, (ULONG) object2 );
	return;

    case itCLOSEWB:	/* Do it */

	DCWB( printf( "itCLOSEWB on task %lx: let's go\n", FindTask(0L) ) );
	{
	    struct Screen	*s;
	    struct Window	*w;

	    if ( ! IBase->WBPort )
	    {
		/* if there's no WBPort, then I don't need to notify WB,
		 * but it helps to make CloseWorkBench think I have
		 */
		ReplyMsg( (struct Message *)object1 );
		*IT_error = 0;
	    }
	    else if ( (s = (struct Screen *)findWorkBench())  )
	    { 
		/* WBPort exists */
		for ( w = s->FirstWindow;  w; w = w->NextWindow )
		{ 
		    if ( ! TESTFLAG( w->Flags, WBENCHWINDOW ) )
		    {
			DCWB( printf("itCWB: found non-wb window\n" ) );
			*IT_error = 1;
			return;
		    }
		} 

		/* OK, all Windows in WorkBench Screen are WBENCHWINDOWS
		 * so send the message along
		 */
		DCWB( printf("itCWB: put message %lx at port %lx\n",
			(struct Message *)object1, IBase->WBPort ) );
		DCWB(printf("port task is %lx\n", IBase->WBPort->mp_SigTask));
		PutMsg( IBase->WBPort, (struct Message *)object1 );
		*IT_error = 0;
	    } 
	    else
	    {
		DCWB( printf("no (port or wb screen).  fail\n") );
		/* want CloseWB to return FALSE if the WB screen isn't
		 * around.  This is probably backwards, but as spec'd
		 */
		*IT_error = 1;
	    }
	}
	DCWB( printf("itCWB done\n") );
	return;

    case itCHANGESCBUF:	/* Do it */
	IChangeScreenBuffer( ITSCREEN, (struct ScreenBuffer *)object2 );
	return;

    case itCOPYSCBUF:	/* Do it */
	ICopyScreenBuffer( ITSCREEN, (struct ScreenBuffer *)object2 );
	return;

    case itSETPOINTER:	/* Do it */
	ISetWindowPointer( ITWINDOW, (struct MousePointer *)object2 );
	return;

    case itGADGETMETHOD:	/* Do it */
	*IT_error = IDoGadgetMethodA( (struct Gadget *)object2, ITWINDOW,
	    (Msg) subcommand );
	return;

    case itMODIFYPROP:		/* Do it */
	IModifyProp( (struct Gadget *)object2, ITWINDOW, subcommand );
	return;

#if WAITTOF_OPTIMIZE
    case itFREECPRLIST:
	if ( subcommand != IBase->GfxBase->VBCounter )
	{
	    FreeCprList( (struct cprlist *)object1 );
	    FreeCprList( (struct cprlist *)object2 );
	}
	else
	{
	    deferToken();
	}
	return;

    case itFREESPRITEDATA:
	if ( subcommand != IBase->GfxBase->VBCounter )
	{
	    FreeSpriteData( (struct ExtSprite *)object1 );
	}
	else
	{
	    deferToken();
	}
	return;
#endif

    case itUNKNOWNIE:	/* Do nothing but pass it down */
	D( printf("doDefault passing along unknown input event\n") );
	activeEvent( IE->ie_Class, IE->ie_Code );
	break;


    DERROR( default: printf( "BAD TOKEN: IT %lx command %ld\n",
	IT, IT->it_Command ); Debug() );
    }
};

/* send normal mouse message with window coordinates,
 * if the mouse has moved (in window coords!), and record
 * last position.
 */
sendWindowMouse()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window		*AWindow = IBase->ActiveWindow;


    /* delta move: always send this, even if mouse has
     * run up against its boundaries
     */
    if ( TESTFLAG( AWindow->IDCMPFlags, DELTAMOVE ) )
    {
	/* sendIDCMP will pick up on DELTAMOVE and use
	 * ie XY coordinates as is
	 */
	activeEvent(IECLASS_RAWMOUSE, IECODE_NOBUTTON );
    }

    /*
     * don't send absolute mouse notice unless it actually moved.
     * New: If there was tablet-data for this message, we always
     * send the message, in case pressure or something changed.
     */
    else if ( ( ( IT->it_Tablet ) &&
	    ( TESTFLAG( AWindow->MoreFlags, WMF_TABLETMESSAGES ) ) ) ||
	( *((ULONG *) &AWindow->MouseY ) !=
	    *((ULONG *) &XWINDOW(AWindow)->LastMouseY ) ) )
    {
	IE->ie_X = AWindow->MouseX;
	IE->ie_Y = AWindow->MouseY;
	*((ULONG *) &XWINDOW(AWindow)->LastMouseY) =
	    *((ULONG *) &AWindow->MouseY );

	activeEvent(IECLASS_RAWMOUSE, IECODE_NOBUTTON );
    }
}

