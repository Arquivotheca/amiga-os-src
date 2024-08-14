/****** ljanus.lib/CallService ******************************************
*
*   NAME   
*		CallService -- Signal all other users of a Janus Service.
*
*   SYNOPSIS
*		ReturnCode = CallService(Service);
*
*		UBYTE CallService(struct ServiceData *);
*
*   FUNCTION
*		Sends a signal to all other users of this service. Except where noted
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
*		AddService(), DeleteService(), GetService(), ReleaseService(),
*		WaitService()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE CallService(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_CALLSERVICE;
	r.x.di = (ULONG)ServiceData & 0xffff;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
