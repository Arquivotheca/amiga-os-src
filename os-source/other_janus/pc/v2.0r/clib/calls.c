/****************************************************************************
 *
 * FUNCTION	CallService();
 *
 * SYNOPSIS	VOID CallService(ServiceData);
 *
 * INPUT		struct ServiceData *ServiceData ;Pointer to service data structure
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

VOID CallService(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_CALLSERVICE;
	r.x.di = (ULONG)ServiceData & 0xffff;

	int86(JFUNC_JINT,&r,&r);

}
