/****** ljanus.lib/FreeJanusMem ******************************************
*
*   NAME   
*		FreeJanusMem -- Frees Janus Dual-Port memory.
*
*   SYNOPSIS
*		ReturnCode = FreeJanusMem(Ptr);
*
*		UBYTE FreeJanusMem(APTR);
*
*   FUNCTION
*		Frees Janus mem previously allocated with AllocJanusMem().
*
*   INPUTS
*		Ptr - Pointer to previously allocated memory.
*
*   RESULT
*		Returns JSERV_OK or crashes on bad Ptr.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		AllocJanusMem()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

UBYTE FreeJanusMem(Ptr)
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
   	if(Temp==BufferSeg)
			r.h.al=MEMF_BUFFER;
		else
			return(100);
	r.h.ah = JFUNC_FREEMEM;
	r.x.bx = (UWORD)((ULONG)Ptr & 0x0000ffff);
	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
