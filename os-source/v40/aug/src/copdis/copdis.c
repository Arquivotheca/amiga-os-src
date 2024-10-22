/*****************************************************************************
*
*	$Id: CopDis.c,v 39.2 93/03/23 13:50:11 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	CopDis.c,v $
 * Revision 39.2  93/03/23  13:50:11  Jim2
 * *** empty log message ***
 * 
 * Revision 39.1  92/11/18  10:41:01  Jim2
 * First Release - works with remote wack.
 *
*
******************************************************************************/

#include "copdis_rev.h"

#include <exec/memory.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include "V:include/graphics/gfxbase.h"
#include "V:include/graphics/view.h"
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/icon_protos.h>
#include <pragmas/icon_pragmas.h>

#include "v39:aug/src/wack/wack_protos.h"
#include "v39:aug/src/wack/wack_pragmas.h"

#include <string.h>

#include "sixteenbitcopper.h"

#include "copdis.h"


#define TEMPLATE "C=COMMENTS/S,D=DATA/S,N=NOCOLOURS/S,I=NOCOPINIT/S,F=NOFINAL/S,SAFELY/S,ID=INTERDISP/S,SS=SPRITESET/S,SC=SPRITECLEAR/S,U=USER/S,NORAW/S,NT=NOTRANS/S,WACKPORT/K" VERSTAG
#define OPT_COMMENT 0
#define OPT_DATA 1
#define OPT_NOCOLOURS 2
#define OPT_NOCOPINIT 3
#define OPT_NOFINAL 4
#define OPT_SAFELY 5
#define OPT_IDISPLAY 6
#define OPT_ISPRITE 7
#define OPT_ISPCLEAR 8
#define OPT_IUSER 9
#define OPT_NORAW 10
#define OPT_NOTRANS 11
#define OPT_WACK 12
#define OPT_COUNT 13


#define EXECBASE (*(struct ExecBase **)4)


struct ExecBase *SysBase = NULL;
struct Library *DOSBase = NULL;
struct GfxBase *GfxBase = NULL;

STATIC struct MsgPort *WackBase;

STATIC BOOL StandAlone;

ULONG cmd (VOID)
{
    struct Process *proc;
    struct WBStartup *msg=NULL;
    struct RDArgs *rdargs = NULL, *myRDArgs=NULL;
    struct View *ActiView;
    struct CopList *UCopperList;
    struct ViewPort *Walking, *StartOfVP;
    struct Library *IconBase;
    struct WBArg *wbArg;
    struct DiskObject *diskObj;
    BPTR tmplock;
    char **ToolTypes, *TotalString;
    ULONG totalSize=3;



    LONG result[OPT_COUNT];

    SysBase = EXECBASE;
    proc = (struct Process *) FindTask(NULL);
    if (!(proc->pr_CLI))
    {
	WaitPort(&(proc->pr_MsgPort));
	msg = (struct WBStartup *) GetMsg (&(proc->pr_MsgPort));
    }
    memset((char *)result, 0, sizeof(result));
    if ((DOSBase = OpenLibrary ("dos.library",37)) != NULL)
    {

	if (msg)
	{
			/*
			 * Started from Workbench so do icon magic...
			 *
			 * What we will do here is try all of the tooltypes
			 * in the icon and keep only those which do not cause
			 * errors in the RDArgs.
			 */
	    if (wbArg=msg->sm_ArgList)
		if ((IconBase=OpenLibrary("icon.library",0)) != NULL)
		{
				/*
				 * Use project icon if it is there...
				 */
		    if (msg->sm_NumArgs > 1)
			wbArg++;

		    tmplock=CurrentDir(wbArg->wa_Lock);
		    if (diskObj=GetDiskObject(wbArg->wa_Name))
		    {
			if (ToolTypes=diskObj->do_ToolTypes)
			{

			    while (*ToolTypes)
			    {
				totalSize+=strlen(*ToolTypes)+1;
				ToolTypes++;
			    }

			    if (TotalString=AllocVec(totalSize,MEMF_PUBLIC))
			    {
				char  *CurrentPos=TotalString;
				ULONG currentLength;

				ToolTypes=diskObj->do_ToolTypes;
				do
				{
				    *CurrentPos='\0';
				    if (*ToolTypes)
					strcpy(CurrentPos,*ToolTypes);
				    currentLength=strlen(CurrentPos);
				    CurrentPos[currentLength+0]='\n';
				    CurrentPos[currentLength+1]='\0';

				    if (rdargs)
					FreeArgs(rdargs);
				    rdargs=NULL;

				    if (myRDArgs)
					FreeDosObject(DOS_RDARGS,myRDArgs);
				    if (myRDArgs=AllocDosObject(DOS_RDARGS,NULL))
				    {
					myRDArgs->RDA_Source.CS_Buffer=TotalString;
					myRDArgs->RDA_Source.CS_Length=strlen(TotalString);

					if (rdargs=ReadArgs(TEMPLATE, result, myRDArgs))
					{
					    CurrentPos[currentLength]=' ';
					    CurrentPos+=currentLength+1;
					}
				    }
				}
				while (*ToolTypes++);
				FreeVec(TotalString);
			    }
			}

			FreeDiskObject(diskObj);
		    }

		    CurrentDir(tmplock);
		    CloseLibrary(IconBase);
		}
	    }
	else
	    rdargs = ReadArgs(TEMPLATE, result, NULL);
	if (!(StandAlone = (result[OPT_WACK] == 0)))
	    WackBase = FindPort( (STRPTR)result[OPT_WACK]);


	FreeArgs(rdargs);
	if (myRDArgs)
	    FreeDosObject(DOS_RDARGS,myRDArgs);
	if (StandAlone || (WackBase != NULL))
	{
	    if (StandAlone)
		GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	    else
		GfxBase = (struct GfxBase *)Wack_FindLibrary("graphics.library");
	    if (GfxBase == NULL)
		CopDisVPrintf ("couldn't open graphics.library\n");
	    else
	    {
		if (result[OPT_SAFELY])
		    if (StandAlone)
		    	ObtainSemaphore(GfxBase->ActiViewCprSemaphore);
		    else
		    	CopDisVPrintf("SAFELY is not available when run from Wack\n");
	    	if ((ActiView = (struct View *)CopDisFindPointer((APTR)&GfxBase-> ActiView)) == NULL)
		    CopDisVPrintf ("NO ACTIVE VIEW PORT?!?!?\n");
	    	else
	    	{
		    if (!result[OPT_NOFINAL])
		    {
			CopDisVPrintf("\nCurrently active LOF copper list:\n");
		    	copdis16((struct copinstruction *) CopDisFindPointer((APTR)&((struct cprlist *) CopDisFindPointer((APTR) &ActiView->LOFCprList))->start), result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS], !result[OPT_NORAW], !result[OPT_NOTRANS]);

		    	if (CopDisFindWord((APTR) &ActiView->Modes) & LACE)
		    	{
			    CopDisVPrintf("\nCurrently active SHF copper list:\n");
			    copdis16((struct copinstruction *)CopDisFindPointer((APTR) &((struct cprlist *) CopDisFindPointer((APTR) &ActiView->SHFCprList))->start), result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS], !result[OPT_NORAW], !result[OPT_NOTRANS]);
		    	}
		    }

		    if (result[OPT_NOCOPINIT] == FALSE)
		    {
		    	CopDisVPrintf("\nGfxBase's copinit list:\n");
		    	copdis16((struct copinstruction *) CopDisFindPointer((APTR) &GfxBase->copinit), result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS], !result[OPT_NORAW], !result[OPT_NOTRANS]);
		    }
		    StartOfVP = (struct ViewPort *) CopDisFindPointer((APTR) &ActiView-> ViewPort);
		    if (result[OPT_IDISPLAY])
		    {
		    	CopDisVPrintf ("\nIntermediate Copper List for display\n");
		    	Walking = StartOfVP;
		    	do
		    	{
			    if (((CopDisFindWord((APTR) &Walking->Modes) & VP_HIDE) == 0) && ((UCopperList = (struct CopList *) CopDisFindPointer((APTR) &Walking-> DspIns)) != NULL))
			    	ICopDis16((struct CopIns *) CopDisFindPointer((APTR) &UCopperList-> CopIns), Walking, result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS], !result[OPT_NORAW], !result[OPT_NOTRANS]);
			    Walking = (struct ViewPort *) CopDisFindPointer((APTR) &Walking-> Next);
		        }
		    	while (Walking != NULL);
		    }
		    if (result[OPT_ISPRITE])
		    {
		    	CopDisVPrintf ("\nIntermediate Sprite Copper List for display\n");
		    	Walking = StartOfVP;
		    	do
		    	{
			    if (((CopDisFindWord ((APTR) &Walking-> Modes) & VP_HIDE) == 0) && ((UCopperList = (struct CopList *) CopDisFindPointer((APTR) &Walking-> SprIns)) != NULL))
			    	ICopDis16((struct CopIns *) CopDisFindPointer((APTR) &UCopperList-> CopIns), Walking, result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS], !result[OPT_NORAW], !result[OPT_NOTRANS]);
			    Walking = (struct ViewPort *) CopDisFindPointer((APTR) &Walking-> Next);
		    	}
		    	while (Walking != NULL);
		    }
		    if (result[OPT_ISPCLEAR])
		    {
		    	CopDisVPrintf ("\nIntermediate Clear Sprite Copper List for display\n");
		    	Walking = StartOfVP;
		    	do
		    	{
			    if (((CopDisFindWord ((APTR) &Walking-> Modes) & VP_HIDE) == 0) && ((UCopperList = (struct CopList *) CopDisFindPointer((APTR) &Walking-> ClrIns)) != NULL))
			    	ICopDis16((struct CopIns *) CopDisFindPointer((APTR) &UCopperList-> CopIns), Walking, result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS], !result[OPT_NORAW], !result[OPT_NOTRANS]);
			    Walking = (struct ViewPort *) CopDisFindPointer((APTR) &Walking-> Next);
		    	}
		    	while (Walking != NULL);
		    }
		    if (result[OPT_IUSER])
		    {
		    	CopDisVPrintf ("\nIntermediate User Copper List for display\n");
		    	Walking = StartOfVP;
		    	do
		    	{
			    if (((CopDisFindWord ((APTR) &Walking-> Modes) & VP_HIDE) == 0) && ((UCopperList = (struct CopList *) CopDisFindPointer((APTR) &Walking-> UCopIns)) != NULL))
			    	if ((UCopperList = (struct CopList *) CopDisFindPointer( &((struct UCopList *) UCopperList)->FirstCopList)) != NULL)
				    ICopDis16((struct CopIns *) CopDisFindPointer((APTR) &UCopperList-> CopIns), Walking, result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS], !result[OPT_NORAW], !result[OPT_NOTRANS]);
			    Walking = (struct ViewPort *) CopDisFindPointer ((APTR) &Walking-> Next);
		    	}
		    	while (Walking != NULL);
		    }
	    	}
	    	if ((result[OPT_SAFELY]) && StandAlone)
		    ReleaseSemaphore(GfxBase->ActiViewCprSemaphore);
	    }
	}
	else
	  Printf ("Could not open the WackPort\n");
    }
    if (msg)
    {
	Forbid();
	ReplyMsg((struct Message *)msg);
    }
    return(0);
}

VOID CopDisVPrintf(STRPTR Fmt, ...)
{
    if (StandAlone)
	VPrintf (Fmt, ((ULONG *)&Fmt)+1 );
    else
	Wack_VPrintf(Fmt,  ((ULONG *)&Fmt)+1);
}

APTR CopDisFindPointer(APTR Address)
{
    if (StandAlone)
	return (*((APTR *)Address));
    return (Wack_ReadPointer(Address));
}

UWORD CopDisFindWord (APTR Address)
{
    if (StandAlone)
	return (*((UWORD *) Address));
    return (Wack_ReadWord(Address));
}

VOID CopDisReadBlock(APTR Address, APTR Buffer, ULONG Size)
{
    if (StandAlone)
	CopyMem(Address, Buffer, Size);
    else
	Wack_ReadBlock(Address,Buffer, Size);
} /* CopDisReadBlock */