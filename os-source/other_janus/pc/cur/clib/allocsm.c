/****** ljanus.lib/AllocServiceMem ***************************************
*
*   NAME   
*     AllocServiceMem -- Allocate Janus memory linked to a ServiceData struct
*
*   SYNOPSIS
*     MemPtr = AllocServiceMem(ServiceData, Size,   Type)
*
*     APTR AllocServiceMem(struct ServiceData *,USHORT,USHORT);
*
*   FUNCTION
*     This routine allocates memory for you and records the details of 
*     the allocation in the specified ServiceData structure. This memory,
*     unless you free it explicitly with a call to FreeServiceMem(), 
*     will be automatically freed when the service is deleted and 
*     removed from the system.  
*     
*     This routine calls the Janus library's AllocJRemember() call for you, 
*     using the ServiceData's JRememberKey field to record the parameters 
*     of the allocation for you.  You may then free the memory using 
*     the FreeServiceMem() routine.  Alternatively, you may pay no 
*     further attention to this memory allocation because after the 
*     service is deleted and all users have released it, any memory 
*     allocated using the ServiceData's JRememberKey will be freed 
*     using the FreeJRemember() function.  
*     
*     You are allowed to call this routine whether you have acquired the 
*     ServiceData address from AddService() or GetService().  
*     
*     The ServiceData structure pointer that you provide to this routine 
*     doesn't have to point to any particular Janus memory-access address 
*     space (although it must point to Janus memory of course).  What this 
*     means is that if you translate the ServiceData pointer you get from 
*     AddService() or GetService() from word-access to byte-access or 
*     anything else, you don't have to translate it back before calling 
*     AllocServiceMem().  
*     
*     The Size and Type arguments are the same as those passed to the 
*     AllocJanusMem() function.  Please refer to the AllocJanusMem() 
*     documentation for a description of these arguments.  
*     
*     If the call succeeds, you are returned a pointer to the Janus 
*     memory that you desire.  If the call fails, a NULL is returned.  
*
*   INPUTS
*     ServiceData = address of a ServiceData structure. This may point to
*                   any type of Janus memory-access address, not
*                   necessarily word-access
*     Size        = the size in bytes of the memory allocation.
*     Type        = the type of the desired memory.  Please refer to the 
*                   AllocJanusMem() documentation for details about this
*                   field
*
*   RESULT
*     MemPtr = a pointer to the Janus memory you've requested, or NULL 
*              if no memory was available.
*
*   EXAMPLE
*     struct ServiceData *SData;
*     if (GetService(&SData, ...) == JSERV_OK))
*     {
*        ptr1 = AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*        ptr2 = AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*        ReleaseService(SData);
*     }
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     FreeServiceMem()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

APTR AllocServiceMem(sdata,Size,Type)
struct ServiceData *sdata;
USHORT Size;
USHORT Type;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;
	UWORD ParamSeg,ParamOff,BufferSeg;

	r.h.ah = JFUNC_ALLOCSERVICEMEM;
	r.x.bx = Size;
	r.h.al = (UBYTE)(Type & 0xff);
	r.x.di = (((ULONG)sdata) & 0xffff);

	int86x(JFUNC_JINT,&r,&r,&sr);
/*	segread(&sr);*/

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