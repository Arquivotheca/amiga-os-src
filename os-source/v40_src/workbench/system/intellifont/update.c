/*
**	$Id: update.c,v 38.3 92/02/07 11:59:46 davidj Exp $
**
**	Fountain/update.c -- update files from structures
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

#define	DB(x)	;
#define	DC(x)	;

extern struct Library *SysBase;
extern struct Library *DiskfontBase;
extern struct Library *DOSBase;
extern struct Library *GadToolsBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;

extern union StaticPool S;
extern int CurrDDirIndex;
extern struct FontsEntry *CurrDDir;

#if sizeof(struct InfoData) >= sizeof(struct FileInfoBlock)
#define  MYDOSBUFFERSIZE	sizeof(struct InfoData)/2+1
#else
#define  MYDOSBUFFERSIZE	sizeof(struct FileInfoBlock)/2+1
#endif

FindOMatches (struct FontsEntry *ae)
{
    struct OTagEntry *oe, *oe2;
    UBYTE *family, weight, weight2, posture, posture2;

    DC (kprintf ("FindOMatches\n"));
    for (oe = (struct OTagEntry *) ae->otList.mlh_Head;
	 oe->node.mln_Succ;
	 oe = (struct OTagEntry *) oe->node.mln_Succ)
    {
	if (oe->bRef && oe->iRef && oe->biRef)
	{
	    DC (kprintf (" oe $%lx %s already full of matches $%lx $%lx $%lx\n", oe, oe->amigaName, oe->bRef, oe->iRef, oe->biRef));
	    continue;
	}
	if ((oe->fFlags & (FFLAG_COMPLETE | FFLAG_DELETE)) != FFLAG_COMPLETE)
	{
	    DC (kprintf (" oe $%lx %s slated for deletion\n", oe, oe->amigaName));
	    continue;
	}
	family = oe->te->family;
	weight = oe->te->stemWeight;
	posture = oe->te->slantStyle;
	DC (kprintf (" oe $%lx %s family %s weight %ld posture %ld\n", oe, oe->amigaName, oe->te->family, oe->te->stemWeight, oe->te->slantStyle));
	/* look for associated .otag family entries */
	for (oe2 = (struct OTagEntry *) ae->otList.mlh_Head;
	     oe2->node.mln_Succ;
	     oe2 = (struct OTagEntry *) oe2->node.mln_Succ)
	{
	    if (oe2->bRef && oe2->iRef && oe2->biRef)
	    {
		DC (kprintf (" oe2 $%lx %s already full of matches $%lx $%lx $%lx\n", oe2, oe2->amigaName, oe2->bRef, oe2->iRef, oe2->biRef));
		continue;
	    }
	    if ((oe2->fFlags & (FFLAG_COMPLETE | FFLAG_DELETE)) != FFLAG_COMPLETE)
	    {
		DC (kprintf (" oe2 $%lx %s slated for deletion\n", oe2, oe2->amigaName));
		continue;
	    }
	    DC (kprintf ("  oe2 $%lx %s family %s weight %ld posture %ld\n", oe2, oe2->amigaName, oe2->te->family, oe2->te->stemWeight, oe2->te->slantStyle));
	    if (StrEquNC (oe2->te->family, family))
	    {
		DC (kprintf ("   same family\n"));
		weight2 = oe2->te->stemWeight;
		posture2 = oe2->te->slantStyle;
		if (weight2 >= weight)
		{
		    DC (kprintf ("   not skinnier\n"));
		    /* not skinnier... */
		    if ((posture == OTS_Upright) && (posture2 == OTS_Italic))
		    {
			/* italic relationship */
			if (weight2 > weight)
			{
			    if ((!oe2->biDef) && (!oe->biRef))
			    {
				DC (kprintf ("  set BI relationship\n"));
				oe2->biDef = oe;
				oe->biRef = oe2;
				oe->fFlags |= FFLAG_CREATE;
			    }
			}
			else
			{
			    if ((!oe2->iDef) && (!oe->iRef))
			    {
				DC (kprintf ("  set I relationship\n"));
				oe2->iDef = oe;
				oe->iRef = oe2;
				oe->fFlags |= FFLAG_CREATE;
			    }
			}
		    }
		    else if ((posture == posture2) && (weight != weight2))
		    {
			if ((!oe->bRef) && (!oe2->bDef))
			{
			    DC (kprintf ("  set B relationship\n"));
			    oe->bRef = oe2;
			    oe2->bDef = oe;
			    oe->fFlags |= FFLAG_CREATE;
			}
		    }
		}
	    }
	}
    }
}


wTag (ptr, t, d)
    ULONG **ptr;
    ULONG t, d;
{
    ULONG *p;

    p = *ptr;
    *p++ = t;
    *p++ = d;
    *ptr = p;
}

wString (ptr, s)
    UBYTE **ptr;
    UBYTE *s;
{
    UBYTE *p;

    p = *ptr;
    while (*p++ = *s++) ;
    *ptr = p;
}

Update (struct FontsEntry *ae, short items)
{
    char workFlag, fixFonts, fixOTag, prevCreateBMOK, cancelCreateBM;
    struct FontContentsHeader *fch;
    struct FontEntry *fe, *feNext;
    struct OTagEntry *oe, *oeNext;
    struct SizeEntry *se, *seNext;
    struct TypeEntry *te, *teNext;
    int length, end1, end2, run;
    BPTR lock;
    FILE *f;
    union
    {
	UBYTE *b;
	UWORD *w;
	ULONG *l;
    } ptr;
    UWORD ySizes;
    short item;
    char *s;

    /* go thru all .font and .otag entries in ae and make current */
    DC (kprintf ("Update $%lx\n", ae));

    item = 0;
    if (items > 0)
	WaitPointer ((((item * 8) + 1) / (items + 1)) + 1);

    workFlag = TRUE;
    while (workFlag)
    {
	workFlag = FALSE;
	for (oe = (struct OTagEntry *) ae->otList.mlh_Head;
	     oeNext = (struct OTagEntry *) oe->node.mln_Succ;
	     oe = oeNext)
	{
	    fixOTag = (oe->fFlags & FFLAG_CREATE);
	    DC (kprintf ("update oe $%lx %ld\n", oe, fixOTag));
	    ySizes = 0;
	    for (se = (struct SizeEntry *) oe->sizes.mlh_Head;
		 seNext = (struct SizeEntry *) se->node.mln_Succ;
		 se = seNext)
	    {
		if (se->fFlags & FFLAG_DIRTY)
		{
		    DC (kprintf ("  y %ld DIRTY\n", se->ySize));
		    fixOTag = TRUE;
		}
		if (se->fFlags & FFLAG_DELETE)
		{
		    DC (kprintf ("  y %ld DELETE\n", se->ySize));
		    Remove ((struct Node *) se);
		    free (se);
		}
		else
		    ySizes++;
		se->fFlags &= ~FFLAG_DIRTY;
	    }
	    if (oe->fFlags & FFLAG_DELETE)
	    {
		buildpath (S.t.TempBuffer, oe->otagPath, oe->amigaName);
		strcat (S.t.TempBuffer, SUFFIX_OTAG);
		DC (kprintf ("  delete %s\n", S.t.TempBuffer));
		DeleteFile (S.t.TempBuffer);
		Remove ((struct Node *) oe);
		if (oe->bRef)
		    oe->bRef->bDef = 0;
		if (oe->iRef)
		    oe->iRef->iDef = 0;
		if (oe->biRef)
		    oe->biRef->biDef = 0;
		if (oe->bDef)
		{
		    oe->bDef->bRef = 0;
		    oe->bDef->fFlags |= FFLAG_CREATE;
		    workFlag = TRUE;
		}
		if (oe->iDef)
		{
		    oe->iDef->iRef = 0;
		    oe->iDef->fFlags |= FFLAG_CREATE;
		    workFlag = TRUE;
		}
		if (oe->biDef)
		{
		    oe->biDef->biRef = 0;
		    oe->biDef->fFlags |= FFLAG_CREATE;
		    workFlag = TRUE;
		}
		FreeList (&oe->sizes);
		if (oe->te)
		    oe->te->fFlags &= ~FFLAG_OTAG;
		free (oe);
	    }
	    else
	    {
		if (fixOTag && (te = oe->te))
		{
		    DB (kprintf ("fixOTags %s\n", oe->amigaName));

		    /* count to end of tag data and subsequent data */
		    end1 = (18 * 8);
		    end2 = sizeof (OTE_Bullet) +
			   sizeof (SUFFIX_TYPE) + 1 +
			   ((ySizes + 1) * 2) +
			   strlen (te->family) +
			   strlen (oe->amigaName);

		    if (oe->bRef)
		    {
			end1 += 8;
			end2 += strlen (oe->bRef->amigaName) + 1;
			DB (kprintf ("  bRef %s\n", oe->bRef->amigaName));
		    }
		    if (oe->iRef)
		    {
			end1 += 8;
			end2 += strlen (oe->iRef->amigaName) + 1;
			DB (kprintf ("  iRef %s\n", oe->iRef->amigaName));
		    }
		    if (oe->biRef)
		    {
			end1 += 8;
			end2 += strlen (oe->biRef->amigaName) + 1;
			DB (kprintf ("  biRef %s\n", oe->biRef->amigaName));
		    }
		    if (oe->symbolSet)
		    {
			end1 += 8;
			DB (kprintf ("  symbolSet\n"));
		    }

		    end2 += end1;
		    DB (kprintf ("file size=%ld\n", (LONG)end2));

		    ptr.b = S.t.TempBuffer;
		    run = end1 + ((ySizes + 1) * 2);
		    wTag (&ptr.l, OT_FileIdent, end2);
		    wTag (&ptr.l, OT_Engine, run);
		    run += sizeof (OTE_Bullet);
		    wTag (&ptr.l, OT_Family, run);
		    run += strlen (te->family) + 1;

		    if (oe->bRef)
		    {
			wTag (&ptr.l, OT_BName, run);
			run += strlen (oe->bRef->amigaName) + 1;
		    }
		    if (oe->iRef)
		    {
			wTag (&ptr.l, OT_IName, run);
			run += strlen (oe->iRef->amigaName) + 1;
		    }
		    if (oe->biRef)
		    {
			wTag (&ptr.l, OT_BIName, run);
			run += strlen (oe->biRef->amigaName) + 1;
		    }
		    if (oe->symbolSet)
		    {
			wTag (&ptr.l, OT_SymbolSet, oe->symbolSet);
		    }

		    wTag (&ptr.l, OT_YSizeFactor, (te->ySizeNumerator << 16) | te->ySizeFactor);
		    wTag (&ptr.l, OT_SpaceWidth, te->spaceWidth);
		    wTag (&ptr.l, OT_IsFixed, te->isFixed);
		    wTag (&ptr.l, OT_SerifFlag, te->serifFlag);
		    wTag (&ptr.l, OT_StemWeight, te->stemWeight);
		    wTag (&ptr.l, OT_SlantStyle, te->slantStyle);
		    wTag (&ptr.l, OT_HorizStyle, te->horizStyle);
		    wTag (&ptr.l, OT_AvailSizes, end1);
		    wTag (&ptr.l, OT_SpecCount, 5);
		    wTag (&ptr.l, OT_Spec + 1, oe->te->typefaceID);
		    wTag (&ptr.l, OT_Spec + 2 + OT_Indirect, run);
/*		    run += strlen(oe->amigaName)+sizeof(SUFFIX_OTAG); */
		    wTag (&ptr.l, OT_Spec + 3, oe->te->faceheaderOffset);
		    wTag (&ptr.l, OT_Spec + 4, oe->te->faceheaderCount);
		    wTag (&ptr.l, OT_Spec + 5, oe->te->bucketBits);
		    wTag (&ptr.l, TAG_DONE, 0);

		    DB (kprintf ("%ld YSizes\n", ySizes));
		    *ptr.w++ = ySizes;
		    for (se = (struct SizeEntry *) oe->sizes.mlh_Head;
			 seNext = (struct SizeEntry *) se->node.mln_Succ;
			 se = seNext)
		    {
			DB (kprintf ("  %ld\n", se->ySize));
			*ptr.w++ = se->ySize;
		    }

		    DB (kprintf("beginning of strings=0x%lx\n", &ptr.b));
		    wString (&ptr.b, OTE_Bullet);
		    wString (&ptr.b, te->family);
		    if (oe->bRef)
			wString (&ptr.b, oe->bRef->amigaName);
		    if (oe->iRef)
			wString (&ptr.b, oe->iRef->amigaName);
		    if (oe->biRef)
			wString (&ptr.b, oe->biRef->amigaName);

		    strcpy (S.t.TempBuffer2, oe->amigaName);
		    strcat (S.t.TempBuffer2, SUFFIX_TYPE);
		    wString (&ptr.b, S.t.TempBuffer2);

		    buildpath (S.t.TempBuffer2, oe->otagPath, oe->amigaName);
		    strcat (S.t.TempBuffer2, SUFFIX_OTAG);
		    if (f = fopen (S.t.TempBuffer2, "w"))
		    {
			fwrite (S.t.TempBuffer, 1, end2, f);
			fclose (f);
		    }
		}
		oe->fFlags &= ~FFLAG_DIRTY;
	    }
	}
	/* delete all bitmaps, then create all bitmaps */
	for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
	     feNext = (struct FontEntry *) fe->node.mln_Succ;
	     fe = feNext)
	{
	    DC (kprintf ("delete fe? $%lx\n", fe));
	    for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
		 seNext = (struct SizeEntry *) se->node.mln_Succ;
		 se = seNext)
	    {
		DC (kprintf ("%s %ld\n", fe->amigaName, se->ySize));
		if ((fe->fFlags & FFLAG_DELETE) || (se->fFlags & FFLAG_DELETE))
		{
		    se->fFlags &= ~FFLAG_DELETE;
		    DC (kprintf ("  delete \"%s\"\n", se->fontFile));
		    buildpath (S.t.TempBuffer, fe->fontPath, se->fontFile);
		    if (DeleteFile (S.t.TempBuffer))
		    {
			Remove ((struct Node *) se);
			free (se);
			/* try to delete font directory */
			s = PathPart (S.t.TempBuffer);
			*s = '\0';
			DeleteFile (S.t.TempBuffer);
		    }
		    else
		    {
			DC (kprintf ("DeleteFile %s failed\n", S.t.TempBuffer));
		    }
		    fe->fFlags |= FFLAG_CREATE;	/* mark for FixFonts */
		}
	    }
	    if (fe->fFlags & FFLAG_DELETE)
	    {
		DC (kprintf ("delete .font\n"));
		buildpath (S.t.TempBuffer, fe->fontPath, fe->amigaName);
		DC (kprintf ("  delete %s\n", S.t.TempBuffer));
		DeleteFile (S.t.TempBuffer);
		strcat (S.t.TempBuffer, SUFFIX_FONT);
		DC (kprintf ("  delete %s\n", S.t.TempBuffer));
		DeleteFile (S.t.TempBuffer);
		FreeList (&fe->sizes);
		Remove ((struct Node *) fe);
		if (fe->oe)
		    fe->oe->fFlags &= ~(FFLAG_FONT | FFLAG_OFONT);
		if (fe->te)
		    fe->te->fFlags &= ~(FFLAG_FONT | FFLAG_OFONT);
		free (fe);
	    }
	}
	prevCreateBMOK = TRUE;
	cancelCreateBM = FALSE;
	for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
	     feNext = (struct FontEntry *) fe->node.mln_Succ;
	     fe = feNext)
	{
	    fixFonts = (fe->fFlags & FFLAG_CREATE);
	    DC (kprintf ("create fe? $%lx %ld\n", fe, fixFonts));
	    for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
		 seNext = (struct SizeEntry *) se->node.mln_Succ;
		 se = seNext)
	    {
		DC (kprintf ("%s %ld\n", fe->amigaName, se->ySize));
		if (se->fFlags & FFLAG_CREATE)
		{
		    if ((!cancelCreateBM) && (!prevCreateBMOK))
		    {
			if (!WarnRequester (WARNING_CreateBMFail))
			    cancelCreateBM = TRUE;
		    }
		    if (!cancelCreateBM)
		    {
			se->fFlags &= ~FFLAG_CREATE;
			if (prevCreateBMOK = CreateDiskFont (se))
			    fixFonts = TRUE;
		    }
		    if (cancelCreateBM || (!prevCreateBMOK))
			Remove ((struct Node *) se);
		    item++;
		    if (items > 0)
			WaitPointer ((((item * 8) + 1) / (items + 1)) + 1);
		}
	    }
	    if (fixFonts)
	    {
		DC (kprintf ("fixFonts %s\n", fe->fontPath));
		lock = Lock (fe->fontPath, SHARED_LOCK);
		if (lock)
		{
		    strcpy (S.t.TempBuffer, fe->amigaName);
		    strcat (S.t.TempBuffer, SUFFIX_FONT);
		    DC (kprintf ("NewFontContents(,%s)...\n", S.t.TempBuffer));
		    fch = NewFontContents (lock, S.t.TempBuffer);
		    buildpath (S.t.TempBuffer2, fe->fontPath,
			       S.t.TempBuffer);
		    if (fch)
		    {
			DC (kprintf ("write from NewFontContents data\n"));
			length = ((LONG *) fch)[-1] - 4;
			fch->fch_FileID = OFCH_ID;
			if (f = fopen (S.t.TempBuffer2, "w"))
			{
			    fwrite ((char *) fch, 1, length, f);
			    fclose (f);
			}
			DisposeFontContents (fch);
		    }
		    else
		    {
			DC (kprintf ("write empty font contents\n"));
			fch = (struct FontContentsHeader *) S.t.TempBuffer;
			fch->fch_NumEntries = 0;
			fch->fch_FileID = OFCH_ID;
			if (f = fopen (S.t.TempBuffer2, "w"))
			{
			    fwrite ((char *) fch, 1,
				    sizeof (struct FontContentsHeader), f);
			    fclose (f);
			}
		    }
		    UnLock (lock);
		}
	    }
	    fe->fFlags &= ~FFLAG_CREATE;
	}
	for (te = (struct TypeEntry *) ae->tfList.mlh_Head;
	     teNext = (struct TypeEntry *) te->node.mln_Succ;
	     te = teNext)
	{
	    if (te->fFlags & FFLAG_DELETE)
	    {
		buildpath (S.t.TempBuffer, te->typePath, te->amigaName);
		strcat (S.t.TempBuffer, SUFFIX_TYPE);
		DC (kprintf ("  delete %s\n", S.t.TempBuffer));
		DeleteFile (S.t.TempBuffer);
		Remove ((struct Node *) te);
		free (te);
	    }
	}
    }
}
