/****** ljanus.lib/FreeServiceMem ****************************************
*
*   NAME   
*     FreeServiceMem -- Free mem added to a ServiceData by AllocServiceMem()
*
*   SYNOPSIS
*     VOID FreeServiceMem(ServiceData, MemPtr)
*
*     VOID FreeServiceMem(struct ServiceData *,APTR);
*
*   FUNCTION
*     This routine frees memory that had been allocated with a call 
*     to AllocServiceMem().  You can choose to free a single block of 
*     memory or all the memory of the ServiceData structure.  
*     
*     The ServiceData structure pointer that you provide to this routine 
*     doesn't have to point to any particular Janus memory-access address 
*     space (although it must point to Janus memory of course).  What this 
*     means is that if you translate the ServiceData pointer you get from 
*     AddService() or GetService() from word-access to byte-access or 
*     anything else, you don't have to translate it back before calling 
*     FreeServiceMem().  
*     
*     Note that the address of the ServiceData structure that you supply to 
*     this routine must be the same address that you passed to the
*     associated AllocServiceMem() call, if any.
*     
*     The MemPtr argument may be one of two things:
*        - the pointer to memory returned by a call to AllocServiceMem()
*          (using the same ServiceData structure as the one provided to
*          this call), or
*        - NULL to designate that you want all (if any) of the ServiceData's
*          allocated memory to be freed (note that this does not include
*          freeing the memory allocated at time of AddService()
*     
*     Note that only memory allocated by AllocServiceMem can be freed.  
*     The memory allocated for the service during AddService() cannot be 
*     deleted using FreeServiceMem(), not even if you call FreeServiceMem() 
*     with a MemPtr argument of NULL.  
*     
*     You are allowed to call this routine whether you have acquired the 
*     ServiceData address from AddService() or GetService().  
*
*   INPUTS
*     ServiceData = address of a ServiceData structure.  This may point to 
*                   any type of Janus memory-access address, not
*                   necessarily word-access
*     MemPtr      = a pointer to the Janus memory returned by a call 
*                   to AllocJanusMem(), or NULL if you want to delete
*                   all of the ServiceData's memory
*
*   RESULT
*     None
*
*   EXAMPLE
*     struct ServiceData *SData;
*     if (GetService(&SData, ...) == JSERV_OK))
*     {
*        \* Allocate a bunch of memory *\
*        AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*        AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*        ptr1 = AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
* 
*        \* Free the last one allocated *\
*        FreeServiceMem(SData, ptr1);
* 
*        \* Free all the rest *\
*        FreeServiceMem(SData, NULL);
* 
*        ReleaseService(SData);
*     }
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     AllocServiceMem()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

VOID FreeServiceMem(sdata,ptr)
struct ServiceData *sdata;
APTR ptr;
{
	union REGS r;
	struct SREGS sr;
	UWORD ParamSeg,ParamOff,BufferSeg;
	UWORD	Temp;

	if(GetBase(JSERV_AMIGASERVICE,&ParamSeg,&ParamOff,&BufferSeg)!=JSERV_OK)
		return;

	if(ptr != 0)
	{
		Temp = (UWORD)((ULONG)ptr>>16);
		if(Temp==ParamSeg)
			r.h.al=MEMF_PARAMETER;
		else
   		if(Temp==BufferSeg)
				r.h.al=MEMF_BUFFER;
			else
				return;
	}
	r.h.ah = JFUNC_FREESERVICEMEM;
	r.x.bx = (((ULONG)ptr) & 0xffff);
	r.x.di = (((ULONG)sdata) & 0xffff);

	int86x(JFUNC_JINT,&r,&r,&sr);
/*	segread(&sr);*/
}
