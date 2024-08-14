
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
*	$Id: frame.c,v 1.3 91/04/24 20:52:34 peter Exp $
*
*	$Locker:  $
*
*	$Log:	frame.c,v $
 * Revision 1.3  91/04/24  20:52:34  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.2  88/01/21  13:37:00  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/


#include "std.h"

extern ULONG FrameSize;
extern APTR  DisasmAddr;
extern UBYTE refresh;

ShowFrame()
{
    refresh = 1;
}


DisAsm()
{
    APTR a;

    NewLine ();

    for (a = DisasmAddr; a < (DisasmAddr + FrameSize);) {
	a = do_decode (a);
	NewLine ();
    }
    DisasmAddr = a;
}
