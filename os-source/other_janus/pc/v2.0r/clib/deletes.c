/****************************************************************************
 *
 * FUNCTION	DeleteService();
 *
 * SYNOPSIS	VOID DeleteService(ServiceData);
 *
 * INPUT		struct ServiceData *ServiceData ;Pointer to service data structure
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

VOID DeleteService(ServiceData)
struct ServiceData *ServiceData;
{
	union REGS r;

	r.h.ah = JFUNC_DELETESERVICE;
	r.x.di = (ULONG)ServiceData & 0xffff;

	int86(JFUNC_JINT,&r,&r);

}
