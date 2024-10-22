/* find.c -- new sun wack find/limit/fill commands
 * $Id: find.c,v 1.5 91/04/24 20:52:25 peter Exp $
 */
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/interrupts.h"
#include "exec/memory.h"
#include "exec/ports.h"
#include "exec/libraries.h"
#include "exec/devices.h"
#include "exec/tasks.h"
#include "exec/resident.h"
#include "exec/execbase.h"

#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "symbols.h"
#include "special.h"

#define NAMESIZE 100

extern CPTR	CurrAddr;
extern short flags;

static APTR  LimitAddr = 0;
static ULONG FindPat = 0xF4;	/* jimm's favorite	*/
static	WORD FindPatSize = 2;	/* 2: word, 4: long */

/*
 * User's guide to Sun Wack "Find Operations"
 *
 * The relevant commands are:
 *	find (or '/')
 *	limit
 *	stacklimit
 *
 * Each of these takes an argument.  It can be provided by
 * enclosing the command and argument in parentheses
 *	(find f4)
 * or by default, the current address is used except for 'find'.
 *
 * The relevant variables are
 *	find pattern
 *	find pattern size	(2 or 4 bytes)
 *	limit
 *
 * The current values can be seen by asking for extended help ('!').
 *
 * (find pattern) - sets pattern size to 4 if the pattern is
 *	larger than a word, by magnitute.  If no argument
 *	is supplied, will find last pattern.
 *	Then, it looks for the pattern (word or long) between
 *	the current address and the 'limit.'  (NOTE: for now,
 *	the limit is turned off, and it looks up to 200 hex bytes
 *	away.)
 *
 * (limit address) - sets limit (NOTE: actual limit used is
 *	current address + 0x200.)
 *
 * (stacklimit address) - 'address' should be a task.  Sets
 *	current address to USP, limit to stack Upper.
 *	
 */


FindHelp()
{
    printf("    current find pattern: %lx, limit %lx, patsize %ld bytes\n",
	FindPat, LimitAddr, FindPatSize);
}

/* parses out single hex argument (doesn't advance argument pointer)
 * uses CurrAddr if argument not present.  returns false if 
 * syntax error (scanf fails).
 */
BOOL
AddrArg(arg, val)
UBYTE	*arg;
ULONG	*val;
{
    BOOL retval = TRUE;
    ULONG argval;

    if (*arg)
    {
	if (sscanf (arg, " %lx ", &argval) >= 1)
	{
	    *val = argval;
	}
	else 
	{
	    printf("\nexpected hex address argument\n");
	    retval = FALSE;
	}
    }
    else 
    {
	*val = (ULONG) CurrAddr;
    }
    return (retval);
}

/* returns false if no argument passed, or bad syntax */
BOOL
HexArg(arg, val)
UBYTE *arg;
ULONG *val;
{
    BOOL retval = TRUE;
    ULONG argval;
    
    if ((*arg) && (sscanf (arg, " %lx ", &argval) >= 1))
    {
	*val = argval;
    }
    else 
    {
	retval = FALSE;
    }

    return (retval);
}

Limit(arg)
UBYTE *arg;
{
    APTR  limaddr;

    if (AddrArg(arg, &limaddr)) 
    {
	(ULONG) LimitAddr = (ULONG) limaddr & ~1;
	printf(" set to %lx", LimitAddr);
    }
    NewLine();
}

StackLimit(arg)
UBYTE *arg;
{
    struct Task  *task;

    if (AddrArg(arg, &task)) 
    {
	/* set limit to top of stack */
	LimitAddr = (APTR) GetMemL(&task->tc_SPUpper);

	/* set curraddr to stack pointer */
	CurrAddr = (CPTR) GetMemL(&task->tc_SPReg);

	printf(" stack limit: %lx\n", LimitAddr);

	REFRESH;
    }
    else 
    {
	puts("\nbad syntax");
    }
}

Find(arg)
unsigned char *arg;
{
    ULONG findpat;

    if (HexArg(arg, &findpat))
    {
	FindPat = findpat;
	FindPatSize = findInc(findpat);
    }

#if 0	/* tired of messing with limit. */
    find(CurrAddr, LimitAddr, FindPat, FindPatSize);
#else
    find(CurrAddr, CurrAddr + 0x200, FindPat, FindPatSize);
#endif
}

/* for now, determines whether it looks for words or longs only
 * by size of find value.  sorry.
 */
findInc(findpat)
ULONG findpat;
{
    WORD retval;

    /* supe this up some day */
    if (findpat & 0xFFFF0000)	/* has high word values */
    {
	retval = 4;
    }
    else retval = 2;

    return (retval);
}

BOOL
match(p, pat, size)
ULONG *p;	/* sun buffer location */
ULONG pat;
WORD	size;
{
    BOOL retval = FALSE;

    switch(size)
    {
     case 2:
	retval = (*((UWORD *) p) == (pat & 0xFFFF));
	break;
     case 4:
	retval = (*((ULONG *) p) == pat);
	break;
     default:
	printf("match: bad size %ld\n", size);
    }

    return (retval);
}

/* 16-bit aligned words and longs */
find(start, limit, pat, size)
UBYTE	*start;		/* (non-aligned) starting position	*/
UBYTE	*limit;		/* (non-aligned) limit address		*/
ULONG	pat;		/* pattern sought (short or long value)	*/
WORD	size;		/* number of bytes to match		*/
{
    BOOL forward;	/* looking through increasing memory?	*/
    UBYTE	*wp;	/* pointer for word comparisons	*/
    BOOL patfound = FALSE; /* pattern was found		*/
    UBYTE	*curpos;	/* current search address (in amiga space) */
    UBYTE	*bufstart;	/* low end of search range	*/
    UBYTE	*buflast;	/* high end of search range	*/
    WORD	bufsize;	/* size (bytes) of search ranges*/
    WORD	bump;		/* signed incr			*/

#ifdef DEBUG
printf("\nfind: start %lx, limit %lx, pat %lx, size %lx\n",
    start, limit, pat, size);
#endif

    /* align start and limit */
    (ULONG) start &= ~1;
    (ULONG) limit &= ~1;

    /* determine which direction to go	*/
    if (start == limit)
    {
	printf("\nfind start = limit = %lx\n", start);
	return;
    }
    if (start > limit) 
    {
	forward = FALSE;
	start -=  2;		/* start looking just before curr. pos */
	bufstart = limit;	/* low address of search region is limit */
	bufsize = (ULONG) start - (ULONG) limit + 2;
	buflast = start;
	puts(" searching backward");
    }
    else
    {
	forward = TRUE;
	start +=  2;		/* start looking just after curr. pos */
	bufstart = start;	/* low address of search region is limit */
	bufsize = (ULONG) limit - (ULONG) start + 2;
	buflast = limit;
    }
    bump =  forward? 2: -2;

#ifdef DEBUG
printf("forward %x, newstart %lx, bufstart %lx, buflast %lx, bufsize %lx\n",
    forward, start, bufstart, buflast, bufsize);
#endif

    /* get buffer of find region	*/
    wp = (UBYTE *)  malloc(bufsize);
    if (wp == NULL)
    {
	printf("\ncouldn't get %ld bytes to store find buffer\n", bufsize);
    }
    GetBlock(bufstart, wp, bufsize);

    /* look for pattern	*/
    for (curpos = start; ((curpos <= buflast) && (curpos >= bufstart));
	    curpos += bump)
    {

	if (match(wp + (curpos - bufstart), pat, size)) 
	{
	    patfound = TRUE;
	    break;
	}

    }

    /* if found set current address to position 
     * found, always refresh
     */
    if (patfound) CurrAddr = (CPTR) curpos;
    else puts(" can't find");
    REFRESH;
}
