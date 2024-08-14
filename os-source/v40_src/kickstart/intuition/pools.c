/*** pools.c *****************************************************************
 *
 * pools.c -- simple pool allocation scheme with pre-alloc
 *
 *  $Id: pools.c,v 38.0 91/06/12 14:28:05 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/lists.h>

#define printf kprintf
#define D(x)	;

/*
 * allocates things that begin with a Node, which it uses
 * to manage a free list.  You supply the free list and
 * specify how big the nodes will be and how many you want
 * preallocated during init.
 *
 * FUNCTIONS:
 * poolInit( list, nodesize, initalloc );
 *	set up the list as a free list for data of size 'nodesize'
 *	and pre-allocate 'initalloc' nodes which will stick around.
 * poolGet( list )
 *	check the free list for a node, alloc a new one if needed
 * poolReturn( node )
 *	return node to free list or freemem, depending on ln_Pri
 *
 * Special uses:
 * - the free list lh_Type and l_pad fields are used together
 *	to hold the size of the member chunks
 * - the node ln_Pri field is used to hold whether the
 *	node is to be free'd when it is returned or kept
 *	in the free list.  If pri > 0, the node is kept around.
 * - the name field of the node is used to point back to its
 *	free list, so you can't change it.
 *
 * So, if you have uses for some nodes, and you don't need the 
 * name field and can live with the use of the pri field,
 * this is for you.
 */

struct Node	*RemHead();

/* same size as list, just using last word for something of my own */
struct PoolList { 
   struct  Node *lh_Head;
   struct  Node *lh_Tail;
   struct  Node *lh_TailPred;
#if 1
   UWORD	pl_MemberSize;		/* my interpretation */
#else
   UBYTE   lh_Type;
   UBYTE   l_pad;
#endif
};

/* additional size of members is stashed in list header */
#define LH_NODESIZE( list )	(((struct PoolList *)(list))->pl_MemberSize)

/* free list back pointer in ln_Name */
#define LN_FREELIST( ln )	( (struct List *)ln->ln_Name)

/*
 * allocate a node, size gotten from list header
 */
STATIC struct Node	*
createPoolNode( lh )
struct List	*lh;
{
    struct Node	*ln;

    /* use size stashed in lh_Type */
    if ( ln = (struct Node *) AllocMem( LH_NODESIZE( lh ) , MEMF_CLEAR))
    {
	LN_FREELIST( ln ) = lh;	/* back link */
	/* ln->ln_Pri = 0; means will be freed  */
    }
    return ( ln );
}

/*
 * a pool is a free list of data chunks preceded by a header,
 * with size of chunks stored in the header.
 *
 * 'initnum' elements will be allocated and kept around.
 */
poolInit( lh, size, initnum )
struct List	*lh;
int		size;
{
    struct Node	*ln;

    D( printf("poolInit: %lx, size %ld, initnum %ld\n", lh, size, initnum ) );

    /* init the list	*/
    NewList( lh );


    LH_NODESIZE( lh ) = size;

    while ( initnum-- )
    {
	/* alloc a few guys, stick 'em on the list */
	if ( ln = createPoolNode( lh ) )
	{
	    ln->ln_Pri = 1;	/* we'll keep these >0 guys around */
	    AddTail( lh, ln );
	}
    }
}

struct Node	*
poolGet( lh )
struct List	*lh;
{
    struct Node	*ln;

    D( printf("poolGet: list at %lx\n", lh ) );

    if ( ! ( ln = RemHead( lh ) ) )
    {
	D( printf("poolGet: go get\n") );
	ln = createPoolNode();
    }

    D( printf("pG: returning %lx\n") );

    return ( ln );
}

poolReturn( ln )
struct Node	*ln;
{
    struct List	*lh;

    if ( ln )	/* feeling generous: check for stupid callers */
    {
	lh = LN_FREELIST( ln );

	D( printf("pR: list: %lx\n", lh ) );

	if ( ln->ln_Pri <= 0 )
	{
	    FreeMem( ln, LH_NODESIZE( lh ) );
	}
	else
	{
	    AddTail( lh, ln );
	}
    }
    D( else {kprintf("poolReturn NULL passed\n");Debug();} )
}

