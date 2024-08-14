/****** ljanus.lib/JanusLockAttempt ******************************************
*
*   NAME   
*		JanusLockAttempt -- Try once to get a lock.
*
*   SYNOPSIS
*		JanusLockAttempt(Lock);
*
*		VOID JanusLockAttempt(UBYTE *);
*
*   FUNCTION
*		Attempts to Lock a lock byte.
*		Except where noted this function behaves identically
*		to the janus.library version. See the janus.library autodocs for a
*		detailed explanation.
*
*   INPUTS
*		Lock - Pointer to the lock byte to be locked.
*
*   RESULT
*		TRUE if lock gotten FALSE otherwise.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		JanusInitLock(), JanusLock(), JanusUnlock()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

BOOL JanusLockAttempt(Lock)
UBYTE *Lock;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

	r.h.ah = JFUNC_LOCKATTEMPT;
	sr.es  = ((ULONG)Lock)>>16;
	r.x.di = ((ULONG)Lock) & 0xffff;

	int86x(JFUNC_JINT,&r,&r,&sr);

	return(r.h.al);
}
