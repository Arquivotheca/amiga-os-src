/*
**	$Id: availfonts.c,v 37.4 92/06/18 11:38:48 darren Exp $
**
**      diskfont.library AvailFonts function
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

/* debug switches */

#ifdef	DEBUG
extern void kprintf (char *,...);

#define	D(a)	kprintf a
#else
#define	D(a)
#endif

#define	DB(x)	;

/* includes */

#include <exec/types.h>

#include "diskfont.h"
#include <diskfont/diskfonttag.h>
#include "dfdata.h"

#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <utility/tagitem.h>

#include <string.h>
#undef    strcat
#undef    strcmp
#undef    strcpy
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*	exports */

LONG __asm DFAvailFonts (register __a6 struct DiskfontLibrary *,
			  register __a0 UBYTE *, register __d0 LONG, register __d1 ULONG);


/*	imports */

#define	DOSBase		dfl->dfl_DOSBase
#define	SysBase		dfl->dfl_SysBase
#define	GfxBase		dfl->dfl_GfxBase
#define	UtilityBase	dfl->dfl_UtilityBase


/*	locals */

    BOOL FillBuffer (struct DiskfontLibrary *,
		      UBYTE **, UBYTE **, LONG *, LONG *, char *,
		      struct TTextAttr *, struct TagList *, UWORD, LONG, LONG);


/****** diskfont.library/AvailFonts **********************************
*
*   NAME
*	AvailFonts -- Inquire available memory & disk fonts.
*
*   SYNOPSIS
*	error = AvailFonts(buffer, bufBytes, flags);
*	                   A0      D0        D1
*
*	LONG AvailFonts( struct AvailFontsHeader *buffer, LONG bufBytes
*		ULONG flags );
*
*   FUNCTION
*	AvailFonts fills a user supplied buffer with the structure,
*	described below, that contains information about all the
*	fonts available in memory and/or on disk.  Those fonts
*	available on disk need to be loaded into memory and opened
*	via OpenDiskFont, those already in memory are accessed via
*	OpenFont.  The TextAttr structure required by the open calls
*	is part of the information AvailFonts supplies.
*
*	When AvailFonts fails, it returns the number of extra bytes
*	it needed to complete the command.  Add this number to your
*	current buffer size, allocate a new buffer, and try again.
*
*   INPUTS
*	buffer - memory to be filled with struct AvailFontsHeader
*		followed by an array of AvailFonts elements, which
*		contains entries for the available fonts and their
*		names.
*
*	bufBytes - the number of bytes in the buffer
*	flags - AFF_MEMORY is set to search memory for fonts to fill
*		the structure, AFF_DISK is set to search the disk for
*		fonts to fill the structure.  AFF_SCALED is set to
*		not filter out memory fonts that are not designed.
*		AFF_BITMAP is set to filter out fonts that are not
*		stored in Amiga font format, i.e. to filter out
*		outline fonts.  Any combination may be specified.
*		AFF_TAGGED is set to fill the buffer with TAvailFonts
*		elements instead of AvailFonts elements.
*
*   RESULTS
*	buffer - filled with struct AvailFontsHeader followed by the
*		[T]AvailFonts elements, There will be duplicate entries
*		for fonts found both in memory and on disk, differing
*		only by type.  The existance of a disk font in the
*		buffer indicates that it exists as an entry in a font
*		contents file -- the underlying font file has not been
*		checked for validity, thus an OpenDiskFont of it may
*		fail.
*	error - if non-zero, this indicates the number of bytes needed
*		for AvailFonts in addition to those supplied.  Thus
*		structure elements were not returned because of
*		insufficient bufBytes.
*
*   EXAMPLE
*	int afShortage, afSize;
*	struct AvailFontsHeader *afh;
*
*	...
*
*	afSize = 400;
*	do {
*	    afh = (struct AvailFontsHeader *) AllocMem(afSize, 0);
*	    if (afh) {
*	        afShortage = AvailFonts(afh, afSize, AFF_MEMORY|AFF_DISK);
*	        if (afShortage) {
*	            FreeMem(afh, afSize);
*	            afSize += afShortage;
*	        }
*	    }
*	    else {
*	        fail("AllocMem of AvailFonts buffer afh failed\n");
*	        break;
*	    }
*	}
*	    while (afShortage);
*
*	\*
*	 * if (afh) non-zero here, then:
*	 * 1. it points to a valid AvailFontsHeader
*	 * 2. it must have FreeMem(afh, afSize) called for it after use
*	 *\
*
*********************************************************************/
#define	TIS	sizeof(struct TagItem)

BOOL FillBuffer (struct DiskfontLibrary * dfl,
		 UBYTE ** bufTopDown, UBYTE ** bufBottomUp, LONG * bufBytes,
		 LONG * bottomBytes, char *fontName, struct TTextAttr * fontAttr,
		 struct TagList * fontTags, UWORD AFType, LONG TTAFFlag, LONG TFCFlag)
{
    struct TAvailFonts *taf;
    WORD nameLen, tagLen;
    ULONG *tagSearch, *tagEnd;
    char *nameSearch, *s1, *s2;
    struct TagItem *tagList1, *tagList2, *tagEntry;
    int i;

    /* search for pre-existant instance of fontName */
    nameLen = strlen (fontName) + 1;	/* include null terminator */
    nameSearch = *bufBottomUp;
    for (i = *bottomBytes - nameLen; i >= 0; i--)
    {
	s1 = fontName;
	s2 = nameSearch;
	while (*s1)
	{
	    if (*s1++ != *s2++)
		goto failNameMatch;
	}
	if (*s2 == '\0')
	{
	    /* name already in buffer */
	    nameLen = 0;
	    break;
	}
      failNameMatch:
	nameSearch++;
    }
    taf = (struct TAvailFonts *) * bufTopDown;
    tagLen = 0;
    tagSearch = 0;		/* null tag list for empty one */
    if (TTAFFlag)
    {
	if (TFCFlag && fontTags)
	{
	    /* search for pre-existant instance of fontTags */

	    /* count tag items */
	    tagList1 = fontTags;
	    while (NextTagItem (&tagList1))
		tagLen++;

	    if (tagLen != 0)
	    {
		tagLen++;
		tagLen = tagLen * TIS;
		/* search for potential TAG_DONE, then for tag list match */
		tagSearch = (ULONG *) ((((ULONG) * bufBottomUp) + 1) & 0xfffffffe);
		tagEnd = (ULONG *) (((ULONG) tagSearch) + tagLen - TIS);
		for (i = (*bottomBytes - tagLen) / 2; i >= 0; i--)
		{
		    if (*tagEnd == TAG_DONE)
		    {
			tagList1 = fontTags;
			tagList2 = (struct TagItem *) tagSearch;
			while (tagEntry = (struct TagItem *)
			       NextTagItem (&tagList1))
			{
			    if ((tagEntry->ti_Tag != tagList2->ti_Tag) ||
				(tagEntry->ti_Data != tagList2->ti_Data))
				goto failTagMatch;
			    tagList2++;
			}
			/* fontTags is at tagSearch */
			tagLen = 0;
			break;
		    }
		  failTagMatch:
		    tagEnd = (ULONG *) (((LONG) tagEnd) + 2);
		    tagSearch = (ULONG *) (((LONG) tagSearch) + 2);
		}
	    }
	}
	*bufBytes -= tagLen + nameLen + sizeof (struct TAvailFonts);
	*bufTopDown += sizeof (struct TAvailFonts);
    }
    else
    {
	*bufBytes -= nameLen + sizeof (struct AvailFonts);
	*bufTopDown += sizeof (struct AvailFonts);
    }
    if (*bufBytes >= 0)
    {
	DB (kprintf ("FillBuffer type %ld, ", AFType));
	taf->taf_Type = AFType;
	if (nameLen)
	{
	    DB (kprintf ("original "));
	    *bottomBytes += nameLen;
	    *bufBottomUp -= nameLen;
	    for (i = 0; i < nameLen; i++)
		(*bufBottomUp)[i] = fontName[i];
	    taf->taf_Attr.tta_Name = *bufBottomUp;
	}
	else
	{
	    taf->taf_Attr.tta_Name = nameSearch;
	}
	DB (kprintf ("name \"%s\", ", taf->taf_Attr.tta_Name));

	taf->taf_Attr.tta_YSize = fontAttr->tta_YSize;
	taf->taf_Attr.tta_Flags = fontAttr->tta_Flags;
	DB (kprintf ("YSize %ld, Flags 0x%lx, ", taf->taf_Attr.tta_YSize,
		     taf->taf_Attr.tta_Flags));

	if (TTAFFlag && TFCFlag)
	{
	    taf->taf_Attr.tta_Style = fontAttr->tta_Style | FSF_TAGGED;
	    if (tagLen)
	    {
		DB (kprintf ("original "));
		*bottomBytes = (*bottomBytes + tagLen + 1) & 0xfffffffe;
		tagList1 = fontTags;
		tagList2 = (struct TagItem *) ((((ULONG) * bufBottomUp) -
						tagLen - 1) & 0xfffffffe);
		*bufBottomUp = (char *) tagList2;
		taf->taf_Attr.tta_Tags = tagList2;
		while (tagEntry = (struct TagItem *) NextTagItem (&tagList1))
		    *tagList2++ = *tagEntry;
		tagList2->ti_Tag = TAG_DONE;
	    }
	    else
	    {
		taf->taf_Attr.tta_Tags = (struct TagItem *) tagSearch;
	    }
	    DB (kprintf ("Tag @ 0x%06lx Style 0x%lx\n",
			 taf->taf_Attr.tta_Tags, taf->taf_Attr.tta_Style));
	}
	else
	{
	    taf->taf_Attr.tta_Style = fontAttr->tta_Style & ~FSF_TAGGED;
	    DB (kprintf ("Style 0x%lx\n", taf->taf_Attr.tta_Style));
	}
	return (TRUE);
    }
    DB (kprintf ("FillBuffer failure\n"));
    return (FALSE);
}


LONG __asm DFAvailFonts (register __a6 struct DiskfontLibrary * dfl,
			  register __a0 UBYTE * bufferIn, register __d0 LONG bufBytesIn,
			  register __d1 ULONG flags)
{
    struct AvailFontsHeader *afh;
    UBYTE *buffer, *bufBottomUp;
    LONG bufBytes, bottomBytes;
    struct TextFont *font;
    struct FileInfoBlock *fib;
    struct DevProc *dvp;
    UWORD memEntries, thisFontFlag;

    ULONG fDirLock, prevCurrDir, fcFile;
    char *nameTemp;
    struct FontContentsHeader fch;
    struct TFontContents tfc;

    struct TagItem *ti, tagItem;
    struct TTextAttr tTextAttr;
    UWORD *avail;
    int i;

    DB (kprintf ("AvailFonts(0x%06lx, %ld, 0x%02lx) TTATTR %s\n", bufferIn, bufBytesIn,
		 flags, (flags & AFF_TAGGED) ? "set" : "clear"));

    /* compatability for 1.3 programs like FED -
     *
     * If someone asks for flags -1, meaning ALL fonts, they likely
     * didn't mean AFF_TAGGED.
     *
     * This als means we need to reserve a bit - the high bit - so
     * that there can never be a legit combination which equals -1
     */
    if (flags==-1L) flags^=AFF_TAGGED;	/* turn OFF AFF_TAGGED bit */

    buffer = bufferIn;
    bufBytes = bufBytesIn;
    fib = 0;

    /* initialize the number of entries */
    bufBytes -= sizeof (struct AvailFontsHeader);
    if (bufBytes >= 0)
    {
	afh = (struct AvailFontsHeader *) buffer;
	afh->afh_NumEntries = 0;
	buffer += sizeof (*afh);
    }
    else
	afh = 0;

    /* initialize the name character buffer */
    bufBottomUp = buffer + bufBytes;
    bottomBytes = 0;

    /* cache the fonts on the graphics font list */
    if ((flags & AFF_MEMORY) || (flags & AFF_SCALED))
    {
	Forbid ();
	font = (struct TextFont *) dfl->dfl_GfxBase->TextFonts.lh_Head;
	while (font->tf_Message.mn_Node.ln_Succ != 0)
	{
	    if ((font->tf_Flags &
		 (FPF_DESIGNED | FPF_DISKFONT | FPF_ROMFONT)) == 0)
		thisFontFlag = AFF_SCALED;
	    else
		thisFontFlag = AFF_MEMORY;
	    if (flags & thisFontFlag)
	    {
		ExtendFont (font, 0);
		if (FillBuffer (dfl, &buffer, &bufBottomUp, &bufBytes,
				&bottomBytes, font->tf_Message.mn_Node.ln_Name,
				(struct TTextAttr *) (((ULONG) & font->tf_YSize) - 4),
				((struct TextFontExtension *) font->tf_Extension)->
				tfe_Tags, thisFontFlag,
				flags & AFF_TAGGED, TRUE))
		    (afh->afh_NumEntries)++;
	    }
	    font = (struct TextFont *) font->tf_Message.mn_Node.ln_Succ;
	}
	Permit ();
    }

    if (flags & AFF_DISK)
    {
	/* save # of non-disk entries */
	if (afh)
	    memEntries = afh->afh_NumEntries;	/* only used when afh exists */
	if (fib = (struct FileInfoBlock *) AllocDosObject (DOS_FIB, 0))
	{
	    /* get FONTS: DevProc entry */
	    dvp = (struct DevProc *) GetDeviceProc ("FONTS:", 0);
	    DB (kprintf ("initial dvp 0x%lx\n", dvp));
	    while (dvp)
	    {
		/* get lock on current dvp */
		fib->fib_FileName[0] = 0;	/* create null BSTR */
		do
		{
		    fDirLock = DoPkt3 (dvp->dvp_Port, ACTION_LOCATE_OBJECT,
				       dvp->dvp_Lock, MKBADDR (fib->fib_FileName), SHARED_LOCK);
		    DB (kprintf ("fDirLock 0x%lx\n", fDirLock));
		    if (!fDirLock)
		    {
			if (ErrorReport (IoErr (), REPORT_LOCK, dvp->dvp_Lock,
					 dvp->dvp_Port))
			    goto missingLockContinue;
		    }
		}
		while (!fDirLock);
		/* ensure lock is valid directory while setting up for ExNext */
		if (Examine (fDirLock, fib) && (fib->fib_DirEntryType > 0))
		{
		    /* set CD here for subsequent Open()s */
		    prevCurrDir = CurrentDir (fDirLock);
		    while (ExNext (fDirLock, fib))
		    {
			/* validate name is xxx.font, and that it is a file */
			if ((nameTemp = strrchr (fib->fib_FileName, '.')) &&
			    (strcmp (nameTemp, ".font") == 0) &&
			    (fib->fib_DirEntryType < 0))
			{
			    /* filter any font contents that existed in a prior path */
			    if (afh)
			    {
				for (i = memEntries; i < afh->afh_NumEntries; i++)
				{
				    if (flags & AFF_TAGGED)
				    {
					if (StrEquNC (fib->fib_FileName, ((struct TAvailFonts *)
									  & afh[1])[i].taf_Attr.tta_Name))
					{
					    DB (kprintf ("duplicate disk font name \"%s\"\n",
							 fib->fib_FileName));
					    goto dupFontContinue;
					}
				    }
				    else
				    {
					if (StrEquNC (fib->fib_FileName, ((struct AvailFonts *)
									  & afh[1])[i].af_Attr.ta_Name))
					{
					    DB (kprintf ("duplicate disk font name \"%s\"\n",
							 fib->fib_FileName));
					    goto dupFontContinue;
					}
				    }
				}
			    }
			    /* try to open font contents file */
			    fch.fch_FileID = 0;
			    if (fcFile = Open (fib->fib_FileName, MODE_OLDFILE))
			    {
				if (Read (fcFile, (char *) &fch, sizeof (fch)) == sizeof (fch))
				{
				    DB (kprintf ("FileID 0x%04lx ", fch.fch_FileID));
				    if ((fch.fch_FileID & 0xfff0) == FCH_ID)
				    {
					for (i = 0; i < fch.fch_NumEntries; i++)
					{
					    if (Read (fcFile, (char *) &tfc,
						      sizeof (struct TFontContents))
						== sizeof (struct TFontContents))
					    {
						tfc.tfc_Flags = (tfc.tfc_Flags & ~FPF_ROMFONT) |
						  FPF_DISKFONT;
						if (FillBuffer (dfl, &buffer, &bufBottomUp, &bufBytes,
								&bottomBytes, FilePart (fib->fib_FileName),
								(struct TTextAttr *) (((ULONG) & tfc.tfc_YSize) - 4),
								(struct TagList *) & tfc.tfc_FileName[MAXFONTPATH -
												  (TIS * (tfc.tfc_TagCount + 1))],
								AFF_DISK, flags & AFF_TAGGED,
								fch.fch_FileID >= TFCH_ID))
						    (afh->afh_NumEntries)++;
					    }	/* Read tfc */
					    else
						/* don't try to continue to read font contents */
						break;
					}	/* for fch_NumEntries */
				    }	/* if FCH_ID */
				}	/* Read fch */
				Close (fcFile);
			    }	/* Open .font */
			    DB (kprintf ("flags $%lx\n", flags));
			    if ((fch.fch_FileID == OFCH_ID) && (!(flags & AFF_BITMAP)))
			    {
				strcpy (nameTemp, OTSUFFIX);
				DB (kprintf (".otag name \"%s\"\n", fib->fib_FileName));
				fcFile = Open (fib->fib_FileName, MODE_OLDFILE);
				DB (kprintf ("otag File 0x%lx, error %ld\n", fcFile, IoErr ()));
				strcpy (nameTemp, ".font");
				if (fcFile)
				{
				    DB (kprintf (".otag open\n"));
				    if ((Read (fcFile, (char *) &tagItem, sizeof (tagItem))
					 == sizeof (tagItem)) &&
					(tagItem.ti_Tag == OT_FileIdent) &&
					(ti = AllocVec (tagItem.ti_Data, 0)))
				    {
					DB (kprintf (".otag valid id, size %ld\n", tagItem.ti_Data));
					if (Read (fcFile, (char *) (ti + 1),
						  tagItem.ti_Data - sizeof (tagItem))
					    == tagItem.ti_Data - sizeof (tagItem))
					{
					    *ti = tagItem;	/* initialize first tag */
					    i = GetTagData (OT_AvailSizes, 0, ti);
					    DB (kprintf ("OT_AvailSizes ($%08lx): %ld\n", OT_AvailSizes, i));
					    if (i)
					    {
						tTextAttr.tta_Style = 0;
						if (GetTagData (OT_IsFixed, 0, ti))
						    tTextAttr.tta_Flags = FPF_DISKFONT;
						else
						    tTextAttr.tta_Flags = FPF_DISKFONT | FPF_PROPORTIONAL;
						if (GetTagData (OT_StemWeight, OTS_Medium, ti) >=
						    ((OTS_Medium + OTS_SemiBold) / 2))
						    tTextAttr.tta_Style |= FSF_BOLD;
						if (GetTagData (OT_SlantStyle, OTS_Upright, ti) !=
						    OTS_Upright)
						    tTextAttr.tta_Style |= FSF_ITALIC;
						if (GetTagData (OT_HorizStyle, OTH_Normal, ti) >=
						    ((OTH_Normal + OTH_SemiExpanded) / 2))
						    tTextAttr.tta_Style |= FSF_EXTENDED;
						avail = (UWORD *) (((ULONG) ti) + i);
						i = *avail++;
						DB (kprintf (".otag %ld sizes\n", i));
						while (i-- > 0)
						{
						    tTextAttr.tta_YSize = *avail++;
						    if (FillBuffer (dfl, &buffer, &bufBottomUp,
								    &bufBytes, &bottomBytes,
								    FilePart (fib->fib_FileName), &tTextAttr, 0,
								    AFF_DISK, flags & AFF_TAGGED, 0))
							(afh->afh_NumEntries)++;
						}	/* each avail size */
					    }	/* OT_AvailSizes exists */
					}	/* read remaining ti */
					FreeVec (ti);
				    }	/* read OT_FileIdent and alloc ti */
				    Close (fcFile);
				}	/* Open .otag */
			    }	/* !AFF_BITMAP */
			}	/* if name OK & is file */
		      dupFontContinue:;
		    }		/* while ExNext */
		    CurrentDir (prevCurrDir);
		}		/* if lock OK & Examines & is dir */
		UnLock (fDirLock);	/* null is OK for UnLock */
missingLockContinue:
		dvp = (struct DevProc *) GetDeviceProc ("FONTS:", dvp);
		DB (kprintf ("next dvp 0x%lx\n", dvp));
	    }			/* while dvp */

	    DB (kprintf ("FreeDosObject 0x%lx\n", fib));
	    FreeDosObject (DOS_FIB, fib);
	}			/* if fib */
    }				/* if AFF_DISK */

    DB (kprintf ("exit with bufBytes %ld\n", (LONG)bufBytes));
    if (bufBytes < 0)
	return (-bufBytes);
    else
	return (0);
}
