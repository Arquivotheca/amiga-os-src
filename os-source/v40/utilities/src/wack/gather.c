
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
*	$Id: gather.c,v 1.4 90/05/30 11:08:09 kodiak Exp $
*
*	$Locker:  $
*
*	$Log:	gather.c,v $
 * Revision 1.4  90/05/30  11:08:09  kodiak
 * change where function to $ key and make @ alphanumeric
 * 
 * Revision 1.3  90/01/15  13:04:08  jimm
 * disable control-D quit
 * 
 * Revision 1.2  88/01/21  13:37:03  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/

#include "symbols.h"

#define CTRL_D 4
#define	NULLKEY		(0)	/* control-@	*/

extern char ThisKey;
extern long LastNum;

char    c[100];
short	cc;

short  BadNumber =0;
short  SkipNum =0;

extern struct KeyMap TopMap;
extern struct KeyMap *ThisMap;
struct KeyMap *SaveMap;
struct KeyMap GatherMap;

struct Symbol *FindSymbol();
extern struct SymbolMap TopSymMap;
extern short flags;
extern long CurrAddr;


GatherKeys ()
{
    if (ThisMap != &GatherMap) {
	cc = 0;
	SaveMap = ThisMap;
	ThisMap = &GatherMap;
    }

    if (ThisKey != ' ') {
	printf ("%c", ThisKey);
	c[cc++] = ThisKey;
    }
    BadNumber = 0;
}


GetNumber(n)
    long *n;
{
    ThisKey = ' ';
    GatherKeys();
    SkipNum = 1;

    while (ThisMap == &GatherMap) {
	DoKey (ThisKey = getchar ());
    }
    SkipNum = 0;

    if (!BadNumber) {
	*n = LastNum;
	return 1;
    }
    return 0;
}


GatherIgnore()
{
}


GatherCancelKey()
{
    if (cc > 0)
	NewLine ();
    cc = -1;
    ThisMap = SaveMap;
    BadNumber = 1;
}


GatherDelKey()
{
    c[cc] = 0;
    putchar (8);
    putchar (32);
    putchar (8);

    if (--cc <= 0) {
	GatherCancelKey ();
    }
}

GatherDoneKey ()
{
    struct Symbol  *sp;

    c[cc] = 0;

    ThisMap = SaveMap;
 
    if (cc <= 0) {
	BadNumber = 1;
	return 0;
    }

    sp = FindSymbol (c, &TopSymMap);
    if (sp != 0) {
	Evaluate (sp, "");	/* jimm: added second argument */
    }
    else {
	if (!IsHexNum (c, &LastNum)) {
	    BadNumber = 1;
	    PutError ("unknown symbol");
	}
	else {
	      if (!SkipNum) {
		CurrAddr = LastNum;
		flags = -1;
	    }
	}
    }
}

extern struct Symbol exit_sym;

struct KeyAction ctrl_d1 = {
#if 1
    (struct KeyAction *) 0, NULLKEY, 0, &exit_sym
#else
    (struct KeyAction *) 0, CTRL_D, 0, &exit_sym
#endif
};


InitGather()
{
    cc = -1;

    GatherMap.head = &ctrl_d1;
    GatherMap.tail = &ctrl_d1;

    BindValue1 ("`GatherMap", ACT_KEYMAP, &GatherMap);

    BindPrim ("`GatherDelKey", GatherDelKey);
    BindPrim ("`GatherCancelKey", GatherCancelKey);
    BindPrim ("`GatherDoneKey", GatherDoneKey);
    BindPrim ("`GatherIgnore", GatherIgnore);

    BindKey (8, "`GatherMap", "`GatherDelKey");
    BindKey ('\n', "`GatherMap", "`GatherDoneKey");
    BindKey (21, "`GatherMap", "`GatherCancelKey");
    BindKey (23, "`GatherMap", "`GatherCancelKey");
    BindKey (24, "`GatherMap", "`GatherCancelKey");
    BindKey (' ', "`GatherMap", "`GatherIgnore");
    BindKey ('@', "`GatherMap", "`GatherKeys");
    BindKey ('`', "`GatherMap", "`GatherKeys");
    BindKey ('_', "`GatherMap", "`GatherKeys");
    BindSet ('0', '9', "`GatherMap", "`GatherKeys");
    BindSet ('a', 'z', "`GatherMap", "`GatherKeys");
    BindSet ('A', 'Z', "`GatherMap", "`GatherKeys");
}
