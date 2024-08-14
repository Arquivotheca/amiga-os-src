/****** ljanus.lib/DeleteService ******************************************
*
*   NAME   
*		DeleteService -- Delete a Janus Service.
*
*   SYNOPSIS
*		ReturnCode = DeleteService(ServiceData);
*
*		UBYTE DeleteService(struct ServiceData *);
*
*   FUNCTION
*		Deletes a Janus Service from the system. Except where noted
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
*		AddService(), CallService(), GetService(), ReleaseService(),
*		WaitService()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE DeleteService(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_DELETESERVICE;
	r.x.di = (UWORD)((ULONG)ServiceData & 0xffff);

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
