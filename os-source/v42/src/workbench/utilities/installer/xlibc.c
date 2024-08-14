/*
 *		xlibc.asm
 *	
 *
 * Prototypes for functions defined in xlibc.c
 *
 *	unsigned char * 	CpyCstrBstr		(unsigned char * , unsigned char * );
 *	long 				GetDEnv			(unsigned char * , unsigned char * , long );
 *	long 				SetDEnv			(unsigned char * , unsigned char * );
 *	LONG 				GetFileSize		(unsigned char * );
 *	void * 				MemAlloc		(long , long );
 *	void 				MemFree			(void * );
 *	void 				AllMemFree		(void);
 *	void 				_AllMemFree		(void);
 *	void * 				RasterAlloc		(long , long );
 *	void 				TransferList	(struct List * , struct List * );
 *	
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 *	lwilton 10/25/93:
 *		Changed GetDEnv to return a null string and a good result if 
 *		the environment variable file can't be opened.
 */


/*****************************************************
 *        Parsec Soft Systems xlib functions         *
 *****************************************************/

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <graphics/gfx.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"
#include "xfunctions.h"
#include "macros.h"



/*====================== CpyCstrBstr ====================*/

#define CTOB(cptr)    ((long)(cptr) >> 2)

char *CpyCstrBstr(char *c,char *b)	/* b is a real pointer, we return BPTR */
{
	b[0] = strlen(c);
	CopyMem(c,b+1,b[0]);
	return (char *)CTOB(b);
}

/*====================== GetDEnv ====================*/

long GetDEnv(char *name,char *value,long max)
{
    short		nlen = strlen (name) + 5;
    char		*ptr = AllocMem (nlen, MEMF_PUBLIC);
    BPTR		fh;
	long 		len,
				result = 0;

    if (ptr)
	{
		strcpy (ptr, "ENV:");
		strcat (ptr, name);
		if (fh = Open(ptr, 1005))					/* get the env variable	*/
		{
			len = (Seek(fh, 0L, 1), Seek(fh, 0L, -1));
			if (len <= max && len >= 0)
			{
				if (Read(fh, value, len) == len)
				{
					value[len] = '\0'; 
					result = 1; 
				}
	    	}
	    	Close(fh);
		}
		else
		{
			value[0] = '\0';						/* return null string	*/
			result = 1;								/* missing is ok		*/
		}
		FreeMem(ptr, nlen);
    }
    return result;
}

/*====================== SetDEnv ====================*/

long SetDEnv(char *name,char *str)
{
    short 		nlen = strlen(name) + 5;
    short 		slen = strlen(str);
    long 		res = 0;
    char 		*ptr = AllocMem(nlen, MEMF_PUBLIC);
    BPTR		fh;

    if (ptr)
	{
		strcpy(ptr, "ENV:");
		strcat(ptr, name);
		if (fh = Open(ptr, 1006))
		{
			if (Write(fh, str, slen) == slen) 
				res = 1;
	    	Close(fh);
		}
		FreeMem(ptr, nlen);
    }

    return res;
}

/*====================== GetFileSize ====================*/

#define SCAT	goto ex

LONG GetFileSize(char *filename)
{
	register BPTR					fl=NULL;
	register struct FileInfoBlock	*fib;
	register LONG					result = -1,
									success;

	if (!MakeStruct(fib)) 
		goto ex;							/* create the file info block */
	if (!(fl = Lock(filename,ACCESS_READ))) 
		goto ex;

	if (success = Examine(fl,fib)) 
		result = fib->fib_Size;				/* object size */
ex:
	if (fl) 
		UnLock(fl);							/* get rid of lock */	
	UnMakeStruct(fib);						/* de-alloc fib */
	return result;
}

/*====================== MemAlloc, Etc. ====================*/

int atexit(void (*_func)(void));

typedef struct
{
	struct MinNode	node;
	long			size;
} MEMNODE;

typedef struct MemAllocEntry
{
	LONG			**address;
	LONG			size;
	LONG			flags;
} MemAllocEntry;

typedef struct MemAllocEntries
{
	WORD			entries;
	MemAllocEntry	table[1];
} MemAllocEntries;

typedef struct MinList MINLIST;

static MINLIST _AllocList;

void _AllMemFree(void);

void *MemAlloc(long size,long type)
{
	MEMNODE *mem;

	if (mem = AllocMem(size+sizeof(MEMNODE),type))
	{
		if (!_AllocList.mlh_Head)
		{
			NewList((struct List *)&_AllocList);
			atexit(_AllMemFree);
		}
		mem->size = size + sizeof(MEMNODE);
		AddTail((struct List *)&_AllocList,(struct Node *)mem++);
	}
	return (mem);
}

void MemFree(void *ptr)
{
	MEMNODE *mem = ptr;

	if (mem)
	{
		Remove((struct Node *)--mem);
		FreeMem(mem,mem->size);
	}
}

void AllMemFree(void)
{
	MEMNODE *mem;

	if (_AllocList.mlh_Head)
		while (mem = (MEMNODE *)RemHead((struct List *)&_AllocList))
			FreeMem(mem,mem->size);
}

void _AllMemFree(void)
{
	AllMemFree();
}

void *RasterAlloc(long x,long y)
{
	return MemAlloc(RASSIZE(x,y),MEMF_CHIP|MEMF_PUBLIC);
}

/*====================== TransferList ====================*/

void TransferList(struct List *ls,struct List *ld)
{
	struct Node 	*head,*tail;

	NewList(ld);
	if ( !(head = GetHead(ls)) ) 
		return;
	tail = GetTail(ls);
	if (tail == head) 
	{ 
		AddHead(ld,head); 
		return; 
	}
	ld->lh_Head = head;
	ld->lh_TailPred = tail;
	head->ln_Pred = (struct Node *)&ld->lh_Head;
	tail->ln_Succ = (struct Node *)&ld->lh_Tail;
}
