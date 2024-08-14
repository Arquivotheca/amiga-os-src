/* MemAlloc - malloc()-like functions that support special memory types */

#include "exec/types.h"
#include "exec/lists.h"
#include "exec/memory.h"
#include "graphics/gfx.h"
#include "functions.h"
#include "xfunctions.h"
#include "stdlib.h"

int atexit(void (*_func)(void));

typedef struct
{	struct MinNode	node;
	long			size;
} MEMNODE;

typedef struct MemAllocEntry
{	LONG			**address;
	LONG			size;
	LONG			flags;
} MemAllocEntry;

typedef struct MemAllocEntries
{	WORD			entries;
	MemAllocEntry	table[1];
} MemAllocEntries;

typedef struct MinList MINLIST;

static MINLIST _AllocList;

void _AllMemFree(void);

void *MemAlloc(long size,long type)
{	MEMNODE *mem;

	if (mem = AllocMem(size+sizeof(MEMNODE),type))
	{	if (!_AllocList.mlh_Head)
		{	NewList((struct List *)&_AllocList);
			atexit(_AllMemFree);
		}
		mem->size = size + sizeof(MEMNODE);
		AddTail((struct List *)&_AllocList,(struct Node *)mem++);
	}
	return (mem);
}

void MemFree(void *ptr)
{	MEMNODE *mem = ptr;

	if (mem)
	{	Remove((struct Node *)--mem);
		FreeMem(mem,mem->size);
	}
}

void AllMemFree(void)
{	MEMNODE *mem;

	if (_AllocList.mlh_Head)
		while (mem = (MEMNODE *)RemHead((struct List *)&_AllocList))
			FreeMem(mem,mem->size);
}

void _AllMemFree(void)
{	AllMemFree();
}

void *RasterAlloc(long x,long y)
{	return MemAlloc(RASSIZE(x,y),MEMF_CHIP|MEMF_PUBLIC);
}

#if 0
	/* not used by installer */

long MemAllocList(void *ptr)
{	WORD	i,j;
	long	*result;
	MemAllocEntries *list = ptr;

	for (i=0;i<list->entries;i++)
	{	result = MemAlloc(list->table[i].size,list->table[i].flags);
		if (result == NULL)
		{	for (j=i-1;j>=0;--j)
			{	MemFree(*list->table[j].address);
				*list->table[j].address = NULL;
			}
			return (long)i;
		}
		*list->table[i].address = result;
	}

	return -1;
}
#endif
