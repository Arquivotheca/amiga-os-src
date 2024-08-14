/****i* ljanus.lib/WaitAmiga ******************************************
*
*   NAME   
*		WaitAmiga -- Wait for an old style service request to complete.
*
*   SYNOPSIS
*		ReturnCode = WaitAmiga(Service);
*
*		UBYTE WaitAmiga(UBYTE);
*
*   FUNCTION
*		Waits for an old style service request to complete.
*
*   INPUTS
*		Service - Number of the service to wait for.
*
*   RESULT
*		Returns JSERV_FINISHED when the call completes.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE WaitAmiga(Service)
UBYTE Service;
{
	union REGS r;

	r.h.ah = JFUNC_WAITAMIGA;
	r.h.al = Service;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}


