
/* includes */
#include <libraries/asl.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <intuition/intuition.h>
#include <dos/dosextens.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/locale_protos.h>
#include <clib/wb_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/utility_protos.h>
#include <clib/layers_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/wb_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/layers_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"


/*****************************************************************************/


VOID EventLoop(EdDataPtr ed);
VOID OpenLib(EdDataPtr ed, struct Library **libBase, STRPTR libName,
             AppStringsID error);
VOID ProcessCommand(EdDataPtr ed, EdCommand ec);
VOID WriteIcon(EdDataPtr ed, STRPTR name);
BOOL FileRequest(EdDataPtr ed, BOOL mode);
UBYTE DoDiskObj(EdDataPtr ed, STRPTR name);
BOOL CreateDisp(EdDataPtr ed);
VOID DisposeDisp(EdDataPtr ed);
VOID RenderGadg(EdDataPtr ed);


/*****************************************************************************/


#define A_EDIT 0
#define A_USE  1
#define A_SAVE 2


int main(int argc, char *argv)
{
struct ExecBase   *SysBase = (*((struct ExecBase **) 4));
struct Process	  *process;
struct WBStartup  *wbMsg = NULL;
struct WBArg      *wbArg;
struct MsgPort    *rendezVous;
EdDataPtr          ed = NULL;
LONG               failureLevel = RETURN_FAIL;
LONG		   failureCode  = 0;
STRPTR             presetName = NULL;
BYTE               action = A_EDIT;
BPTR		   oldLock;
BPTR               progLock;
AppStringsID       result;
struct Message    *msg;

    process = (struct Process *)SysBase->ThisTask;
    if (!(process->pr_CLI))
    {
        WaitPort(&process->pr_MsgPort);
        wbMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    if (SysBase->LibNode.lib_Version < 37)
    {
	failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
    }
    else
    {
        Forbid();
        if (rendezVous = FindPort(PORT_NAME))
        {
            Signal(rendezVous->mp_SigTask,SIGBREAKF_CTRL_F);
            failureLevel = RETURN_WARN;
        }
        else
        {
            if (ed = AllocVec(sizeof(EdData),MEMF_PUBLIC|MEMF_CLEAR))
            {
                SysBase->ThisTask->tc_UserData    = (APTR)ed;
                ed->ed_RendezVous.mp_Node.ln_Name = PORT_NAME;
                ed->ed_RendezVous.mp_Node.ln_Pri  = -128;
                ed->ed_RendezVous.mp_Node.ln_Type = NT_MSGPORT;
                ed->ed_RendezVous.mp_SigTask      = (struct Task *)process;
                ed->ed_RendezVous.mp_Flags        = PA_IGNORE;
                AddPort(&ed->ed_RendezVous);
            }
            else
            {
                failureLevel = RETURN_FAIL;
                failureCode  = ERROR_NO_FREE_STORE;
            }
        }
        Permit();

        if (ed)
        {
            ed->ed_SysBase           = SysBase;
            ed->ed_TextAttr.ta_Name  = "topaz.font";
            ed->ed_TextAttr.ta_YSize = 8;
            ed->ed_TextAttr.ta_Style = FS_NORMAL;
            ed->ed_TextAttr.ta_Flags = FPF_ROMFONT;
            ed->ed_SaveIcons         = (wbMsg != NULL);
            ed->ed_Quit              = FALSE;
            ed->ed_Cancelled         = FALSE;
            ed->ed_InitFail          = FALSE;
            ed->ed_CheckCmdState     = TRUE;

            ed->ed_AppPort.mp_Node.ln_Type = NT_MSGPORT;
            ed->ed_AppPort.mp_SigTask      = (struct Task *)process;
            ed->ed_AppPort.mp_Flags        = PA_SIGNAL;
            ed->ed_AppPort.mp_SigBit       = AllocSignal(-1);
            NewList(&ed->ed_AppPort.mp_MsgList);

            OpenLib(ed,&ed->ed_IntuitionBase,"intuition.library", MSG_NOTHING);
            OpenLib(ed,&ed->ed_GfxBase,      "graphics.library",  MSG_NOTHING);
            OpenLib(ed,&ed->ed_DOSBase,      "dos.library",       MSG_NOTHING);
            OpenLib(ed,&ed->ed_GadToolsBase, "gadtools.library",  MSG_NOTHING);
            OpenLib(ed,&ed->ed_IconBase,     "icon.library",      MSG_NOTHING);
            OpenLib(ed,&ed->ed_AslBase,      "asl.library",       MSG_NO_ASL_LIBRARY);
            OpenLib(ed,&ed->ed_IFFParseBase, "iffparse.library",  MSG_NO_IFFPARSE_LIBRARY);
            OpenLib(ed,&ed->ed_UtilityBase,  "utility.library",   MSG_NOTHING);
#ifdef LOCKLAYERINFO
            OpenLib(ed,&ed->ed_LayersBase,   "layers.library",    MSG_NOTHING);
#endif

#ifdef OPEN_NIPC
	    OpenLib(ed,&ed->ed_NIPCBase,     "nipc.library",      MSG_NO_NIPC_LIBRARY);
#endif
	    OpenLib(ed,&ed->ed_EnvoyBase,    "envoy.library",	  MSG_NO_ENVOY_LIBRARY);

            if (!ed->ed_InitFail)
            {
                CurrentDir(oldLock=CurrentDir(NULL));

/*                if (ed->ed_LocaleBase = OpenLibrary("locale.library",38))
                {
                    ed->ed_LocaleInfo.li_LocaleBase = ed->ed_LocaleBase;
                    ed->ed_LocaleInfo.li_Catalog = OpenCatalogA(NULL,"Sys/prefs.catalog",NULL);
                }
*/
                result = InitEdData(ed);
                if (result == ES_NORMAL)
                {
                    if (wbMsg)
                    {
                        wbArg = wbMsg->sm_ArgList;
                        CurrentDir(wbArg->wa_Lock);
                        if (progLock = Lock(wbArg->wa_Name,ACCESS_READ))
                        {
                            if (!NameFromLock(progLock,ed->ed_ProgramPath,PROGPATHSIZE))
                            {
                                ed->ed_InitFail = TRUE;
                            }
                            UnLock(progLock);

                            action = DoDiskObj(ed,wbArg->wa_Name);

                            if (wbMsg->sm_NumArgs > 1)
                            {
                                wbArg++;
                                presetName = wbArg->wa_Name;
                                CurrentDir(wbArg->wa_Lock);
                                action = DoDiskObj(ed,presetName);
                            }
                        }
                        else
                        {
                            ed->ed_InitFail = TRUE;
                        }
                    }
                    else
                    {
                        ed->ed_InitFail = TRUE;
                        if (GetProgramName(ed->ed_NameBuf,NAMEBUFSIZE))
                        {
                            strcpy(ed->ed_NameBuf,FilePart(ed->ed_NameBuf));
                            if (GetProgramDir())
                            {
                                if (NameFromLock(GetProgramDir(),ed->ed_ProgramPath,PROGPATHSIZE))
                                {
                                    ed->ed_InitFail = FALSE;
                                }
                            }
                            else
                            {
                                ed->ed_InitFail = FALSE;
                                strcpy(ed->ed_ProgramPath,"SYS:Prefs");
                            }

                            if (!ed->ed_InitFail)
                                ed->ed_InitFail = !AddPart(ed->ed_ProgramPath,ed->ed_NameBuf,PROGPATHSIZE);
                        }

                        memset(ed->ed_Args,0,sizeof(ed->ed_Args));
                        if (ed->ed_RdArgs = ReadArgs(TEMPLATE,ed->ed_Args,NULL))
                        {
                            presetName = (STRPTR)ed->ed_Args[OPT_FROM];

                            if (ed->ed_Args[OPT_USE])
                                action = A_USE;
                            if (ed->ed_Args[OPT_SAVE])
                                action = A_SAVE;
                            if (ed->ed_Args[OPT_EDIT])
                                action = A_EDIT;

                            if (ed->ed_Args[OPT_SCREEN])
                                ed->ed_PubScreenName = (STRPTR)ed->ed_Args[OPT_SCREEN];

                            ProcessArgs(ed,NULL);
                        }
                        else
                        {
                            ed->ed_InitFail = TRUE;
                            PrintFault(IoErr(),NULL);
                        }
                    }

		    if (!ed->ed_InitFail)
		    {
		        CopyPrefs(ed,&ed->ed_PrefsWork,&ed->ed_PrefsDefaults);
                        if (presetName)
                        {
                            if ((result = OpenPrefs(ed,presetName)) != ES_NORMAL)
                            {
                                ShowError2(ed,result);  /* couldn't open requested presets */
                                CopyPrefs(ed,&ed->ed_PrefsWork,&ed->ed_PrefsDefaults);
                            }
                        }
                        else
                        {
                            if ((result = OpenPrefs(ed,ENV_NAME)) != ES_NORMAL)
                            {
                                if (ed->ed_SecondaryResult != ERROR_OBJECT_NOT_FOUND)
                                    ShowError2(ed,result);  /* couldn't load saved prefs */
                                CopyPrefs(ed,&ed->ed_PrefsWork,&ed->ed_PrefsDefaults);
                            }
                        }

                        CopyPrefs(ed,&ed->ed_PrefsInitial,&ed->ed_PrefsWork);

                        if (action == A_SAVE)
                        {
                            ProcessCommand(ed,EC_SAVE);
                            failureLevel = RETURN_OK;
                        }
                        else if (action == A_USE)
                        {
                            ProcessCommand(ed,EC_USE);
                            failureLevel = RETURN_OK;
                        }
                        else
                        {
                            ed->ed_MachineStr = ed->ed_MachineName;
                            if(GetVar("Envoy/AccountsHost",ed->ed_NewMachineName,128,0) > 0)
                            {
                                stccpy(ed->ed_MachineName,ed->ed_NewMachineName,128);
                            }
			    sprintf(ed->ed_WindowTitle,GetString(&ed->ed_LocaleInfo,MSG_GROUPS_WINDOWFMT),ed->ed_MachineName);

                            if (InitDisp(ed))
                            {
                                if (ed->ed_Font = OpenFont(&ed->ed_TextAttr))
                                {
                                    if (ed->ed_Screen = LockPubScreen(ed->ed_PubScreenName))
                                    {
                                        if (ed->ed_VisualInfo = GetVisualInfoA(ed->ed_Screen,NULL))
                                        {
                                            if (ed->ed_DrawInfo = GetScreenDrawInfo(ed->ed_Screen))
                                            {
                                                if (CreateDisp(ed))
                                                {
                                                    /* Try to hook up with a server. */

						    if(!ConnectToServer(ed))
						    {
						    	/* No local server or server in $AccountsHost is down */

                                                        if(HostReq(ed, HREQ_Title, GetString(&ed->ed_LocaleInfo,MSG_GROUPS_SELECTHOST),
                                                               HREQ_Top, ed->ed_Window->TopEdge + ed->ed_Window->BorderTop,
                                                               HREQ_Left,ed->ed_Window->LeftEdge + (ed->ed_Window->Width >> 2),
                                                               HREQ_Width,ed->ed_Window->Width >> 1,
                                                               HREQ_Height,ed->ed_Window->Height - ed->ed_Window->BorderTop - ed->ed_Window->BorderBottom,
                                                               HREQ_Buffer, ed->ed_MachineName,
                                                               HREQ_BuffSize, 128,
                                                               MATCH_ENTITY, "Accounts Server",
                                                               TAG_DONE))
                                                        {
                                                            ConnectToServer(ed);
                                                        }
                                                    }
                                                    if(ed->ed_RemoteEntity)
                                                    {
							sprintf(ed->ed_LoginTitle,GetString(&ed->ed_LocaleInfo,MSG_GROUPS_LOGINFMT),ed->ed_MachineName);
                                                        if(LoginReq(ed, LREQ_Title, ed->ed_LoginTitle,
                                                                     LREQ_NameBuff, ed->ed_AuthUser.UserName,
                                                                     LREQ_NameBuffLen, 32,
                                                                     LREQ_PassBuff, ed->ed_AuthUser.Password,
                                                                     LREQ_PassBuffLen, 32,
                                                                     LREQ_OptimWindow, ed->ed_Window,
                                                                     LREQ_Window, ed->ed_Window,
                                                                     TAG_DONE))
                                      			{
                                                            struct Node *node;
                                      			    LoginUser(ed);

                                                            GetGroupList(ed);
							    RenderGadgets(ed);
                                                        }
						    }
                                                    EventLoop(ed);
                                                    DisposeDisp(ed);

                                                    ed->ed_SecondaryResult = 0;
                                                    failureLevel = RETURN_OK;
                                                }
                                                FreeScreenDrawInfo(ed->ed_Screen,ed->ed_DrawInfo);
                                            }
                                            FreeVisualInfo(ed->ed_VisualInfo);
                                        }
                                        UnlockPubScreen(NULL,ed->ed_Screen);
                                    }
                                    CloseFont(ed->ed_Font);
                                }
                            }
                        }
                    }
                    CleanUpEdData(ed);

                    if (failureCode = ed->ed_SecondaryResult)
                        failureLevel = RETURN_FAIL;
                }
                else
                {
                    ShowError2(ed,result);
                }
                CurrentDir(oldLock);

                FreeAslRequest(ed->ed_FileReq);
            }

            RemPort(&ed->ed_RendezVous);
            if (ed->ed_RdArgs)
                FreeArgs(ed->ed_RdArgs);

	    if (ed->ed_LocaleBase)
                CloseCatalog(ed->ed_LocaleInfo.li_Catalog);

	    while (msg = GetMsg(&ed->ed_AppPort))
	        ReplyMsg(msg);

	    CloseLibrary(ed->ed_EnvoyBase);

#ifdef OPEN_NIPC
	    CloseLibrary(ed->ed_NIPCBase);
#endif

#ifdef LOCKLAYERINFO
            CloseLibrary(ed->ed_LayersBase);
#endif

            CloseLibrary(ed->ed_IntuitionBase);
            CloseLibrary(ed->ed_GfxBase);
            CloseLibrary(ed->ed_DOSBase);
            CloseLibrary(ed->ed_GadToolsBase);
            CloseLibrary(ed->ed_AslBase);
            CloseLibrary(ed->ed_IFFParseBase);
            CloseLibrary(ed->ed_IconBase);
            CloseLibrary(ed->ed_UtilityBase);
            CloseLibrary(ed->ed_LocaleBase);
            CloseLibrary(ed->ed_WorkbenchBase);
            FreeSignal(ed->ed_AppPort.mp_SigBit);
            FreeVec(ed);
        }
    }

    if (wbMsg)
    {
    	Forbid();
    	ReplyMsg(wbMsg);
    }

    process->pr_Result2 = failureCode;  /* can't use SetIoErr() cause DOS ain't open! */
    return(failureLevel);
}


/*****************************************************************************/


#define SysBase ed->ed_SysBase


/*****************************************************************************/


VOID OpenLib(EdDataPtr ed, struct Library **libBase, STRPTR libName,
             AppStringsID error)
{
    if (!ed->ed_InitFail)
    {
        if (!(*libBase = OpenLibrary(libName,37)))
        {
            ed->ed_InitFail = TRUE;
            if (error != MSG_NOTHING)
                ShowError1(ed,error);
        }
    }
}


/*****************************************************************************/


UBYTE DoDiskObj(EdDataPtr ed, STRPTR name)
{
struct DiskObject *diskObj;
STRPTR             tvalue;
UBYTE              action = A_EDIT;

    if (diskObj = GetDiskObject(name))
    {
        if (tvalue = FindToolType(diskObj->do_ToolTypes,"ACTION"))
        {
            if (MatchToolValue(tvalue,"EDIT"))
                action = A_EDIT;
            if (MatchToolValue(tvalue,"USE"))
                action = A_USE;
            if (MatchToolValue(tvalue,"SAVE"))
                action = A_SAVE;
        }

        if (tvalue = FindToolType(diskObj->do_ToolTypes,"CREATEICONS"))
        {
            if (!Stricmp(tvalue,"YES"))
                ed->ed_SaveIcons = TRUE;
            else if (!Stricmp(tvalue,"NO"))
                ed->ed_SaveIcons = FALSE;
        }

        if (tvalue = FindToolType(diskObj->do_ToolTypes,"PUBSCREEN"))
        {
            ed->ed_PubScreenName = ed->ed_ScreenNameBuf;
            strncpy(ed->ed_ScreenNameBuf,tvalue,sizeof ed->ed_ScreenNameBuf);
        }

        ProcessArgs(ed,diskObj);
        FreeDiskObject(diskObj);
    }

    return(action);
}


/*****************************************************************************/


VOID ProcessCommand(EdDataPtr ed, EdCommand ec)
{
EdStatus result = ES_NORMAL;

    if (ed->ed_Window)
        SyncTextGadgets(ed);

    ed->ed_CheckCmdState = TRUE;
    switch (ec)
    {
        case EC_ACTIVATE : ScreenToFront(ed->ed_Window->WScreen);
                           if (!(ed->ed_Window->Flags & WFLG_BACKDROP))
                               WindowToFront(ed->ed_Window);
                           ActivateWindow(ed->ed_Window);
            		   if (ed->ed_Window->Flags & ZOOMED)
                	       ZipWindow(ed->ed_Window);
                           break;

        case EC_OPEN     : if (FileRequest(ed,TRUE))
                           {
                               result = OpenPrefs(ed,ed->ed_NameBuf);
                               RenderGadg(ed);
                           }
                           break;

        case EC_SAVE     : if ((result = SavePrefs(ed,ARC_NAME)) == ES_NORMAL)
                           {
                               DisposeDisp(ed);
                               ed->ed_Quit = TRUE;
			       result = SavePrefs(ed,ENV_NAME);
                           }
                           break;

        case EC_USE      : DisposeDisp(ed);
                           ed->ed_Quit = TRUE;
                           result = SavePrefs(ed,ENV_NAME);
                           break;

        case EC_SAVEAS   : if (FileRequest(ed,FALSE))
                               if ((result = SavePrefs(ed,ed->ed_NameBuf)) == ES_NORMAL)
                                   if (ed->ed_SaveIcons)
                                       WriteIcon(ed,ed->ed_NameBuf);

                           break;

        case EC_RESETALL : CopyPrefs(ed,&ed->ed_PrefsWork,&ed->ed_PrefsDefaults);
                           RenderGadg(ed);
                           break;

        case EC_LASTSAVED: result = OpenPrefs(ed,ARC_NAME);
                           RenderGadg(ed);
                           break;

        case EC_RESTORE  : CopyPrefs(ed,&ed->ed_PrefsWork,&ed->ed_PrefsInitial);
                           RenderGadg(ed);
                           break;

        case EC_CANCEL   : ed->ed_Quit      = TRUE;
                           ed->ed_Cancelled = TRUE;
                           break;

        case EC_SAVEICONS: ed->ed_SaveIcons = !ed->ed_SaveIcons;
                           break;

        case EC_NOP      : break;

        default          : ProcessSpecialCommand(ed,ec);
                           break;
    }

    if (result != ES_NORMAL)
        ShowError2(ed,result);
}


/*****************************************************************************/


VOID GetCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    switch (ec)
    {
        case EC_OPEN     :
        case EC_SAVE     :
        case EC_USE      :
        case EC_SAVEAS   :
        case EC_RESETALL :
        case EC_LASTSAVED:
        case EC_RESTORE  :
        case EC_CANCEL   :
        case EC_NOP      : state->cs_Available = TRUE;
                           state->cs_Selected  = TRUE;
                           break;


        case EC_SAVEICONS: state->cs_Available = TRUE;
                           state->cs_Selected  = ed->ed_SaveIcons;
                           break;

        default          : GetSpecialCmdState(ed,ec,state);
                           break;
    }
}


/*****************************************************************************/


VOID ProcessItem(EdDataPtr ed, struct MenuItem *item)
{
CmdState state;

    if ((EdCommand)MENU_USERDATA(item) != EC_NOP)
    {
        GetCmdState(ed,(EdCommand)MENU_USERDATA(item),&state);

        if (state.cs_Available)
            item->Flags |= ITEMENABLED;
        else
            item->Flags &= ~ITEMENABLED;

        if (state.cs_Selected)
            item->Flags |= CHECKED;
        else
            item->Flags &= ~CHECKED;
    }
}


/*****************************************************************************/


VOID EventLoop(EdDataPtr ed)
{
struct IntuiMessage *intuiMsg;
struct MenuItem     *menuItem;
struct MenuItem     *subItem;
struct Menu         *menu;
UWORD                menuNum;
struct Gadget       *gadget;
ULONG                sigs;
struct WBArg        *wbArg;
struct AppMessage   *appMsg;
struct Gadget       *activeGad;
BPTR                 oldLock;
enum EdStatus        result;

    while (!ed->ed_Quit)
    {
        if (ed->ed_CheckCmdState && ((SetSignal(0,0) & (1L << window->UserPort->mp_SigBit)) == 0))
        {
            menu = window->MenuStrip;
            ClearMenuStrip(window);

            while (menu)
            {
                menuItem = menu->FirstItem;
                while (menuItem)
                {
                    subItem = menuItem->SubItem;
                    while (subItem)
                    {
                        ProcessItem(ed,subItem);
                        subItem = subItem->NextItem;
                    }
                    ProcessItem(ed,menuItem);
                    menuItem = menuItem->NextItem;
                }
                menu = menu->NextMenu;
            }

            ResetMenuStrip(window,ed->ed_Menus);
            ed->ed_CheckCmdState = FALSE;
        }

        sigs = Wait(1L<<window->UserPort->mp_SigBit | 1L<<ed->ed_AppPort.mp_SigBit | SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_C);

        if (sigs & SIGBREAKF_CTRL_F)
            ProcessCommand(ed,EC_ACTIVATE);

        if (sigs & SIGBREAKF_CTRL_C)
        {
            ed->ed_Quit      = TRUE;
            ed->ed_Cancelled = TRUE;
        }

	while ((!ed->ed_Quit) && (appMsg = (struct AppMessage *)GetMsg(&ed->ed_AppPort)))
	{
            if (appMsg->am_NumArgs)
            {
                if (appMsg->am_Type == AMTYPE_APPWINDOW)
                    ActivateWindow(window);
                wbArg = appMsg->am_ArgList;
                oldLock = CurrentDir(wbArg->wa_Lock);
                NameFromLock(wbArg->wa_Lock,ed->ed_NameBuf,NAMEBUFSIZE);
                AddPart(ed->ed_NameBuf,wbArg->wa_Name,NAMEBUFSIZE);
                if ((result = OpenPrefs(ed,wbArg->wa_Name)) != ES_NORMAL)
                    ShowError2(ed,result);
                RenderGadg(ed);
                CurrentDir(oldLock);
            }
            else
            {
                ProcessCommand(ed,EC_ACTIVATE);
            }
            ReplyMsg(appMsg);
        }

	while ((!ed->ed_Quit) && (intuiMsg = GT_GetIMsg(window->UserPort)))
	{
            ed->ed_CurrentMsg = *intuiMsg;
	    GT_ReplyIMsg(intuiMsg);

	    gadget = (struct Gadget *)ed->ed_CurrentMsg.IAddress;
	    switch (ed->ed_CurrentMsg.Class)
	    {
		case IDCMP_REFRESHWINDOW:
#ifdef LOCKLAYERINFO
					  LockLayerInfo(&window->WScreen->LayerInfo);
#endif

                                          GT_BeginRefresh(window);
                                          RenderDisplay(ed);
                                          GT_EndRefresh(window,TRUE);

#ifdef LOCKLAYERINFO
					  UnlockLayerInfo(&window->WScreen->LayerInfo);
#endif
                                          break;

		case IDCMP_CLOSEWINDOW  : ProcessCommand(ed,EC_CANCEL);
                                          break;

		case IDCMP_MENUPICK     : menuNum = intuiMsg->Code;
                                          while ((menuNum != MENUNULL) && (!ed->ed_Quit))
                                          {
                                              menuItem = ItemAddress(intuiMsg->IDCMPWindow->MenuStrip,menuNum);
                                              ProcessCommand(ed,(EdCommand)MENU_USERDATA(menuItem));
                                              menuNum = menuItem->NextSelect;
                                          }
                                          break;

		case IDCMP_GADGETDOWN   : activeGad = gadget;
		case IDCMP_MOUSEMOVE    : ProcessCommand(ed,(EdCommand)activeGad->GadgetID);
                                          break;

		case IDCMP_GADGETUP     : ProcessCommand(ed,(EdCommand)gadget->GadgetID);
                                          break;

                default                 : ProcessCommand(ed,GetCommand(ed));
	    }
	}
    }
}


/*****************************************************************************/


BOOL CreateDisp(EdDataPtr ed)
{
struct Process *process;

    if (CreateDisplay(ed))
    {
        SetFont(window->RPort,ed->ed_Font);
        SetMenuStrip(window,ed->ed_Menus);
        GT_RefreshWindow(window,NULL);
        RenderDisplay(ed);

        if (ed->ed_WorkbenchBase = OpenLibrary("workbench.library",37))
        {
            ed->ed_AppWindow = AddAppWindowA(0,NULL,window,&ed->ed_AppPort,NULL);
/*
            ed->ed_AppMenu   = AddAppMenuItemA(0,NULL,window->Title,&ed->ed_AppPort,NULL);
*/
        }

        process = (struct Process *)SysBase->ThisTask;
        ed->ed_OldWindowPtr = process->pr_WindowPtr;
        process->pr_WindowPtr = ed->ed_Window;
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID DisposeDisp(EdDataPtr ed)
{
struct Process *process;

    if (ed->ed_Window)
    {
        process = (struct Process *)SysBase->ThisTask;
        process->pr_WindowPtr = ed->ed_OldWindowPtr;
/*
	if (ed->ed_AppMenu)
	    RemoveAppMenuItem(ed->ed_AppMenu);
*/
	if (ed->ed_AppWindow)
	    RemoveAppWindow(ed->ed_AppWindow);

        DisposeDisplay(ed);
        ed->ed_Window = NULL;
    }
}


/*****************************************************************************/


VOID RenderGadg(EdDataPtr ed)
{
    if (ed->ed_Window)
        RenderGadgets(ed);
}


/*****************************************************************************/


UWORD far PrefImageData[] = {
    0x0000, 0x0000, 0x0004, 0x0000,
    0x0000, 0x0000, 0x0001, 0x0000,
    0x0000, 0x07FF, 0x8000, 0x4000,
    0x0000, 0x1800, 0x6000, 0x1000,
    0x0000, 0x20FC, 0x1000, 0x0800,
    0x0000, 0x4102, 0x0800, 0x0C00,
    0x0000, 0x4082, 0x0800, 0x0C00,
    0x0000, 0x4082, 0x0800, 0x0C00,
    0x0000, 0x2104, 0x0800, 0x0C00,
    0x0000, 0x1E18, 0x1000, 0x0C00,
    0x0000, 0x0060, 0x2000, 0x0C00,
    0x0000, 0x0080, 0xC000, 0x0C00,
    0x0000, 0x0103, 0x0000, 0x0C00,
    0x0000, 0x021C, 0x0000, 0x0C00,
    0x0000, 0x0108, 0x0000, 0x0C00,
    0x0000, 0x00F0, 0x0000, 0x0C00,
    0x0000, 0x0108, 0x0000, 0x0C00,
    0x0000, 0x0108, 0x0000, 0x0C00,
    0x4000, 0x00F0, 0x0000, 0x0C00,
    0x1000, 0x0000, 0x0000, 0x0C00,
    0x0400, 0x0000, 0x0000, 0x0C00,
    0x01FF, 0xFFFF, 0xFFFF, 0xFC00,
    0x0000, 0x0000, 0x0000, 0x0000,
/**/
    0xFFFF, 0xFFFF, 0xFFF8, 0x0000,
    0xD555, 0x5555, 0x5556, 0x0000,
    0xD555, 0x5000, 0x5555, 0x8000,
    0xD555, 0x47FF, 0x9555, 0x6000,
    0xD555, 0x5F03, 0xE555, 0x5000,
    0xD555, 0x3E55, 0xF555, 0x5000,
    0xD555, 0x3F55, 0xF555, 0x5000,
    0xD555, 0x3F55, 0xF555, 0x5000,
    0xD555, 0x5E53, 0xF555, 0x5000,
    0xD555, 0x4147, 0xE555, 0x5000,
    0xD555, 0x551F, 0xD555, 0x5000,
    0xD555, 0x557F, 0x1555, 0x5000,
    0xD555, 0x54FC, 0x5555, 0x5000,
    0xD555, 0x55E1, 0x5555, 0x5000,
    0xD555, 0x54F5, 0x5555, 0x5000,
    0xD555, 0x5505, 0x5555, 0x5000,
    0xD555, 0x54F5, 0x5555, 0x5000,
    0xD555, 0x54F5, 0x5555, 0x5000,
    0x3555, 0x5505, 0x5555, 0x5000,
    0x0D55, 0x5555, 0x5555, 0x5000,
    0x0355, 0x5555, 0x5555, 0x5000,
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
};


STRPTR far TTypes[] =
{
    "ACTION=USE",
    NULL
};


struct Image far PrefImage =
{
    0, 0,		/* LeftEdge, TopEdge */
    54, 23,		/* Width, Height */
    2,			/* Depth */
    PrefImageData,	/* ImageData */
    0x033, 0x00,	/* PlanePick, PlaneOnOff */
    NULL		/* NextImage */
};

struct DiskObject far PrefObject =
{
    WB_DISKMAGIC,
    WB_DISKVERSION,

    NULL,
    0, 0,
    54, 23,
    GADGHBOX|GADGIMAGE,
    RELVERIFY|GADGIMMEDIATE,
    BOOLGADGET,
    (APTR)&PrefImage,
    NULL,
    NULL,
    NULL, NULL, NULL,
    NULL,

    WBPROJECT,
    NULL,
    TTypes,
    NO_ICON_POSITION,
    NO_ICON_POSITION,
    NULL, NULL, NULL
};


VOID WriteIcon(EdDataPtr ed,STRPTR name)
{
struct DiskObject diskObj;

    diskObj                = PrefObject;
    diskObj.do_DefaultTool = ed->ed_ProgramPath;
    PutDiskObject(name,&diskObj);
}


/*****************************************************************************/


#undef SysBase
#define ASM __asm
#define REG(x) register __## x

VOID ASM IntuiHook(REG(a0) struct Hook *hook,
                   REG(a2) struct FileRequester *fr,
                   REG(a1) struct IntuiMessage *intuiMsg)
{
EdDataPtr        ed;
struct ExecBase *SysBase = (*((struct ExecBase **) 4));

    if (intuiMsg->Class == IDCMP_REFRESHWINDOW)
    {
        ed = (EdDataPtr)SysBase->ThisTask->tc_UserData;
        GT_BeginRefresh(window);
        RenderDisplay(ed);
        GT_EndRefresh(window,TRUE);
    }
}


BOOL FileRequest(EdDataPtr ed, BOOL mode)
{
BOOL        success;
struct Hook hook;

    hook.h_Entry = (ULONG (*)()) &IntuiHook;    /* what should this be cast to to avoid warnings?? */

    if (!ed->ed_FileReq)
    {
        if (!(ed->ed_FileReq = AllocPrefsRequest(ed, ASL_FileRequest,
                                                 ASLFR_InitialDrawer,   PRESET_DIR,
                                                 ASLFR_InitialFile,     PRESET_NAME,
                                                 ASLFR_InitialLeftEdge, window->LeftEdge+12,
                                                 ASLFR_InitialTopEdge,  window->TopEdge+12,
                                                 ASLFR_Window,          ed->ed_Window,
                                                 ASLFR_SleepWindow,     TRUE,
                                                 ASLFR_RejectIcons,     TRUE,
                                                 TAG_DONE)))
        {
            ShowError2(ed,ES_NO_MEMORY);
            return(FALSE);
        }
    }

    if (mode)
	success = RequestPrefsFile(ed,ASLFR_TitleText,    GetString(&ed->ed_LocaleInfo,MSG_REQ_LOAD),
                                      ASLFR_DoSaveMode,   FALSE,
                                      ASLFR_IntuiMsgFunc, &hook,
	                              TAG_DONE);
    else
	success = RequestPrefsFile(ed,ASLFR_TitleText,    GetString(&ed->ed_LocaleInfo,MSG_REQ_SAVE),
                                      ASLFR_DoSaveMode,   TRUE,
                                      ASLFR_IntuiMsgFunc, &hook,
	                              TAG_DONE);

    if (success)
    {
	stccpy(ed->ed_NameBuf,ed->ed_FileReq->rf_Dir, NAMEBUFSIZE);
	AddPart(ed->ed_NameBuf,ed->ed_FileReq->rf_File, NAMEBUFSIZE);
	return(TRUE);
    }

    return(FALSE);
}
