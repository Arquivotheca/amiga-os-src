/****i* ljanus.lib/CheckAmiga ******************************************
*
*   NAME   
*		CheckAmiga -- Check the status of an old style service.
*
*   SYNOPSIS
*		ReturnCode = CheckAmiga(Service);
*
*		UBYTE CheckAmiga(UBYTE);
*
*   FUNCTION
*		Ceck the interrupt status of an old style service.
*
*   INPUTS
*		Service - Number of the service to check.
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

UBYTE CheckAmiga(Service)
UBYTE Service;
{
	union REGS r;

	r.h.ah = JFUNC_CHECKAMIGA;
	r.h.al = Service;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}


