

/***********************************************************************
*
*			   G R A N D W A C K
*
************************************************************************
*
*	Copyright (C) 1985, Commodore-Amiga. All rights reserved.
*
************************************************************************
*
*   Source Control:
*
*	$Id: ops.c,v 1.5 92/08/27 12:09:07 peter Exp $
*
*	$Locker:  $
*
*	$Log:	ops.c,v $
 * Revision 1.5  92/08/27  12:09:07  peter
 * When full, the indirection stack is shifted down to make room for
 * new indirections, instead of failing.
 * 
 * Revision 1.4  91/04/24  20:50:06  peter
 * Added Nextaddr() function.
 * Changed $Header to $Id.
 * 
 * Revision 1.3  89/09/21  19:35:35  jimm
 * bringing rcs up to sync
 * 
 * Revision 1.2  88/01/21  13:37:24  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/

#include "wack.h"
#include "symbols.h"
#include "watch.h"

#define REFRESH flags = -1;

extern	short watch;

extern long FrameSize;
extern short flags;
extern unsigned long CurrAddr;
extern unsigned long SpareAddr;

long indirStack[MAXINDIR];
short indirCount = 0;


/**********************************************************************/
/* jimm: 4/15/89: whereis: indirect/where/exdirect		      */
Whereis()
{
    if ( Indirect() ) {
	ApproxSym();
	Exdirect();
    }
    flags = 0;		/* don't refresh	*/
}

/**********************************************************************/

/* Puts current address on stack, bumps stack down if full
 * (so we lose old stacked values, instead of being unable
 * to do new indirections
 */

stackCurrent()
{
    if (indirCount < MAXINDIR)
    {
	indirStack[indirCount++] = CurrAddr;
    }
    else
    {
	int i;
	for ( i = 0; i < MAXINDIR-1; i++ )
	{
	    indirStack[ i ] = indirStack[ i+1 ];
	}
	indirStack[MAXINDIR-1] = CurrAddr;
    }
}

Indirect()
{
    stackCurrent();

    CurrAddr = GetMemL (CurrAddr) & 0xfffffe;
    REFRESH
    return ( 1 );
}

/**********************************************************************/

IndirectBptr()
{
    stackCurrent();

    CurrAddr = GetMemL (CurrAddr) << 2;
    REFRESH
}

/**********************************************************************/

Exdirect()
{
    if (indirCount > 0) {
	CurrAddr = indirStack[--indirCount];
	REFRESH
    }
    else {
	PutError ("no indirections to undo");
    }
}

/**********************************************************************/

/* We now keep a spare address pointer you can swap with the
 * current one.  Also, certain functions that display a particular
 * address (eg.  xxxbase, awindow, etc.) set up the SpareAddr
 * variable.  This routine will swap the current address with
 * the spare one.
 */
swapSpareAddr()
{
    unsigned long temp;

    temp = CurrAddr;
    CurrAddr = SpareAddr;
    SpareAddr = temp;
    REFRESH
    return ( 1 );
}

/**********************************************************************/

Evaluate(sp, argStr)
    struct Symbol *sp;
    char *argStr;
{
    char s[20];
    if (watch & WATCH_EVALUATE) {
	printf ("** evalutating symbol: ");
	ShowSymbol (sp);
	WWAIT;
    }
    switch (sp -> stype) {
	case ACT_CONSTANT: 
	case ACT_BASE: 
	    CurrAddr = sp -> value1;
	    REFRESH
	    break;
	case ACT_PRIM: 
	    Execute (sp -> value1, argStr);
	    break;
	case ACT_OFFSET:
	    CurrAddr = (sp->value2) + (sp -> value1);
	    REFRESH
	    break;
	default: 
	    PutError ("unknown symbol type");
    }
}

/**********************************************************************/

SizeFrame()
{
    putchar (':');
    GetNumber (&FrameSize);
    REFRESH
}

/**********************************************************************/

NextCount()
{
    long i;

    putchar ('+');
    if (GetNumber (&i)) {
	CurrAddr += i;
	REFRESH
    }
    else
        putchar ('\n');
}

/**********************************************************************/

BackCount()
{
    long i;

    putchar ('-');
    if (GetNumber (&i)) {
	CurrAddr -= i;
	REFRESH
    }
    else
        putchar ('\n');
}

/**********************************************************************/

Resume()
{
    putchar ('\n');
    freeCPU();
}

/**********************************************************************/

SetBreakPoint()
{
    long i;

    printf ("%06lx %ld ^", CurrAddr, getBrkCnt (CurrAddr));
    if (GetNumber (&i)) {
	setBrkPt (i);
    }
    else
        putchar ('\n');
}

/**********************************************************************/

AssignMem()
{
    long    i;

    printf ("%06lx ", CurrAddr);

    if (FrameSize > 0)
	printf ("%04lx =", GetMem (CurrAddr) & 0xffff);
    else
	printf ("xxxx =");

    if (GetNumber (&i)) {
	PutMem (CurrAddr, i);
        REFRESH
    }
    else
        putchar ('\n');
}

/**********************************************************************/

Nothing()
{
    puts ("\nnothing happens.");
}

/**********************************************************************/
