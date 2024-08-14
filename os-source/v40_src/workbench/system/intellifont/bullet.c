/*
**	$Id: bullet.c,v 38.4 92/04/07 09:48:44 davidj Exp $
**
**	Fountain/bullet.c -- access bullet index files
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include "fountain.h"

#define	DB(x)	;

extern struct Library *SysBase;
extern struct Library *DiskfontBase;
extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *UtilityBase;

extern struct Process *Process;
extern struct Window *Window;

extern BOOL ValidateOnly;
extern union StaticPool S;

extern ULONG DeviceDPI;
extern ULONG DotSize;
extern UWORD SymbolSet;

struct MinList FontsList =
{
    (struct MinNode *) & FontsList.mlh_Tail, 0,
    (struct MinNode *) & FontsList.mlh_Head
};

char *GDDirNames[MAXDDIRS + 4];

int LastDDirIndex;
int CurrDDirIndex;
struct FontsEntry *CurrDDir;

installChk (buffer, name)
    char *buffer, *name;
{
    BPTR lock;

    DBG2 ("installChk %s %s\n", buffer, name);
    strcpy (buffer, name);
    if (!(lock = Lock (S.t.TempBuffer, SHARED_LOCK)))
	EndGame (RETURN_ERROR, ENDGAME_BadInstallation, S.t.TempBuffer);
    UnLock (lock);
}


int validateRequester (formatIndex, gadgetIndex, args)
    int formatIndex, gadgetIndex;
    char *args;
{
    struct EasyStruct ez;

    ez.es_StructSize = sizeof (struct EasyStruct);
    ez.es_Flags = 0;
    ez.es_Title = LzS[TITLE_Validate];
    ez.es_TextFormat = LzS[formatIndex];
    ez.es_GadgetFormat = LzS[gadgetIndex];

    return (EasyRequestArgs (Window, &ez, 0, &args));
}


void validateEntry (fFlags, ae, fe, oe, te)
    BYTE fFlags;
    struct FontsEntry *ae;
    struct FontEntry *fe;
    struct OTagEntry *oe;
    struct TypeEntry *te;
{
    char *name, *path;
    int result;

    name = 0;
    if (fe)
    {
	name = fe->amigaName;
	path = fe->fontPath;
	if (!oe)
	    oe = fe->oe;
	if (!te)
	    te = fe->te;
    }
    if (oe && !te)
    {
	name = oe->amigaName;
	path = oe->otagPath;
	te = oe->te;
    }
    if (!name)
    {
	name = te->amigaName;
	path = te->typePath;
    }

    switch (fFlags & FFLAG_COMPLETE)
    {
	case FFLAG_FONT | FFLAG_TYPE:
	    /* conflict between Amiga font & partial typeface */
	case FFLAG_OTAG | FFLAG_FONT | FFLAG_TYPE:
	    /* conflict between Amiga font & partial typeface, maybe old FixFonts */
	    if (ValidateOnly)
		result = 0;
	    else
		result = validateRequester (VALIDATE_Conflict,
					    VALIDATE_DelAddIgnore, name, path);
	    break;

	case FFLAG_OFONT | FFLAG_FONT:
	    /* orphan OFC_ID .font, convert to Amiga font */
	    if (ValidateOnly)
		result = 1;
	    else
		result = validateRequester (VALIDATE_Partial,
					    VALIDATE_DeleteIgnore, name, path);
	    break;

	case FFLAG_OTAG | FFLAG_OFONT | FFLAG_FONT:
	    /* orphan .otag and OFC_ID .font, maybe an aspect kludge */
	    result = 0;
	    break;

	case FFLAG_TYPE:
	    /* missing .otag and .font, offer to install typeface */
	case FFLAG_OFONT | FFLAG_FONT | FFLAG_TYPE:
	    /* missing .otag, offer to install typeface */
	case FFLAG_OTAG | FFLAG_TYPE:
	    /* missing .font, offer to install typeface */
	    if (ValidateOnly)
		result = 2;
	    else
		result = validateRequester (VALIDATE_Install,
					    VALIDATE_DelAddIgnore, name, path);
	    break;

	case FFLAG_OTAG:
	    /* orphan .otag, delete */
	case FFLAG_OTAG | FFLAG_FONT:
	    /* orphan .otag, delete */
	    if (ValidateOnly)
		result = 1;
	    else
		result = validateRequester (VALIDATE_Partial,
					    VALIDATE_DeleteIgnore, name, path);
	    break;

	default:
	    /* AOK */
	    result = 3;
    }
    DBG1 ("result %ld\n", result);

    switch (result)
    {
	case 0:
	    /* ignore inconsistancies */
	    if (fe && (fe->fFlags & FFLAG_OFONT))
	    {
		Remove ((struct Node *) fe);
		free (fe);
	    }
	    if (oe)
	    {
		Remove ((struct Node *) oe);
		free (oe);
	    }
	    if (te)
	    {
		Remove ((struct Node *) te);
		free (te);
	    }
	    break;
	case 1:
	    /* delete inconsistancies */
	    if (fe)
	    {
		if (fe->fFlags & FFLAG_OFONT)
		    fe->fFlags = FFLAG_DELETE;
		else
		    fe->fFlags = FFLAG_CREATE;
	    }
	    if (oe)
		oe->fFlags = FFLAG_DELETE;
	    if (te)
		te->fFlags = FFLAG_DELETE;
	    break;
	case 2:
	    /* install */
	    CreateInstall (ae, &fe, &oe, te);
	    break;
	case 3:
	    /* do nothing */
	    ;
    }

    if (fe)
	fe->fFlags |= FFLAG_TAG;
    if (oe)
	oe->fFlags |= FFLAG_TAG;
    if (te)
	te->fFlags |= FFLAG_TAG;
}


struct FontsEntry *AddFontsEntry (BPTR lock, BOOL fontsFlag)
{
    struct MyDir *md;
    struct FontsEntry *ae;
    struct TypeEntry *te, *te2;
    struct FontEntry *fe, *fe2;
    struct OTagEntry *oe, *oe2;
    struct SizeEntry *se;
    struct FontContentsHeader fch;
    struct TFontContents tfc;
    struct TagItem tagItem, *ti;
    FILE *f;
    char *s, *t;
    short n, *ySizes;
    int size, actual;

    if (!(ae = malloc (sizeof (struct FontsEntry))))
	EndGame (RETURN_ERROR, ENDGAME_NoMemory, sizeof (struct FontsEntry));

    if (!NameFromLock (lock, ae->assignPath, PATHNAMELEN))
    {
	DBG1 ("NameFromLock $%lx failed\n", lock);
	free (ae);
	return (0);
    }

    DBG1 ("AddFontsEntry %s\n", ae->assignPath);

    if (fontsFlag && (LastDDirIndex < (MAXDDIRS - 1)))
    {
	LastDDirIndex++;
	sprintf (ae->indexName, LzS[GADGET_RDirNext], LastDDirIndex + 1);
	GDDirNames[LastDDirIndex] = ae->indexName;
    }

    AddTail ((struct List *) & FontsList, (struct Node *) ae);
    NewList ((struct List *) & ae->otList);
    NewList ((struct List *) & ae->fcList);
    NewList ((struct List *) & ae->tfList);

    /* find all .otag, and .font files */
    if (!(md = MyDirStart (ae->assignPath)))
    {
	DBG1 ("MyDirStart %s failed\n", ae->assignPath);
	free (ae);
	return (0);
    }

    DBG1 ("while MyDirNext 0x%lx\n", md);
    while (MyDirNext (md))
    {
	if (md->md_FIB.fib_DirEntryType >= 0)
	{
	    /* this is a directory, see if it is a _Bullet_Outline directory */
	    if (Stricmp (md->md_File, DIR_OUTLINES))
		continue;
	    /* search here for all .type files */
	    LIBInfo (md->md_Path, &ae->tfList, 0);
	}

	/* this is a file: see if it has an interesting suffix */
	s = strrchr (md->md_File, '.');
	if (!s)
	    continue;

	if ((!strcmp (s, SUFFIX_FONT)) && (f = fopen (md->md_File, "r")))
	{
	    /* this is a potential .font file */
	    DBG1("fopen [%s]\n", md->md_File);
	    if ((fread ((char *) &fch, sizeof (fch), 1, f) == 1) &&
		((fch.fch_FileID & 0xfff0) == FCH_ID))
	    {
		/* it *is* a .font file */
		if (!(fe = malloc (sizeof (struct FontEntry))))
		    EndGame (RETURN_ERROR, ENDGAME_NoMemory,
			     sizeof (struct FontEntry));
		NewList ((struct List *) & fe->sizes);
		fe->oe = 0;
		fe->te = 0;
		if (fch.fch_FileID == OFCH_ID)
		    fe->fFlags = FFLAG_FONT | FFLAG_OFONT;
		else
		    fe->fFlags = FFLAG_FONT;
		*s = '\0';
		strcpy (fe->amigaName, md->md_File);
		*s = '.';
		strcpy (fe->fontPath, ae->assignPath);

		/* read in any sizes */
		for (n = 0; n < fch.fch_NumEntries; n++)
		{
		    if ((fread ((char *) &tfc, sizeof (tfc), 1, f) != 1) ||
			(strlen (tfc.tfc_FileName) > (FONTFILELEN - 1)))
			EndGame (RETURN_ERROR, ENDGAME_BadFontContents,
				 md->md_Path);
		    if (!(se = malloc (sizeof (struct SizeEntry))))
			EndGame (RETURN_ERROR, ENDGAME_NoMemory,
				 sizeof (struct SizeEntry));
		    se->ref.fe = fe;
		    se->ySize = tfc.tfc_YSize;
		    if ((fch.fch_FileID >= TFCH_ID) && tfc.tfc_TagCount)
		    {
			ti = (struct TagItem *) & tfc.tfc_FileName[MAXFONTPATH -
								   (tfc.tfc_TagCount * sizeof (struct TagItem))];
			DBG2 ("tfc_TagCount %ld, ti $%lx\n", tfc.tfc_TagCount, ti);
			se->dpi = GetTagData (OT_DeviceDPI, DeviceDPI, ti);
			se->dotSize = GetTagData (OT_DotSize, DotSize, ti);
			se->symbolSet = GetTagData (OT_SymbolSet, SymbolSet, ti);
		    }
		    else
		    {
			se->dpi = DeviceDPI;
			se->dotSize = DotSize;
			se->symbolSet = SymbolSet;
		    }
		    se->fFlags = 0;
		    strcpy (se->fontFile, tfc.tfc_FileName);
		    AddTail ((struct List *) & fe->sizes, (struct Node *) se);
		}
		AddTail ((struct List *) & ae->fcList, (struct Node *) fe);
	    }
	    DBG1 ("fclose 0x%lx\n", f);
	    fclose (f);
	}
	else if ((!strcmp (s, SUFFIX_OTAG)) &&
		 (size = MyFileSize (md->md_File)) &&
		 (f = fopen (md->md_File, "r")))
	{
	    /* this is a potential .otag file */
	    if ((fread ((char *) &tagItem, sizeof (tagItem), 1, f) == 1) &&
		(tagItem.ti_Tag == OT_FileIdent) &&
		(tagItem.ti_Data == size))
	    {
		/* it *is* an .otag file */
		DB (kprintf(".otag : %s (tags=%ld)\n", md->md_File, (LONG)(size)));
		if (!(oe = malloc (sizeof (struct OTagEntry))))
		    EndGame (RETURN_ERROR, ENDGAME_NoMemory,
			     sizeof (struct OTagEntry));
		if (!(ti = (struct TagItem *) malloc (size)))
		    EndGame (RETURN_ERROR, ENDGAME_NoMemory, size);
		if ((actual = fread ((char *) (ti + 1), 1,
				     size - sizeof (struct TagItem), f))
		    != (size - sizeof (struct TagItem)))
		    EndGame (RETURN_ERROR, ENDGAME_FileRead, md->md_Path,
			     size - sizeof (struct TagItem), actual, ferror (f));

		*ti = tagItem;
		NewList ((struct List *) & oe->sizes);
		oe->symbolSet = GetTagData (OT_SymbolSet, SymbolSet, ti);

		/* Get the sizes */
		if (ySizes = (UWORD *) GetTagPtr (OT_AvailSizes, ti))
		{
		    DB (kprintf ("%ld sizes available\n", (LONG)ySizes));
		    for (n = *ySizes++; n > 0; n--)
		    {
			if (!(se = malloc (sizeof (struct SizeEntry))))
			    EndGame (RETURN_ERROR, ENDGAME_NoMemory,
				     sizeof (struct SizeEntry));
			se->ref.oe = oe;
			se->ySize = *ySizes++;
			se->dpi = DeviceDPI;
			se->dotSize = DotSize;
			se->symbolSet = oe->symbolSet;
			se->fFlags = 0;
			AddTail ((struct List *) &oe->sizes, (struct Node *) se);
		    }
		}
		else
		{
		    DB (kprintf("No sizes available\n"));
		}

		oe->bDef = oe->iDef = oe->biDef = 0;
		oe->bRef = oe->iRef = oe->biRef = 0;
		oe->te = 0;
		oe->fFlags = FFLAG_OTAG;
		*s = '\0';
		strcpy (oe->amigaName, md->md_File);
		*s = '.';
		oe->bRefName[0] = oe->iRefName[0] = oe->biRefName[0] = '\0';
		if (t = (char *) GetTagPtr (OT_Engine, ti))
		{
		    DB (kprintf ("OT_Engine=%s\n", t));
		}
		if (t = (char *) GetTagPtr (OT_BName, ti))
		{
		    DB (kprintf ("OT_BName=%s\n", t));
		    strcpy (oe->bRefName, t);
		}
		if (t = (char *) GetTagPtr (OT_IName, ti))
		{
		    DB (kprintf ("OT_IName=%s\n", t));
		    strcpy (oe->iRefName, t);
		}
		if (t = (char *) GetTagPtr (OT_BIName, ti))
		{
		    DB (kprintf ("OT_BIName=%s\n", t));
		    strcpy (oe->biRefName, t);
		}
		strcpy (oe->otagPath, ae->assignPath);
		free (ti);
		AddTail ((struct List *) & ae->otList, (struct Node *) oe);
	    }
	    else
		DBG2 (".otag fail $%08lx, %ld\n", tagItem.ti_Tag,
		      tagItem.ti_Data);
	    fclose (f);
	}
    }
    DBG1 ("calling MyDirEnd 0x%lx\n", md);
    MyDirEnd (md);

#ifdef  DEBUG
    DBG ("ae->fcList...\n");
    for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
	 fe->node.mln_Succ;
	 fe = (struct FontEntry *) fe->node.mln_Succ)
    {
	DBG3 ("name \"%s\" path \"%s\" fFlags $%02lx\n", fe->amigaName,
	      fe->fontPath, fe->fFlags);
	for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
	     se->node.mln_Succ;
	     se = (struct SizeEntry *) se->node.mln_Succ)
	{
	    DBG6 ("  Size %ld DPI %ld %ld DotSize %ld %ld SymbolSet %.2s\n",
		  se->ySize, se->dpi >> 16, se->dpi & 0xffff, se->dotSize >> 16,
		  se->dotSize & 0xffff, &se->symbolSet);
	}
    }
    DBG ("ae->otList...\n");
    for (oe = (struct OTagEntry *) ae->otList.mlh_Head;
	 oe->node.mln_Succ;
	 oe = (struct OTagEntry *) oe->node.mln_Succ)
    {
	DBG3 ("name \"%s\" path \"%s\" fFlags $%02lx\n", oe->amigaName,
	      oe->otagPath, oe->fFlags);
	DBG1 ("  otag SymbolSet %.2s\n", &oe->symbolSet);
	for (se = (struct SizeEntry *) oe->sizes.mlh_Head;
	     se->node.mln_Succ;
	     se = (struct SizeEntry *) se->node.mln_Succ)
	{
	    DBG6 ("  Size %ld DPI %ld %ld DotSize %ld %ld SymbolSet %.2s\n",
		  se->ySize, se->dpi >> 16, se->dpi & 0xffff, se->dotSize >> 16,
		  se->dotSize & 0xffff, &se->symbolSet);
	}
	DBG3 ("  b \"%s\" i \"%s\" bi \"%s\"\n", oe->bRefName, oe->iRefName,
	      oe->biRefName);
    }
    DBG ("ae->tfList...\n");
    for (te = (struct TypeEntry *) ae->tfList.mlh_Head;
	 te->node.mln_Succ;
	 te = (struct TypeEntry *) te->node.mln_Succ)
    {
	DBG4 ("font id %ld name \"%s\" path \"%s\" fFlags $%02lx\n",
	      te->typefaceID, te->amigaName, te->typePath, te->fFlags);
	DBG5 ("  fhOff $%lx fhCnt $%lx bucket %ld family \"%s\" spaceReq %ld\n",
	      te->faceheaderOffset, te->faceheaderCount, te->bucketBits,
	      te->family, te->spaceReq);
	DBG3 ("  ySize %ld / %ld space %ld\n",
	      te->ySizeNumerator, te->ySizeFactor, te->spaceWidth);
	DBG5 ("  serif %ld weight %ld slant %ld horiz %ld fixed %ld\n",
	      te->serifFlag, te->stemWeight, te->slantStyle, te->horizStyle,
	      te->isFixed);
	DBG1 ("  source \"%s\".\n", te->sourceFile);
    }
#endif

    /* validate the fonts */
    /* .otag nodes are at the center of the links */
    for (oe = (struct OTagEntry *) ae->otList.mlh_Head;
	 oe->node.mln_Succ;
	 oe = (struct OTagEntry *) oe->node.mln_Succ)
    {
	/* look for associated .otag family entries */
	for (oe2 = (struct OTagEntry *) ae->otList.mlh_Head;
	     oe2->node.mln_Succ;
	     oe2 = (struct OTagEntry *) oe2->node.mln_Succ)
	{
	    if (FontMatch (oe->otagPath, oe->bRefName,
			   oe2->otagPath, oe2->amigaName))
	    {
#ifdef  DEBUG
		if (oe->bRef)
		    DBG1 ("*** oe->bRef already $%lx\n", oe->bRef);
		if (oe2->bDef)
		    DBG1 ("*** oe2->bDef already $%lx\n", oe2->bDef);
#endif
		oe->bRef = oe2;
		oe2->bDef = oe;
	    }
	    if (FontMatch (oe->otagPath, oe->iRefName,
			   oe2->otagPath, oe2->amigaName))
	    {
#ifdef  DEBUG
		if (oe->iRef)
		    DBG1 ("*** oe->iRef already $%lx\n", oe->iRef);
		if (oe2->iDef)
		    DBG1 ("*** oe2->iDef already $%lx\n", oe2->iDef);
#endif
		oe->iRef = oe2;
		oe2->iDef = oe;
	    }
	    if (FontMatch (oe->otagPath, oe->biRefName,
			   oe2->otagPath, oe2->amigaName))
	    {
#ifdef  DEBUG
		if (oe->biRef)
		    DBG1 ("*** oe->biRef already $%lx\n", oe->biRef);
		if (oe2->biDef)
		    DBG1 ("*** oe2->biDef already $%lx\n",
			  oe2->biDef);
#endif
		oe->biRef = oe2;
		oe2->biDef = oe;
	    }
	}
	/* look for associated .font entry */
	for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
	     fe->node.mln_Succ;
	     fe = (struct FontEntry *) fe->node.mln_Succ)
	{
	    if (FontMatch (fe->fontPath, fe->amigaName,
			   oe->otagPath, oe->amigaName))
	    {
		fe->oe = oe;
		fe->fFlags |= oe->fFlags;
		oe->fFlags |= fe->fFlags;
		break;
	    }
	}
	if (!fe->node.mln_Succ)
	    fe = 0;
	/* look for associated .type entry */
	for (te = (struct TypeEntry *) ae->tfList.mlh_Head;
	     te->node.mln_Succ;
	     te = (struct TypeEntry *) te->node.mln_Succ)
	{
	    if (FontMatch (te->typePath, te->amigaName,
			   oe->otagPath, oe->amigaName))
	    {
		te->fFlags |= oe->fFlags;
		oe->fFlags |= te->fFlags;
		if (fe)
		{
		    fe->fFlags |= te->fFlags;
		    fe->te = te;
		}
		oe->te = te;
		break;
	    }
	}
    }
    /* pick up any .font/.type combos missing .otags */
    for (te = (struct TypeEntry *) ae->tfList.mlh_Head;
	 te->node.mln_Succ;
	 te = (struct TypeEntry *) te->node.mln_Succ)
    {
	for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
	     fe->node.mln_Succ;
	     fe = (struct FontEntry *) fe->node.mln_Succ)
	{
	    if (FontMatch (te->typePath, te->amigaName,
			   fe->fontPath, fe->amigaName))
	    {
		fe->fFlags |= te->fFlags;
		te->fFlags |= fe->fFlags;
		fe->te = te;
		break;
	    }
	}
    }


    /* validate relationships between .otag, .font and /.type entries */
    DBG ("ae->fcList...\n");
    for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
	 fe2 = (struct FontEntry *) fe->node.mln_Succ;
	 fe = fe2)
    {
	DBG3 ("name \"%s\" path \"%s\" fFlags $%02lx\n", fe->amigaName,
	      fe->fontPath, fe->fFlags);
	if (!(fe->fFlags & FFLAG_TAG))
	{
	    validateEntry (fe->fFlags, ae, fe, 0, 0);
	}
    }
    DBG ("ae->otList...\n");
    for (oe = (struct OTagEntry *) ae->otList.mlh_Head;
	 oe2 = (struct OTagEntry *) oe->node.mln_Succ;
	 oe = oe2)
    {
	DBG4 ("$%lx name \"%s\" path \"%s\" fFlags $%02lx\n", oe, oe->amigaName,
	      oe->otagPath, oe->fFlags);
	DBG3 ("  bDef $%lx iDef $%lx biDef $%lx\n",
	      oe->bDef, oe->iDef, oe->biDef);
	DBG3 ("  bRef $%lx iRef $%lx biRef $%lx\n",
	      oe->bRef, oe->iRef, oe->biRef);
	if (!(oe->fFlags & FFLAG_TAG))
	{
	    validateEntry (oe->fFlags, ae, 0, oe, 0);
	}
    }
    DBG ("ae->tfList...\n");
    for (te = (struct TypeEntry *) ae->tfList.mlh_Head;
	 te2 = (struct TypeEntry *) te->node.mln_Succ;
	 te = te2)
    {
	DBG4 ("font id %ld name \"%s\" path \"%s\" fFlags $%02lx\n",
	      te->typefaceID, te->amigaName, te->typePath, te->fFlags);
	if (!(te->fFlags & FFLAG_TAG))
	{
	    validateEntry (te->fFlags, ae, 0, 0, te);
	}
    }
    FindOMatches (ae);
    Update (ae, 0);

    return (ae);
}


OpenBullet ()
{
    APTR oldPRWindowPtr;
    struct DosList *lock, *fonts;
    struct AssignList *alist;
    char *s;
    short i;

    DBG ("OpenBullet\n");
    for (i = 0; i <= MAXDDIRS + 2; i++)
	GDDirNames[i] = "";
    GDDirNames[MAXDDIRS] = LzS[GADGET_NotFontsPath];
    GDDirNames[MAXDDIRS + 1] = LzS[GADGET_NotValidPath];
    GDDirNames[MAXDDIRS + 3] = 0;

    strcpy (S.t.TempBuffer, VOL_FONTS ":" DIR_BULLET "/");
    s = S.t.TempBuffer + strlen (S.t.TempBuffer);

    /* check existance of invariant files */
    installChk (s, FILE_IFFNT);
    installChk (s, FILE_PLUGIN);
    if (DiskfontBase->lib_Version == 37)
	installChk (s, FILE_IFSS);
    else
	/* must be 38 or greater or OpenLibrary would have failed */
	installChk (s, FILE_IFUC);

    oldPRWindowPtr = Process->pr_WindowPtr;
    Process->pr_WindowPtr = (APTR) - 1;

    lock = LockDosList (LDF_ALL | LDF_READ);
    fonts = FindDosEntry (lock, "FONTS", LDF_ALL);
    LastDDirIndex = -1;
    if (fonts)
    {
	AddFontsEntry (fonts->dol_Lock, TRUE);
	if (fonts->dol_Type == DLT_DIRECTORY)
	{
	    for (alist = fonts->dol_misc.dol_assign.dol_List; alist;
		 alist = alist->al_Next)
		AddFontsEntry (alist->al_Lock, TRUE);
	}
    }
    UnLockDosList (LDF_ALL | LDF_READ);
    Process->pr_WindowPtr = oldPRWindowPtr;

    if (LastDDirIndex < 0)
    {
	EndGame (RETURN_ERROR, ENDGAME_NoFontsAssign);
    }

    CurrDDirIndex = 0;
    CurrDDir = (struct FontsEntry *) FontsList.mlh_Head;

    if (ValidateOnly)
	EndGame (RETURN_OK, 0);
    DBG ("OpenBullet done.\n");
}
