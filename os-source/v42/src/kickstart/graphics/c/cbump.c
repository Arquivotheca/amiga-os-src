/******************************************************************************
*
*	$Id: cbump.c,v 42.0 93/06/16 11:15:06 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "tune.h"
#include <graphics/copper.h>
#include "/macros.h"
#include "c.protos"

/*#define DEBUG*/

struct CopIns * __regargs cbump(acl,c)
struct CopList **acl;
register struct CopIns *c;
{
    register struct CopList *cl = *acl;
    struct CopIns *ctemp;
#ifdef DEBUG
    printf("cbump(%lx,%lx)",acl,c);
#endif
    if (++cl->Count >= cl->MaxCount)
    {
#ifdef DEBUG
	printf(": look for another list\n");
#endif

	if (cl->Next == 0)  /* another buffer allocated? */
	{   /* nope */
	    if((cl->Next = (struct CopList *)AllocMem(sizeof( struct CopList),MEMF_PUBLIC)) == 0)
	    {
#ifdef MEMDEBUG
		printf("cbump: no copdsp mem\n");
#endif
		--cl->Count;
		return(c);
	    }
	    if ((ctemp = (struct CopIns *)AllocMem(sizeof(struct CopIns) * COPINSINC,MEMF_PUBLIC)) == 0)
	    {
		FreeMem( cl->Next, sizeof( struct CopList) );
		cl->Next = NULL;
#ifdef MEMDEBUG
		printf("cbump: no copins mem dsp\n");
#endif
		--cl->Count;
		return(c);
	    }
	    cl->Next->CopIns = ctemp;
	    cl->Next->MaxCount = COPINSINC;
	    cl->Next->Next = 0;
	}
	cl = cl->Next;
	ctemp = cl->CopIns;
	*acl = cl;
	c--;
	ctemp->OpCode = c->OpCode;
	ctemp->NXTLIST = c->NXTLIST;
	c->OpCode = CPRNXTBUF;
	c->NXTLIST = cl;
	cl->Count = 1;
	return(++ctemp);
    }
    return(c);
}
