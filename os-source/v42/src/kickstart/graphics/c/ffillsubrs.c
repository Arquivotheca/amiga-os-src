/******************************************************************************
*
*	$Id: ffillsubrs.c,v 42.0 93/06/16 11:15:29 chrisg Exp $
*
******************************************************************************/

/* ffillsubrs.c -- flood fill using dumb pixel read routines
 *
 *
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "/gfx.h"
#include "/macros.h"

#include "ffill.h"
#include "c.protos"

/*********************************************************************/
#ifndef MAKE_PROTOS
PUSH( fi, p1, p2, p3, p4 )          /* push onto point stack */
register struct flood_info *fi;
SHORT p1, p2, p3, p4;
{
    register struct stacknode *p;
    if ((p = fi->freelist) == 0)
    {   /* need to get some stack space */

                /* bart - 10.09.85 replaced with block allocation code */
		/*
		if ((p = (struct stacknode *)AllocMem(sizeof(struct stacknode),MEMF_PUBLIC)) == 0)
		{
#ifdef MEMDEBUG
	    	printf("memory flood fill\n");
#endif
			return (FALSE);
		}
		*/


		/* bart - 10.09.85 buy in bulk and save */
		{
		    struct stackblock *b;

		    if ((b = (struct stackblock *)
		       AllocMem(sizeof(struct stackblock),MEMF_PUBLIC)) == 0)
		    {
#ifdef MEMDEBUG
		    printf("push: can't allocate stackblock\n");
#endif
			    return (FALSE);
		    }
		    else
		    {
			int i;

			/* link into floodinfo */
			b->Next = fi->blocklist;
			fi->blocklist = b;

			/* use the first stacknode */
			p = &(b->nodes[0]);

			/* put the other nodes on the freelist */
                        for (i=1; i < STKBLKNUM; i++)
			{
			    (b->nodes[i]).Next = fi->freelist;
			    fi->freelist = &(b->nodes[i]);
			}
		    }
		}
    }
    else fi->freelist = p->Next;    /* remove from freelist */

    p->Next = fi->top;
    fi->top = p;

    /* bart - 10.09.85 removed to make way for packing code */
    /*
    p->p1 = p1;
    p->p2 = p2;
    p->p3 = p3;
    p->p4 = p4;
    */
    
    /* bart - 10.09.85 pack ulong p1234 from shorts p1,p2,p3,p4 */
    /* three ten bit values and one one bit flag value */
    /* bit 31 - sign bit: 1 if direction is -1 , 0 if direction is 1 */
    /* bits 20-29: p3 */
    /* bits 10-19: p2 */
    /* bits 00-09: p1 */

    p->p1234 = ( (ULONG)(p1 & 0x3ff) |
	      (ULONG)(p2 & 0x3ff)<<10 |
	      (ULONG)(p3 & 0x3ff)<<20 |
	      (ULONG)((p4 < 0)?1<<31:0) );

#ifdef DEBUG
    printf( "Push %lx:",p);
#endif
#ifdef BDEBUG
    printf( "\nPush %lx: L %ld, R %ld, Y %ld, DIR %ld",
    p,p1,p2,p3,p4 );
#endif

    return(TRUE);
}

void POP( struct flood_info *fi, SHORT *p1, SHORT *p2, SHORT *p3, SHORT *p4 )	/* get from stack */
{
    register struct stacknode *p;
    p = fi->top;
#ifdef DEBUG
    printf("POP %lx:",p);
#endif

    /* bart - 10.09.85 removed to make way for unpacking code */
    /*
    *p1 = p->p1;
    *p2 = p->p2;
    *p3 = p->p3;
    *p4 = p->p4;
    */
    
    /* bart - 10.09.85 unpack from ulong p1234 to shorts p1,p2,p3,p4 */
    /* three ten bit values and one one bit flag value */
    /* bit 31 - sign bit: 1 if direction is -1 , 0 if direction is 1 */
    /* bits 20-29: p3 */
    /* bits 10-19: p2 */
    /* bits 00-09: p1 */

    *p1 = (((p->p1234)) & 0x3ff);
    *p2 = (((p->p1234)>>10) & 0x3ff);
    *p3 = (((p->p1234)>>20) & 0x3ff);
    *p4 = (((p->p1234)>>31) & 0x1)?-1:1;

    fi->top = p->Next;
    p->Next = fi->freelist;
    fi->freelist = p;
}

void CLEANUP( struct flood_info *fi )          /* deallocate stackblocks for this floodinfo */
{
    struct stackblock *b;

    while (b =fi->blocklist)
    {
	fi->blocklist = b->Next;
	FreeMem(b,sizeof(struct stackblock));
    }
}

#endif // MAKE_PROTOS
