
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
*	$Id: hunks.c,v 1.3 91/04/24 20:52:56 peter Exp $
*
*	$Locker:  $
*
*	$Log:	hunks.c,v $
 * Revision 1.3  91/04/24  20:52:56  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.2  88/01/21  13:37:07  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/

#include "symbols.h"

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/tasks.h>
#include <exec/resident.h>

#include <exec/execbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#define LTOB(b_ptr) (b_ptr << 2)

APTR GetMemL();
extern APTR CurrAddr;


BindHunks()
{
    long    hunk_entry;		/* hunk which is part of linked list */
    int     hunk_num = 0;	/* Count of hunks we have seen       */
    char    basename[40];

    puts("");
    if ((ULONG) CurrAddr & 0x3) {
	printf ("  hunks must be long word aligned\n");
    }
    else {
	for (hunk_entry = ((ULONG) CurrAddr >> 2);
		hunk_entry & hunk_entry + 1;/* ???? */
		hunk_entry = (long) GetMemL (LTOB (hunk_entry))) {
	    printf ("hunk %3ld at %06lx\n", hunk_num, LTOB (hunk_entry + 1));
	    sprintf (basename, "`hunk_%lx", hunk_num++);
	    BindValue1 (basename, ACT_BASE, LTOB (hunk_entry + 1));
	}
	FixupTopMap ();
    }
}



AttachHunks()
{
    struct Process  proc;
    struct CommandLineInterface cli;
    long    hunk_entry;		/* hunk which is part of linked list */
    int     hunk_num = 0;	/* Count of hunks we have seen       */
    char    basename[40];

    getBlock (CurrAddr, &proc, sizeof (proc));
    puts (" ");
    getBlock (LTOB (proc.pr_CLI), &cli, sizeof (cli));

 /* Hunks are formed with a link list of pointers one word before the
    start of the actual hunk area (hunk_entry[1]) */
    for (hunk_entry = cli.cli_Module;
	    hunk_entry & hunk_entry + 1; /* ??? */
	    hunk_entry = (long) GetMemL (LTOB (hunk_entry))) {
	printf ("hunk %3ld at %06lx\n", hunk_num, LTOB (hunk_entry+1));
	sprintf (basename, "`hunk_%lx", hunk_num++);
	BindValue1 (basename, ACT_BASE, LTOB (hunk_entry + 1));
    }
    FixupTopMap ();
}
