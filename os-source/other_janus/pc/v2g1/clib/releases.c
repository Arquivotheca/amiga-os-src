/****************************************************************************
 *
 * FUNCTION	ReleaseService();
 *
 * SYNOPSIS	VOID ReleaseService(ServiceData);
 *
 * INPUT		struct ServiceData *ServiceData ;Pointer to service data structure
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

UBYTE ReleaseService(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_RELEASESERVICE;
	r.x.di = (ULONG)ServiceData & 0xffff;

	int86(JFUNC_JINT,&r,&r);

}
