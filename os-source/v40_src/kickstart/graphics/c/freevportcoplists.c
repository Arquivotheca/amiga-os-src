/******************************************************************************
*
*	$Id: freevportcoplists.c,v 39.2 92/10/07 14:01:06 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/view.h>
#include <graphics/copper.h>
#include "/macros.h"
#include "c.protos"

/*#define DEBUG*/

/* freeCopList frees entire coplist including header */
void freecoplist(struct CopList *cl)
{
	struct CopList *nextcl;

#ifdef DEBUG
	printf("freecoplist(%lx)\n",cl);
#endif
	for ( ; cl ; cl = nextcl)
	{
		FreeMem(cl->CopIns,sizeof(struct CopIns) * cl->MaxCount);
		nextcl = cl->Next;
		FreeMem(cl,sizeof(struct CopList));
	}
}

/*****************************************************************************/
/* search a user copperlist, freeing intermediate copperlists as you go */
/*****************************************************************************/
void freeucoplist(struct UCopList *ucl)
{
	struct UCopList *nextucl;

#ifdef DEBUG
	printf("freeing user copperlist\n");
#endif
	for ( ; ucl ; ucl = nextucl)
	{
		freecoplist(ucl->FirstCopList);
		nextucl = ucl->Next;
		FreeMem(ucl,sizeof(struct UCopList));
	}
}


/*****************************************************************************/
/* search for a viewport's copper lists and free them */
/*****************************************************************************/
void freevportcoplists(struct ViewPort *vp)
{
#ifdef DEBUG
	printf("freevportcoplists(%lx)\n",vp);
#endif
	if (vp)
    {
		/* now take care of copper stuff */
		freecoplist(vp->DspIns);
		freecoplist(vp->ClrIns);
		freecoplist(vp->SprIns);
		freeucoplist(vp->UCopIns);
		vp->DspIns = NULL;
		vp->ClrIns = NULL;
		vp->SprIns = NULL;
		vp->UCopIns = NULL;
    }
#ifdef DEBUG
	printf("exit freevportcoplists\n");
#endif
}
