/*** ism.c *****************************************************************
 *
 *  ism.c -- basic state machine routines
 *
 *  $Id: ism.c,v 38.5 92/12/09 18:13:41 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"

#include "ism_protos.h"

/*---------------------------------------------------------------------------*/

static int dispatchToken(struct InputToken * it);

struct InputToken * newToken(void);

static int disposeToken(struct InputToken * it);

static int notifyToken(struct InputToken * it);

/*---------------------------------------------------------------------------*/

#define DERROR(x)	;
#define D(x)		;
#define DDOISM(x)	;
#define DCT(x)		;
#define DDO(x)		;
#define DALERT(x)	;
#define DREUSE(x)	;

/* "not timer" macro	*/
#define NT(it) (it->it_Command != itTIMER  )

#define IT_ALLOC	(10)	/* this many pre-allocated input tokens */

int	dNoWindow();
int	dIdleWindow();
struct InputToken *newToken();



/*
 * doIO front end: waits until done
 * returns error code from InputToken
 */
doISM( command, obj1, obj2, subcommand )
enum ITCommand command;
CPTR	obj1;
CPTR	obj2;
ULONG	subcommand;
{
    struct InputToken	itoken;
    struct Task		*my_task;
    struct Task		*FindTask();
    struct IntuitionBase *IBase = fetchIBase();

    if ( ( my_task = FindTask(0) ) == IBase->InputDeviceTask )
    {
	/* DANGER: Someone has called doISM() on input device's
	 * task.  If the token ends up being deferred, we could
	 * be in big trouble.  So what we do is just sendISM()
	 * instead.
	 * This fixes CygnusEd, which calls SetPointer()/ClearPointer()
	 * on input-device's task (those functions now go through the
	 * state machine).  It also makes it possible to route
	 * Amiga-M/N and screen-depth-gadget processing through
	 * the ScreenDepth() function, which is useful.
	 */
	 sendISM( command, obj1, obj2, subcommand );
	 return( NULL );
    }
    itoken.it_Command = command;
    itoken.it_Object1 = obj1;
    itoken.it_Object2 = obj2;
    itoken.it_SubCommand = subcommand;	/* often garbage	*/
    itoken.it_Error = 0;
    itoken.it_IE = &fetchIBase()->IECopy;

    /* I want to hear about it	*/
    itoken.it_Flags = ITF_NOPOOL | ITF_NOTIFYDONE | ITF_SIGNAL | ITF_QUICK;
    itoken.it_Task = my_task;
    itoken.it_Tablet = NULL;

    DDOISM( printf("doISM: command %ld, obj1 %lx\n", command, obj1 ) );

    LOCKSTATE();
    beginToken( &itoken );
    UNLOCKSTATE();

    /* it might have been deferred */
    DDOISM( printf("doISM: waiting\n") );
    Wait( SIGF_INTUITION );	/* wait for 'done' notification */

    /* What's this weird construct doing here?  Fine question, now
     * the answer:  if an event is deferred, it ends up on the
     * DeferredQueue, then moved to the TokenQueue and retried
     * inside runIntuition(), which calls beginToken() on it.
     * Before that call to beginToken() returns, we'll be signalled
     * by notifyToken().  If we exit before beginToken() is done,
     * we'll invalidate the token since it's an automatic variable
     * in doISM()'s context.  Thus, this little lock/unlock pair
     * ensures that input.device is truly done with our token
     * before we go away.  NB: This should only be a problem if
     * the calling task has priority > 20.
     */

    LOCKSTATE();
    UNLOCKSTATE();

    DDOISM( printf("doISM returning %ld\n", itoken.it_Error ) );
    return ( itoken.it_Error );
}

sendISM( command, obj1, obj2, subcommand )
{
    struct InputToken	*it;
    struct InputToken	*preSendISM();

    LOCKSTATE();

    if ( it = preSendISM( command, obj1, obj2, subcommand ) )
    {
	beginToken( it );
    }
    UNLOCKSTATE();
}

/*
 * pure defer: don't even try it on the current task
 * 5/11/90: one reason for this is ...
 * layerops functions 5/11/90: better run at input device 
 * priority to lessen the chance of somebody sneaking in
 * and trashing a damage list (some people don't use
 * the proper BeginRefresh() protocol, which does
 * the LockLayerInfo() ) protection.
 */
sendISMNoQuick( command, obj1, obj2, subcommand )
{
    struct InputToken	*it;
    struct InputToken	*preSendISM();

    LOCKSTATE();

    if ( it = preSendISM( command, obj1, obj2, subcommand ) )
    {
	it->it_Flags &= ~ITF_QUICK;
	beginToken( it );
    }
    UNLOCKSTATE();
}

/* lock before calling
 * note that this sets ITF_QUICK bit: you must clear
 * it before beginToken() if it is inappropriate
 */
struct InputToken	*
preSendISM( command, obj1, obj2, subcommand )
enum ITCommand command;
CPTR	obj1;
CPTR	obj2;
ULONG	subcommand;
{
    struct InputToken	*it;

    if ( it = newToken() )
    {
	it->it_Command = command;
	it->it_Object1 = obj1;
	it->it_Object2 = obj2;
	it->it_SubCommand = subcommand;	/* often garbage		*/
	it->it_Flags = ITF_QUICK;	/* why not?			*/
	it->it_Error = 0;		/* don't really use this	*/

	/* special case for itCHANGEWIN */
	if ( command == itCHANGEWIN )
	{
	    it->it_Box = *( (struct IBox *) obj2 );
	}

	DDO( printf("sendISM: command %ld, obj1 %lx\n", command, obj1 ) );
    }

    return ( it );
}

/*
 * be sure that STATELOCK is held
 * 
 * process a token at large (assumed not to be on any list)
 * and if it doesn't make it, defer it.
 *
 * If the token is marked ITF_QUICK, then dispatch it, else mark it
 * ITF_DEFERRED (so it'll be placed on the deferred queue) and
 * mark it ITF_QUICK (so it'll get run next time through).
 *
 * In post-processing, if ITF_DEFERRED is set, we enqueue it on
 * the deferred queue.  If ITF_REUSE is set, we reload it at the
 * head of the token queue, so it'll get processed right-away.
 *
 * The deferred queue is moved into line for processing the next
 * time the Intuition input-handler runs.
 */
beginToken( it )
struct InputToken	*it;
{
    struct IntuitionBase	*IBase = fetchIBase();

    /* All our callers must ensure we're locked.  This is vital! */
    assertLock( "beginToken", ISTATELOCK );

    /* try to get it through */
    if ( TESTFLAG( it->it_Flags, ITF_QUICK ) )
    {
	/* note: real input events always QUICK */
	dispatchToken( it );		/* DISPATCH */
    }
    else
    {
	SETFLAG( it->it_Flags, ITF_DEFERRED );
    }
    /* so I'll run it next time */
    SETFLAG( it->it_Flags, ITF_QUICK );

    /* TOKEN POSTPROCESSING
     * After processing, token is either deferred,
     * to be reused, or done (to be deallocated)
     */

    if ( TESTFLAG( it->it_Flags, ITF_DEFERRED ) )
    {
	DDO( printf("beginToken: deferring %ld\n", it->it_Command) );

	/* DEBUG: bitch about advisory ITF_DONOTDEFER flag */
	if ( TESTFLAG( it->it_Flags, ITF_DONOTDEFER ) )
	{
	    DALERT( printf("doToken() DEFERRING WRONGLY!!!\n"); Debug() );
	}

	/* enqueue it and let others run	*/
	AddTail( &IBase->DeferredQueue, it );
    }
    else if ( TESTFLAG( it->it_Flags, ITF_REUSE ) )
    {
	DREUSE( printf("reusing token\n"));
	CLEARFLAG( it->it_Flags, ITF_REUSE );
	AddHead( &IBase->TokenQueue, it );
    }
    else	/* token is done	*/
    {
	/* Note very well: notifyToken() may signal the waiting task. 
	 * If that task is waiting in doISM(), then the InputToken
	 * 'it' was allocated on its stack.  If that task has priority
	 * > input.device's, it could exit, invalidating the token
	 * before disposeToken() has a chance to look at it (and
	 * ultimately decide it's not to be freed).  To fix this,
	 * doISM() does a LOCKSTATE()/UNLOCKSTATE() before exiting.
	 */
	if ( TESTFLAG( it->it_Flags, ITF_NOTIFYDONE ) )
	{
	    D( printf("bT: notify the guy\n") );
	    notifyToken( it );
	}
	disposeToken( it );	/* only throws away tokens from pool */
    }
}

/*
 * makes a token the "current" IT, sends it to proper routine
 * assumes STATELOCK
 */
static
dispatchToken( it )
struct InputToken	*it;
{
    struct IntuitionBase	*IBase = fetchIBase();

    /* make this the official current token	*/
    IT = it;

    /* InputEvent->ie_TimeStamp vs. IBase->Seconds/Micros:
     *
     * Until release 3.00, Intuition would only update IBase->Seconds/Micros
     * when processing IECLASS_TIMER events.  The original reason was
     * probably that pre-2.0 input.device didn't time-stamp events it
     * received.  Since anybody could write mouse-events, their TimeStamp
     * had to be viewed with suspicion.  Few to no people write timer
     * events, so timer-event TimeStamps are relatively trustworthy.
     *
     * Under 2.0 and higher, InputEvent ie_TimeStamps are (relatively)
     * trustworthy (bad input-handlers aside), because input.device
     * stamps each one it receives.
     *
     * The key problem is that Intuition stamps outgoing IntuiMessages
     * based on IBase->Seconds/Micros.  Thus, it was not possible to
     * correlate by timestamp an IntuiMessage with the InputEvent that
     * caused it.  This correlation is needed to do things like add
     * pressure info under pre-V39.
     *
     * So here we get with the program and obey all timestamps,
     * for all events.  A patch will be shipped for V37 and 3.00
     * systems (and maybe pre-V37 too).
     *
     * We are paranoid, so we refuse to set IBase->Seconds/Micros
     * if that would set ourselves back in time or more than one second
     * forward.  We are depending on timing.c/doTiming() still _always_
     * setting these fields for timer events (that's how the user can
     * change the system time or how an application choking off Intuition
     * can stop doing that, and have Intuition get back in step with
     * the real time).
     *
     * (Commodities.library is _known_ to send bad (zero) timestamps
     * prior to 3.01.
     */

    {
	struct timeval *eventtime = &it->it_IE->ie_TimeStamp;
	LONG deltasecs = eventtime->tv_secs - IBase->Seconds;

	if ( ( eventtime->tv_micro - IBase->Micros ) < 0 )
	{
	    /* If the difference of microseconds is negative, we
	     * "borrow" a second.  If we did care about the numeric
	     * value of microseconds, we'd add a million to the value
	     * of the difference.
	     */
	    deltasecs--;
	}
	/* Don't update the IBase time values if the new event would
	 * be backwards in time (deltasecs < 0) or far forward, which
	 * we conveniently define as ( deltasecs >= 1).
	 */
	if ( deltasecs == 0 )
	{
	    IBase->Seconds = eventtime->tv_secs;
	    IBase->Micros = eventtime->tv_micro;
	}
    }

    DREUSE( if ( it->it_Command == itSELECTUP ) printf("dT SUP\n") );

    /* new lease on life */
    CLEARFLAG( IT->it_Flags, ITF_DEFERRED | ITF_REUSE );

    D( if NT(IT) printf("  [s %ld t %ld] ",
	IBase->CurrentState, IT->it_Command ) );

    /* we'll do fancy dispatch later	*/
#if 1
    switch ( IBase->CurrentState )
    {
    case sNoWindow:
	dNoWindow();
	break;
    case sIdleWindow:
	dIdleWindow();
	break;
    case sGadget:
	dGadget();
	break;
    case sWSizeDrag:
	dWSizeDrag();
	break;
    case sScreenDrag:
        dScreenDrag();
	break;
    case sMenu:
	dMenu();
	break;
    case sRequester:
	dRequester();
	break;
    case sVerify:
	dVerify();
	break;
    case sDMRTimeout:
	dDMRTimeout();
	break;
    default:
	DERROR( printf("dispatchToken default state %ld!!!\n",
	    IBase->CurrentState) );
	DERROR( Debug() );
    }
#else
    /* dispatch token to state handler	*/
    (*StateDispatchTable[ IBase->CurrentState ])();
#endif
}

/*
 * may return dirty token: be sure to initialize
 * all relevant fields, including it_Flags and it_Error
 */
struct InputToken *
newToken()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct InputToken		*it;

    it = (struct InputToken *) poolGet( &IBase->ITFreeList );
    if ( it )
    {
	/* it may be leftover: make sure he's clean enough */
	it->it_IE = &IBase->IECopy;
	it->it_Flags = 0;
	it->it_Tablet = 0;
    }
    return ( it );
}

/* 
 * return tokens to pool, if they came from there
 */
static
disposeToken( it )
struct InputToken *it;
{
    if ( ! TESTFLAG( it->it_Flags, ITF_NOPOOL ) )
    {
	poolReturn( it );
    }
}

/*
 * put the current token on the back of the deferred action queue
 */
deferToken()
{
    struct IntuitionBase	*IBase = fetchIBase();

    SETFLAG( IT->it_Flags, ITF_DEFERRED );

    /* point IE at the IBase input event copy:
     * 	can't leave him pointing to a transient real input event
     *
     * NOTE: you could run into trouble deferring "true input"
     * events anyway, so perhaps this is never really needed.
     */
    IE = &fetchIBase()->IECopy;
}

reuseToken()
{
    struct IntuitionBase	*IBase = fetchIBase();

    D( printf("reuseToken: command %ld\n", IT->it_Command ) );

    SETFLAG( IT->it_Flags, ITF_REUSE );
    CLEARFLAG( IT->it_Flags, ITF_DEFERRED );
    /* IE = &IBase->IECopy; * ZZZ: don't need this, since
     *  the only input I'm worried about is REAL input,
     * and IntuitionHandler will run until the TokenQueue is
     * clear
     */
}

/*
 * send a signal or call the guy who queued this token
 * may conceivably be sending a signal to myself, for later
 */
static
notifyToken( it )
struct InputToken	*it;
{
    D( printf("notifyToken: flags %lx, task %lx, me %lx\n",
	it->it_Flags, it->it_Task, FindTask( 0L ) ) );

#if 0	/* unused	 */
    if ( TESTFLAG( it->it_Flags, ITF_CALLBACK ) )
    {
	if ( it->it_CallBack )
	    (*it->it_CallBack)( it );
    }
#endif

    if ( TESTFLAG( it->it_Flags, ITF_SIGNAL ) )
    {
	if ( it->it_Task )
	{
	    D( printf("nT: signal him ... ") );
	    Signal( it->it_Task, SIGF_INTUITION );
	    D( printf(" done\n") );
	}
    }
}

initTokens()
{
    struct IntuitionBase	*IBase = fetchIBase();

    poolInit( &IBase->ITFreeList, sizeof (struct InputToken), IT_ALLOC );
    NewList( &IBase->TokenQueue );
    NewList( &IBase->DeferredQueue );
}

