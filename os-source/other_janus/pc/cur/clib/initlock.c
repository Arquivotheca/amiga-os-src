/****** ljanus.lib/JanusInitLock ******************************************
*
*   NAME   
*		JanusInitLock -- Initialize a lock byte.
*
*   SYNOPSIS
*		JanusInitLock(Lock);
*
*		VOID JanusInitLock(UBYTE *);
*
*   FUNCTION
*		Does everything necessary to initialize a lock byte.
*		Except where noted this function behaves identically
*		to the janus.library version. See the janus.library autodocs for a
*		detailed explanation.
*
*   INPUTS
*		Lock - Pointer to the lock byte to be initialized.
*
*   RESULT
*		The lock byte is initialized.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		JanusLockAttempt(), JanusLock(), JanusUnlock()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

VOID JanusInitLock(Lock)
UBYTE *Lock;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

	r.h.ah = JFUNC_INITLOCK;
	sr.es  = ((ULONG)Lock)>>16;
	r.x.di = ((ULONG)Lock) & 0xffff;

	int86x(JFUNC_JINT,&r,&r,&sr);
}
