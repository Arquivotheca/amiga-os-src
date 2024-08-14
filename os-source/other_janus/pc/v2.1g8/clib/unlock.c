/****** ljanus.lib/JanusUnlock ******************************************
*
*   NAME   
*		JanusUnlock -- Release a lock.
*
*   SYNOPSIS
*		JanusUnlock(Lock);
*
*		VOID JanusUnlock(UBYTE *);
*
*   FUNCTION
*		Does everything necessary to Unlock a lock byte.
*		Except where noted this function behaves identically
*		to the janus.library version. See the janus.library autodocs for a
*		detailed explanation.
*
*   INPUTS
*		Lock - Pointer to the lock byte to be unlocked.
*
*   RESULT
*		The lock is released.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		JanusInitLock(), JanusLockAttempt(), JanusLock()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

VOID JanusUnlock(Lock)
UBYTE *Lock;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

	r.h.ah = JFUNC_UNLOCK;
	sr.es  = ((ULONG)Lock)>>16;
	r.x.di = ((ULONG)Lock) & 0xffff;

	int86x(JFUNC_JINT,&r,&r,&sr);
}
