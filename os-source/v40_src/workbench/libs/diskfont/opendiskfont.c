/*
**      $Id: opendiskfont.c,v 39.1 92/07/10 10:12:59 darren Exp $
**
**      diskfont.library OpenDiskFont function
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

/*	Debug switches */


#ifdef	DEBUG
extern void kprintf(char *,...);
#define	D(a)	kprintf a
#else
#define	D(a)
#endif


/*	Includes */

#include <exec/types.h>

#include "diskfont.h"
#include "dfdata.h"

#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <libraries/dos.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <utility/tagitem.h>
#include <diskfont/diskfonttag.h>

#include <string.h>
#undef    strcat
#undef    strcmp
#undef    strcpy
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>

#define	SysBase		dfl->dfl_SysBase
#define	DOSBase		dfl->dfl_DOSBase
#define	GfxBase		dfl->dfl_GfxBase
#define	UtilityBase	dfl->dfl_UtilityBase

#ifndef TE0F_DUPLICATE
#define TE0F_DUPLICATE 0x80
#endif

struct TextFont * __saveds GenFont(struct DiskfontLibrary *dfl, char *fontPath,
	struct TTextAttr *tta, struct FontContentsHeader *fch,
	struct TFontContents *tfc, struct TextFont *tfRam,
	WORD wRam);

extern AddDiskFont();
extern EqualAspect();


/****** diskfont.library/OpenDiskFont ********************************
*
*   NAME
*       OpenDiskFont - load and get a pointer to a disk font.
*
*   SYNOPSIS
*       font = OpenDiskFont(textAttr)
*       D0                  A0
*
*   FUNCTION
*       This function finds the font with the specified textAttr on
*       disk, loads it into memory, and returns a pointer to the font
*       that can be used in subsequent SetFont and CloseFont calls.
*       It is important to match this call with a corresponding
*       CloseFont call for effective management of font memory.
*
*       If the font is already in memory, the copy in memory is used.
*       The disk copy is not reloaded.
*
*   INPUTS
*       textAttr - a TextAttr structure that describes the text font
*               attributes desired.
*
*   RESULTS
*       D0 is zero if the desired font cannot be found.
*
*   NOTES
*       As of V36, OpenDiskFont() will automatically attempt to
*       construct a font for you if:
*
*               You have requested a font size which does not exist
*               as a designed font, and
*
*               You have not set the DESIGNED bit in the ta_Flags
*               field of the TextAttr, or TTextAttr struct.
*
*       Constructed fonts are created by scaling a designed font.
*       A designed font is one which typically resides on disk,
*       or in ROM (e.g., a font which has been designed by hand
*       using a drawing tool).  Designed fonts generally look better
*       than fonts constructed by the font scaler, but designed
*       fonts also require disk space for each font size.
*
*       Always set the DESIGNED bit if you do not want constructed fonts,
*       or use AvailFonts() to find out which font sizes already exist.
*       
*       As of V37 the diskfont.library supported built-in outline
*       fonts.  Then in V38 the outline font engine was moved to
*       a new library, "bullet.library."
*
*   BUGS
*       This routine will not work well with font names whose file
*       name components are longer than the maximum allowed
*       (30 characters).
*
*********************************************************************/
struct TextFont *PatchDiskFont(
	struct DiskfontLibrary *dfl,
	struct DiskFontHeader *dfh)
{
    struct TagItem *dfhTags;

    /* fix bugs in some font editors: */
    dfh->dfh_TF.tf_Message.mn_Node.ln_Name = dfh->dfh_Name;
    if (dfh->dfh_TF.tf_Baseline >= dfh->dfh_TF.tf_YSize)
	dfh->dfh_TF.tf_Baseline = dfh->dfh_TF.tf_YSize - 1;
    dfh->dfh_TF.tf_Flags = (dfh->dfh_TF.tf_Flags & ~FPF_ROMFONT) |
	    FPF_DISKFONT;

    /* grab any tags from dfh_Segment */
    if (dfh->dfh_TF.tf_Style & FSF_TAGGED) {
	dfh->dfh_TF.tf_Style = (dfh->dfh_TF.tf_Style & ~FSF_TAGGED);
	dfhTags = (struct TagItem *) dfh->dfh_Segment;
    }
    else
	dfhTags = 0;

    /* set the dfh_Segment */
    dfh->dfh_Segment = (((ULONG) dfh)>>2)-2;	/* two longwords before dfh */

    if (ExtendFont(&dfh->dfh_TF, dfhTags)) {
	Forbid();
	AddDiskFont(dfl, dfh);
	Permit();
	return(&dfh->dfh_TF);
    }
    return(0);
}

struct TextFont *
loadDiskFont(
	struct DiskfontLibrary *dfl,
	struct TFontContents *tfc,
	char *name,
	char *fontPath)
{
    struct DiskFontHeader *dfh;
    struct TextFont *tf;
    BPTR dfs;
    char fontFile[256], *fp;

    strcpy(fontFile, fontPath);
    fp = FilePart(fontFile);
    tf = 0;
    strcpy(fp, tfc->tfc_FileName);
    D(("LoadSeg(\"%s\")", fontFile));
    dfh = (struct DiskFontHeader *) ((((ULONG) (dfs = (BPTR)
	    LoadSeg(fontFile))) << 2) + 8);
    D((", dfh = 0x%lx\n", dfh));
    if (((ULONG) dfh) != 8) {
	if (dfh->dfh_FileID == DFH_ID) {
	    strcpy(dfh->dfh_Name, name);
	    tf = PatchDiskFont(dfl, dfh);
	}
	if (!tf)
	    UnLoadSeg(dfs);
    }
    return(tf);
}


struct TextFont *
openSimilarFont(
	struct DiskfontLibrary *dfl,
	struct TTextAttr *tta,
	struct TFontContents *tfc,
	BOOL tfcTFlag,
	UWORD tfcSize,
	BOOL scaleOKFlag,
	char *fontPath)
{
    struct TextFont *tf;

    union {
	ULONG TagDPI;
	struct {
	    UWORD DPIX;
	    UWORD DPIY;
	} Tag;
    } t1, t2;
    int y1, y2;
    short i;
    BOOL isTagAware;

    D(("openSimilarFont((,%s, %ld, 0x%02lx, 0x%02lx),,,,)\n", tta->tta_Name,
	    tta->tta_YSize, tta->tta_Style, tta->tta_Flags));
    /* look for match in ram */
    Forbid();
    tf = (struct TextFont *) &GfxBase->TextFonts;
    D(("  ram\n"));
    while (tf = (struct TextFont *)
	    FindName((struct List *) tf, tta->tta_Name)) {
	D(("    (, %ld, 0x%02lx, 0x%02lx), ", tf->tf_YSize, tf->tf_Style,
		tf->tf_Flags));

	if ((!scaleOKFlag) &&
		(!(tf->tf_Flags & (FPF_ROMFONT|FPF_DISKFONT|FPF_DESIGNED)))) {
	    D(("font already scaled\n"));
	    continue;
	}
	/* check YSize match */
	if (tf->tf_YSize != tta->tta_YSize) {
	    D(("YSize mismatch\n"));
	    continue;
	}
	/* check attributes match */
	if ((tf->tf_Style & (FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC)) !=
		(tta->tta_Style & (FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC))) {
	    D(("Style mismatch\n"));
	    continue;
	}

/* If tagged request, must match DPI for RAM fonts */

	isTagAware = FALSE;

	if ((tta->tta_Style & FSF_TAGGED)) { 
	    /* check aspect match */
	    t1.TagDPI = GetTagData(TA_DeviceDPI, 0, tta->tta_Tags);
	    if (t1.TagDPI) {

		isTagAware = TRUE;

/* assume 1:1 if no DPI */

		t2.TagDPI = GetTagData(TA_DeviceDPI, 0x00010001,
			((struct TextFontExtension *) tf->tf_Extension)->
			tfe_Tags);
		if (t2.TagDPI) {
		    y1 = (tta->tta_YSize * t1.Tag.DPIX) / t1.Tag.DPIY;
		    y2 = (tf->tf_YSize * t2.Tag.DPIX) / t2.Tag.DPIY;
		    D(("    tagged: %ld vs. %ld\n", y2, y1));
		    if (y2 != y1)
			continue;
		}
	    }
	}

	/* If not tag aware, make sure this is not a DUP font which cannot
	   be opened
	*/
	if(!(isTagAware))
	{
		if(((struct TextFontExtension *) tf->tf_Extension)->tfe_Flags0 & TE0F_DUPLICATE)
		{
			D(("   NOT TAG AWARE; FONT IS A DUPLCATE\n"));
			continue;
		}
	}
	/* this candidate matches */
	break;
    }

    if (tf) {
	tf->tf_Accessors++;
	D(("bumped similar tf 0x%lx accessors to %ld\n",
		tf, tf->tf_Accessors));
	Permit();
	return(tf);
    }

    Permit();

    /* look for match in contents */
    D(("  disk\n"));

    for (i = 0; i < tfcSize; i++)
    {
	D(("    (, %ld, 0x%02lx, 0x%02lx), ", tfc[i].tfc_YSize,
		tfc[i].tfc_Style, tfc[i].tfc_Flags));
	/* check YSize match */
	if (tfc[i].tfc_YSize != tta->tta_YSize)
	{
	    D(("YSize mismatch\n"));
	    continue;
	}
	/* check attributes match */
	if ((tfc[i].tfc_Style & (FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC)) !=
		(tta->tta_Style & (FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC)))
	{
	    D(("Style mismatch\n"));
	    continue;
	}

/**
   @@@ Revisit - Darren

   This code seems wrong to me; I think what we really want here is
   to find best DPI fit given 1-N choices for matching Y size.

   Actually we really want best proportion, but I won't change the
   behavior of this code for the initial V39 release (safety).

   The problem with this code is if it fails because of a DPI mismatch,
   we end up using OpenWeighedFont(), which will try to come up with
   something closest in Y size rather than ideal proportions, and we
   are probably better off using a 1x, 1/2x, or 2x font.

   This code is a NOP for our diskfonts which do not use tags; gack!!

**/

	if ((tfcTFlag) && (tta->tta_Style & FSF_TAGGED) &&
		(tfc[i].tfc_TagCount))
	    {
	    t1.TagDPI = GetTagData(TA_DeviceDPI, 0, tta->tta_Tags);
	    if (t1.TagDPI) {
		t2.TagDPI = GetTagData(TA_DeviceDPI, 0,
			(struct TagItem *) &tfc[i].tfc_FileName[MAXFONTPATH-
			(tfc[i].tfc_TagCount*sizeof(struct TagItem))]);
		if (t2.TagDPI) {
		    y1 = (tta->tta_YSize * t1.Tag.DPIX) / t1.Tag.DPIY;
		    y2 = (tfc[i].tfc_YSize * t2.Tag.DPIX) / t2.Tag.DPIY;
		    D(("tagged: %ld vs. %ld, ", y2, y1));
		    if (y2 != y1) {
			D(("tag mismatch\n"));
			continue;
		    }
		}
	    }
	}

	D(("MATCH.  Attempt load.\n"));
	tf = loadDiskFont(dfl, tfc+i, tta->tta_Name, fontPath);
	if (tf)
	    return(tf);

    }

    return(0);				/* indicate no match */
}


struct TextFont *
openScaledFontSource(
	struct DiskfontLibrary *dfl,
	struct TTextAttr *tta,
	struct TFontContents *tfc,
	BOOL tfcTFlag,
	UWORD tfcSize,
	char *fontPath)
{
    struct TTextAttr ttta;
    struct TextFont *tf;

    ttta = *tta;

    for (;;) {
	/* look for exact size font */
	ttta.tta_YSize = tta->tta_YSize;
	if (tf = openSimilarFont(dfl, &ttta, tfc, tfcTFlag, tfcSize, 1,
		fontPath)) {
	    D(("found exact size font 0x%lx\n", tf));
	    return(tf);
	}

	/* look for double size font */
	ttta.tta_YSize = tta->tta_YSize*2;
	if (tf = openSimilarFont(dfl, &ttta, tfc, tfcTFlag, tfcSize, 0,
		fontPath)) {
	    D(("found double size font 0x%lx\n", tf));
	    return(tf);
	}

	/* look for half size font */
	if ((tta->tta_YSize & 1) == 0) {
	    /* half is an even integer */
	    ttta.tta_YSize = tta->tta_YSize/2;
	    if (tf = openSimilarFont(dfl, &ttta, tfc, tfcTFlag, tfcSize, 0,
		    fontPath)) {
		D(("found half size font 0x%lx\n", tf));
		return(tf);
	    }
	}

	/* try looser style restrictions that can be AlgoStyle'd later */
	if (ttta.tta_Style & FSF_UNDERLINED)
	    ttta.tta_Style &= ~FSF_UNDERLINED;
	else if (ttta.tta_Style & FSF_BOLD)
	    ttta.tta_Style &= ~FSF_BOLD;
	else if (ttta.tta_Style & FSF_ITALIC)
	    ttta.tta_Style &= ~FSF_ITALIC;
	else
	    break;
    }
    D(("found no applicable font\n"));
    return(0);
}


struct TextFont *
OpenWeighedFont(
	struct DiskfontLibrary *dfl,
	char *fontPath,
	struct TTextAttr *tta,		/* textAttr w/ FilePart of name */
	struct FontContentsHeader *fch,
	struct TFontContents *tfc,
	struct TextFont *tfRam,
	WORD wRam)
{
    struct TextFont *tf;
    WORD wCurr, wTemp;
    WORD wdIndex, i;

    /* grab the closest font a la OpenFont */
    D(("OpenWeighedFont(,,,,,, %ld)\n", wRam));
    do {
	wCurr = wRam;
	wdIndex = -1;
	if (fch->fch_FileID == FCH_ID) {
	    D(("FCH_ID "));
	    for (i = 0; i < fch->fch_NumEntries; i++) {
		wTemp = WeighTAMatch((struct TextAttr *) tta,
			(struct TextAttr *) (((ULONG) &tfc[i].tfc_YSize)-4), 0);
		D(("tfc[%ld] (%ld, 0x%lx, 0x%lx): %ld\n", i, tfc[i].tfc_YSize,
			tfc[i].tfc_Style, tfc[i].tfc_Flags, wTemp));
		if (wTemp > wCurr) {
		    wCurr = wTemp;
		    wdIndex = i;
		}
	    }
	}
	else {
	    D(("TFCH_ID "));
	    for (i = 0; i < fch->fch_NumEntries; i++) {
		if (tfc[i].tfc_TagCount) {
		    wTemp = WeighTAMatch((struct TextAttr *) tta,
			    (struct TextAttr *) (((ULONG) &tfc[i].tfc_YSize)-4),
			    (struct TagItem *) &tfc[i].tfc_FileName[MAXFONTPATH-
			    (tfc[i].tfc_TagCount*
			    sizeof(struct TagItem))]);
		    D(("tfc[%ld] (%ld, 0x%lx, 0x%lx, tagged): %ld\n", i,
			    tfc[i].tfc_YSize, tfc[i].tfc_Style,
			    tfc[i].tfc_Flags, wTemp));
		}
		else {
		    wTemp = WeighTAMatch((struct TextAttr *) tta,
			    (struct TextAttr *) (((ULONG) &tfc[i].tfc_YSize)-4),
			    0);
		    D(("tfc[%ld] (%ld, 0x%lx, 0x%lx, untagged): %ld\n", i,
			    tfc[i].tfc_YSize, tfc[i].tfc_Style,
			    tfc[i].tfc_Flags, wTemp));
		}
		if (wTemp > wCurr) {
		    wCurr = wTemp;
		    wdIndex = i;
		}
	    }
	}
	D(("wdIndex %ld\n", wdIndex));
	if (wdIndex >= 0) {
	    tf = loadDiskFont(dfl, tfc+wdIndex, tta->tta_Name, fontPath);
	    if (tf) {
		if (tfRam != 0) {
		    D(("close weighed ram font 0x%lx from %ld accessors\n",
			    tfRam, tfRam->tf_Accessors));
		    CloseFont(tfRam);
		}
		return(tf);
	    }
	    tfc[wdIndex].tfc_YSize = 0;	/* mark entry bad */
	}
    }
	while(wdIndex >= 0);

    return(tfRam);
}


struct TextFont * __asm DFOpenDiskFont(
	register __a6 struct DiskfontLibrary *dfl,
	register __a0 struct TTextAttr *textAttr)
{
    struct TTextAttr tta;		/* textAttr w/ FilePart of name */
    struct TextFont *tf, *tfRam, *tfSrc;
    struct DiskFontHeader *dfh;

    char fontPath[256];

    LONG fcFile, fcSize;

    struct FontContentsHeader fch;
    struct TFontContents *tfc;

    WORD wRam;

    LONG srcDPI, destDPI;

    BOOL scaleFlag;

    D(("OpenDiskFont((\"%s\", %ld, 0x%02lx, 0x%02lx)) from %s 0x%lx\n",
	    textAttr->tta_Name, textAttr->tta_YSize, textAttr->tta_Style,
	    textAttr->tta_Flags,
	    ((struct Task *) FindTask(0))->tc_Node.ln_Type == NT_PROCESS ?
	    "process" : "task", FindTask(0)));

    /* copy textAttr into tta, but with FilePart of name */
    tta = *textAttr;
    tta.tta_Name = FilePart(textAttr->tta_Name);
    if (tta.tta_YSize == 0)
	tta.tta_YSize = 1;

    /* single thread loading from disk */
    ObtainSemaphore(&dfl->dfl_BSemaphore);
    D(("  have semaphore\n"));

    wRam = 0;
    /* find best ram font */
    tfRam = OpenFont((struct TextAttr *) &tta);
    D(("  OpenFont((\"%s\", )): 0x%lx\n", tta.tta_Name, tfRam));
    if (tfRam != 0) {
	D(("    ram YSize %ld, Style 0x%02lx, Flags0x%02lx\n",
	    tfRam->tf_YSize, tfRam->tf_Style, tfRam->tf_Flags));
	/* get the weight for the resulting ram font */
	wRam = WeighTAMatch((struct TextAttr *) &tta,
		(struct TextAttr *) (((ULONG) &tfRam->tf_YSize)-4),
		((struct TextFontExtension *) tfRam->tf_Extension)->tfe_Tags);
	D(("tfRam 0x%lx, wRam %ld\n", tfRam, wRam));

        if (wRam == MAXFONTMATCHWEIGHT) {
            /* exact match: cannot get better from here! */
	    ReleaseSemaphore(&dfl->dfl_BSemaphore);
            return(tfRam);
	}
    }

    /* construct path to font from FONTS: */
    if (strchr(textAttr->tta_Name, ':'))
	strcpy(fontPath, textAttr->tta_Name);
    else {
	strcpy(fontPath, "FONTS:");
	AddPart(fontPath, textAttr->tta_Name, 256);
    }

    /* try to open the font contents file associated with the font */
    fch.fch_NumEntries = 0;
    fch.fch_FileID = OFCH_ID;
    tfc = 0;

    D(("  Open(\"%s\")", fontPath));
    if (fcFile = Open(fontPath, MODE_OLDFILE)) {
	D((": 0x%lx\n  Read()", fcFile));
	/* read and verify the font contents header */
	if ((Read(fcFile, &fch, sizeof(fch)) == sizeof(fch)) &&
		((fch.fch_FileID & 0xfff0) == FCH_ID)) {
	    fcSize = fch.fch_NumEntries*sizeof(struct TFontContents);
	    D((", n = %ld\n  AllocMem(%ld)", fch.fch_NumEntries, fcSize));
	    /* read the font contents into a buffer */
	    if ((fcSize) &&
		    (tfc = (struct FontContentsEntry *) AllocMem(fcSize, 0))) {
		D((": 0x%lx\n  Read()", tfc));
		if (fcSize != Read(fcFile, tfc, fcSize)) {
		    D((": failure\n"));
		    /* font contents read failure */
		    FreeMem(tfc, fcSize);
		    tfc = 0;
		}
	    }
	}
	D(("  Close($%lx)", fcFile));
	Close(fcFile);
	D(("\n"));
    }

    D(("tfc 0x%lx\n", tfc));
    if (!tfc)
	fch.fch_NumEntries = 0;

    if ((tfc) && (tfRam) && (fch.fch_FileID == FCH_ID) &&
	    (tta.tta_Style & FSF_TAGGED)) {

	D(("!!! RETRY WEIGHT !!!\n"));

	wRam = WeighTAMatch((struct TextAttr *) &tta,
		(struct TextAttr *) (((ULONG) &tfRam->tf_YSize)-4), 0);
    }

    /* special case outline font */
    if (fch.fch_FileID == OFCH_ID) {
	D(("outline font\n"));
	tf = GenFont(dfl, fontPath, &tta, &fch, tfc,
		tfRam, wRam);
    }

    /* redo match strategy if scaling */
    else if (!(textAttr->tta_Flags & FPF_DESIGNED)) {
	D(("scaling OK\n"));

	if (tfRam) {
	    D(("close original ram font 0x%lx from %ld accessors\n", tfRam,
		    tfRam->tf_Accessors));
	    CloseFont(tfRam);
	}

	/* scaling will occur for inexact match */
	if (tfc)
	    /* disk font available */
	    tfSrc = openScaledFontSource(dfl, &tta, tfc,
		    (BOOL) (fch.fch_FileID != FCH_ID), fch.fch_NumEntries,
		    fontPath);
	else
	    /* just rom/memory font available */
	    tfSrc = openScaledFontSource(dfl, &tta, 0, 1, 0, 0);

	if (tfSrc == 0) {
	    /* find best match for scaling that's *not* exact, *2 or /2 */
	    tta.tta_Flags |= FPF_DESIGNED;
	    tfRam = OpenFont((struct TextAttr *) &tta);
	    D(("  OpenFont((\"%s\", )): 0x%lx\n", tta.tta_Name, tfRam));
	    if (tfRam != 0) {
		/* get the weight for the resulting ram font */
		wRam = WeighTAMatch((struct TextAttr *) &tta,
			(struct TextAttr *) (((ULONG) &tfRam->tf_YSize)-4),
			((struct TextFontExtension *) tfRam->tf_Extension)->
			tfe_Tags);
		D(("tfRam 0x%lx, wRam %ld\n", tfRam, wRam));
	    }
	    else {
		wRam = 0;
	    }
	    tfSrc = OpenWeighedFont(dfl, fontPath, &tta, &fch, tfc,
		    tfRam, wRam);
	}

	tf = tfSrc;

	if (tfSrc == 0)
	    goto unDOS;

	scaleFlag = FALSE;

	/* check for differing Y size or aspect ratio */
	if (tta.tta_YSize != tfSrc->tf_YSize) scaleFlag = TRUE;
	
	if ((tta.tta_Style & FSF_TAGGED) &&
		    (ExtendFont(tfSrc,0))) {
		srcDPI = GetTagData(TA_DeviceDPI, 0x00010001,
			((struct TextFontExtension *) tfSrc->tf_Extension)->tfe_Tags);
	D(("srcDPI=$%lx\n",srcDPI));

		if (srcDPI) {
		    destDPI = GetTagData(TA_DeviceDPI, 0, tta.tta_Tags);

	D(("destDPI=$%lx\n",destDPI));

		    if (destDPI) {
			if (!EqualAspect(srcDPI, destDPI))
			{

			    scaleFlag = TRUE;
			}
		    }
		}
            }

	if (scaleFlag) {
	    /* construct scaled font */
	    dfh = (struct DiskFontHeader *) NewScaledDiskFont(tfSrc, &tta);
	    D(("scaling font 0x%lx, resulting in 0x%lx\n", tfSrc, dfh));
	    if (dfh != 0) {
		/* release source font and add new scaled font */
		D(("close source font 0x%lx from %ld accessors\n", tfSrc,
			tfSrc->tf_Accessors));
		CloseFont(tfSrc);
		tf = &dfh->dfh_TF;

		Forbid();
		AddDiskFont(dfl, dfh);
		Permit();
	    }
	}
    }
    else {
	tf = OpenWeighedFont(dfl, fontPath, &tta, &fch, tfc,
		tfRam, wRam);
    }

unDOS:
    D(("unDOS\n"));
    if (tfc) {
	D(("FreeMem(tfc)\n"));
	FreeMem(tfc, fcSize);
    }
    D(("OpenDiskFont result $%lx\n", tf));

    ReleaseSemaphore(&dfl->dfl_BSemaphore);
    return(tf);
}
