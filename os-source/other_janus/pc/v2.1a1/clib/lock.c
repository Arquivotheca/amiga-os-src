/****** ljanus.lib/JanusLock ******************************************
*
*   NAME   
*		JanusLock -- Get a lock.
*
*   SYNOPSIS
*		JanusLock(Lock);
*
*		VOID JanusLock(UBYTE *);
*
*   FUNCTION
*		Does everything necessary to Lock a lock byte.
*		Except where noted this function behaves identically
*		to the janus.library version. See the janus.library autodocs for a
*		detailed explanation.
*
*   INPUTS
*		Lock - Pointer to the lock byte to be locked.
*
*   RESULT
*		The lock is yours.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		JanusInitLock(), JanusLockAttempt(), JanusUnlock()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

VOID JanusLock(Lock)
UBYTE *Lock;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

	r.h.ah = JFUNC_LOCK;
	sr.es  = ((ULONG)Lock)>>16;
	r.x.di = ((ULONG)Lock) & 0xffff;

	int86x(JFUNC_JINT,&r,&r,&sr);
}
