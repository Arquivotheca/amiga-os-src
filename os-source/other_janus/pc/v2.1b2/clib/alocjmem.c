/****** ljanus.lib/AllocJanusMem ******************************************
*
*   NAME   
*		AllocJanusMem -- Allocate Janus Dual-Port memory.
*
*   SYNOPSIS
*		MemPtr = AllocJanusMem(Size,Type);
*
*		APTR AllocJanusMem(ULONG,ULONG);
*
*   FUNCTION
*		This routine allocates memory from the special Janus RAM.
*		Except where noted this function behaves identically to the 
*		janus.library version. see the janus.library autodocs for a
*		detailed explanation.
*
*   INPUTS
*		Type - One of MEMF_PARAMETER or MEMF_BUFFER. Access modes are not
*				 necessary.
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		FreeJanusMem()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

APTR AllocJanusMem(Size,Type)
ULONG Size;
ULONG Type;
{
	union REGS r;
	ULONG ptr;
	UWORD ParamSeg,ParamOff,BufferSeg;

	r.h.ah = JFUNC_ALLOCMEM;
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