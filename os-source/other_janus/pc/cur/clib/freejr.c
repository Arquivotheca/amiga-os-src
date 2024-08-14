/****** ljanus.lib/FreeJRemember *****************************************
*
*   NAME   
*     FreeJRemember -- Free memory allocated by calls to AllocJRemember()
*
*   SYNOPSIS
*     VOID FreeJRemember(JRememberKey, ReallyForget)
*
*     VOID FreeJRemember(RPTR *,BOOL);
*
*   FUNCTION
*		Except where noted this function behaves identically to the 
*     janus.library version. See the janus.library autodocs for a 
*     detailed explanation.
*
*   INPUTS
*     JRememberKey = the address of word in Janus memory that is a pointer 
*                    to a struct JanusRemember. This pointer should either 
*                    be NULL or set to some value (possibly NULL) by a call
*                    to AllocRemember()
*     ReallyForget = a BOOL FALSE or TRUE describing, respectively, 
*                    whether you want to free up only the Remember nodes
*                    or if you want this procedure to really forget about
*                    all of the memory, including both the nodes and the
*                    memory buffers pointed to by the nodes.
*
*   RESULT
*     None
*
*   EXAMPLE
*     RPTR JRememberKey;
*     JRememberKey = NULL;
*
*     while (AllocJRemember(&JRememberKey, BUFSIZE, MEMF_BUFFER)) ;
*        FreeJRemember(&JRememberKey, TRUE);
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     AllocJRemember(), intuition.library/FreeRemember(), AttachJRemember()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

VOID FreeJRemember(JRememberKey,ReallyForget)
RPTR *JRememberKey;
BOOL ReallyForget;
{
	union REGS r;
	struct SREGS sr;

	r.h.ah = JFUNC_FREEJREMEMBER;
	r.h.al = (UBYTE)(ReallyForget & 0xff);
	sr.es  = ((ULONG)JRememberKey)>>16;
	r.x.di = (((ULONG)JRememberKey) & 0xffff);

	int86x(JFUNC_JINT,&r,&r,&sr);
/*	segread(&sr);*/

}
