/*
**	$Id: install.c,v 37.7 92/02/07 11:55:49 davidj Exp $
**
**	Fountain/install.c -- handle install button
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

#define	DB(x)	;

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *GadToolsBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;

extern union StaticPool S;
extern UWORD YSizes[];
extern UWORD YSizesCnt;
extern ULONG DeviceDPI;
extern ULONG DotSize;
extern UWORD SymbolSet;

extern struct Window *Window;
extern struct MinList SFaceList;
extern struct Gadget *GPSFace;
extern struct Gadget *GPDDir;
extern int CurrDDirIndex;
extern struct FontsEntry *CurrDDir;
extern struct MinList FontsList;
extern char SDirName[];

#if sizeof(struct InfoData) >= sizeof(struct FileInfoBlock)
#if sizeof(struct InfoData) >= PATHNAMELEN
#define  MYDOSBUFFERSIZE	sizeof(struct InfoData)/2+1
#else
#define  MYDOSBUFFERSIZE	PATHNAMELEN
#endif
#else
#if sizeof(struct FileInfoBlock) >= PATHNAMELEN
#define  MYDOSBUFFERSIZE	sizeof(struct FileInfoBlock)/2+1
#else
#define  MYDOSBUFFERSIZE	PATHNAMELEN
#endif
#endif


CreateInstall (ae, feP, oeP, te)
    struct FontsEntry *ae;
    struct FontEntry **feP;
    struct OTagEntry **oeP;
    struct TypeEntry *te;
{
    struct FontEntry *fe;
    struct OTagEntry *oe;
    struct SizeEntry *se;
    short i;

    fe = *feP;
    oe = *oeP;

    te->fFlags = FFLAG_COMPLETE;

    if (!oe)
    {
	DB (kprintf ("create oe\n"));
	if (!(oe = (struct OTagEntry *) malloc (sizeof (struct OTagEntry))))
	    EndGame (RETURN_ERROR, ENDGAME_NoMemory,
		     sizeof (struct OTagEntry));

	*oeP = oe;
	/* create .otag structure */
	NewList (&oe->sizes);
	for (i = 0; i < YSizesCnt; i++)
	{
	    if (!(se = (struct SizeEntry *)
		  malloc (sizeof (struct SizeEntry))))
		EndGame (RETURN_ERROR, ENDGAME_NoMemory,
			 sizeof (struct SizeEntry));
	    DB (kprintf ("  oe: y %ld\n", YSizes[i]));
	    se->ref.oe = oe;
	    se->ySize = YSizes[i];
	    se->dpi = DeviceDPI;
	    se->dotSize = DotSize;
	    se->symbolSet = SymbolSet;
	    se->fFlags = FFLAG_CREATE;
	    AddTail ((struct List *) & oe->sizes, (struct Node *) se);
	}
	oe->bDef = oe->iDef = oe->biDef = 0;
	oe->bRef = oe->iRef = oe->biRef = 0;
	oe->symbolSet = SymbolSet;
	oe->fFlags = FFLAG_COMPLETE | FFLAG_CREATE;
	strcpy (oe->amigaName, te->amigaName);
	oe->bRefName[0] = oe->iRefName[0] = oe->biRefName[0] = '\0';
	strcpy (oe->otagPath, ae->assignPath);
	AddTail ((struct List *) & ae->otList, (struct Node *) oe);
    }

    oe->te = te;
    oe->fFlags |= FFLAG_TYPE;

    DB (kprintf ("oe name \"%s\" path \"%s\" fFlags $%02lx\n", oe->amigaName, oe->otagPath, oe->fFlags));
    DB (kprintf ("  otag SymbolSet %.2s\n", &oe->symbolSet));

    if (!fe)
    {
	DB (kprintf ("create fe\n"));
	if (!(fe = (struct FontEntry *) malloc (sizeof (struct FontEntry))))
	    EndGame (RETURN_ERROR, ENDGAME_NoMemory,
		     sizeof (struct FontEntry));

	*feP = fe;
	/* create .font structure */
	NewList (&fe->sizes);
	fe->fFlags = FFLAG_COMPLETE | FFLAG_CREATE;
	strcpy (fe->amigaName, te->amigaName);
	strcpy (fe->fontPath, ae->assignPath);
	AddTail ((struct List *) & ae->fcList, (struct Node *) fe);
    }
    fe->oe = oe;
    fe->te = te;
    fe->fFlags |= FFLAG_TYPE;

    DB (kprintf ("fe name \"%s\" path \"%s\" fFlags $%02lx\n", fe->amigaName, fe->fontPath, fe->fFlags));
}


SelectInstall ()
{
    struct FaceDisplay *fd;
    struct TypeEntry *te, *teI;
    struct FontEntry *fe, *feI;
    struct OTagEntry *oeI;
    struct FontsEntry *ae;
    struct SizeEntry *se;
    BPTR lock1, lock2, lock3;
    FILE *libSrc, *libDest;
    UWORD myDosBuffer[MYDOSBUFFERSIZE];
    struct FileInfoBlock *fib;
    struct InfoData *info;
    int diskBlocks, installations, installed, actual, length;
    char prevFailed;

    if (!CurrDDir)
    {
	ErrRequester (ERROR_INoDest, ((struct StringInfo *)
				      GPDDir->SpecialInfo)->Buffer);
	return;
    }

    WaitPointer (1);
    SetWindowTitles(Window, LzS[TITLE_Install], (char *) ~0);

    /* check destination directory and available disk space */
    diskBlocks = 0;
    prevFailed = FALSE;
    if (lock1 = Lock (CurrDDir->assignPath, SHARED_LOCK))
    {
	fib = (struct FileInfoBlock *)
	  ((((ULONG) myDosBuffer) + 2) & 0xfffffffc);
	if (Examine (lock1, fib) && (fib->fib_DirEntryType >= 0))
	{
	    info = (struct InfoData *)
	      ((((ULONG) myDosBuffer) + 2) & 0xfffffffc);
	    if (Info (lock1, info))
	    {
		diskBlocks = info->id_NumBlocks - info->id_NumBlocksUsed;
		DB (kprintf ("diskBlocks %ld\n", diskBlocks));
		/* ensure the outline subdirectory exists */
		buildpath (S.t.TempBuffer, CurrDDir->assignPath, DIR_OUTLINES);
		lock2 = Lock (S.t.TempBuffer, SHARED_LOCK);
		if (lock2)
		{
		    /* ensure different than source directory */
		    lock3 = Lock (SDirName, SHARED_LOCK);
		    if (lock3)
		    {
			if (SameLock (lock2, lock3) == LOCK_SAME)
			{
			    ErrRequester (ERROR_ISrcEquDest, SDirName);
			    prevFailed = TRUE;
			}
			UnLock (lock3);
		    }
		}
		else
		{
		    /* create new outline subdirectory */
		    lock2 = CreateDir (S.t.TempBuffer);
		}
		if (lock2)
		{
		    UnLock (lock2);
		}
		else
		{
		    ErrRequester (ERROR_ICreateDirFail, S.t.TempBuffer);
		    prevFailed = TRUE;
		}
	    }
	    else
	    {
		ErrRequester (ERROR_IInternal, "Info");
		prevFailed = TRUE;
	    }
	}
	else
	{
	    ErrRequester (ERROR_INotDir, CurrDDir->assignPath);
	    prevFailed = TRUE;
	}
	UnLock (lock1);
    }
    else
    {
	ErrRequester (ERROR_INoDest, CurrDDir->assignPath);
	prevFailed = TRUE;
    }

    if (prevFailed)
    {
	WaitPointer (-1);
	return;
    }

    installations = installed = 0;
    for (fd = (struct FaceDisplay *) SFaceList.mlh_Head;
	 fd->node.ln_Succ;
	 fd = (struct FaceDisplay *) fd->node.ln_Succ)
    {
	if (fd->name[0] == '+')
	{
	    installations++;
	    diskBlocks -= ((fd->ref.te->spaceReq + 511) >> 9) + 1;
	    DB (kprintf ("  diskblocks -= %ld (%ld) to %ld\n", ((fd->ref.te->spaceReq + 511) >> 9) + 1, fd->ref.te->spaceReq, diskBlocks));
	}
    }

    if ((diskBlocks < 0) && (!WarnRequester (WARNING_INoDestSpace,
					     CurrDDir->assignPath, -diskBlocks * 2)))
    {
	WaitPointer (-1);
	return;
    }

    GT_SetGadgetAttrs (GPSFace, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
    prevFailed = FALSE;
    for (fd = (struct FaceDisplay *) SFaceList.mlh_Head;
	 fd->node.ln_Succ;
	 fd = (struct FaceDisplay *) fd->node.ln_Succ)
    {
	teI = 0;
	oeI = 0;
	feI = 0;
	/* check if this is to be installed */
	if (fd->name[0] != '+')
	    continue;
	DB (kprintf ("Install %s\n", fd->name));
	WaitPointer ((((installed * 7) + 1) / (installations + 1)) + 2);
	te = fd->ref.te;
	if ((prevFailed) && (!WarnRequester (WARNING_IContinueInstall,
					     te->amigaName)))
	    break;
	prevFailed = FALSE;
	/* check for existing font in destination */
	for (fe = (struct FontEntry *) CurrDDir->fcList.mlh_Head;
	     fe->node.mln_Succ;
	     fe = (struct FontEntry *) fe->node.mln_Succ)
	{
	    DB (kprintf ("check match \"%s\" vs \"%s\"\n", te->amigaName, fe->amigaName));
	    if (StrEquNC (te->amigaName, fe->amigaName))
	    {
		DB (kprintf ("found font %s already exists\n", te->amigaName));
		if (fe->fFlags & FFLAG_OFONT)
		{
		    if (!WarnRequester (WARNING_IOverwrite, te->amigaName))
			goto nextInstall;
		}
		else
		{
		    if (!WarnRequester (WARNING_IOverwriteStd, te->amigaName))
			goto nextInstall;
		}
		feI = fe;
		feI->fFlags |= FFLAG_CREATE;
		for (se = (struct SizeEntry *) feI->sizes.mlh_Head;
		     se->node.mln_Succ;
		     se = (struct SizeEntry *) se->node.mln_Succ)
		{
		    DB (kprintf ("reinstall .font size %ld\n", se->ySize));
		    se->fFlags |= FFLAG_CREATE;
		}
		oeI = feI->oe;
		if (oeI)
		{
		    DB (kprintf ("reinstall .otag %s\n", oeI->amigaName));
		    oeI->fFlags |= FFLAG_CREATE;
		    teI = oeI->te;
		}
		break;
	    }
	}
	if (CurrDDirIndex < MAXDDIRS)
	{
	    DB (kprintf ("check for blocking\n"));
	    /* check if other entry is before this one in Fonts: path */
	    for (ae = (struct FontsEntry *) FontsList.mlh_Head;
		 ae->node.mln_Succ && (ae != CurrDDir);
		 ae = (struct FontsEntry *) ae->node.mln_Succ)
	    {
		for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
		     fe->node.mln_Succ;
		     fe = (struct FontEntry *) fe->node.mln_Succ)
		{
		    if (StrEquNC (te->amigaName, fe->amigaName))
		    {
			if (!WarnRequester (WARNING_Hidden,
					    te->amigaName, fe->fontPath))
			    goto nextInstall;
			goto checkAfter;
		    }
		}
	    }
	    /* check if other entry is before this one in Fonts: path */
	  checkAfter:
	    for (ae = (struct FontsEntry *) CurrDDir->node.mln_Succ;
		 ae->node.mln_Succ;
		 ae = (struct FontsEntry *) ae->node.mln_Succ)
	    {
		for (fe = (struct FontEntry *) ae->fcList.mlh_Head;
		     fe->node.mln_Succ;
		     fe = (struct FontEntry *) fe->node.mln_Succ)
		{
		    if (StrEquNC (te->amigaName, fe->amigaName))
		    {
			if (!WarnRequester (WARNING_Hides,
					    te->amigaName, fe->fontPath))
			    goto nextInstall;
			goto createType;
		    }
		}
	    }
	}
      createType:
	/* free any previous type description structure */
	if (teI)
	{
	    DB (kprintf ("free old .type $%lx\n", teI));
	    Remove ((struct Node *) teI);
	    free (teI);		/* free old structure */
	    feI->fFlags &= ~FFLAG_TYPE;
	    oeI->fFlags &= ~FFLAG_TYPE;
	    oeI->te = 0;
	}

	/* create type description structures */
	if (!(teI = (struct TypeEntry *) malloc (sizeof (struct TypeEntry))))
	    EndGame (RETURN_ERROR, ENDGAME_NoMemory, sizeof (struct TypeEntry));
	*teI = *te;

	buildpath (teI->typePath, CurrDDir->assignPath, DIR_OUTLINES);
	sprintf ((char *) myDosBuffer, "%s/%s" SUFFIX_TYPE, teI->typePath,
		 teI->amigaName);
	DB (kprintf ("typePath \"%s\" assignPath \"%s\" :: \"%s\"\n", teI->typePath, CurrDDir->assignPath, (char *) myDosBuffer));

	/* install based on whether it's a FAIS or an intellifont file */
	if (teI->complementID)
	{
	    /* FAIS file */
	    DB (kprintf ("install FAIS\n"));
	    if (!FAISLoad (teI, (char *) myDosBuffer, 0))
	    {
		prevFailed = TRUE;
	    }
	}
	else
	{
	    /* intellifont file */
	    DB (kprintf ("install Intellifont\n"));
	    if (libSrc = fopen (teI->sourceFile, "r"))
	    {
		if (libDest = fopen ((char *) myDosBuffer, "w"))
		{
		    while ((length = fread (S.t.TempBuffer, 1, 10000, libSrc))
			   > 0)
		    {
			if ((actual =
			     fwrite (S.t.TempBuffer, 1, length, libDest))
			    != length)
			{
			    ErrRequester (ERROR_IFileWrite,
					  teI->amigaName, (char *) myDosBuffer,
					  length, actual, ferror (libDest));
			    prevFailed = TRUE;
			}
		    }
		    if (length < 0)
		    {
			ErrRequester (ERROR_IFileRead,
				      teI->amigaName, (char *) myDosBuffer,
				      10000, length, ferror (libSrc));
			prevFailed = TRUE;
		    }
		    fclose (libDest);
		}
		fclose (libSrc);
	    }
	}
	if (prevFailed)
	{
	    free (teI);
	    goto nextInstall;
	}

	AddTail ((struct List *) & CurrDDir->tfList, (struct Node *) teI);

	CreateInstall (CurrDDir, &feI, &oeI, teI);

	FindOMatches (CurrDDir);
	Update (CurrDDir, 0);

	/* indicate installation success */
	fd->node.ln_Name[0] = ' ';

      nextInstall:
	installed++;
    }

    GT_SetGadgetAttrs (GPSFace, Window, 0, GTLV_Labels, &SFaceList, TAG_DONE);
    RedisplayDFace ();
    WaitPointer (-1);
    SetWindowTitles(Window, LzS[TITLE_Main], (char *) ~0);
}
