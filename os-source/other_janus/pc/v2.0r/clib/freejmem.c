/****************************************************************************
 *
 * FUNCTION	FreeJanusMem();
 *
 * SYNOPSIS	VOID FreeJanusMem(Ptr);
 *
 * INPUT	 	APTR	Ptr			;Ptr returned by AllocJanusMem
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

VOID FreeJanusMem(Ptr)
APTR	Ptr;
{
	union REGS r;
	UWORD ParamSeg,ParamOff,BufferSeg;
	ULONG	Temp;

	GetBase(JSERV_AMIGASERVICE,&ParamSeg,&ParamOff,&BufferSeg);

	Temp = (ULONG)Ptr>>16;
	if(Temp==ParamSeg)
		r.h.al=MEMF_PARAMETER;
	else
		r.h.al=MEMF_BUFFER;
	r.h.ah = 0x03;
	r.x.bx = (UWORD)((ULONG)Ptr & 0x0000ffff);
	int86(JFUNC_JINT,&r,&r);

}
