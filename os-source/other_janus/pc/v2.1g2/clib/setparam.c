/****i* ljanus.lib/SetParam ******************************************
*
*   NAME   
*		SetParam -- Set the parameter offset for an old style service.
*
*   SYNOPSIS
*		ReturnCode = SetParam(Service,Offset);
*
*		UBYTE SetParam(UBYTE,UWORD);
*
*   FUNCTION
*		Sets the parameter memory offset for an old style service.
*
*   INPUTS
*		Service - Number of old style service.
*
*		Offset  - Parameter memory offset for this service.
*
*   RESULT
*		Returns JSERV_OK if all went well.
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

UBYTE SetParam(Service,ParmOff)
UBYTE Service;
UWORD ParmOff;
{
	union REGS r;

	r.h.ah = JFUNC_SETPARAM;
	r.h.al = Service;
	r.x.bx = ParmOff;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}

