/*
**	$Id: fontcontents.c,v 37.3 92/03/26 11:54:29 davidj Exp $
**
**      diskfont.library NewFontContents function
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

/*	Debug switches */

#ifdef	DEBUG
#define	D(a)	kprintf a
#else
#define	D(a)
#endif


/*	Includes */

#include <exec/types.h>

#include "diskfont.h"
#include <diskfont/diskfonttag.h>
#include "dfdata.h"

#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <libraries/dos.h>
#include <graphics/text.h>
#include <utility/tagitem.h>

#include <string.h>
#undef    strcat
#undef    strcmp
#undef    strcpy
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*	Imports */

#define	SysBase		dfl->dfl_SysBase
#define	DOSBase		dfl->dfl_DOSBase
#define	UtilityBase	dfl->dfl_UtilityBase


/****** diskfont.library/NewFontContents *****************************
*
*   NAME
*	NewFontContents -- Create a FontContents image for a font. (V34)
*
*   SYNOPSIS
*	fontContentsHeader = NewFontContents(fontsLock,fontName)
*       D0                                   A0        A1
*
*	struct FontContentsHeader *NewFontContents( BPTR, char * );
*
*   FUNCTION
*	This function creates a new array of FontContents entries
*	that describe all the fonts associated with the fontName,
*	specifically, all those in the font directory whose name
*	is that of the font sans the ".font" suffix.
*
*   INPUTS
*	fontsLock - a DOS lock on the FONTS: directory (or other
*	    directory where the font contents file and associated
*	    font directory resides).
*	fontName - the font name, with the ".font" suffix, which
*	    is also the name of the font contents file.
*
*   RESULT
*	fontContentsHeader - a struct FontContentsHeader pointer.
*
*   EXCEPTIONS
*	This command was first made available as of version 34.
*
*	D0 is zero if the fontName is does not have a ".font" suffix,
*	if the fontName is too long, if a DOS error occurred, or if
*	memory could not be allocated for the fontContentsHeader.
*
*   SEE ALSO
*	DisposeFontContents to free the structure acquired here.
*
*********************************************************************/
struct TempFontEntry {
    struct TempFontEntry *tfe_Next;
    struct TFontContents tfe_TFC;
};

struct FontContentsHeader * __asm DFNewFontContents(
	register __a6 struct DiskfontLibrary *dfl,
	register __a0 BPTR fontsLock, register __a1 char *fontName)
{
    struct FileInfoBlock *fib;
    char fontDirName[MAXFONTNAME];
    LONG prevCurrDir, otagLock, diskfontDir, fontSeg;
    LONG count;
    struct TempFontEntry *tfeHead, *tfeNext, *tfe;
    struct DiskFontHeader *dfh;
    struct FontContentsHeader *fch;
    struct TFontContents *tfc;
    WORD fileID, namePad, tagItemCount;
    struct TagItem *tagEntry, *tagList1, *tagList2;
    char *b;

    D(("NewFontContents(0x%lx, \"%s\")\n", fontsLock, fontName));

    fib = 0;
    tfeHead = 0;
    tfeNext = (struct TempFontEntry *) &tfeHead;
    fch = 0;
    otagLock = 0;
    diskfontDir = 0;
    fontSeg = 0;
    count = 0;
    fileID = FCH_ID;			/* guess no tags */
    prevCurrDir = CurrentDir(fontsLock);

    /* construct font directory name from font name sans .font suffix */
    if (strlen(fontName) > 30)
	goto ENDGAME;				/* font name too long */
    strcpy(fontDirName, fontName);
    if ((b = (char *) strrchr(fontDirName, '.')) == 0)
	goto ENDGAME;				/* no . for .font suffix */
    if (strcmp(b, ".font") != 0)
	goto ENDGAME;		 		/* suffix not .font */

    /* look for .otag file */
    strcpy(b, OTSUFFIX);
    otagLock = Lock(fontDirName, SHARED_LOCK);
    *b = '\0';					/* null terminate at . */

    /* look for font directory */
    if (!(diskfontDir = Lock(fontDirName, SHARED_LOCK)))
	goto OTAGCHECK;

    /* allocate the necessary FileInfoBlock */
    if ((fib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB, 0)) == 0)
	goto ENDGAME;				/* out of memory */

    /* look at all the files in the directory */
    CurrentDir(diskfontDir);

    if (Examine(diskfontDir, fib) == 0)
	goto ENDGAME;
    while (ExNext(diskfontDir, fib)) {
	if ((fib->fib_DirEntryType < 0) &&	/* entry is file */
		(fontSeg = LoadSeg(fib->fib_FileName))) {
	    dfh = (struct DiskFontHeader *) ((fontSeg<<2) + 8);
	    if ((dfh->dfh_FileID & 0xfff0) == DFH_ID) {
		tfe = (struct TempFontEntry *)
			AllocVec(sizeof(struct TempFontEntry), MEMF_CLEAR);
		if (tfe == 0)
		    goto ENDGAME;		/* out of memory */
		/* link temporary entry into entry list */
		tfeNext->tfe_Next = tfe;
		tfe->tfe_Next = 0;
		tfeNext = tfe;
		/* build the entry for this diskfont */
		strcpy(tfe->tfe_TFC.tfc_FileName, fontDirName);
		strcat(tfe->tfe_TFC.tfc_FileName, "/");
		strcat(tfe->tfe_TFC.tfc_FileName, fib->fib_FileName);
		D(("  tfc_FileName \"%s\"\n", tfe->tfe_TFC.tfc_FileName));
		namePad = MAXFONTPATH - strlen(tfe->tfe_TFC.tfc_FileName) - 3;
		D(("namePad %ld\n", namePad));
		if (namePad < 0)
		    goto ENDGAME;		/* name too long */
		tfe->tfe_TFC.tfc_YSize = dfh->dfh_TF.tf_YSize;
		tfe->tfe_TFC.tfc_Style = dfh->dfh_TF.tf_Style;
		tfe->tfe_TFC.tfc_Flags = (dfh->dfh_TF.tf_Flags & ~FPF_ROMFONT) |
			FPF_DISKFONT;
		tfe->tfe_TFC.tfc_TagCount = 0;
		if (dfh->dfh_TF.tf_Style & FSF_TAGGED) {
		    tagList1 = (struct TagItem *) dfh->dfh_Segment;
		    tagItemCount = 0;
		    while (NextTagItem(&tagList1)) {
			tagItemCount++;
			namePad -= 8;
		    }
		    D(("  TAGGED (n = %ld)\n", tagItemCount));
		    if (tagItemCount) {
			if (namePad < 8) {
			    D(("namePad %ld too long\n"));
			    goto ENDGAME;	/* name or tags too long */
			}
			tagItemCount++;
			tfe->tfe_TFC.tfc_TagCount = tagItemCount;
			tagList1 = (struct TagItem *) dfh->dfh_Segment;
			tagList2 = (struct TagItem *)
				&tfe->tfe_TFC.tfc_FileName[MAXFONTPATH-
				(tagItemCount*sizeof(struct TagItem))];
			while (tagEntry = (struct TagItem *)
				NextTagItem(&tagList1)) {
			    *tagList2++ = *tagEntry;
			    D(("    tag 0x%08lx value 0x%08lx\n",
				    tagEntry->ti_Tag, tagEntry->ti_Data));
			}
			tagList2->ti_Tag = TAG_DONE;
			fileID = TFCH_ID;	/* tags exist */
		    }
		}
		count++;
	    }			/* if dfh_FileID */
	    UnLoadSeg(fontSeg);
	    fontSeg = 0;
	}				/* file & LoadSeg */
    }					/* while ExNext */
    if (IoErr() != ERROR_NO_MORE_ENTRIES)
	goto ENDGAME;

OTAGCHECK:
    if (otagLock) {
	D((".otag exists\n"));
	fileID = OFCH_ID;
    }
    else
	if (!fib)
	    goto ENDGAME;

    /* allocate a buffer for the font contents */
    D(("  %ld tfc entries\n", count));
    if ((fch = (struct FontContentsHeader *)
	    AllocVec(sizeof(struct FontContentsHeader) +
	    (count * sizeof(struct TFontContents)), MEMF_PUBLIC)) == 0)
	goto ENDGAME;

    fch->fch_FileID = fileID;
    fch->fch_NumEntries = count;
    tfc = (struct TFontContents *) &fch[1];
    tfe = tfeHead;
    while (tfe) {
	*tfc++ = tfe->tfe_TFC;
	tfe = tfe->tfe_Next;
    }

ENDGAME:
    if (fontSeg != 0)
	UnLoadSeg(fontSeg);
    tfe = tfeHead;
    while (tfe) {
	tfeNext = tfe->tfe_Next;
	FreeVec(tfe);
	tfe = tfeNext;
    }
    UnLock(diskfontDir);			/* null is OK for UnLock */
    if (fib != 0)
	FreeDosObject(DOS_FIB, fib);
    UnLock(otagLock);				/* null is OK for UnLock */
    CurrentDir(prevCurrDir);

    return(fch);
}


/****** diskfont.library/DisposeFontContents *************************
*
*   NAME
*	DisposeFontContents -- Free the result from NewFontContents. (V34)
*
*   SYNOPSIS
*	DisposeFontContents(fontContentsHeader)
*			    A1
*
*	VOID DisposeFontContents( struct FontContentsHeader * );
*
*   FUNCTION
*	This function frees the array of FontContents entries
*	returned by NewFontContents.
*
*   INPUTS
*	fontContentsHeader - a struct FontContentsHeader pointer
*	    returned by NewFontContents.
*
*   EXCEPTIONS
* 	This command was first made available as of version 34.
*
*	A fontContentsHeader other than one acquired by a call
*	NewFontContents will crash.
*
*   SEE ALSO
*	NewFontContents to get structure freed here.
*
*********************************************************************/
void __asm DFDisposeFontContents(register __a6 struct DiskfontLibrary *dfl,
	register __a1 struct FontContentsHeader * fontContentsHeader)
{
    FreeVec(fontContentsHeader);
}
