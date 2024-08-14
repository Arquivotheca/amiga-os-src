/*
**	$Id: source.c,v 37.8 92/02/10 10:58:07 davidj Exp $
**
**	Fountain/source.c -- handle source font side of display
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

#define	DB(x)	;

static int num_install = 0;

extern struct Process *Process;

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *GadToolsBase;
extern struct Library *IntuitionBase;

extern struct Window *Window;
extern struct Gadget *GPSDir;
extern struct Gadget *GPSFace;
extern struct Gadget *GPInstall;

extern union StaticPool S;

extern struct MinList SFaceList;

struct MinList SourceList =
{
    (struct MinNode *) & SourceList.mlh_Tail, 0,
    (struct MinNode *) & SourceList.mlh_Head
};

char SDirName[PATHNAMELEN] = {0};

void HuntPath (int lDirType, char *pathName, int huntLimit)
{
    DB (kprintf ("HuntPath(%ld, \"%s\")\n", lDirType, pathName));
    if (lDirType == GLDTYPE_PCDOS)
	FAISInfo (pathName);
    else
	LIBInfo (pathName, &SourceList, huntLimit);
}

FindSourceDisk (int lDirType)
{
    APTR oldPRWindowPtr;
    struct DosList *dol;
    struct FileSysStartupMsg *fssm;
    struct DosEnvec *de;
    BPTR lock;
    UWORD fibBuffer[sizeof (struct FileInfoBlock) / 2 + 1];
    char sourcePath[PATHNAMELEN], *s;
    struct FileInfoBlock *fib;

    DB (kprintf ("FindSourceDisk\n"));
    oldPRWindowPtr = Process->pr_WindowPtr;
    Process->pr_WindowPtr = (APTR) - 1;
    dol = LockDosList (LDF_DEVICES | LDF_READ);
    while (dol = NextDosEntry (dol, LDF_DEVICES))
    {
	if ((dol->dol_Task) && (dol->dol_Type == DLT_DEVICE))
	{
	    fssm = (struct FileSysStartupMsg *) BADDR (dol->dol_misc.dol_handler.dol_Startup);
	    if (((ULONG) fssm) >= 0x40)
	    {
		de = (struct DosEnvec *) BADDR (fssm->fssm_Environ);
		if (lDirType == GLDTYPE_PCDOS)
		{
		    DB (kprintf ("GLDTYPE_PCDOS\n"));
		    DB (kprintf ("fssm_Device %ld \"%s\"\n", ((UBYTE *) BADDR (fssm->fssm_Device))[0], ((UBYTE *) BADDR (fssm->fssm_Device)) + 1));
		    if ((de->de_TableSize < DE_DOSTYPE) ||
			((de->de_DosType & 0xffffff00) != ID_PC_DISK))
			continue;
		}
		else		/* GLDTYPE_AMIGA */
		{
		    DB (kprintf ("GLDTYPE_AMIGA\n"));
		    DB (kprintf ("fssm_Device %ld \"%s\"\n", ((UBYTE *) BADDR (fssm->fssm_Device))[0], ((UBYTE *) BADDR (fssm->fssm_Device)) + 1));
		    if ((strcmp (BADDR (fssm->fssm_Device), DOSDEV_TRACKDISK) != 0) ||
			((de->de_TableSize > DE_DOSTYPE) && ((de->de_DosType & 0xffffff00) != ID_DOS_DISK)))
			continue;
		}
		lock = 0;	/* null BSTR */
		lock = DoPkt3 (dol->dol_Task, ACTION_LOCATE_OBJECT, 0,(((ULONG) & lock) + 2) >> 2, SHARED_LOCK);
		if (lock)
		{
		    if (NameFromLock (lock, sourcePath, 256))
		    {
			if (lDirType == GLDTYPE_PCDOS)
			{
			    HuntPath (GLDTYPE_PCDOS, sourcePath, 0);
			}
			else	/* GLDTYPE_AMIGA */
			{
			    fib = (struct FileInfoBlock *)(((ULONG) & fibBuffer[1]) & 0xfffffffc);
			    if (Examine (lock, fib) && (fib->fib_DirEntryType >= 0))
			    {
				while (ExNext (lock, fib))
				{
				    if (fib->fib_DirEntryType >= 0)
				    {
					/* Build pathname of dir to examine */
					s = sourcePath + strlen (sourcePath);
					AddPart (sourcePath, fib->fib_FileName, PATHNAMELEN);
					HuntPath (GLDTYPE_AMIGA, sourcePath, 11);
					*s = '\0';
					break;
				    }
				}
			    }
			}
		    }
		    else
		    {
			DB (kprintf ("Couldn't get name from lock\n"));
		    }
		    UnLock (lock);
		}
		if (SourceList.mlh_Head->mln_Succ)
		{
		    strcpy (SDirName, sourcePath);
		    break;
		}
	    }
	}
    }
    UnLockDosList (LDF_DEVICES | LDF_READ);
    Process->pr_WindowPtr = oldPRWindowPtr;

    /* Show it, please! */
    redisplaySFace ();
}

redisplaySFace ()
{
    struct TypeEntry *src;

    if (GPSFace)
    {
	GT_SetGadgetAttrs (GPSDir,    Window, 0, GTST_String, SDirName, TAG_DONE);
	GT_SetGadgetAttrs (GPInstall, Window, 0, GA_Disabled, TRUE, TAG_DONE);
	GT_SetGadgetAttrs (GPSFace,   Window, 0, GTLV_Labels, ~0L, TAG_DONE);
    }

    FreeList (&SFaceList);
    num_install = 0;
    for (src = (struct TypeEntry *) SourceList.mlh_Head;
	 src->node.mln_Succ;
	 src = (struct TypeEntry *) src->node.mln_Succ)
    {
	AddFace (&SFaceList, src->amigaName, src);
    }

    if (GPSFace)
    {
	GT_SetGadgetAttrs (GPSFace,   Window, 0, GTLV_Labels, &SFaceList, TAG_DONE);
    }
}

FindSource ()
{
    DB (kprintf ("FindSource\n"));
    FreeList (&SourceList);

    FindSourceDisk (GLDTYPE_PCDOS);
    if (!SourceList.mlh_Head->mln_Succ)
	FindSourceDisk (GLDTYPE_AMIGA);
}

SelectSDirReq ()
{
    DirRequester (GPSDir, TITLE_OutlineSource);
    SelectSDir ();
    SelectDDir ();
}

SelectSDir ()
{
    BPTR lock;

    int lDirType;
    char *s;

    FreeList (&SourceList);

    s = ((struct StringInfo *) GPSDir->SpecialInfo)->Buffer;

    strcpy (SDirName, s);

    WaitPointer (0);
    if (strlen (s) == 0)
    {
	FindSource ();
    }
    else
    {
	if (lock = Lock (s, SHARED_LOCK))
	{
	    if (NameFromLock (lock, S.t.TempBuffer, 256))
	    {
		strcpy (SDirName, S.t.TempBuffer);
		for (lDirType = 0; lDirType < GLDTYPE_COUNT; lDirType++)
		{
		    HuntPath (lDirType, s, 0);
		    if (SourceList.mlh_Head->mln_Succ)
			break;
		}
	    }
	    UnLock (lock);
	}
    }
    WaitPointer (-1);

    redisplaySFace ();
}

SelectSFace (int newSFaceNum)
{
    struct FaceDisplay *fd;
    int i;

    GT_SetGadgetAttrs (GPSFace, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
    for (fd = (struct FaceDisplay *) SFaceList.mlh_Head, i = 0;
	 fd && (i < newSFaceNum);
	 fd = (struct FaceDisplay *) fd->node.ln_Succ, i++) ;

    if (fd->name[0] == ' ')
    {
	num_install++;
	fd->name[0] = '+';
    }
    else
    {
	num_install--;
	fd->name[0] = ' ';
    }
    GT_SetGadgetAttrs (GPInstall, Window, 0, GA_Disabled, ((num_install) ? FALSE : TRUE), TAG_DONE);
    GT_SetGadgetAttrs (GPSFace, Window, 0, GTLV_Labels, &SFaceList, TAG_DONE);
}

ValidateSource ()
{
    if (strcmp (SDirName, ((struct StringInfo *) GPSDir->SpecialInfo)->Buffer))
	SelectSDir ();
}
