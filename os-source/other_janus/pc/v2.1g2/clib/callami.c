/****i* ljanus.lib/CallAmiga ******************************************
*
*   NAME   
*		CallAmiga -- Call an old style service.
*
*   SYNOPSIS
*		ReturnCode = CallAmiga(Service);
*
*		UBYTE CallAmiga(UBYTE);
*
*   FUNCTION
*		Calls an old style Service.
*
*   INPUTS
*		Service - Number of the service to call.
*
*   RESULT
*		Returns one of JSERV_OK, JSERV_PENDING, JSERV_FINISHED as defined
*		in services.[h,inc].
*
*   EXAMPLE
*
*   NOTES
*		This is a low level Janus call retained for compatibility with V1.0.
*		Service programmers should NOT use this function.
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

UBYTE CallAmiga(Service)
UBYTE Service;
{
	union REGS r;

	r.h.ah = JFUNC_CALLAMIGA;
	r.h.al = Service;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}


