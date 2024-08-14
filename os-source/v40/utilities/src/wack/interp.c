
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
*	$Id: interp.c,v 1.3 91/04/24 20:53:11 peter Exp $
*
*	$Locker:  $
*
*	$Log:	interp.c,v $
 * Revision 1.3  91/04/24  20:53:11  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.2  88/01/21  13:37:14  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/


#include "std.h"
#include "symbols.h"

extern ULONG FrameSize;
extern APTR  DisasmAddr;
extern UBYTE refresh;
extern char ThisKey;
extern char getchar();
struct Symbol *FindSymbol();
extern struct SymbolMap TopSymMap;

extern short flags;
#define NOREFRESH (flags = 0)

GetLine(c)
    char *c;
{
    short   cc = 0;

    while ((ThisKey = getchar()) != ')') {
	if (ThisKey == 8) {
	    if (cc > 0) {
		c[cc--] = 0;
		printf ("\010 \010");
	    }
	}
	else {
	    c[cc++] = ThisKey;
	    printf ("%lc", ThisKey);
	}
    }
    c[cc] = 0;
    c[cc+1] = 0;
    return cc;
}

char *
FindSpace(c,i)
    char *c;
    int i;
{
    while ((*c != 0) & (*c != ' ') & (*c != '\011')) c++;
    return c;
}

char *
SkipSpace(c,i)
    char *c;
    int i;
{
    while ((*c != 0) & ((*c == ' ') | (*c == '\011'))) c++;
    return c;
}

/* Interpret: fake it for now just to get it out, much work to do here. */
Interpret()
{
   struct Symbol *sp;
    char    c[200];
    char   *e, *s;
    int     len;

    printf ("(");
    len = GetLine (c);
    printf (")");

    s = SkipSpace (c);
    e = FindSpace (s);
    *e++ = 0;

    sp = FindSymbol (s, &TopSymMap);
    if (sp != 0) {
	Evaluate (sp, e);
	/* jimm: don't want to refresh again */
	NOREFRESH;
    }
    else {
	printf ("  %s not bound\n", s);
    }
}


ShiftBPtr(argStr)
    char *argStr;
{
    long bptr;

    if (sscanf (argStr, " %x ", &bptr) >= 1) {
	printf (" => %lx\n", bptr<<2);
    }
    else {
	puts ("expected a hex number argument");
    }
}

ToDec(argStr)
    char *argStr;
{
    long num;

    if (sscanf (argStr, " %x ", &num) >= 1) {
	printf (" => %ld\n", num);
    }
    else {
	puts ("\nexpected a hex number argument");
    }
}

ToHex(argStr)
    char *argStr;
{
    long num;

    if (sscanf (argStr, " %d ", &num) >= 1) {
	printf (" => %lx\n", num);
    }
    else {
	puts ("\nexpected a decimal number argument");
    }
}

ClearScreen()
{
    putchar ('L'-'A'+1);
}
