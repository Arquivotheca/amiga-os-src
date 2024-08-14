
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
*	$Id: symbols.c,v 1.4 91/04/24 20:54:29 peter Exp $
*
*	$Locker:  $
*
*	$Log:	symbols.c,v $
 * Revision 1.4  91/04/24  20:54:29  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.3  89/11/22  16:24:32  kodiak
 * 1.5: Forbid()/Permit() using BGACK hold; "faster" symbol loading option.
 * 
 * Revision 1.2  88/01/21  13:38:23  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/


#include "std.h"
#include "special.h"
#include "symbols.h"

extern struct SymbolMap TopSymMap;

extern struct Symbol *FindSymbol();
extern struct Symbol *BindValue1();
extern struct Symbol *BindValue2();
extern struct Symbol *NewSymbol();

extern short Faster;


SetOffset(hunkNum, name, offset)
    long hunkNum;
    char *name;
    long offset;
{
    struct Symbol  *sp;

    if (Faster)
	sp = BindValue2 (name, ACT_OFFSET, offset);
    else
	sp = BindValue1 (name, ACT_OFFSET, offset);
    sp -> value2 = hunkNum;
}


long GetValue(name)
    char *name;
{
    struct Symbol  *sp;

    sp = FindSymbol (name, TopSymMap);
    if (sp == 0) {
	return (0);
    }
    return (sp -> value1);
}
