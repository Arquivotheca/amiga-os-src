/****** ljanus.lib/GetBase ******************************************
*
*   NAME   
*		GetBase -- Get segment pointers for buffer and parameter mem,as
*					  well as the default parameter memory for the old
*					  style service.
*
*   SYNOPSIS
*		ReturnCode = GetBase(Service,ParmSeg,ParmOff,BuffSeg);
*
*		UBYTE GetBase(UBYTE,UWORD *,UWORD *,UWORD *);
*
*   FUNCTION
*		Returns the segment addresses for buffer and parameter mem. Also
*		returns the default parameter mem offset for this service if
*		defined else -1.
*
*   INPUTS
*		Service - Service number of old style service.
*
*		ParmSeg - Pointer to a UWORD to recieve the Parameter segment pointer.
*
*		ParmOff - Pointer to a UWORD to recieve the offset of this services
*					 parameter memory.
*
*		BuffSeg - Pointer to a UWORD to recieve the Buffer segment pointer.
*
*   RESULT
*		Returns JSERV_OK, JSERV_NOJANUSBASE.
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

UBYTE GetBase(Service,ParmSeg,ParmOff,BuffSeg)
UBYTE Service;
UWORD *ParmSeg,*ParmOff,*BuffSeg;
{
	union REGS r;
	struct SREGS sr;

	r.h.al = Service;
	r.h.ah = JFUNC_GETBASE;

	r.x.di = 0xffff;
	r.x.dx = 0xffff;
	
	int86x(JFUNC_JINT,&r,&r,&sr);
	segread(&sr);

	*ParmSeg = sr.es;
	*ParmOff = r.x.di;
	*BuffSeg = r.x.dx;

	return(r.h.al);

}
