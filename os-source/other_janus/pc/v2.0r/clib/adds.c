/****************************************************************************
 *
 * FUNCTION	AddService();
 *
 * SYNOPSIS	UBYTE AddService(ApplicationID,LocalID,Size,Type,Flags,Handler,SData);
 *
 * INPUT		ULONG ApplicationID	  
 *				UWORD LocalID			 
 *				UWORD Size
 *				UWORD Type 
 *				UWORD Flags				  
 *				int (*Handler)()		  
 *				struct ServiceData **SData
 *
 * OUTPUT	returns Error code defined in  services.h
 *					JSERV_OK
 *				   SData points to valid ServiceData structure if JSERV_OK.
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

UBYTE AddService(ApplicationID,LocalID,Size,Type,Flags,Handler,SData)
ULONG ApplicationID;
UWORD LocalID;
UWORD	Size;
UWORD	Type;
UWORD Flags;
int (*Handler)();
struct ServiceData **SData;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

/*
	printf("Adds Handler = %lx\n",Handler);
	(*Handler)();
	printf("Adds Handler called\n");
*/

	r.h.ah = JFUNC_ADDSERVICE;
	sr.ds = ApplicationID>>16;
	r.x.si = ApplicationID &=0xffff;
	r.x.cx = LocalID;
	r.x.bx = Size;
	r.x.dx = Type;
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