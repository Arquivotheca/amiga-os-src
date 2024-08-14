/*** smenu.c *****************************************************************
 *
 *  sMenu state processing
 *
 *  $Id: smenu.c,v 38.20 93/01/14 14:24:44 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"

#include "smenu_protos.h"

/*---------------------------------------------------------------------------*/

static int returnMenuOK(int okcode);

static int returnMenuDMR(int okcode);

static int returnAllButMenuOK(int okcode);

static int realStartMenu(void);

static int returnDMROK(int okcode);

static int returnMenuKey(int okcode);

/*---------------------------------------------------------------------------*/

#define D(x)	;
#define DMH(x)	;	/* menu help	*/
#define DEND(x)	;

/* state sMenu
 *
 * global data used:
 *
 * local state data:
 */


/* transition startMenu()
 * called by:
 *	
 * with state data:
 * description:
 */
startMenu()
{
    struct IntuitionBase	*IBase = fetchIBase();
    int				returnMenuOK();

    D( printf("transition function startMenu, cstate: %ld\n", 
	IBase->CurrentState ));

    startVerify( returnMenuOK, IBase->ActiveWindow, MENUVERIFY, MENUHOT );
    return;
}


/* called after MENUVERIFY for the active window */
static
returnMenuOK( okcode )
{
    struct IntuitionBase	*IBase = fetchIBase();
    int				returnMenuDMR();
    struct Window		*awindow = IBase->ActiveWindow;

    D( printf("returnMenuOK okcode: %lx\n", okcode ) );

    D( if ( okcode != OKOK ) printf( "rMOK: didn't get OKOK\n") );

    /* The programmer may be poking window->DetailPen and window->BlockPen
     * in a pre-V39 attempt to control his menu colors.  He'd do that
     * at MENUVERIFY time, so now that we're back from there, let's
     * get a fresh concept of MenuBlockPen and MenuDetailPen if
     * he's not a NewLook Menus guy:
     */
    if ( !TESTFLAG( awindow->Flags, WFLG_NEWLOOKMENUS ) )
    {
	XWINDOW(awindow)->MenuBlockPen = awindow->BlockPen;
	XWINDOW(awindow)->MenuDetailPen = awindow->DetailPen;
    }

    switch ( okcode )
    {
    case OKCANCEL:
	D(printf("rMOK cancel menu verify with menuup\n") );
#if 0
	/* ZZZ: might I be sending two menuups, if the guy
	 * is really holding down the menu button?
	 *
	 * That's right, jimm, idlewindow will send this
	 * guy his MENUUP
	 */
	activeEvent( IECLASS_RAWMOUSE, MENUUP );
#endif
	startIdleWindow();
	return;
    case OKABORT:	/* go on with DMR, but don't show menu bar */
    case OKOK:
	/* go on ahead */
	D( printf("rMOK go on with DMR (maybe won't show menu bar)") );
	IBase->MenuAbort = ( okcode == OKABORT );	/* remember */
	startDMRTimeout( returnMenuDMR, okcode );
	return;
    default:
	D( PANIC( "rMOK default OKCODE!!!" ) );
	startIdleWindow();
	return;
    }
}

/*
 * pass this thing OKOK if you want the menus,
 * OKCANCEL if you want the DMR's, OKABORT if you want 
 * to bail out and go back to sIdleWindow
 *
 */
static
returnMenuDMR( okcode )
{
    struct IntuitionBase	*IBase = fetchIBase();
    int				returnDMROK();

    D( printf("returnMenuDMR code %lx\n", okcode ) );

    switch ( okcode )
    {
    case OKOK:		/* wants menus	*/
	D( printf("rMDMR: wants menus.\n") );
	if ( ! IBase->MenuAbort ) break; /* go do menus */

	/* fall through into abort case */
    case OKABORT:
	/* let him know that menus are a over with */
	D( printf("rMDMR abort from DMR timeout.  release or no dmr\n"));
	activeEvent( IECLASS_MENULIST, MENUNULL );
	startIdleWindow();
	return;

    case OKCANCEL:	/* wants DMR	*/
	D( printf("rMDMR: wants dmr, do reqverify.\n") );
	startVerify( returnDMROK, IBase->ActiveWindow, REQVERIFY, 0 );
	return;

    default:
	D( PANIC( "returnMenuDMR: unanticipated code" ) );
	startIdleWindow();
	return;
    }

    /*** verify other windows ***/
    /* was: "getAllButMenuOK()" */

    /* dummy pointer "before" first window */
    IBase->AllButWindow =
	(struct Window *) &IBase->ActiveScreen->FirstWindow;
    returnAllButMenuOK( OKOK );
}

/*
 * in here I assume that the window list won't be
 * changing around during sVerify
 *
 * have to be a little careful: want to be input.device if
 * I'm to call realStartMenu().
 */
static
returnAllButMenuOK( okcode )
{
    struct Window	*window;
    struct IntuitionBase	*IBase = fetchIBase();


    /* jimm: 5/14/90: bail out if the guy says ABORT	*/
    if ( okcode == OKABORT )
    {
	/* OKABORT indicates a failure (eg. no memory), since
	 * the inactive windows can't cancel their request and
	 * a time-out converts into OKOK.  Send an IECLASS_MENULIST
	 * with code MENUNULL to let the active window know that
	 * menus are over with, then send the "menu clear" signal
	 * to all inactive windows that got their MENUWAITING
	 * MENUVERIFY message.
	 */

	/* let him know that menus are a over with */
	activeEvent( IECLASS_MENULIST, MENUNULL );

	sendAllMenuClear( IBase->ActiveScreen );
	/* this fix is the correct thing to prevent a hanging menu
	 * strip.
	 */
	removeMStrip( IBase->ActiveScreen );
	startIdleWindow();
	return;
    }

    /* find next inactive window that wants MENUVERIFY/MENUWAITING */
    window = IBase->AllButWindow->NextWindow; 	/* IBase->ABW never null */
    while ( window
	    && ((window == IBase->ActiveWindow) 
		|| ! TESTFLAG( window->IDCMPFlags, MENUVERIFY )) )
    {
	window = window->NextWindow;
    }

    if ( window )	/* another contestant */
    {
	IBase->AllButWindow = window;
	SETFLAG( window->MoreFlags, WMF_NEEDMENUCLEAR );

	/* come back to this routine  right here */
	startVerify( returnAllButMenuOK, window, MENUVERIFY, MENUWAITING );
	return;
    }
    else		/* checked with everybody	*/
    {
	realStartMenu();
	return;
    }
}

/*
 * Have to be a little careful: this routine grabs the
 * locks,  so it has to be the input.device
 */
static
realStartMenu()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Screen		*AScreen = IBase->ActiveScreen;

    /* Set the ShiftBack variables to something safe, just in case */
    IBase->ShiftBack.X = 0;
    IBase->ShiftBack.Y = 0;

    if ( TESTFLAG( IBase->NewIControl, IC_DOMAGICMENU ) )
    {
	int	asleft, astop;
	struct Point	movepoint;

	asleft = AScreen->LeftEdge;
	astop = AScreen->TopEdge;

	/* head toward 0,0 from negative positions
	 * might not go to (0,0)
	 */
	movepoint.X =  imax( -asleft, 0 );
	movepoint.Y = imax( -astop, 0 );
	IMoveScreen( AScreen, &movepoint, SPOS_MENUSNAPRANGE );
	/* Left-Amiga means don't shift the menus back.  So
	 * we leave ShiftBack at zero unless left-Amiga is not
	 * held.
	 */
	if ( !TESTFLAG( IE->ie_Qualifier, IEQUALIFIER_LCOMMAND ) )
	{
	    IBase->ShiftBack.X = asleft - AScreen->LeftEdge;
	    IBase->ShiftBack.Y = astop - AScreen->TopEdge;
	}
    }
    
    /* fresh menus, boys */
    IBase->OptionList = MENUNULL;
    IBase->MenuDrawn = MENUNULL;
    IBase->MenuSelected = MENUNULL;

    CLEARFLAG( IBase->Flags, DRAGSELECT );
    SETFLAG( IBase->ActiveWindow->Flags, MENUSTATE );

    /* limit mouse to top edge of active screen for menus */
    {
	struct Point	holdmin;

	holdmin.X = 0;
	holdmin.Y = AScreen->TopEdge;
	screenToMouse( &holdmin, AScreen );
	IBase->HoldMinYMouse = holdmin.Y;
    }

    D( printf("sMenu lock layers, linfo %lx\n", &AScreen->LayerInfo ) );
    LockLayers( &AScreen->LayerInfo );
    D( printf("LOCKED\n") );
    getMenu();

    /*** SKIP_WAIT business: reuse the event as a normal menu
     *** input unless doing straight in menudown deal ZZZ:
     ***/

    IBase->CurrentState = sMenu;
}


static
returnDMROK( okcode )
{
    struct IntuitionBase	*IBase = fetchIBase();

    switch ( okcode )
    {
    case OKOK:		/* wants the DMR	*/
    D( printf("returnDMROK wants the DMR\n") );
    if ( IRequest( IBase->ActiveWindow->DMRequest, IBase->ActiveWindow, 1))
    {
	D( printf("rDMROK: succeeded with DMrequester\n") );
	 startRequester();   
	 return;
    }
    /* fall through: cancel if can't get up Requester */

    case OKCANCEL:	/* shouldn't happen */
    case OKABORT:	/* timeout */
	D( printf("rDMROK cancelling REQVERIFY\n") );

	/* give him termination notification */
	activeEvent( IECLASS_REQUESTER, IECODE_REQCLEAR );
	startIdleWindow();
	return;

    default:
	D( PANIC( "returnDMROK: unanticipated code" ) );
	startIdleWindow();
	return;
    }
}


/*
 * caller has found a valid match for a command key, and wants
 * us to check menu verify then send the idcmp message.
 *
 * ZZZ: might have to use a return state to do this during
 * string gadgets or other states than sIdleWindow.
 */
startMenuKey( menunum )
UWORD	menunum;
{
    struct IntuitionBase	*IBase = fetchIBase();
    int				returnMenuKey();

    D( printf("startMenuKey: menunum (set MenuSelected)  %lx\n", menunum) );
    IBase->MenuSelected = menunum;	/* menus can't be in use */
    startVerify( returnMenuKey, IBase->ActiveWindow, MENUVERIFY, MENUHOT );
}

/*
 * called after MENUVERIFY for command-key menu equivalents
 */
static
returnMenuKey( okcode )
{
    struct IntuitionBase	*IBase = fetchIBase();
    UWORD			firstoption = MENUNULL;

    D( printf("rMK: verify code %lx\n", okcode ) );
    
    switch ( okcode )
    {
    /* NB: 3.00 and prior didn't send the IECLASS_MENULIST/MENUNULL
     * event for OKABORT
     */
    case OKABORT:
    case OKCANCEL:
	activeEvent( IECLASS_MENULIST, MENUNULL );
	break;
	break;
    case OKOK:
	D( printf("rMK resetMenu\n") );
	resetMenu( IBase->ActiveWindow,
		~MIDRAWN,
		~ISDRAWN & ~HIGHITEM & ~MENUTOGGLED,
		~HIGHITEM & ~MENUTOGGLED);

	D( printf("rMK updateOptions, MenuSelected %lx\n",
	    IBase->MenuSelected) );

	updateOptions( &firstoption, FALSE );
	D( printf("send menu equiv. for this: %lx\n", firstoption ) );
	activeEvent( IECLASS_MENULIST, firstoption );
    }


    if ( IBase->MenuLendingReturn )
    {
	/* Transmogrify token into an itACTIVATEWIN one,
	 * and then fall through to startIdleWindow().
	 */
	IT->it_Command = itACTIVATEWIN;
	IT->it_Object1 = (CPTR) IBase->MenuLendingReturn;
	IT->it_Object2 = AWIN_NORMAL;
    }
    startIdleWindow();
}

/*
 * state dispatcher
 * state transitions called:
 */
dMenu()
{
    struct IntuitionBase	*IBase = fetchIBase();
    UWORD			saveselected;
    struct Screen		*AScreen = IBase->ActiveScreen;

    /* initial processing --- does menu select list processing.
     * don't worry about MENUUP: it's checked again later for
     * termination condition
     */
    switch ( IT->it_Command )
    {
    case itMOUSEMOVE:	/* do menu stuff	*/
	doDefault();
	/* fall through */
    case itMENUDOWN:	/* shouldn't happen	*/
    case itMENUUP:	/* end menu session	*/
    case itSELECTDOWN:	/* start drag select	*/
    case itSELECTUP:	/* end drag select	*/
    case itRAWKEY:	/* update the menu in case this is the first event */

	getMenu();
	if ( TESTFLAG( IBase->Flags, DRAGSELECT ) )
	{
	    updateOptions( &IBase->OptionList, TRUE );
	}
	/* go down and do some more processing */
	break;

    case itACTIVATEWIN:	/* defer */
    case itSETREQ:	/* defer */
    case itCLEARREQ:	/* defer */
    case itCLOSEWIN:	/* defer */
    case itSHOWTITLE:	/* defer */
    case itOPENWIN:	/* defer */
    case itONOFFMENU:	/* defer */
    case itSETMENU:	/* defer */
    case itCLEARMENU:	/* defer */
    case itCHANGEWIN:	/* defer */
    case itZOOMWIN:	/* defer */
    case itDEPTHWIN:	/* defer */
    case itOPENSCREEN:	/* defer */
    case itCLOSESCREEN:	/* defer */
    case itCOPYSCBUF:	/* defer */
    case itGADGETMETHOD:/* defer */
    case itMODIFYPROP:	/* defer */
	deferToken();
	return;

    /* skip */
    case itMOVESCREEN:	/* skip */
    case itDEPTHSCREEN:	/* skip */
	return;		

    case itCHANGESCBUF:	/* error */
	if ( (struct Screen *)IT->it_Object1 == AScreen )
	{
	    IT->it_Error = 1;
	}
	else
	{
	    doDefault();
	}
	return;

    /*** default processing	***/
#if 0
    /*case itRAWKEY: */		/* skip */
    case itTIMER:	/* do default */
    case itDISKCHANGE:	/* do default */
    case itACTIVATEGAD:	/* do default */
    case itMETADRAG:	/* do default */
    case itVERIFY:	/* do default */
    case itREMOVEGAD:	/* do default */
    case itNEWPREFS:	/* do default */
    case itMODIFYIDCMP:	/* do default */
    case itSETPOINTER:	/* do default */
#endif
    default:		/* itDEFAULT: do default */
	doDefault();
	return;
    }

    /* If there is more than one screen visible, the mouse can be
     * higher up than the drag bar of the active window's screen.
     * as soon as the mouse crosses below that fated point, we want to
     * limit the mouse movement to never rise above the top of
     * the drag bar while we're in menu-state:
     */
    if ( IBase->LongMouse.LY >= IBase->HoldMinYMouse )
    {
	SETFLAG( IBase->Flags, INMOUSELIMITS|SCREENMLIMITS );
	IBase->ScreenMouseLimits.MinX = IBase->ScreenMouseLimits.MinY = 0;
	IBase->ScreenMouseLimits.MaxX = AScreen->Width - 1;
	IBase->ScreenMouseLimits.MaxY = AScreen->Height - 1;
	rethinkMouseLimits();
    }

    /* some more processing */
    switch ( IT->it_Command )
    {
    case itSELECTDOWN:	/* start drag select	*/
	resetMenu(IBase->ActiveWindow, -1, ~MENUTOGGLED, ~MENUTOGGLED);
	SETFLAG( IBase->Flags, DRAGSELECT ); /* start drag selecting */
	updateOptions(&IBase->OptionList, TRUE);
	return;

    case itSELECTUP:	/* end drag select	*/
	CLEARFLAG( IBase->Flags, DRAGSELECT ); /* stop drag selecting */
	return;

    case itMENUUP:	/* end menu session	*/
    case itRAWKEY:	/* end session if HELP	*/

	if ( IT->it_Command == itMENUUP )
	{
	    DMH( printf("menu up\n") );
	    updateOptions( &IBase->OptionList, FALSE );
	    DMH( printf("update done\n") );
	}
	else	/* rawkey	*/
	{

	    DMH( printf("rawkey during menus, code %lx\n", IE->ie_Code ) );

	    if ( IE->ie_Code != KEYCODE_HELP ||
		! TESTFLAG( IBase->ActiveWindow->MoreFlags,
		   WMF_USEMENUHELP ) )
	    {
		break;
	    }
	}

	/*** Menu-end processing shared by MENUUP and MENUHELP ***/

	IBase->MenuDrawn = MENUNULL;
	saveselected = IBase->MenuSelected;
	IBase->MenuSelected = MENUNULL;

	CLEARFLAG( IBase->Flags, INMOUSELIMITS|SCREENMLIMITS );
	CLEARFLAG( IBase->ActiveWindow->Flags, MENUSTATE );

	if ( TESTFLAG( IBase->Flags, SUBDRAWN ) ) eraseSub();
	if ( TESTFLAG( IBase->Flags, ITEMDRAWN ) ) eraseItem();

	/* want to protect my BorderPatrol, so I put this
	 * under the layers lock.
	 * BUT WAIT: that's bad locking order.
	 * removeMStrip has to redraw gadgets, and thus
	 * get gadgets lock.  That must come BEFORE
	 * a layer lock (but AFTER LayerInfo lock).
	 *
	 * So, we nest the LayerInfo, which will protect BorderPatrol(),
	 * and we release the layers.
	 */
#if 1
	DEND( printf("start toremove mstrip\n"));
	LockLayerInfo( &AScreen->LayerInfo );
	UnlockLayers( &AScreen->LayerInfo );
	removeMStrip( AScreen );
	UnlockLayerInfo( &AScreen->LayerInfo );
	DEND( printf("done remove mstrip\n"));
#else
	removeMStrip( AScreen );
	UnlockLayers( &AScreen->LayerInfo );
#endif

	if ( TESTFLAG( IBase->NewIControl, IC_DOMAGICMENU ) )
	    IMoveScreen( AScreen, &IBase->ShiftBack, SPOS_REVALIDATE );
	freeMouse();

	sendAllMenuClear( AScreen );

	if ( IT->it_Command == itMENUUP )
	{
	    DMH( printf("menupick %lx\n", IBase->OptionList ) );
	    activeEvent( IECLASS_MENULIST, IBase->OptionList );
	}
	else
	{
	    DMH( printf("send menuhelp, code %lx\n", 
		saveselected ) );
	    activeEvent( IECLASS_MENUHELP, saveselected );
	}

	if ( IBase->MenuLendingReturn )
	{
	    /* Transmogrify token into an itACTIVATEWIN one,
	     * and then fall through to startIdleWindow().
	     */
	    IT->it_Command = itACTIVATEWIN;
	    IT->it_Object1 = (CPTR) IBase->MenuLendingReturn;
	    IT->it_Object2 = AWIN_NORMAL;
	}
	startIdleWindow();
	return;
    }
}

sendAllMenuClear( screen )
struct Screen	*screen;
{
    struct Window	*w;

    for ( w = screen->FirstWindow; w; w = w->NextWindow)
    {
	if ( TESTFLAG( w->MoreFlags, WMF_NEEDMENUCLEAR ) )
	{
	    CLEARFLAG( w->MoreFlags, WMF_NEEDMENUCLEAR );
	    windowEvent( w, IECLASS_RAWMOUSE, MENUUP );
	}
    }
}

/*
 * searches a list of Menus for a command key match.
 * returns menunum, MENUNULL if no match
 */
UWORD
findMenuKey( menu, code )
struct Menu	*menu;
{
    WORD	menunum;
    WORD	itemnum;
    WORD	subnum;

    struct MenuItem	*item, *sub;

    D( printf("findMenuKey, menu %lx, code %lx\n", menu, code ) );

    if ( ! code ) return ( MENUNULL );
    code = ToUpper( code );

    D( printf("uppercode: %lx\n", code ) );

    menunum = -1;
    while ( menu )
    {
	menunum++;
	item = menu->FirstItem;
	itemnum = -1;
	while (item)
	{
	    itemnum++;
	    subnum = -1;
	    if (sub = item->SubItem)
	    {
		while (sub)
		{
		    subnum++;
		    if ( ToUpper( sub->Command ) == code )
		    {
			D( printf("found match subitem\n") );
			goto FOUNDMATCH;
		    }
		    sub = sub->NextItem;
		}
	    }
	    else if ( ToUpper( item->Command ) == code )
	    {
		D( printf("found match item\n") );
		goto FOUNDMATCH;
	    }
	    D(else if ( item->Command)
		printf("no match at item level %lx, (upper) %lx\n", 
		item->Command, ToUpper( item->Command ));)

	    item = item->NextItem;
	}
	menu = menu->NextMenu;
    }

    /* no match */
    return (  MENUNULL );

FOUNDMATCH:
    D( printf("fMK: returning menunum %lx\n",
	SHIFTSUB(subnum) | SHIFTITEM(itemnum) | SHIFTMENU(menunum) ) );

    D( printf("fMK, that's %lx, %lx, %lx\n", menunum, itemnum, subnum ) );

    {
    UWORD composite;

    composite =  ( SHIFTSUB(subnum) | SHIFTITEM(itemnum) | SHIFTMENU(menunum) );
    D( printf("composite var %lx\n", composite ) );
    }

    return ( SHIFTSUB(subnum) | SHIFTITEM(itemnum) | SHIFTMENU(menunum) );
}
