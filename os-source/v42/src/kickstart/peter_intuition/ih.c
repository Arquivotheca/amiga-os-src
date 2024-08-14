/*** ih.c *****************************************************************
 *
 *  ih.c -- new intuition handler
 *
 *  $Id: ih.c,v 38.7 92/08/03 15:53:42 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"

#include "ih_protos.h"

/*---------------------------------------------------------------------------*/

static int runIntuition(struct InputEvent * ie);

/*---------------------------------------------------------------------------*/

#define D(x)	;
#define DC(x)	;	/* cursor key mouse	*/
#define DI(x)	;	/* sendIDCMP		*/
#define DIE(x)	;	/* input event copy	*/


struct InputEvent	*
IntuitionHandler( ie )
struct InputEvent	*ie;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct InputToken		*it;
    struct InputEvent		*ieresult;
    struct InputEvent		*returnIEvents();


    /* I'm glad you asked that question.
     * This is to choke off all input processing
     * (all the way along the food chain) while
     * and Alert is running.  We don't hold onto
     * this semaphore to avoid deadlocks.  The status
     * is: alerts will not wait for the current
     * IntuitionHandler() invocation to complete, 
     * but they will exclude other alerts, and
     * prevent IntuitionHandler() from running any more.
     */
    ObtainSemaphore( &IBase->ISemaphore[ ALERTLOCK ] );
    ReleaseSemaphore( &IBase->ISemaphore[ ALERTLOCK ] );

    LOCKSTATE();

    reclaimIEvents();

    /* Graphics may have killed our pointer.  If so, we need
     * to re-instate it:
     */
    reinstatePointer();

    /*
     * only IntuitionHandler() processes tokens on the deferred queue
     */
    while ( it = (struct InputToken *) RemTail( &IBase->DeferredQueue ) )
    {
	AddHead( &IBase->TokenQueue, it );
    }

    runIntuition( ie );

    D( printf("IH: rI returned, return IEvents\n") );

    /* return accumulated events */
    ieresult = returnIEvents();

#if 0
    D( printf( "****-> ieresult: %lx\n", ieresult ) );
    {
	struct InputEvent *ie;
	for ( ie = ieresult;  ie; ie = ie->ie_NextEvent )
	{
	    printf("ie pass class %lx addr %lx\n",
		ie->ie_Class, ie->ie_EventAddress );
	}
    }
#endif

    UNLOCKSTATE();

    return( ieresult );
}

Intuition( ie )
struct InputEvent	*ie;
{
    LOCKSTATE();

    /* ZZZ: init time stamps? */

    runIntuition( ie );

    UNLOCKSTATE();
    return ( 0 );	/* I don't know why */
}

/*
 * don't clear deferred queue input tokens: just run what's there
 */
static
runIntuition( ie )
struct InputEvent	*ie;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct InputToken		*it;

    reclaimMessages( IBase->ActiveWindow );
    reclaimMessages( IBase->VerifyWindow );
    reclaimWBMsg();

    /*
     * queue InputTokens corresponding to InputEvents.
     * perform translations of events to commands and mouse
     * events at this time.
     */
    D( printf("/") );
    createInputTokens( ie );

    /* process deferred and new messages */
    D( printf("rI: clear token queue\n" ) );
    while ( it = (struct InputToken *) RemHead( &IBase->TokenQueue ) )
    {
	beginToken( it );
    }
}

/*
 * send event to ActiveWindow
 */
activeEvent( ieclass,  code )
{
    windowEvent( fetchIBase()->ActiveWindow, ieclass, code );
}

/*
 * sends IDCMP or appends input event to food chain.
 * uses IT/IE as base input event.
 * must be directly called for these events:
 *	IECLASS_REFRESHWINDOW
 *	IECLASS_REQUESTER
 *	IECLASS_SIZEWINDOW
 */
windowEvent( sendwindow, ieclass, iecode )
struct Window	*sendwindow;
ULONG	ieclass, iecode;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct InputEvent		*ie;

    struct InputEvent		*queueIEvent();
    struct InputEvent		*qie;	/* new, queued input event	*/

    /* iaddress is the value that will be put in the IAddress field
     * of the IntuiMessage.  Note that this is not always the same
     * as ie_EventAddress for the equivalent message.
     * It differs for RAWKEY and MOUSEMOVE messages, as follows:
     * RAWKEY: EventAddress = deadkey-info
     *         IAddress = pointer to deadkey-info
     * MOUSEMOVE: EventAddress = Concatenation of MouseX/MouseY
     *            IAddress = pointer to window
     * The correction for IAddress happens in sendIDCMP().
     *
     * For rawkey, ie_EventAddress is the dead-key info, but
     * IAddress is a pointer to a ULONG with the deadkey info.
     *
     * For rawmouse, ie_EventAddress (actually union member ie_xy)
     * is absolute window or DELTAMOVE raw coordinate position.
     */
    APTR			iaddress;

    ie = IE;

    DIE( printf("windowEvent: w %lx, ie to copy %lx, ieclass 0x%lx/0x%lx\n",
	sendwindow, ie, ieclass, iecode ) );

    /* if a REAL input event, update my copy */
    if ( ie != &IBase->IECopy )
    {
	DIE( printf("update IECopy with real input\n") );
	IBase->IECopy.ie_Qualifier =  ie->ie_Qualifier;
	IBase->IECopy.ie_TimeStamp =  ie->ie_TimeStamp;
    }
    DIE( else printf("wE: a 'fake' input event with this token\n") );

    /*
     * figure out who to send the message to, and
     * what value the IAddress field should be.
     * Also, change the ie_EventAddress if needed.
     */

    switch ( ieclass )
    {

    case IECLASS_EVENT:		/* special for the console	*/
#if 1
	/* iaddress  will be the window for the message */
	switch ( iecode )
	{
	case IECODE_NEWSIZE:
	case IECODE_REFRESH:
	    iaddress = sendwindow;
	    break;
#if 0
	case IECODE_NEWACTIVE:
#endif
	default:		/* mouse buttons */
	    iaddress = (APTR) IBase->ActiveWindow;
	}
	break;
#endif

    case IECLASS_REFRESHWINDOW:
    case IECLASS_REQUESTER:
    case IECLASS_ACTIVEWINDOW:
    case IECLASS_INACTIVEWINDOW:
    case IECLASS_TIMER:
    case IECLASS_SIZEWINDOW:
    case IECLASS_CHANGEWINDOW:
    /* By special arrangement, console.device uses ie->ie_EventAddress
     * for the routing of these classes of message.  Console.device
     * must be kept up-to-date of additions or changes here.
     */
	ie->ie_EventAddress = iaddress = (APTR) sendwindow;
	break;

    case IECLASS_GADGETDOWN:
    case IECLASS_GADGETUP:
	ie->ie_EventAddress = iaddress = (APTR) IBase->ActiveGadget;
	break;

    case IECLASS_GADGETHELP:
	ie->ie_EventAddress = iaddress = (APTR) IBase->HelpGadget;
	break;

    /*
     * use contents already in ie_EventAddress.
     * example: RAWKEY has deadkey information there.
     * NB: sendIDCMP() changes RAWKEY IAddress to be a pointer to the
     * deadkey information.
     * NB: sendIDCMP() changes MOUSEMOVE IAddress to be a pointer to
     * the window.
     * Peter 27-Mar-91: RAWMOUSE events already have the mouse coordinates
     * there, and we were trashing them with the sendwindow value!
     */
#if 0
    case IECLASS_RAWKEY:
    case IECLASS_RAWMOUSE:
#endif
    default:
	iaddress = (APTR) ie->ie_EventAddress;
    }

    /* Always send IECLASS_EVENT (for console), even if window is NULL.
     * Else, for known windows, send event if window has no IDCMPFlags or
     * if sendIDCMP doesn't want to deal with it.
     */

    if  ( ( ( !sendwindow ) && ( ieclass == IECLASS_EVENT ) )
	||
	( ( knownWindow( sendwindow ) )
	    && ( ( !sendwindow->IDCMPFlags )
		|| ( !sendIDCMP( ieclass, iecode, IT, sendwindow, iaddress ) ) ) ) )
    {
	if  ( qie = queueIEvent() )
	{
	    D( printf("wE: qie %lx\n", qie ));
	    qie->ie_Class =		ieclass;
	    qie->ie_SubClass =	ie->ie_SubClass;
	    qie->ie_Code =		iecode;
	    qie->ie_Qualifier =	ie->ie_Qualifier;
	    qie->ie_EventAddress =	iaddress;
	    qie->ie_TimeStamp =	ie->ie_TimeStamp;
	}
    }
}


/*
 * clone the current input event and
 * set the class and multibroadcast
 */
broadcastIEvent( ieclass )
ULONG	ieclass;
{
    struct InputEvent *cloneie;
    struct IntuitionBase	*IBase = fetchIBase();

    if ( cloneie = queueIEvent() )
    {
	*cloneie = *IE;
	cloneie->ie_Class = ieclass;
	SETFLAG( cloneie->ie_Qualifier,  IEQUALIFIER_MULTIBROADCAST );
    }
}

/*
 * returns FALSE if it wants the event to be propagated
 * down the InputEvent food chain
 */
sendIDCMP( ieclass, code, it, window, iaddress )
ULONG	ieclass, code;
struct InputToken	*it;
struct Window	*window;
APTR	iaddress;
{
    struct IntuiMessage	*imsg;
    struct IntuiMessage	*initIMsg();
    ULONG	idcmpclass;
    struct InputEvent	*ie = it->it_IE;

    /* DI( printf("sendIDCMP [%lx %lx]", ieclass, code ) ); */

    if ( ! (idcmpclass = translateIEtoIDCMP( ieclass, code ) ) )
    {
	/* in particular, IECLASS_EVENT doesn't get sent */
	return ( FALSE );
    }

    /*
     * final conversion of RAWKEY to VANILLAKEY
     * swallow all upstrokes if VANILLAKEY is set,
     * but let non-vanilla downstrokes get sent if RAWKEY is set.
     */
    if ( idcmpclass == RAWKEY && TESTFLAG( window->IDCMPFlags, VANILLAKEY ) )
    {
	if ( TESTFLAG( code, IECODE_UP_PREFIX ) )
	{
	    return ( TRUE );	/* swallow it */
	}
	else if ( it->it_Code != 0 )
	{
	    idcmpclass = VANILLAKEY;
	    code = it->it_Code;		/* copy over the converted one */
	}
	else if ( !TESTFLAG( window->IDCMPFlags, RAWKEY ) )
	{
	    return ( TRUE );   /* swallow non-vanilla unless he wants rawkey */
	}
    }

    /* if he wants it as a message, send it to him	*/
#if 0
    /* here's the extension filtering scheme	*/
    if ( ( idcmpclass & IDCMPEXTEND & window->IDCMPFlags )
         || TESTFLAG( window->IDCMPFlags, idcmpclass ) )
#else
    if ( TESTFLAG( window->IDCMPFlags, idcmpclass ) )
#endif
    {
	/* limit queue of mouse messages	*/
	if ( idcmpclass == MOUSEMOVE )
	{
	    if (XWINDOW(window)->MousePending<XWINDOW(window)->MouseQueueLimit)
		XWINDOW(window)->MousePending++;
	    else
		return ( TRUE );
	}

	/* limit queue of key repeat messages	*/
	if ( ( idcmpclass == RAWKEY || idcmpclass == VANILLAKEY )
	    &&  TESTFLAG( ie->ie_Qualifier, IEQUALIFIER_REPEAT ) )
	{
	    if ( XWINDOW(window)->RptPending  < XWINDOW(window)->RptQueueLimit )
		XWINDOW(window)->RptPending++;
	    else
		return ( TRUE );
	}

	/****** initIMsg() *****/
	if ( imsg = initIMsg( idcmpclass, code, ie, window, it->it_Tablet ) )
	{
	    /* By default, the IAddress is to contain the same
	     * thing as the InputEvent (passed in as 'iaddress')
	     */
	    imsg->IAddress = iaddress;

	    /* special shit for dead keys:
	     *  the ie_EventAddress (iaddress) contains
	     *  the deadkey information.  I need to stash
	     *  it behind the IntuiMessage and point
	     *  IAddress at it
	     */
	    if ( idcmpclass == RAWKEY )
	    {
		ULONG	*pk;

		imsg->IAddress = (APTR) (pk = &IIMSG(imsg)->iim_PrevKeys);
     
		*pk = (ULONG) iaddress;
	    }
	    else if ( ieclass == IECLASS_RAWMOUSE )
	    {
		/* Peter 25-Sep-91:  MOUSEMOVE and MOUSEBUTTON messages
		 * contain the window in the IAddress field.  2.04 puts the
		 * MouseX/Y concatenation there, which crashes
		 * some people.
		 */
		imsg->IAddress = window;

		/* 1.3 DELTAMOVE behavior -- what imsg->MouseX/Y contains:
		 * MOUSEMOVE message contain correct deltas
		 * SIZEVERIFY and MENUVERIFY contain correct absolutes (!)
		 * REFRESHWINDOW, MENUPICK, and ...KEYS contain trash
		 * REQ... were untested
		 * Everything else contained a copy of the most recent
		 * delta - often (0,0), but VERY bad when not (0,0)
		 *
		 * 2.0 DELTAMOVE behavior: -- what imsg->MouseX/Y contains:
		 * MOUSEMOVE message contains correct deltas
		 * 2.05: MOUSEBUTTONS also contains correct deltas
		 * everything else contains (0,0).
		 * (initIMsg() init's MouseX/Y to zero if DELTAMOVE)
		 */

		if ( TESTFLAG( window->IDCMPFlags, DELTAMOVE ) )
		{
		    imsg->MouseX = ie->ie_X;
		    imsg->MouseY = ie->ie_Y;
		}
	    }
	    DI( printf( "-imsg-> %lx to %lx\n", idcmpclass, window ) );
	    /* Peter 16-Oct-90:
	     * As a result of CloseWindowSafely(), there is a possibility
	     * of encountering a window with IDCMPFlags != NULL which
	     * nevertheless has a UserPort of NULL.  This closes that problem.
	     */
	    PutMsgSafely( window->UserPort, imsg );
	}
	DI( else printf( "sI: initIMsg failed!!!\n") );
	/* used to say "false" if no message memory */

	return ( TRUE );  /* if can't get message, don't send it as ie */
    }
    return ( FALSE );
}


/* As a result of CloseWindowSafely(), there is a possibility
 * of encountering a window with IDCMPFlags != NULL which
 * nevertheless has a UserPort of NULL.  This can happen when
 * the user calls ModifyIDCMP() inside his Forbid(), which breaks
 * the Forbid().  The state machine may then find some other token
 * while win->UserPort = NULL.
 * This closes that problem.
 */
PutMsgSafely( port, imsg )
struct MsgPort		*port;
struct IntuiMessage	*imsg;
{
    int retval = TRUE;

    Forbid();
    if ( port )
    {
	PutMsg( port, imsg );
    }
    else
    {
	/* We'll pick it up on the back-side during reclaimMessages */
	ReplyMsg( imsg );
	retval = FALSE;
    }
    Permit();

    return( retval );
}
