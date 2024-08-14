/****** ljanus.lib/AttachJRemember ***************************************
*
*   NAME   
*     AttachJRemember -- Attach the list of one Janus memory key to another
*
*   SYNOPSIS
*     VOID AttachJRemember(ToKey, FromKey);
*
*     VOID AttachJRemember(RPTR *,RPTR *);
*
*   FUNCTION
*		Except where noted this function behaves identically to the 
*     janus.library version. See the janus.library autodocs for a 
*     detailed explanation.
*
*   INPUTS
*     ToKey   = address of a word in Janus memory that is the ToKey, which 
*               is going to receive the list pointed to by FromKey
*     FromKey = address of a word in Janus memory that is the FromKey, 
*					 which has the list that's going to be attached ToKey, 
*               after which the FromKey variable will be set to NULL
*
*   RESULT
*     None
*
*   EXAMPLE
*     RPTR JanusRemember GlobalJKey = NULL;
*     
*     exampleAllocJ(&GlobalJKey);
*     exampleAllocJ(&GlobalJKey);
*     exampleAllocJ(&GlobalJKey);
*     FreeJRemember(&GlobalJKey, TRUE);
*     
*     exampleAllocJ(globalkey)
*     RPTR *globalkey;
*     {
*        BOOL success;
*        RPTR localkey;
*     
*        success = FALSE;
*        localkey = NULL;
*     
*        if(AllocJRemember(&localkey,10000,MEMF_BUFFER | MEM_WORDACCESS))
*        if(AllocJRemember(&localkey,100  ,MEMF_BUFFER | MEM_WORDACCESS))
*        if(AllocJRemember(&localkey,1    ,MEMF_BUFFER | MEM_WORDACCESS))
*           success = TRUE;
*     
*        if (success) AttachJRemember(globalkey, &localkey);
*           else FreeJRemember(&localkey, TRUE);
*     }
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     intuition.library/AllocRemember(), AllocJRemember(), FreeJRemember()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

VOID AttachJRemember(ToKey,FromKey)
RPTR *ToKey,*FromKey;
{
	union REGS r;
	struct SREGS sr;
	ULONG ptr;

	r.h.ah = JFUNC_ATTACHJREMEMBER;
	sr.es  = ((ULONG)ToKey)>>16;
	r.x.di = ((ULONG)ToKey) & 0xffff;
	sr.ds  = ((ULONG)FromKey)>>16;
	r.x.si = (((ULONG)FromKey) & 0xffff);

	int86x(JFUNC_JINT,&r,&r,&sr);
/*	segread(&sr);*/

}
