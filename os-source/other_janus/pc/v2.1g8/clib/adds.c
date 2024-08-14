/****** ljanus.lib/AddService ******************************************
*
*   NAME   
*		AddService -- Adds a Service to the system.
*
*   SYNOPSIS
*		result = AddService(
*			ServiceData,AppID,LocalID,MemSize,MemType,Handler,Flags)
*
*		UBYTE AddService(struct ServiceData **,ULONG,USHORT,USHORT,USHORT
*								ULONG,USHORT);
*
*   FUNCTION
*		This routine adds a Service to the system. Except where noted
*		this function behaves identically to the janus.library version.
*		see the janus.library autodocs for a detailed explanation.
*
*   INPUTS
*		Handler - Pointer to an assembly language routine. This routine
*					 will be called whenever anyone else calls this service.
*					 If Handler = NULL no routine will be called.
*
*   RESULT
*		Returns a status byte defined in services.[h,inc]. JSERV_OK if
*		all went well.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		GetService(), CallService(), DeleteService(), ReleaseService()
*		WaitService()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE AddService(ServiceData,AppID,LocalID,MemSize,MemType,Handler,Flags)
struct ServiceData **ServiceData;
ULONG  AppID;
USHORT LocalID;
USHORT MemSize;
USHORT MemType;
ULONG  Handler;
USHORT Flags;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

	r.h.ah = JFUNC_ADDSERVICE;
	sr.ds  = AppID>>16;
	r.x.si = AppID & 0xffff;
	r.x.cx = LocalID;
	r.x.bx = MemSize;
	r.x.dx = MemType;
	r.h.al = (UBYTE)(Flags & 0xff);
	sr.es  = ((ULONG)Handler)>>16;
	r.x.di = (((ULONG)Handler) & 0xffff);

	int86x(JFUNC_JINT,&r,&r,&sr);
	segread(&sr);

   ptr =((unsigned long)(((unsigned long)sr.es) << 16) +
                                     (unsigned long)r.x.di);

	(*ServiceData) = (struct ServiceData *)ptr;

	return(r.h.al);
}
