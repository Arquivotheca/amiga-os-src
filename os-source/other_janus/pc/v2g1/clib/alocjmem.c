/****************************************************************************
 *
 * FUNCTION	AllocJanusMem();
 *
 * SYNOPSIS	APTR AllocJanusMem(Size,Type);
 *
 * INPUT		ULONG Size		;Size of memory to allocate, in BYTES.
 *				ULONG Type		;Type of mem to allocate MEMF_PARAMETER or 
 *																    MEMF_BUFFER
 *
 * OUTPUT	returns pointer to memory or NULL if allocation failed
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

APTR AllocJanusMem(Size,Type)
ULONG Size;
ULONG Type;
{
	union REGS r;
	ULONG ptr;
	UWORD ParamSeg,ParamOff,BufferSeg;

	r.h.ah = 0x02;
	r.h.al = (UBYTE)Type & 0xff;
	r.x.bx = (UWORD)Size & 0xffff;

	int86(JFUNC_JINT,&r,&r);

	if(GetBase(JSERV_AMIGASERVICE,&ParamSeg,&ParamOff,&BufferSeg)!=JSERV_OK)
		return(NULL);
	if(r.h.al) return(NULL);


	if(Type==MEMF_PARAMETER)
	{
		ptr = (ULONG)((ULONG)ParamSeg << 16);
	} else 
	if(Type==MEMF_BUFFER)
	{
		ptr = (ULONG)((ULONG)BufferSeg << 16);
	} else 
		return(NULL);
	
	return( (APTR)((ULONG)(ptr + (UWORD)r.x.bx)) );
}
