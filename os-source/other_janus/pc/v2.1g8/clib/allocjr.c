/****** ljanus.lib/AllocJRemember ****************************************
*
*   NAME   
*     AllocJRemember -- Allocate Janus memory and link into a Remember list
*
*   SYNOPSIS
*     MemPtr = AllocJRemember(JRememberKey, Size, Type)
*
*     APTR AllocJRemember(RPTR *,USHORT,USHORT);
*
*   FUNCTION
*		Except where noted this function behaves identically to the 
*     janus.library version. See the janus.library autodocs for a 
*     detailed explanation.
*
*   INPUTS
*     JRememberKey = Pointer to an RPTR in Janus memory that will be the
*							JRemeberKey. Before your very first call to 
*                    AllocJRemember(), the RPTR should be set to NULL.
*							The RPTR MUST reside in Janus memory!
*     Size         = the size in bytes of the memory allocation.
*     Type         = the type of the desired memory.  Please refer to the
*                    AllocJanusMem() documentation for details about this
*                    field
*
*   RESULT
*     MemPtr = a pointer to the Janus memory you've requested, or NULL 
*              if no memory was available.
*
*   EXAMPLE
*     RPTR JRememberKey;
*     JRememberKey = NULL;
*
*     if (ptr = AllocJRemember(&JRememberKey, BUFSIZE, MEMF_BUFFER))
*     {
*        \* Use the ptr memory *\
*     }
*     FreeJRemember(&JRememberKey, TRUE);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     intuition.library/AllocRemember(), AttachJRemember(), FreeJRemember()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

APTR AllocJRemember(JRememberKey,Size,Type)
RPTR   *JRememberKey;
USHORT Size;
USHORT Type;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;
	UWORD ParamSeg,ParamOff,BufferSeg;

	r.h.ah = JFUNC_ALLOCJREMEMBER;
	r.x.bx = Size;
	r.h.al = (UBYTE)(Type & 0xff);
	sr.es  = ((ULONG)JRememberKey)>>16;
	r.x.di = (((ULONG)JRememberKey) & 0xffff);

	int86x(JFUNC_JINT,&r,&r,&sr);
	segread(&sr);

	if(GetBase(JSERV_AMIGASERVICE,&ParamSeg,&ParamOff,&BufferSeg)!=JSERV_OK)
		return(NULL);

/*	printf("ALLOCJR.C al = %lx\n",(ULONG)r.h.al);*/

	if(r.h.al) return(NULL);

	if( ! r.x.bx ) return(NULL);

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