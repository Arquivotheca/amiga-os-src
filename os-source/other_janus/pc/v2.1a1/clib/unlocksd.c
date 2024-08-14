/****** ljanus.lib/UnlockServiceData ******************************************
*
*   NAME   
*		UnlockServiceData -- Unlock a previously locked ServiceData structure.
*
*   SYNOPSIS
*		ReturnCode = UnlockServiceData(ServiceData);
*
*		UBYTE UnlockServiceData(struct ServiceData *);
*
*   FUNCTION
*		Unlocks a ServiceData structure previously locked with 
*		LockServieData(). Except where noted
*		this function behaves identically to the janus.library version.
*		see the janus.library autodocs for a detailed explanation.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		LockServiceData()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE UnlockServiceData(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_UNLOCKSERVICEDATA;
	r.x.di = (UWORD)ServiceData;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
