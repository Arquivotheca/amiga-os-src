/*
**	$Id: main.c,v 37.9 92/06/01 12:37:53 darren Exp $
**
**	Fountain/main.c -- main entry and program loop
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

/*
 *  Usage: Intellifont [VALIDATE]
 */

#include "fountain.h"
#include "Intellifont_rev.h"

extern struct WBStartup *WBenchMsg;
extern int (*_ONBREAK)();

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *GadToolsBase;
extern struct Library *IconBase;
extern struct Library *IntuitionBase;

extern struct Window *Window;
extern struct RastPort *RastPort;
extern BOOL ModifyActive;

char VersionTag[] = VERSTAG;
#if 0
"\0$VER: fountain " VERSION_TEXT " (30.1.91)";
#endif

struct Node FountainResource = {
    0, 0, 0, -127, RESOURCE_FOUNTAIN
};

BOOL ValidateOnly = FALSE;

struct Process *Process;
struct IntuiMessage *IMsg = 0;

void EndGame(code, formatIndex, args)
int code, formatIndex;
char *args;
{
    struct EasyStruct ez;

    if (IntuitionBase && formatIndex) {
	ez.es_StructSize = sizeof(struct EasyStruct);
	ez.es_Flags = 0;
	ez.es_Title = LzS[TITLE_EndGame];
	ez.es_TextFormat = LzS[formatIndex];
	DBG1("EndGame \"%s\"\n", ez.es_TextFormat);
	ez.es_GadgetFormat = LzS[GADGET_OK];
	EasyRequestArgs(Window, &ez, 0, &args);
    }
    if (IMsg)
	GT_ReplyIMsg(IMsg);
    Help(FALSE);
    CloseUI();
    CloseFountainWindow();
    CloseLibraries();
    if (FountainResource.ln_Type)
	RemResource(&FountainResource);
    exit(code);
}

int CXBRK()
{
    struct EasyStruct ez;

    ez.es_StructSize = sizeof(struct EasyStruct);
    ez.es_Flags = 0;
    ez.es_Title = LzS[TITLE_EndGame];
    ez.es_TextFormat = LzS[ENDGAME_ControlC];
    ez.es_GadgetFormat = LzS[GADGET_ConfirmIgnore];

    if (EasyRequestArgs(Window, &ez, 0, 0))
	EndGame(0, 0);
    return(0);
}


HandleIM()
{
    struct Gadget *gp;

    if (IMsg->IDCMPWindow != Window)
	return;

    switch(IMsg->Class) {
      case REFRESHWINDOW:
	GT_BeginRefresh(Window);
	RefreshUI();
	GT_EndRefresh(Window, TRUE);
	break;
      case CLOSEWINDOW:
	EndGame(RETURN_OK, 0);
	break;
      case RAWKEY:
	DBG1("RAWKEY $%lx\n", IMsg->Code);
	if (IMsg->Code == 0x5f)
	    Help(TRUE);
	break;
      case VANILLAKEY:
	DBG1("VANILLAKEY $%lx\n", IMsg->Code);
	if (IMsg->Code == 0x1b) {
	    /* Esc */
	    Help(FALSE);
	    break;
	}
	if (ModifyActive)
	    MVanilla(IMsg->Code);
	break;
      case GADGETUP:
	DBG1("GADGETUP code $%lx\n", IMsg->Code);
	if (IMsg->Code == 0x5f) {
	    Help(TRUE);
	    break;
	}
      case GADGETDOWN:
      case MOUSEMOVE:
	gp = (struct Gadget *) IMsg->IAddress;
	switch (gp->GadgetID) {
	  case G_SDIRREQ:
	    SelectSDirReq();
	    break;
	  case G_SDIR:
	    SelectSDir();
	    break;
	  case G_SFACE:
	    SelectSFace(IMsg->Code);
	    break;
	  case G_DDIRNEXT:
	    SelectDDirNext(IMsg->Code);
	    break;
	  case G_DDIRREQ:
	    SelectDDirReq();
	    break;
	  case G_DDIR:
	    SelectDDir();
	    break;

	  case G_INSTALL:
	    ValidateSource();
	    ValidateDest();
	    SelectInstall();
	    break;

	  case G_MODIFY:
	    ValidateDest();
	    SelectModify();
	    break;

	  /* Modify */
	  case G_MFACE:
	    MFace(IMsg->Code);
	    break;
	  case G_ADDSIZE:
	    MAddSize();
	    break;
	  case G_DELSIZE:
	    MDelSize();
	    break;
	  case G_SIZENUM:
	    break;
	  case G_CREATEBM:
	    MCreateBitmap();
	    break;
	  case G_DELETEBM:
	    MDeleteBitmap();
	    break;
	  case G_SIZE:
	    MSize(IMsg->Code);
	    break;
	  case G_DELETEFACE:
	    MDeleteFace();
	    break;
	  case G_PERFORM:
	    MUpdate();
	    break;
	  case G_CANCEL:
	    OpenMainUI();
	    break;

	  /* Help */
	  case G_HELP:
	    SelectHelp();
	  default:
	    break;
	}
	break;
      default:
	break;
    }
}

#if 0
#ifdef  DEBUG
void main(int argc, char *argv[])
#else
void _main(char *line)
#endif
#endif

void main (int argc, char *argv)
{
    ULONG opts[1];
    struct RDArgs *rdArgs;
    struct DiskObject *diskObj;
    short i;
    ULONG signal;

    /* initialization */
    Process = (struct Process *) FindTask(0);
    OpenLibraries();
    _ONBREAK = CXBRK;

    /* exclusion */
    Forbid();
    if (OpenResource(FountainResource.ln_Name)) {
	Permit();
	EndGame(RETURN_ERROR, ENDGAME_DuplicateRun);
    }
    FountainResource.ln_Type = NT_RESOURCE;
    AddResource(&FountainResource);
    Permit();

    /* determine style of invocation */
    if (Process->pr_CLI) {
	/* CLI invocation */
	opts[0] = 0;
	if (rdArgs = ReadArgs(RDARGS_FOUNTAIN, opts, 0)) {
	    ValidateOnly = opts[0];
	    FreeArgs(rdArgs);
	}
    }
    else {
	/* Workbench invocation */
	/* Look in every icon bound to this execution for arguments */
	for (i = 0; (!ValidateOnly) && (i < WBenchMsg->sm_NumArgs); i++) {
	    if (diskObj = GetDiskObject(WBenchMsg->sm_ArgList[i].wa_Name)) {
		if (FindToolType(diskObj->do_ToolTypes,
			TOOLARG_VALIDATE) != 0) {
		    ValidateOnly = TRUE;
		}
		FreeDiskObject(diskObj);
	    }
	}
    }

    /* check for early termination */
    if (SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
	EndGame(RETURN_OK, 0);

    /* more initialization */
    OpenFountainWindow();

    MyGetEnv();

    OpenBullet();			/* does not return if ValidateOnly */
    FindSource();

    OpenMainUI();

    /* event loop */
    for (;;)
    {
	while (IMsg = GT_GetIMsg(Window->UserPort))
	{
	    HandleIM();
	    GT_ReplyIMsg(IMsg);
	}
	signal = Wait((1<<Window->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

	if (signal & SIGBREAKF_CTRL_C)
	    EndGame(RETURN_OK, 0);

	if (signal & SIGBREAKF_CTRL_F)
	{
	    MakeReady ();
	}
    }
}

void MakeReady (void)
{
    if (Window)
    {
	WindowToFront (Window);
	ActivateWindow (Window);
	if (Window->Flags & WFLG_ZOOMED)
	    ZipWindow (Window);
    }
}
