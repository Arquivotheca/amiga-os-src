/******************************************************************************
*
*	$Id: gfxallocmem.c,v 37.2 91/05/20 11:11:57 chrisg Exp $
*
******************************************************************************/

/* GfxAllocMem - bart ( barry a. whitebook ) 08.12.85 */
/* !!! this function is for Layers Internal Use ONLY !!! */
/* call AllocMem() ... if allocation sucessful return a pointer to the memory */
/* allocated... if unsucessful call Alert() to display No Memory message    */

#include <exec/types.h>
#include "/macros.h"
#include <graphics/gfxbase.h>
#include "c.protos"

/*#define DEBUG*/

#define ALERTGFXNOMEM 0x82010000
#define ALERTGFXNOLCM 0x82011234

APTR GfxAllocMem(numbytes,flags)
ULONG numbytes;
ULONG flags;
{
	APTR memoryptr;
	if ( (memoryptr = (APTR)AllocMem(numbytes,flags) ) == NULL )
	{
#ifdef DEBUG
		printf("gfx outof memory now you die(%ld,%lx\n",numbytes,flags);
		Debug();
#endif
	    Alert(ALERTGFXNOMEM);
	    return(NULL);
	}
	else
	{
	    return(memoryptr);
	}

}
