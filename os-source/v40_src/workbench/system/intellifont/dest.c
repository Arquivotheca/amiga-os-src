/*
**	$Id: dest.c,v 40.0 93/04/19 14:10:27 darren Exp $
**
**	Fountain/dest.c -- handle destination font list
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

#define	DB(x)	;

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *GadToolsBase;
extern struct Library *IntuitionBase;
extern struct Library *UtilityBase;

extern union StaticPool S;

extern char *GDDirNames[MAXDDIRS + 4];
extern struct Window *Window;
extern struct RastPort *RastPort;
extern struct Gadget *GPDDirNext;
extern struct Gadget *GPDDir;
extern struct Gadget *GPDFace;
extern struct Gadget *GPModify;
extern struct MinList FontsList;
extern int LastDDirIndex;
extern int CurrDDirIndex;
extern struct FontsEntry *CurrDDir;

extern struct MinList DFaceList;

RedisplayDFace ()
{
    struct FontEntry *fe;
    int num = 0;

    DB (kprintf ("RedisplayDFace()\n\n"));
    GT_SetGadgetAttrs (GPDFace, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
    FreeList (&DFaceList);

    if (CurrDDir)
    {
	for (fe = (struct FontEntry *) CurrDDir->fcList.mlh_Head;
	     fe->node.mln_Succ;
	     fe = (struct FontEntry *) fe->node.mln_Succ)
	{
	    num++;
	    AddFace (&DFaceList, fe->amigaName, fe);
	}
    }
    GT_SetGadgetAttrs (GPModify, Window, 0, GA_Disabled, ((num) ? FALSE : TRUE), TAG_DONE);
    GT_SetGadgetAttrs (GPDFace, Window, 0, GTLV_Labels, &DFaceList, TAG_DONE);
}

SelectDDirNext (int newDDirNum)
{
    DB (kprintf ("SelectDDirNext\n"));
    if (LastDDirIndex >= 0)
    {
	if ((CurrDDir == 0) || (newDDirNum > LastDDirIndex))
	{
	    if (CurrDDirIndex > (LastDDirIndex / 2))
	    {
		CurrDDirIndex = 0;	/* roll up to zero */
		CurrDDir = (struct FontsEntry *) FontsList.mlh_Head;
	    }
	    else
	    {
		CurrDDirIndex = LastDDirIndex;	/* roll back to last */
		CurrDDir = (struct FontsEntry *) FontsList.mlh_TailPred;
	    }
	    DB (kprintf ("GTCY_Active, %ld\n", CurrDDirIndex));
	    GT_SetGadgetAttrs (GPDDirNext, Window, 0,
			       GTCY_Active, CurrDDirIndex, TAG_DONE);
	}
	else
	{
	    if (CurrDDirIndex > newDDirNum)
	    {
		while (CurrDDirIndex != newDDirNum)
		{
		    CurrDDir = (struct FontsEntry *) CurrDDir->node.mln_Pred;
		    CurrDDirIndex--;
		}
	    }
	    else
	    {
		while (CurrDDirIndex != newDDirNum)
		{
		    CurrDDir = (struct FontsEntry *) CurrDDir->node.mln_Succ;
		    CurrDDirIndex++;
		}
	    }
	    CurrDDirIndex = newDDirNum;
	}

	GT_SetGadgetAttrs (GPDDir, Window, 0, GTST_String, CurrDDir->assignPath, TAG_DONE);
	DB (kprintf ("CurrDDir->assignPath \"%s\"\n", CurrDDir->assignPath));
    }

    RedisplayDFace ();
}

SelectDDir ()
{
    BPTR lock;

    DB (kprintf ("\nSelectDDir\n"));
    CurrDDir = 0;
    if (lock = Lock (((struct StringInfo *) GPDDir->SpecialInfo)->Buffer, SHARED_LOCK))
    {
	if (NameFromLock (lock, S.t.TempBuffer, 256))
	{
	    GT_SetGadgetAttrs (GPDDir, Window, 0, GTST_String, S.t.TempBuffer, TAG_DONE);

	    DB (kprintf ("S.t.TempBuffer \"%s\"\n", S.t.TempBuffer));
	    for (CurrDDirIndex = 0,
		 CurrDDir = (struct FontsEntry *) FontsList.mlh_Head;
		 CurrDDir->node.mln_Succ;
		 CurrDDirIndex++,
		 CurrDDir = (struct FontsEntry *) CurrDDir->node.mln_Succ)
	    {
		if (StrEquNC (S.t.TempBuffer, CurrDDir->assignPath))
		{
		    if (CurrDDirIndex > LastDDirIndex)
		    {
			DB (kprintf ("previously accessed non-Fonts: drawer %s\n", S.t.TempBuffer));
			CurrDDirIndex = MAXDDIRS;
		    }
		    goto srdUpdate;
		}
	    }

	    if (CurrDDir = (struct FontsEntry *) AddFontsEntry (lock, FALSE))
	    {
		CurrDDirIndex = MAXDDIRS;
		if (LastDDirIndex < (MAXDDIRS - 1))
		{
		    LastDDirIndex++;
		    GDDirNames[CurrDDirIndex=LastDDirIndex] = LzS[GADGET_NotFontsPath];
		}
		goto srdUpdate;
	    }
	}
    }

    /* indicate failure */
    CurrDDirIndex = MAXDDIRS + 1;

  srdUpdate:
    DB (kprintf ("GTCY_Active, %ld\n", CurrDDirIndex));
    if(lock)	UnLock (lock);
    GT_SetGadgetAttrs (GPDDirNext, Window, 0, GTCY_Active, CurrDDirIndex, TAG_DONE);
    RedisplayDFace ();
}

SelectDDirReq ()
{
    DirRequester (GPDDir, TITLE_DestinationDrawer);
    SelectDDir ();
}

ValidateDest ()
{
    if ((!CurrDDir) ||
	strcmp (CurrDDir->assignPath, ((struct StringInfo *)GPDDir->SpecialInfo)->Buffer))
    {
	SelectDDir ();
    }
}
