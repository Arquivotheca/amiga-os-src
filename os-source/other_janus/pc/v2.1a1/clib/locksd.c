/****** ljanus.lib/LockServiceData ******************************************
*
*   NAME   
*		LockServiceData -- Lock a ServiceData structure for exclusive use.
*
*   SYNOPSIS
*		ReturnCode = LockServiceData(ServiceData);
*
*		UBYTE LockServiceData(struct ServiceData *);
*
*   FUNCTION
*		Does everything necessary to lock a ServiceData structure for 
*		exclusive use. Except where noted this function behaves identically
*		to the janus.library version. See the janus.library autodocs for a
*		detailed explanation.
*
*   INPUTS
*		ServiceData - Pointer to the ServiceData structure that you want to
*						  lock.
*
*   RESULT
*		Returns JSERV_OK if the lock was obtained.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		UnlockServiceData()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE LockServiceData(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_LOCKSERVICEDATA;
	r.x.di = (UWORD)ServiceData;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
