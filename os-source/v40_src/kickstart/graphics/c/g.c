/******************************************************************************
*
*	$Id: g.c,v 39.5 93/04/15 16:16:42 spence Exp $
*
******************************************************************************/

/* graphics  kernel routines */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "/copper.h"
#include "/gfx.h"
#include "/macros.h"
#include "tune.h"
#include "/view.h"
#include "c.protos"

/*#define DEBUG*/

void __asm CBump(register __a1 struct UCopList *ucl)
{
    struct CopIns *ci;

    ci = ucl->CopList->CopPtr;
    /* let new cbump do the work - bart 08.01.85 */
    ci = cbump(&ucl->CopList,++ci);
    /* do the following rather than directly assign value of
	 cbump to ucl->CopList->CopPtr because of side effects */

    ucl->CopList->CopPtr = ci;

}

struct CopList * __regargs copinit(cl,num)
struct CopList *cl;
int num;
{
    struct CopIns *ci;
	UBYTE	flag;	/* set TRUE if head node allocated */

#ifdef DEBUG
	printf("copinit(%lx,%ld)\n",cl,num);
#endif
	flag = FALSE;
	if (cl == NULL)
	{
		/* allocate head node */
    		if ( (cl = (struct CopList *)
				AllocMem(sizeof(struct CopList),MEMF_CLEAR)) == 0)
    		{
#ifdef MEMDEBUG
	    		printf("copinit: error in getting cop list header\n");
#endif
			return (0);
    		}
		flag = TRUE;
		/*cl->CopIns = 0;
		cl->Next = 0;*/
	}
	if (( cl->CopIns == NULL ) ||
	    (cl->MaxCount < num))
	{
	    if (cl->CopIns)
	    {
	    	FreeMem(cl->CopIns, (sizeof(struct CopIns) * cl->MaxCount));
	    }
	    /* get actual memory for coplist instructions */
	    if ((ci = (struct CopIns *)
		 AllocMem(sizeof(struct CopIns)*num,0)) == 0)
	    {
#ifdef MEMDEBUG  
		printf("copinit: error in getting copper list memory\n");
#endif   
		if (flag)	
		{
		    FreeMem(cl,sizeof(struct CopList));
		    return(FALSE);
		}
		cl->MaxCount = 0;
		cl->CopIns = 0;
		cl->CopPtr = cl->CopIns;
		cl->Count = 0;
		return(cl);
	    }
	    cl->CopIns = ci;
	    cl->MaxCount = num;
	}
	cl->CopPtr = cl->CopIns;
	cl->Count = 0;
	cl->SLRepeat = 1;	/* always subtract 1 in CalcIVG() for the CEND */
	cl->Flags = 0;
	return(cl);
}

struct CopList * __asm UCopperListInit(register __a0 struct UCopList *ucl,
					register __d0 num)
{
  struct CopList *cl;

  /* bart - 10.03.85 */

  if ( ( cl = copinit(ucl->FirstCopList,num) ) == NULL ) return(NULL);
  ucl->FirstCopList = cl;
  ucl->CopList = cl;

  return(cl);
}

void __asm CLoad(register __a1 struct UCopList *ucl, register __d0 a, register __d1 b, register __d2 opcode)
{
	struct CopList *cl;
	struct CopIns *c;

	if ((cl = ucl->CopList) == 0)
	{
	    /* ucl->CopList = copinit(0,UCLSIZE);*/
	    /* cl = ucl->CopList; */

	    /* bart - 10.03.85 */
	    cl = UCopperListInit(ucl,UCLSIZE);
	}

	if( (cl) && (cl->MaxCount) && (cl->Count <= cl->MaxCount) )
	{
	    c = cl->CopPtr;
	    c->OpCode = opcode;
	    c->DESTADDR = a;	/* also c->VWAITPOS, c->HIRGB */
	    c->DESTDATA = b;	/* also c->HWAITPOS, c->LORGB */
	}
}

void __asm CMove(register __a1 struct UCopList *ucl, register __d0 a, register __d1 b)
{
	CLoad(ucl, a, b, COPPER_MOVE);
}

void __asm CWait(register __a1 struct UCopList *ucl, register __d0 a, register __d1 b)
{
	CLoad(ucl, a, b, COPPER_WAIT);
}

