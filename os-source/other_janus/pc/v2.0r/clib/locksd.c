/****************************************************************************
 *
 * FUNCTION	LockServiceData();
 *
 * SYNOPSIS	UBYTE LockServiceData(ServiceData);
 *
 * INPUT		struct ServiceData *ServiceData ;Pointer to service data structure
 *
 * OUTPUT	returns Error code defined in  services.h
 *					JSERV_OK, JSERV_NOJANUSMEM, JSERV_NOJANUSBASE
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

UBYTE LockServiceData(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_LOCKSERVICEDATA;
	r.x.di = (UWORD)ServiceData;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
