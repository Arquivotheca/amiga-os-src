/****** ljanus.lib/ReleaseService ******************************************
*
*   NAME   
*		ReleaseService -- Release a service previously gotten with
*								GetService().
*
*   SYNOPSIS
*		ReturnCode = ReleaseService(ServiceData);
*
*		UBYTE ReleaseService(struct ServiceData *);
*
*   FUNCTION
*		Release a Janus Service. Except where noted
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
*		AddService(), CallsService(), DeleteService(), GetService()
*		WaitService()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE ReleaseService(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_RELEASESERVICE;
	r.x.di = (ULONG)ServiceData & 0xffff;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
