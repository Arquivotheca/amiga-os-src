/******************************************************************************
*
*	$Id: cleanup.c,v 39.1 92/06/05 11:47:19 mks Exp $
*
******************************************************************************/

#define	NEWCLIPRECTS_1_1

#include <exec/types.h>
#include <graphics/layers.h>

#include "layersbase.h"

struct mem_node
{
    UWORD   *where;
    LONG    how_big;
};

#define MAX_MN_PER_TABLE    48

struct mem_table
{
    struct Node n;
    struct mem_node *current;
    LONG    slots_available;
    struct mem_node mem_node[MAX_MN_PER_TABLE];
};

void __stdargs __asm common_cleanup(register __a0 struct Layer_Info *li,register __d0 long abortion)
{
struct mem_table *mt;
struct mem_node *p;
struct LayerInfo_extra *lie=li->LayerInfo_extra;
LONG max_nodes;

	while (mt = (struct mem_table *)RemHead((struct List *)&(lie->mem)))
	{
		if (abortion)
		{
			max_nodes = MAX_MN_PER_TABLE - mt->slots_available;
			p = mt->mem_node;
			while (max_nodes--)
			{
				if (LMN_REGION==p->how_big) DisposeRegion((struct Region *)(p->where));
				else if (LMN_BITMAP==p->how_big) FreeBitMap((struct BitMap *)(p->where));
				else FreeMem(p->where,p->how_big);
				p++;
			};
		}
		FreeMem(mt,sizeof(struct mem_table));
	}
}

struct mem_table *new_mem_table(struct Layer_Info *li)
{
    struct mem_table *mt;
    mt = (struct mem_table *)AllocMem(sizeof(struct mem_table),0);
    if (mt)
    {
        mt->current = mt->mem_node;
        mt->slots_available = MAX_MN_PER_TABLE;
        AddHead((struct List *)&(((struct LayerInfo_extra *)(li->LayerInfo_extra))->mem),mt);
    }
    return (mt);
}

BOOL __stdargs __asm remember_to_free(register __a0 struct Layer_Info *li,register __a1 VOID *where,register __d0 ULONG how_big)
{
struct mem_node *mn;
struct mem_table *mt;

    if (li == 0)    return (TRUE);

    mt = (struct mem_table *)(((struct LayerInfo_extra *)(li->LayerInfo_extra))->mem.mlh_Head);
    if ( mt == (struct mem_table *)&(((struct LayerInfo_extra *)(li->LayerInfo_extra))->mem.mlh_Tail)) /* is there a table ?*/
        if ( !(mt = new_mem_table(li)) )    return (FALSE);

    if (--mt->slots_available < 0)  /* any room left in current table? */
    {
        mt->slots_available = 0;
        if ( !(mt = new_mem_table(li)) )    return(FALSE); /* get another */
        mt->slots_available--;
    }

    mn = mt->current++; /* where to stash this guy */
    mn->where = where;
    mn->how_big = how_big;
    return(TRUE);
}

void cleanup(struct Layer_Info *li)
{
	common_cleanup(li,FALSE);
	UnlockLayerInfo(li);
}
