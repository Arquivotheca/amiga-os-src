/*
**	$Id: bitmap.c,v 37.7 92/02/07 11:50:58 davidj Exp $
**
**	Fountain/bitmap.c -- handle bitmap creation
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

#define	DB(x)	;

extern struct Library *SysBase;
extern struct Library *DiskfontBase;
extern struct Library *DOSBase;
extern struct Library *GfxBase;

extern struct Process *Process;

extern union StaticPool S;

BOOL CreateDiskFont (struct SizeEntry *se)
{
    struct TTextAttr tTextAttr;
    struct TagItem tags[4];
    struct TextFont *tf;
    struct DiskFontHeader *srcDFH, *destDFH;
    BPTR lock;
    ULONG *src, *dest, *tDest, t, hunkSize, actual;
    FILE *f;

    tags[0].ti_Tag = OT_DeviceDPI;
    tags[0].ti_Data = se->dpi;
    tags[1].ti_Tag = OT_DotSize;
    tags[1].ti_Data = se->dotSize;
    tags[2].ti_Tag = OT_SymbolSet;
    tags[2].ti_Data = se->symbolSet;
    tags[3].ti_Tag = TAG_DONE;
    tags[3].ti_Data = 0;
    buildpath (S.t.TempBuffer2, se->ref.fe->fontPath, se->ref.fe->amigaName);
    strcat (S.t.TempBuffer2, SUFFIX_FONT);
    tTextAttr.tta_Name = S.t.TempBuffer2;
    tTextAttr.tta_YSize = se->ySize;
    tTextAttr.tta_Style = FSF_TAGGED;
    tTextAttr.tta_Flags = 0;
    tTextAttr.tta_Tags = tags;
    DB (kprintf ("OpenDiskFont \"%s\" %ld $%lx $%lx ...\n", S.t.TempBuffer2, se->ySize, FSF_TAGGED, 0));
    Process->pr_Result2 = 0;	/* clear IoErr() */
    if ((!(tf = OpenDiskFont ((struct TextAttr *) & tTextAttr))) || (tf->tf_YSize != se->ySize))
    {
	DB (kprintf ("tf=0x%lx : ysize=%ld %ld\n", tf, (LONG) ((tf) ? tf->tf_YSize : 0), (LONG) se->ySize));
	if (Process->pr_Result2 == ERROR_NO_FREE_STORE)
	{
	    DB (kprintf ("  failed: out of memory\n"));
	    ErrRequester (ERROR_POpenFontMem, se->ref.fe->amigaName, se->ySize);
	}
	else
	    ErrRequester (ERROR_POpenFont, se->ref.fe->amigaName, se->ySize);
	if (tf)
	{
	    DB (kprintf ("  CloseFont($%lx)\n", tf));
	    CloseFont (tf);
	}
	return (FALSE);
    }

    /* create memory copy of disk image of font */
    srcDFH = (struct DiskFontHeader *)
      (((ULONG) tf) - ((ULONG) OFFSET (DiskFontHeader, dfh_TF)));
    src = ((ULONG *) srcDFH) - 1;
    DB (kprintf ("  ... $%lx src $%lx srcDFH $%lx\n", tf, src, srcDFH));

    if (src[-1])
    {
	ErrRequester (ERROR_POpenFontHunk, se->ref.fe->amigaName, se->ySize);
	DB (kprintf ("  CloseFont($%lx)\n", tf));
	CloseFont (tf);
	return (FALSE);
    }

    hunkSize = src[-2] - 8;
    dest = (ULONG *) malloc (hunkSize + 80);
    DB (kprintf ("dest $%lx hunkSize %ld\n", dest, hunkSize));
    if (!dest)
    {
	ErrRequester (ENDGAME_NoMemory, hunkSize + 80);
	DB (kprintf ("  CloseFont($%lx)\n", tf));
	CloseFont (tf);
	return (FALSE);
    }

    /* set up header */
    tDest = dest;
    *tDest++ = 0x000003f3;	/* hunk.header */
    *tDest++ = 0;		/* (no resident library) */
    *tDest++ = 1;		/* one hunk */
    *tDest++ = 0;		/* from 0 */
    *tDest++ = 0;		/* to 0 */
    *tDest++ = hunkSize / 4;
    *tDest++ = 0x000003ea;	/* hunk.data */
    *tDest++ = hunkSize / 4;
    /* copy bulk of data */
    memmove (tDest, src, hunkSize);
    /* set up first part of trailer */
    destDFH = (struct DiskFontHeader *) (tDest + 1);
    tDest += hunkSize / 4;	/* skip over font data moved with memmove */
    *tDest++ = 0x000003ec;	/* hunk.reloc32 */
    *tDest++ = 0;		/* # of relocations (filled in later) */
    *tDest++ = 0;		/* to hunk #0 */

    /* massage data and complete trailer */
    destDFH->dfh_DF.ln_Succ = destDFH->dfh_DF.ln_Pred = 0;
    if (srcDFH->dfh_DF.ln_Name)
    {
	destDFH->dfh_DF.ln_Name = (char *) (((ULONG) srcDFH->dfh_DF.ln_Name) -
					    ((ULONG) src));
	*tDest++ = OFFSET (DiskFontHeader, dfh_DF.ln_Name) + 4;
    }

    t = (ULONG) ((struct TextFontExtension *)
		 (srcDFH->dfh_TF.tf_Extension))->tfe_Tags;
    if (t)
    {
	destDFH->dfh_TF.tf_Style |= FSF_TAGGED;
	destDFH->dfh_Segment = t - ((ULONG) src);
	*tDest++ = OFFSET (DiskFontHeader, dfh_Segment) + 4;
    }
    else
	destDFH->dfh_Segment = 0;
    destDFH->dfh_TF.tf_Extension = 0;

    destDFH->dfh_TF.tf_Message.mn_Node.ln_Succ =
      destDFH->dfh_TF.tf_Message.mn_Node.ln_Pred = 0;
    if (srcDFH->dfh_TF.tf_Message.mn_Node.ln_Name)
    {
	destDFH->dfh_TF.tf_Message.mn_Node.ln_Name = (char *)
	  (((ULONG) srcDFH->dfh_TF.tf_Message.mn_Node.ln_Name) -
	   ((ULONG) src));
	*tDest++ = OFFSET (DiskFontHeader, dfh_TF.tf_Message.mn_Node.ln_Name) + 4;
    }
    if (srcDFH->dfh_TF.tf_CharData)
    {
	destDFH->dfh_TF.tf_CharData = (APTR)
	  (((ULONG) srcDFH->dfh_TF.tf_CharData) -
	   ((ULONG) src));
	*tDest++ = OFFSET (DiskFontHeader, dfh_TF.tf_CharData) + 4;
    }
    if (srcDFH->dfh_TF.tf_CharLoc)
    {
	destDFH->dfh_TF.tf_CharLoc = (APTR)
	  (((ULONG) srcDFH->dfh_TF.tf_CharLoc) -
	   ((ULONG) src));
	*tDest++ = OFFSET (DiskFontHeader, dfh_TF.tf_CharLoc) + 4;
    }
    if (srcDFH->dfh_TF.tf_CharSpace)
    {
	destDFH->dfh_TF.tf_CharSpace = (APTR)
	  (((ULONG) srcDFH->dfh_TF.tf_CharSpace) -
	   ((ULONG) src));
	*tDest++ = OFFSET (DiskFontHeader, dfh_TF.tf_CharSpace) + 4;
    }
    if (srcDFH->dfh_TF.tf_CharKern)
    {
	destDFH->dfh_TF.tf_CharKern = (APTR)
	  (((ULONG) srcDFH->dfh_TF.tf_CharKern) -
	   ((ULONG) src));
	*tDest++ = OFFSET (DiskFontHeader, dfh_TF.tf_CharKern) + 4;
    }
    *tDest++ = 0;		/* no more relocations */
    *tDest++ = 0x000003f2;	/* hunk.end */

    t = (((ULONG) tDest) - ((ULONG) dest) - hunkSize - 52) / 4;
    DB (kprintf ("%ld relocations\n", t));
    *((ULONG *) (dest + (hunkSize / 4) + (8 + 1))) = t;	/* # of reloc's */

    DB (kprintf ("  CloseFont($%lx)\n", tf));
    CloseFont (tf);

    DB (kprintf ("dest $%lx destDFH $%lx hunkSize %ld\n", dest, destDFH, hunkSize));

    /* write out disk image */
    buildpath (S.t.TempBuffer2, se->ref.fe->fontPath, se->ref.fe->amigaName);
    if (lock = CreateDir (S.t.TempBuffer2))
    {
	DB (kprintf ("created %s\n", S.t.TempBuffer2));
	UnLock (lock);
    }
    else
	DB (kprintf ("did not create %s\n", S.t.TempBuffer2));

    buildpath (S.t.TempBuffer2, se->ref.fe->fontPath, se->fontFile);
    if (!(f = fopen (S.t.TempBuffer2, "w")))
    {
	ErrRequester (ERROR_PCreateFail, se->ref.fe->amigaName, se->ySize,
		      S.t.TempBuffer2);
	free (dest);
	return (FALSE);
    }
    t = hunkSize + 52 + (t * 4);
    actual = fwrite ((char *) dest, 1, t, f);
    free (dest);
    fclose (f);
    if (actual != t)
    {
	ErrRequester (ERROR_PFailWrite, se->ref.fe->amigaName, se->ySize,
		      S.t.TempBuffer2, t, actual, ferror (f));
	return (FALSE);
    }
    return (TRUE);
}
