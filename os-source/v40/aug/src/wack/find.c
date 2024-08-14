/*
 * Amiga Grand Wack
 *
 * find.c -- Find and limit commands
 *
 * $Id: find.c,v 39.6 93/08/23 19:31:59 peter Exp $
 *
 * 
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

#include "sys_proto.h"
#include "parse_proto.h"
#include "main_proto.h"
#include "link_proto.h"
#include "ops_proto.h"
#include "io_proto.h"
#include "asm_proto.h"
#include "find_proto.h"

#define NAMESIZE 100

extern CPTR	CurrAddr;

static APTR  LimitAddr = 0;
static ULONG FindPat = 0xF4;	/* jimm's favorite	*/
static ULONG FindPatSize = 2;	/* 2: word, 4: long */

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


#if 0
void
FindHelp( void )
{
    PPrintf("    current find pattern: %lx, limit %lx, patsize %ld bytes\n",
	FindPat, LimitAddr, FindPatSize);
}
#endif

STRPTR
Limit( UBYTE *arg )
{
    APTR  limaddr;

    if (parseAddress(arg, (ULONG *) &limaddr)) 
    {
	LimitAddr = (APTR) ( ((ULONG) limaddr) & ~1 );
	PPrintf("limit set to %lx\n", LimitAddr);
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

STRPTR
StackLimit( UBYTE *arg )
{
    struct Task  *task;

    if (parseAddress(arg, (ULONG *)&task)) 
    {
	/* set limit to top of stack */
	LimitAddr = ReadPointer(&task->tc_SPUpper);

	/* set curraddr to stack pointer */
	CurrAddr = ReadLong(&task->tc_SPReg);

	PPrintf(" stack limit: %lx\n", LimitAddr);

	ShowFrame();
    }
    else 
    {
	BadSyntax();
    }

    return( NULL );
}

STRPTR
Find( char *arg )
{
    ULONG findpat;
    APTR limit;
    ULONG initial = FALSE;

    if ( parseHexNum( arg, &findpat, &FindPatSize ) )
    {
	FindPat = findpat;
	if ( FindPatSize < 2 )
	{
	    FindPatSize = 2;
	}
	initial = TRUE;
    }

    limit = LimitAddr;
    if ( LimitAddr == 0 )
    {
	limit = (APTR)( (ULONG)CurrAddr + 0x400 );
    }
    find( (UBYTE *)CurrAddr, limit, FindPat, FindPatSize, initial );

    return( NULL );
}

BOOL
match( ULONG *p, ULONG pat, WORD size )
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
	PPrintf("match: bad size %ld\n", size);
    }

    return (retval);
}

/* 16-bit aligned words and longs */
void
find(
UBYTE	*start,		/* (non-aligned) starting position	*/
UBYTE	*limit,		/* (non-aligned) limit address		*/
ULONG	pat,		/* pattern sought (short or long value)	*/
WORD	size,		/* number of bytes to match		*/
ULONG   initial )	/* If initial search for this pattern   */
{
    BOOL forward;	/* looking through increasing memory?	*/
    UBYTE	*wp;	/* pointer for word comparisons	*/
    BOOL patfound = FALSE; /* pattern was found		*/
    UBYTE	*curpos;	/* current search address (in amiga space) */
    UBYTE	*bufstart;	/* low end of search range	*/
    UBYTE	*buflast;	/* high end of search range	*/
    ULONG	bufsize;	/* size (bytes) of search ranges*/
    WORD	bump;		/* signed incr			*/

#if 0
PPrintf("Find: start %lx, limit %lx, pat %lx, size %lx\n",
    start, limit, pat, size);
#endif

    /* align start and limit */
    start = (UBYTE *) ( ( (ULONG) start ) & ~1 );
    limit = (UBYTE *) ( ( (ULONG) limit ) & ~1 );

    /* determine which direction to go	*/
    if (start == limit)
    {
	PPrintf("Find: start = limit = %lx\n", start);
	return;
    }
    if (start > limit) 
    {
	forward = FALSE;
	if ( !initial )
	{
	    start -= 2;		/* start looking just after current position */
	}
	bufstart = limit;	/* low address of search region is limit */
	bufsize = (ULONG) start - (ULONG) limit + 2;
	buflast = start;
	PPrintf(" searching backward\n");
    }
    else
    {
	forward = TRUE;
	if ( !initial )
	{
	    start += 2;		/* start looking just after current position */
	}
	bufstart = start;	/* low address of search region is limit */
	bufsize = (ULONG) limit - (ULONG) start + 2;
	buflast = limit;
    }
    bump =  forward? 2: -2;

#if 0
PPrintf("forward %lx, newstart %lx, bufstart %lx, buflast %lx, bufsize %lx\n",
    forward, start, bufstart, buflast, bufsize);
#endif

    /* get buffer of find region	*/
    wp = (UBYTE *)malloc( bufsize );
    if ( wp )
    {
	ReadBlock( bufstart, wp, bufsize );

	/* look for pattern	*/
	for (curpos = start; ((curpos <= buflast) && (curpos >= bufstart));
		curpos += bump)
	{
	    if (match((ULONG *)(wp + (curpos - bufstart)), pat, size)) 
	    {
		patfound = TRUE;
		break;
	    }

	}

	/* if found set current address to position 
	 * found, always refresh
	 */
	if (patfound)
	{
	    CurrAddr = (CPTR) curpos;
	}
	else
	{
	    PPrintf(" can't find %lx between addresses %lx and %lx\n", pat, start, limit);
	    CurrAddr = (CPTR)limit;
	}
	ShowFrame();
	free( wp );
    }
    else
    {
	PPrintf("Couldn't get %ld bytes to store find buffer\n", bufsize);
    }
}
