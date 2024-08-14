/*** idcmp.c *****************************************************************
 *
 *  the IDCMP routines
 *
 *  $Id: idcmp.c,v 38.8 93/04/02 11:01:34 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/** #define DEBUG 1 **/
/*???#define LOCK 1*/

#include "intuall.h"

#define D(x)	;
#define DN(x)	;	/* notifyidcmp	*/
#define DV(x)	;
#define DFLAGS(x) ;
#define DTAB(x)	;	/* tablet stuff */

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif
#include "idcmp_protos.h"

/*---------------------------------------------------------------------------*/

static int clearPending(struct Window * window,
                        ULONG class,
                        int qualifier);

static struct IntuiMessage * findFreeMessage(struct Window * AWindow);

static int sendAllOne(ULONG class,
                      ULONG code,
                      ULONG sendflag,
                      struct Screen * screen);

/*---------------------------------------------------------------------------*/



/* ======================================================================= */
/* some of these mappings might be overridden based on code value	*/
ULONG	IDCMPclasses[] =
{
    0,				/*  IECLASS_NULL            */
    IDCMP_RAWKEY,		/*  ** IECLASS_RAWKEY       */
    IDCMP_MOUSEBUTTONS,		/*  ** IECLASS_RAWMOUSE     */
    0,				/*  IECLASS_EVENT           */
    0,				/*  IECLASS_POINTERPOS      */
    0,				/*  missing class	    */
    IDCMP_INTUITICKS,		/*  IECLASS_TIMER           */
    IDCMP_GADGETDOWN,		/*  IECLASS_GADGETDOWN      */
    IDCMP_GADGETUP,		/*  IECLASS_GADGETUP        */
    IDCMP_REQSET,		/*  ** IECLASS_REQUESTER    */
    IDCMP_MENUPICK,		/*  IECLASS_MENULIST        */
    IDCMP_CLOSEWINDOW,		/*  IECLASS_CLOSEWINDOW     */
    IDCMP_NEWSIZE,		/*  IECLASS_SIZEWINDOW      */
    IDCMP_REFRESHWINDOW,	/*  IECLASS_REFRESHWINDOW   */
    IDCMP_NEWPREFS,		/*  IECLASS_NEWPREFS        */
    IDCMP_DISKREMOVED,		/*  IECLASS_DISKREMOVED     */
    IDCMP_DISKINSERTED,		/*  IECLASS_DISKINSERTED    */
    IDCMP_ACTIVEWINDOW,		/*  IECLASS_ACTIVEWINDOW    */
    IDCMP_INACTIVEWINDOW,	/*  IECLASS_INACTIVEWINDOW  */
    0,				/*  IECLASS_NEWPOINTERPOS   */
    IDCMP_MENUHELP,		/*  IECLASS_MENUHELP   	    */
    IDCMP_CHANGEWINDOW,		/*  IECLASS_CHANGEWINDOW    */
    IDCMP_GADGETHELP		/*  IECLASS_GADGETHELP      */
};

#define  NUMIDCMPCLASSES ((sizeof (IDCMPclasses ))/( sizeof (ULONG )))

/* returns 0 if translation failed */
ULONG
translateIEtoIDCMP( ieclass, iecode )
UBYTE	ieclass;
UWORD	iecode;
{
    ULONG	idcmpclass = 0;

    if ( ieclass < NUMIDCMPCLASSES )
    {
	idcmpclass = IDCMPclasses[ ieclass ];

	/* patch up special cases	*/
	switch ( ieclass )
	{
	/* caller has to handle VANILLAKEY stuff */
	case IECLASS_RAWMOUSE:	/* starting out with MOUSEBUTTONS */
	    if ( iecode == IECODE_NOBUTTON ) idcmpclass = MOUSEMOVE;
	    break;
	case IECLASS_REQUESTER:	/* starting out with REQSET	*/
	    if ( iecode == IECODE_REQCLEAR ) idcmpclass = REQCLEAR;
	    break;
	}
    }

    return ( idcmpclass );
}


#if 0

IDCMP ports are now PRIVATE

UBYTE PortMessage[] = "IDCMP";
#else
#define PortMessage (NULL)
#endif

/*** intuition.library/ModifyIDCMP ***/

/*
 * Return value added for 2.03.  Internally, we rely on this!
 * Externally, we should publish it for V37.
 *
 */

ModifyIDCMP(window, flags)
struct Window *window;
ULONG flags;
{
    struct Message	*GetMsg();
    struct MsgPort	*tmpport;

    int portcreated = 0;

    DFLAGS( printf( "ModifyIDCMP window %lx, flags %lx\n", window, flags ));

    /*
     * when the flags are non-zero, there must be msg ports
     * create if necessary, before changing the flags
     */
    if ( flags )
    {
	/* flags value was non-zero, so we want the message ports */
	if ( window->WindowPort == NULL ) /* does it exist already? */
	{
	    /* The reply-MsgPort structure is embedded in the XWindow
	     * structure, so initialize it and set the WindowPort
	     * pointer to point at it.
	     */
	    XWINDOW( window )->WinPort.mp_Flags = PA_IGNORE;
	    NewList( &XWINDOW( window )->WinPort.mp_MsgList );
	    window->WindowPort = &XWINDOW( window )->WinPort;

	    /* Initialize this PA_IGNORE port */
	    portcreated = 1;

	    /* my Task addr will be Console's address when I need to Wait() */
	}
	if ( window->UserPort == NULL ) /* does it exist already? */
	{
	    /* it doesn't exist yet, so let's get it */
	    /* first, allocate and initialize the message port */
	    if (! ( window->UserPort = (struct MsgPort *)CreateMsgPort() ) )
	    {
		if ( portcreated )
		{
		    window->WindowPort = NULL;
		}
		return( FALSE );
	    }
	}
    }

    DFLAGS( printf("MIDCMP, call doISM\n") );

    /* get the flags changed in a token-atomic way 	*/
    doISM( itMODIFYIDCMP, window, flags );

    DFLAGS( printf("MIDCMP, doISM returned\n") );

    DV( if ( flags != window->IDCMPFlags ) {PANIC( "ModifyIDCMP" );} )

    /*
     * when the flags are changed to zero, delete any ports we find
     * but not until the flag values have been changed in the window
     */
    if ( flags == 0 )
    {
	/* if they exist already, delete them */
	if ( tmpport = window->UserPort )
	{
	    window->UserPort = NULL;

	    DFLAGS( printf("free userport at %lx\n", window->UserPort ) );
	    DFLAGS( kpause( "check memory" ) );
	    /* Peter 15-Aug-90; FreeSignal() used to be needed when we
	     * called intuition's deletePort(), which didn't itself call
	     * FreeSignal().
	     */

	    DeleteMsgPort(tmpport);

	    DFLAGS( printf( "port freed\n") );
	}

	/*
	 * we know that no messages are hanging off of the reply port,
	 * because: 
	 *	doISM [IModifyIDCMP] shut the flags off, so nobody would
	 *	send any more messages.  Then it replied all messages
	 *	still on the UserPort (to the WindowPort),
	 *	then it reclaimed all messages on the WindowPort.
	 *  
	 */

	/* know intuition is not waiting on WindowPort, since
	 * reclaimMessages() (inside IModifyIDCMP()) took care
	 * of that condition
	 */

	/* Since the WindowPort isn't created anymore, "freeing" it is
	 * as simple as this:
	 */
	window->WindowPort = NULL;
    }

    DFLAGS( printf("MIDCMP done\n") );
    return( TRUE );
}


/*
 * called within state machine.
 * do all the atomic stuff here, including:
 *	change flag values
 *	reclaim messages (deal with VerifyReturn)
 */
IModifyIDCMP( window, flags )
struct Window	*window;
ULONG		flags;
{

    DFLAGS( printf( "IMIDCMP window %lx, flags %lx\n", window, flags ));
    window->IDCMPFlags = (ULONG) flags;
    if ( ! flags )
    {
	if (window->UserPort)
	{
	    struct IntuiMessage *msg;

	    /* have to reply all messages to get them on
	     * the WindowPort Queue
	     */
	    while (msg = (struct IntuiMessage *) GetMsg(window->UserPort)) 
	    {
		DFLAGS( printf("+%lx",msg ) );
		ReplyMsg(msg);
		DFLAGS( printf("-") );
	    }
	    DFLAGS( printf("msgs freed\n") );
	}

	/* Now reclaim all the messages on the WindowPort queue */
	reclaimMessages( window );	
    }
}

/* reclaim and free replied  asynchronous message to
 * Workbench port IBase->WBPort
 */
reclaimWBMsg()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Message	*msg;
    struct Message	*GetMsg();

    while ( msg = GetMsg( &IBase->WBReplyPort ) )
    {
	FreeMem( msg, sizeof ( struct IIntuiMessage ));
    }
}


reclaimMessages(window)
struct Window *window;
/* GetMsg() while there's any to get.  For all those that are of type
 * NT_REPLYMSG, turn into LONELY messages in the big list.  For all that
 * we don't know about, flag errors for now (receive them warmly later)
 */
{
    struct IntuitionBase	*IBase = fetchIBase();	
    struct IntuiMessage *message;
    struct TabletData *td;

    if (NOT knownWindow(window)) goto OUT;
    if (window->WindowPort == NULL) goto OUT;

    while (message = (struct IntuiMessage *)GetMsg(
	    window->WindowPort))
	{
	if (message->ExecMessage.mn_Node.ln_Type == NT_REPLYMSG)
	    {

	    clearPending( window, message->Class, message->Qualifier );

	    /* delete the taglist */
	    if ( message->Class == IDCMPUPDATE )
	    {
		DN( printf("reclaim message at %lx, tags at %lx\n",
			message, message->IAddress ));
		FreeTagItems( message->IAddress );
	    }

	    if ( td = IIMSG(message)->iim_TabletData )
	    {
		FreeTagItems( td->td_TagList );
	    }
	    IIMSG(message)->iim_TabletData = NULL;

	    /* is somebody waiting for a verify return */
	    if ( message == IBase->VerifyMessage )
	    {
		DV( printf("create a verify message\n") );
		/* ZZZ: if this works, move it next to ism.c	*/
		{
		    struct InputToken	*it;

		    if ( it= (struct InputToken *)
			preSendISM(itVERIFY, window, NULL, message->Code))
		    {
			/* just stick it on there right now and
			 * reuse it this invocation of the
			 * state machine
			 */
			AddHead( &IBase->TokenQueue, it );
		    }
		}
		IBase->VerifyMessage = NULL;
	    }

	    message->Class = LONELYMESSAGE;
	    }
/*	else Alert( AN_BadMessage ); */
	}
OUT:
}

/*
 * have a look and see if the VerifyMessage has been responded to,
 * but I haven't gotten to it yet.  Don't remove it or anything.
 */
snoopVerifyReply()
{
    struct Node	*ln, *succ;
    struct IntuitionBase	*IBase = fetchIBase();
    int	retval = 0;

    Forbid();
    if ( IBase->VerifyWindow )
    {
	for ( ln = IBase->VerifyWindow->WindowPort->mp_MsgList.lh_Head;
	    succ = ln->ln_Succ; ln = succ )
	{
	    if ( ln == (struct Node *) IBase->VerifyMessage )
	    {
		retval = 1;
		break;
	    }
	}
    }
    Permit();
    return ( retval );
}

/*
 * some messages received allow more to be queued.
 */
static
clearPending( window, class, qualifier )
struct Window	*window;
ULONG	class;
{
    if ( class == INTUITICKS)		/* one tick	*/
    {
	CLEARFLAG( window->Flags, WINDOWTICKED );
    }
    else if ( class == MOUSEMOVE )	/* several mouse messages	*/
    {
	--(XWINDOW(window)->MousePending);
    }
    else if ( ( class==RAWKEY || class==VANILLAKEY || class==IDCMPUPDATE )
	&&  TESTFLAG( qualifier, IEQUALIFIER_REPEAT ) )
    {
	--(XWINDOW(window)->RptPending);
    }

    /* ZZZ: repeat qual in IDCMPUPDATE */
}


static struct IntuiMessage *
findFreeMessage(AWindow)
struct Window *AWindow;
/* feel through the list of allocated messages and return the pointer to the
 * first LONELY (not currently in use) message found.  If no LONELY messages
 * are found, return NULL
 */
{
    struct IntuiMessage *message;
    LONG msgsize;

    assertLock("findFreeMessage", IBASELOCK );

    reclaimMessages(AWindow);

    message = AWindow->MessageKey;

    while (message)
	{
	if (message->Class == LONELYMESSAGE) return (message);
	message = message->SpecialLink;
	}

    msgsize = sizeof( struct IIntuiMessage );
    if ( TESTFLAG( AWindow->MoreFlags, WMF_TABLETMESSAGES ) )
    {
	/* Tablet-aware windows get bigger messages! */
	msgsize = sizeof( struct TIIntuiMessage );
    }
    /* Use AllocVec/FreeVec so freeing them is independent of
     * size differences between tablet & non-tablet windows
     */
    if (message = (struct IntuiMessage *)AllocVec( msgsize, MEMF_PUBLIC | MEMF_CLEAR))
	{
	message->Class = LONELYMESSAGE;
	message->ExecMessage.mn_Node.ln_Type = NT_MESSAGE;

	/* following is a valid link, whether or not Key is NULL */
	message->SpecialLink = AWindow->MessageKey;
	AWindow->MessageKey = message;
	}

    return(message);
}

/*
 * allocates and initializes IDCMP message
 * 'ie' might be NULL if used for boopsi stuff
 */
struct IntuiMessage *
initIMsg( class, code, ie, window, tablet )
ULONG class;
ULONG code;
struct InputEvent *ie;
struct Window *window;
struct TabletData *tablet;
{
    struct IntuiMessage	*imsg;
    struct IntuitionBase *IBase = fetchIBase();
    struct IntuiMessage *findFreeMessage();

    if ( imsg = findFreeMessage( window ) )
    {
	imsg->ExecMessage.mn_ReplyPort = window->WindowPort;
	imsg->ExecMessage.mn_Length = sizeof *imsg - sizeof (struct Message);
	imsg->Class = class;
	imsg->Code = code;
	imsg->IAddress = ie? (APTR)ie->ie_EventAddress: NULL;
	imsg->MouseX = 0;	/* For MOUSEMOVE: replaced by deltas */
	imsg->MouseY = 0;
	if ( !TESTFLAG( window->IDCMPFlags, DELTAMOVE ) )
	{
	    imsg->MouseX = window->MouseX;
	    imsg->MouseY = window->MouseY;
	}
	imsg->IDCMPWindow = window;
	imsg->Seconds = IBase->Seconds;
	imsg->Micros = IBase->Micros;
	imsg->Qualifier = ie? ie->ie_Qualifier: 0;

	/* The IntuiMessage contains a pointer to TabletData.  Our
	 * trick here is that for WATabletMessage windows, the
	 * IntuiMessage actually contains an Instance of TabletData,
	 * pointed at by iim_TabletData!
	 */
	DTAB( printf("Tablet %lx\n", tablet) );
	IIMSG(imsg)->iim_TabletData = NULL;
	if ( TESTFLAG( window->MoreFlags, WMF_TABLETMESSAGES ) && ( tablet ) )
	{
	    struct TagItem *CloneTagItems();

	    IIMSG(imsg)->iim_TabletData = &IIMSG(imsg)->tiim_TabletDataInstance;

	    /* Structure copy the guy! */
	    IIMSG(imsg)->tiim_TabletDataInstance = *tablet;
	    IIMSG(imsg)->tiim_TabletDataInstance.td_TagList =
		CloneTagItems( tablet->td_TagList );


	    DTAB( printf("Done copying\n" ) );
	}

#if 0
	/* Peter 2-Jul-91: No we don't need this any time */
	/* do I need to do this everytime? */
	window->WindowPort->mp_SigTask = NULL;
	window->WindowPort->mp_Flags = PA_IGNORE;
#endif
    }
    return ( imsg );
}


static
sendAllOne( class, code, sendflag, screen)
ULONG class, code, sendflag;
struct Screen *screen;
/* Test all Windows for an IDCMPFlag that matches with sendflag.  If a
 * match is found, send the event via the IDCMP.
 *
 * NOTE: will not broadcast RAWMOUSE DELTAMOVE correctly (jimm: hmmm?)
 */
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window *window;

    assertLock("sendAllOne", IBASELOCK );

    if (knownScreen(screen)) window = screen->FirstWindow;
    else goto OUT;

    while ( window )
    {
	if ( TESTFLAG( window->IDCMPFlags, sendflag ) )
	{
	    sendIDCMP( class, code, IT, window, 0);
	}
	window = window->NextWindow;
    }
OUT:
}


/*
 * big broadcast, used for diskchange and newprefs,
 * does input.device/console multi-broadcast, in addition
 * to idcmp blitz.
 */
sendAllAll( class, sendflag)
ULONG class, sendflag;
{
    struct Screen *screen;
    struct IntuitionBase *IBase = fetchIBase();

    assertLock("sendAllAll", IBASELOCK);

    screen = IBase->FirstScreen;
    
    while (screen)
    {
	sendAllOne( class, NULL, sendflag, screen);
	screen = screen->NextScreen;
    }

    /*
     * and for the non-idcmp users,
     * broadcast a copy of the input event
     */
    broadcastIEvent( class );

OUT:
}


#if 0	/* should this be called??? */
sendAllNewPrefs( ie )
struct InputEvent *ie;
{
    /* jimm: 3/20/86: NEWPREFS, not IECLASS_NEWPREFS	*/
    sendAllAll( ie, IECLASS_NEWPREFS, NEWPREFS);
}
#endif

#if NOISM
sendAllMenuClear( ie )
struct InputEvent *ie;
{
    sendAllOne( ie, IECLASS_RAWMOUSE, MENUUP, MENUVERIFY, 
	    fetchIBase()->ActiveScreen);
}
#endif

#if NOISM

USHORT getAllButMenuOK( ie )
struct InputEvent *ie;
{
    struct Window *workwindow;
    struct IntuitionBase *IBase = fetchIBase();

    assertLock("gABMOK", IBASELOCK );

    workwindow = IBase->ActiveScreen->FirstWindow;
    while (workwindow)
	{
	if (workwindow NOT= IBase->ActiveWindow)
	    if (workwindow->IDCMPFlags & MENUVERIFY)
		if (getMenuOK(workwindow, ie, MENUWAITING) == OKABORT) return (OKABORT);
	workwindow = workwindow->NextWindow;
	}

    return(MENUWAITING);
}
#endif


