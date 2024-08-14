
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
*	$Id: symload.c,v 1.3 91/04/24 20:54:44 peter Exp $
*
*	$Locker:  $
*
*	$Log:	symload.c,v $
 * Revision 1.3  91/04/24  20:54:44  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.2  88/01/21  13:38:28  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/
#define VERSION "0.1.2"

#include "std.h"           /* Amiga standard types */
#undef NULL
#undef EOF
#include <stdio.h>         /* FILE type            */

#define MAX_SYMBOL_SIZE	100

#define MIN(a,b)           (a<b)?a:b
#define MAX(a,b)           (a>b)?a:b
#include "hunks.h"

static short debug = TRUE;

long SymbolCount;

SkipHunk ()
{
    long    size;

    size = GetLong ();
    while (size > 0) {
	GetLong ();
	size--;
    }
}

PrintSymbol(size)
   LONG size;
{
    WORD i = 0;
    ULONG symName[MAX_SYMBOL_SIZE / sizeof(ULONG)];

    while (size > 0) {
	symName[i++] = GetLong ();
	size--;
    }
    symName[i] = 0;
    printf ("'%s'", symName);
}

NewHunk()
{
}

ProcessSymbol(hunkNum)
    ULONG hunkNum;
{
    LONG size;
    UBYTE symType;
    WORD i = 0;
    ULONG symOffset;
    char    basename[20];
    ULONG symName[(MAX_SYMBOL_SIZE + 1) / sizeof (ULONG)];

    while ((size = GetLong ()) != 0) {
	symType = size >> 24;
	size &= 0xffffff;
	i = 0;
	while (size > 0) {
	    symName[i++] = GetLong ();
	    size--;
	}

	symName[i] = 0;
	symOffset = GetLong ();

	if ((symName[0] >> 24) != '.') {
/*
	if (symOffset == 0) {
	    printf ("zero offset!  %s in %s\n", symName, basename);
	}
*/
	    SetOffset (hunkNum, symName, symOffset);
	    SymbolCount++;
	}
    }
}


SuckSymbols () 
{
    ULONG hunkType;	/* Type of hunk block about to be read    */
    ULONG numLongs;	/* Number of long words in data following */
    ULONG hunkTableSize;/* Number of hunks that should be kept    */
    ULONG * hunkTable;	/* Table of addresses (indexes) in memory */
    ULONG firstHunk;	/* Index of first hunk into hunkTable     */
    ULONG last_hunk;	/* Index of last hunk into hunkTable      */
    ULONG hunkNum;	/* Which hunk in the table is being dealt */
    LONG size;

    SHORT counter = 0;

    hunkType = GetLong ();
    if (hunkType != HUNK_HEADER) {
	quit ("not a load file");
    }

    while ((size = GetLong ()) != 0) {
	printf ("resident library ");
	PrintSymbol (size);
	printf ("\n");
    }

    SymbolCount = 0;
    printf ("\n  loading symbols...\n");
   
    hunkTableSize = GetLong ();
    firstHunk = GetLong ();
    last_hunk = GetLong ();
/*
    printf ("table size=%ld  first=%ld  last=%ld\n"
	    ,hunkTableSize, firstHunk, last_hunk);
*/
    for (hunkNum = firstHunk; hunkNum < hunkTableSize; hunkNum++) {
	numLongs = GetLong ();
    }

    hunkNum = -1;
    while (!EOFGetLong (&hunkType)) {
	switch (hunkType) {
	    case HUNK_CODE: 
	    case HUNK_DATA: 
		SkipHunk ();
		NewHunk (hunkNum++);
		break;
	    case HUNK_BSS: 
		GetLong();
		NewHunk (hunkNum++);
		break;
	    case HUNK_DEBUG: 
	    case HUNK_NAME: 
		SkipHunk ();
		break;
	    case HUNK_CONT: 
		printf ("don't know how to handle hunk_cont\n");
		hunkNum = GetLong ();
		GetLong ();
		SkipHunk ();
		break;
	    case HUNK_RELOC32: 
		while ((size = GetLong ()) != 0) {
		    GetLong ();
		    for (; size > 0; size--)
			GetLong ();
		}
		break;
	    case HUNK_END: 
		break;
	    case HUNK_SYMBOL: 
		ProcessSymbol (hunkNum);
		break;
	    case HUNK_BREAK: 
	    case HUNK_OVERLAY: 
		quit ("found overlay type hunk");
	    default: 
		printf ("hunk type = %ld ", hunkType);
		quit ("unknown hunk");
	}
    }
    printf ("  %ld symbols loaded from %ld hunks\n", SymbolCount, hunkNum);
}
