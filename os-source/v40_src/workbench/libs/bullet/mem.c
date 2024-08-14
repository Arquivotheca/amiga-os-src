/*
**	$Id: mem.c,v 7.0 91/03/19 18:37:04 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	mem.c,v $
 * Revision 7.0  91/03/19  18:37:04  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:27:52  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 3.1  91/02/07  15:40:19  kodiak
 * Kodiak Phase 1 Final Release
 * 
 * Revision 3.0  90/11/09  17:11:40  kodiak
 * Kodiak's Alpha 1
 * 
*/
/*  mem.c  */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */
/* History
 *    05-May-90  awr   Character buffer is fixed size- allocated once
 *    22-Jul-90  awr   added caller id parameter to CGIFmove_block()
 *                     for consistency.
 *    20-Nov-90  dET   CGIFdefund() added #if CACHE
 *    04-Dec-90  jfd   Moved "include" statement for  "cgconfig.h" to line
 *                     following "port.h"
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

extern struct Library *SysBase;

#include "port.h"
#include "cgconfig.h"
#include "debug.h"

/*----------------*/
/*  MemAlloc      */
/*----------------*/
GLOBAL void *
MemAlloc(size)
    UWORD size;
{
    void *result;

    result = AllocVec(size, 0);

    DBG2("MemAlloc(%ld): $%lx\n", size, result);

    return(result);
}



/*----------------*/
/*  MemFree       */
/*----------------*/
GLOBAL void
MemFree(m)
void *m;
{
    DBG1("MemFree($%lx)\n", m);

    FreeVec(m);
}
