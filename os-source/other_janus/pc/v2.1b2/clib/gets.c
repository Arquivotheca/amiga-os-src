/****** ljanus.lib/GetService ******************************************
*
*   NAME   
*		GetService -- Get a Janus Service.
*
*   SYNOPSIS
*		ReturnCode = GetService(ServiceData,AppID,LocalID,Handler,Flags);
*
*		UBYTE GetService(struct ServiceData **,ULONG,UWORD,ULONG,UWORD);
*
*   FUNCTION
*		This routine gets a Janus Service. Except where noted
*		this function behaves identically to the janus.library version.
*		see the janus.library autodocs for a detailed explanation.
*
*   INPUTS
*		Handler - Pointer to an assembly language routine to be called
*					 whenever someone else calls this service ot NULL for
*					 no handler.
*
*   RESULT
*		Returns JSERV_OK if no error.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		AddService(), CallService(), DeleteService(), ReleaseService(),
*		WaitService()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE GetService(SData,ApplicationID,LocalID,Handler,Flags)
struct ServiceData **SData;
ULONG ApplicationID;
UWORD LocalID;
ULONG Handler;
UWORD Flags;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

	r.h.ah = JFUNC_GETSERVICE;
	sr.ds = ApplicationID>>16;
	r.x.si = ApplicationID &=0xffff;
	r.x.cx = LocalID;
	r.h.al = Flags &= 0xff;
	sr.es  = ((ULONG)Handler)>>16;
	r.x.di = (((ULONG)Handler) & 0xffff);

	int86x(JFUNC_JINT,&r,&r,&sr);
	segread(&sr);

   ptr =((unsigned long)(((unsigned long)sr.es) << 16) +
                                     (unsigned long)r.x.di);

	(*SData) = (struct ServiceData *)ptr;

	return(r.h.al);
}
