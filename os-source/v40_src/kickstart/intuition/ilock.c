/*** ilock.c **************************************************************
 *
 * intuition locking routines
 *
 *  $Id: ilock.c,v 38.8 92/07/28 15:13:32 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

/* Time for a little discussion about locks:
 *
 * Intuition routinely violates its own locking protocol.  During menu
 * state or window size/dragging state, the system is left with layers
 * locked, but lesser locks like the ISTATELOCK are not held between
 * input events.  In fact, the ISTATELOCK cannot be held during that
 * time.  When a new event comes in, input.device must obtain the
 * ISTATELOCK even though it holds all the layer locks.
 *
 * This behavior is strictly necessary, and there is a lot of implicit
 * stuff going on to keep this from being a problem.  How this works
 * without deadlocking goes something like this:
 *
 * The input device task may exceptionally obtain the ISTATELOCK in a
 * manner which violates the locking order.  This happens when every
 * input token is processed during menu and window size/drag states.
 * This creates a very dangerous situation, since it exposes a deadlock
 * possibility.
 *
 * Any other task (and that includes other tasks executing under the
 * state machine) must respect locking order.  Further, when Intuition is
 * in its dangerous state, other tasks must NEVER obtain any of the layer
 * locks during the time that the ISTATELOCK is held.  Thus, most
 * Intuition tokens must be deferred during menu and window size/drag
 * states.
 *
 */

/* Lock-debugging generates enforcer hits, alerts, and/or descriptive messages
 * when a problem is found.
 * Set LOCKDEBUG to 1 in lockstub.asm to enable the assembler part.
 * Set LOCKDEBUG to 1 in intuall.h to enable light lock-debugging,
 *     which generates enforcer hits when a problem is found.
 * Set LOCKDEBUG to 2 in intuall.h to enable full lock-debugging,
 *     which generates enforcer hits and serial output when
 *     a problem is found.
 *
 * The macros DP(), DE(), and DA() control whether printing, enforcer hits,
 * or alerts happen, respectively.  These are set for you by the level of
 * LOCKDEBUG.
 *
 * 0x000y000z	Bad order: attempting to lock 'y', already hold lock 'z'
 * 0x0000000y	CheckLock: assertLock() lock 'y' not held, would be illegal
 * 0x8000000y	CheckLock: assertLock() lock 'y' not held
 * 0x4000000y	LockFree: Lock 'y' held when it is required not to be
 */

#if LOCKDEBUG==2
#define DP(x)	x
#define DE(x)	x
#define DA(x)	;
#endif

#if LOCKDEBUG==1
#define DP(x)	;
#define DE(x)	x
#define DA(x)	;
#endif

#if LOCKDEBUG==0
#define DP(x)	;
#define DE(x)	;
#define DA(x)	;
#endif

/* Macro to write 'x' to location zero.
 * Does nothing if DE() is ";"
 */
#define ENFORCER(x)	DE( *( ( ULONG * ) 0 ) = (x) )

struct Task *FindTask();

lockIBase( isem )
int	isem;
{
    struct IntuitionBase *IBase = fetchIBase();

#if LOCKDEBUG
    {
	struct Task *mytask = FindTask( NULL );

	/* Test for proper locking order.  However, we allow bad-order
	 * locking of the ISTATELOCK by input.device.  This happens
	 * during menu-state and window size/drag state.
	 */
	if ( ( isem != ISTATELOCK ) || ( mytask != IBase->InputDeviceTask ) )
	{
	    int i;

	    if (mytask == IBase->ISemaphore[isem].ss_Owner)
	    {
		goto DONTBITCH;	/* already had lock */
	    }

	    if (i = badOrder(mytask, isem))
	    {
		DP( printf( "Bad Locking order: task %lx; lock: %ld; conflict :%ld\n",
		    mytask, isem, i) );
		ENFORCER( (isem<<16) + i );
		DA( Alert( AN_ISemOrder ) );
		DP( kdebug() );
	    }
	}
    }
DONTBITCH:
#endif

    ObtainSemaphore(&IBase->ISemaphore[isem]);
    if ( isem == VIEWCPRLOCK )
    {
	ObtainSemaphore( IBase->GfxBase->ActiViewCprSemaphore );
    }
}

unlockIBase(isem)
int	isem;
{
    struct IntuitionBase *IBase = fetchIBase();

#if LOCKDEBUG

    if ( FindTask(0) != IBase->ISemaphore[isem].ss_Owner )
    {
	DP( printf("Unlocking, but doesn't own lock %ld!!!\n", isem) );
	DP( kdebug() );
    }
#endif

    if ( isem == VIEWCPRLOCK )
    {
	ReleaseSemaphore( IBase->GfxBase->ActiViewCprSemaphore );
    }
    ReleaseSemaphore(&IBase->ISemaphore[isem]);
}

#if LOCKDEBUG

/* supplant LockLayer...() with these
 * things which check locking order when LOCKDEBUG
 * is enabled
 */

fakeLockLayers(x)
APTR x;
{
    lockIBase(LAYERINFOLOCK);
    lockIBase(LAYERROMLOCK);
    realLockLayers(x);
}
fakeUnlockLayers(x)
APTR x;
{
    unlockIBase(LAYERROMLOCK);
    unlockIBase(LAYERINFOLOCK);
    realUnlockLayers(x);
}
fakeLockLayerInfo(x)
APTR x;
{
    lockIBase(LAYERINFOLOCK);
    realLockLayerInfo(x);
}
fakeUnlockLayerInfo(x)
APTR x;
{
    unlockIBase(LAYERINFOLOCK);
    realUnlockLayerInfo(x);
}
fakeLockLayerRom(x)
APTR x;
{
    lockIBase(LAYERROMLOCK);
    realLockLayerRom(x);
}
fakeUnlockLayerRom(x)
APTR x;
{
    unlockIBase(LAYERROMLOCK);
    realUnlockLayerRom(x);
}

/* WARN if 'sem' is not held	*/
CheckLock(str, sem)
UBYTE	*str;
{
    if ( badOrder( FindTask(0), sem ))
    {
	DP( printf("LOCK WOULD BE ILLEGAL-- %s, lock %ld\n", str, sem) );
	DP( dumpLocks("here's why: ") );
	ENFORCER( sem );
	DA( Alert( AN_NoISem ) );
	DP( kdebug() );
    }
    DP( printf("could get lock -- %s, lock %ld\n", str, sem) );
}

/* WARN if 'sem' would be in badOrder */
AssertLock(str, sem)
UBYTE	*str;
{
    struct IntuitionBase *IBase = fetchIBase();

    if ( FindTask(0) != IBase->ISemaphore[sem].ss_Owner )
    {

	/* ISTATELOCK is a substitute for IBASELOCK	*/
	if ( ( sem == IBASELOCK ) &&
	    ( FindTask(0) == IBase->ISemaphore[ ISTATELOCK ].ss_Owner) )
	{
	    ;	/* do nothing */
	}
	else
	{
	    /* During Intuition's init, we don't want complaints about
	     * unobtained locks, unless someone else is the owner.
	     * initIntuition() sets IDF_LOCKDEBUG as it exits.
	     */
	    if ( ( TESTFLAG( IBase->DebugFlag, IDF_LOCKDEBUG ) ) ||
		( IBase->ISemaphore[sem].ss_Owner ) )
	    {
		DP( printf("DON'T HAVE LOCK-- %s, lock %ld\n", str, sem) );
		checkLock( str, sem );
		DP( dumpLocks("but I do have ...") );
		ENFORCER( 0x80000000|sem );
		DA( Alert( AN_NoISem ) );
		DP( kdebug() );
	    }
	}
    }
    /* else { DP( printf("have lock -- %s, lock %ld\n", str, sem) ); } */
}

/* WARN if 'sem' is held */
LockFree(str, sem)
UBYTE	*str;
{
    struct IntuitionBase *IBase = fetchIBase();

    /* ISTATELOCK is a substitute for IBASELOCK	*/
    if ( ( FindTask(0) == IBase->ISemaphore[sem].ss_Owner ) ||
	( ( sem == IBASELOCK ) &&
	    ( FindTask(0) == IBase->ISemaphore[ ISTATELOCK ].ss_Owner) ) )
    {
	DP( printf("HAVE UNWANTED LOCK-- %s, lock %ld\n", str, sem) );
	DP( dumpLocks("I do have ...") );
	ENFORCER( 0x40000000|sem );
	DP( kdebug() );
    }
    /* else { DP( printf("have lock -- %s, lock %ld\n", str, sem) ); } */
}

badOrder(task, sem)
struct Task *task;
int sem;
{
    int i;
    BOOL  thereyet = FALSE;
    struct IntuitionBase *IBase = fetchIBase();

    for (i = 0; i < NUMILOCKS; i++)
    {
	if (! thereyet)
	{
	    thereyet = (i == sem);
	}
	else	/* past point where you should have any locks */
	{
	    if (task == IBase->ISemaphore[i].ss_Owner) return (i);
	}
    }

    /* So we got here, we think the order is good.  However,
     * if we're in menu-state or window size/drag state, then it's
     * illegal for anyone but input.device to get any of the layer locks
     * if the ISTATELOCK is held by us.
     */

    if ( ( ( IBase->CurrentState == sMenu ) || ( IBase->CurrentState == sWSizeDrag ) ) &&
	( task != IBase->InputDeviceTask ) &&
	( task == IBase->ISemaphore[ISTATELOCK].ss_Owner ) &&
	( ( sem == LAYERROMLOCK ) || ( sem == LAYERINFOLOCK ) ) )
    {
	return( 0xFFFF );
    }

    return (0);
}

#endif
/*** intuition.library/LockIBase ***/


#if 0 /* DOWNCODED */
ULONG LockIBase(lock)
{
    if (!lock)	/* original usage: lock both ISTATE and LISTS */
    {
	LOCKSTATE();
	LOCKIBASE();
    }
    else lockIBase(lock);
    return (lock);
}
#endif
/*** intuition.library/UnlockIBase ***/


#if 0 /* DOWNCODED */
UnlockIBase(lock)
{
    if (!lock)	/* original usage: lock both ISTATE and LISTS */
    {
	UNLOCKIBASE();
	UNLOCKSTATE();
    }
    else unlockIBase(lock);
}
#endif

