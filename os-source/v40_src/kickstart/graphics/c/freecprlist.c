/******************************************************************************
*
*	$Id: freecprlist.c,v 39.1 92/02/20 13:48:37 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/copper.h>
#include <graphics/gfx.h>
#include "/macros.h"
#include "c.protos"

/*#define DEBUG*/

/******************************************************************************/
/* search a hardware copperlist, freeing bufferblocks as you go		      */
/******************************************************************************/
	
freecprlist(cl)
register struct cprlist *cl;
{
    register struct cprlist *nextcl;

#ifdef DEBUG
	printf("freecprlist(%lx)\n",cl);
#endif

	for ( ; cl ; cl = nextcl)
	{
#ifdef DEBUG
		printf("freeing buffer at %lx size=%lx\n",cl->start,cl->MaxCount << 2);
#endif
		FreeMem(cl->start,cl->MaxCount << 2);	/* free buffer */
		nextcl = cl->Next;
#ifdef DEBUG
		printf("free list head at %lx\n",cl);
#endif
		FreeMem(cl,sizeof(struct cprlist));	/* free struct */
	}
#ifdef DEBUG
	printf("return from freecprlist\n");
#endif
}
