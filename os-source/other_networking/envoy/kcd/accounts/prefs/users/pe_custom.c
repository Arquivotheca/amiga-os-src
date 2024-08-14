
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/keymap.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/icon_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"
#include <devices/sana2.h>
#include <pragmas/nipc_pragmas.h>
#include <clib/nipc_protos.h>
#include <envoy/accounts.h>

#define SysBase ed->ed_SysBase

typedef ULONG (*HOOK_FUNC)();
ULONG __asm passwordHook(register __a0 struct Hook *hook,
			          register __a2 struct SGWork *sgw,
			          register __a1 ULONG *msg);

/*****************************************************************************/


/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,   /* version */
    0,   /* type    */
    0    /* flags   */
};


/*****************************************************************************/


#define WRITE_ALL    0
#define WRITE_WB     1
#define WRITE_SYS    2
#define WRITE_SCREEN 3


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
    EdStatus error = ES_NO_MEMORY;

    InitPrefs(&ed->ed_PrefsDefaults);
    InitPrefs(&ed->ed_PrefsWork);
    InitPrefs(&ed->ed_PrefsInitial);

    ed->ed_CurrentUser = NULL;
    ed->ed_MachineStr = ed->ed_MachineName;
    ed->ed_LocalEntity = NULL;
    ed->ed_RemoteEntity = NULL;

    ed->ed_PWEditHook.h_Entry = (HOOK_FUNC)&passwordHook;
    ed->ed_PWEditHook.h_SubEntry = (HOOK_FUNC)ed->ed_TmpPassStr;

    ed->ed_V39 = (ed->ed_IntuitionBase->lib_Version >= 39) ? TRUE : FALSE;

    if(ed->ed_Transaction = AllocTransactionA(NULL))
    {
	if(ed->ed_LocalEntity = CreateEnt(ed, ENT_Signal,ed->ed_EntitySignal,
					  TAG_DONE))
	{
	    error = ES_NORMAL;
	    GetHostName(NULL,ed->ed_MachineName,127);
	}
    }
    return(error);
}

void InitPrefs(struct ExtPrefs *prefs)
{
    NewList(&prefs->ep_UserList);
    NewList(&prefs->ep_GroupList);
}

/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreePrefs(ed, &ed->ed_PrefsWork);

    FreeAslRequest(ed->ed_FileReq);

    if(ed->ed_RemoteEntity)
    	LoseEntity(ed->ed_RemoteEntity);
    if(ed->ed_LocalEntity)
    	DeleteEntity(ed->ed_LocalEntity);
    if(ed->ed_Transaction)
    	FreeTransaction(ed->ed_Transaction);


}

VOID FreePrefs(EdDataPtr ed, struct ExtPrefs *prefs)
{
    FreeList(ed, &prefs->ep_UserList,sizeof(struct UserNode));
    FreeList(ed, &prefs->ep_GroupList,sizeof(struct GroupNode));
}

VOID FreeList(EdDataPtr ed, struct List *list, ULONG nodeSize)
{
struct Node *node;

    while(node = RemHead(list))
        FreeMem(node, nodeSize);
}

/*****************************************************************************/

/* I grabbed this from 2.0 Libs RKM, pg. 275 */
UWORD __chip waitPointer[] =
{
   0x0000,0x0000,
   0x0400,0x07c0,
   0x0000,0x07c0,
   0x0100,0x0380,
   0x0000,0x07e0,
   0x07c0,0x1ff8,
   0x1ff0,0x3fec,
   0x3ff8,0x7fde,
   0x3ff8,0x7fbe,
   0x7ffc,0xff7f,
   0x7efc,0xffff,
   0x7ffc,0xffff,
   0x3ff8,0x7ffe,
   0x3ff8,0x7ffe,
   0x1ff0,0x3ffc,
   0x07c0,0x1ff8,
   0x0000,0x07e0,
   0x0000,0x0000
};

struct Requester * SetBusyPointer(EdDataPtr ed, struct Window *win)
{
    ULONG tags[3]={WA_BusyPointer,TRUE,TAG_DONE};
    struct Requester *r;

    r = (struct Requester *) AllocMem(sizeof(struct Requester),MEMF_PUBLIC);
    if (r)
    {
        InitRequester(r);

        Request(r,win);
        if (IntuitionBase->lib_Version < 39)
            SetPointer(win,waitPointer,16,16,-6,0);
        else
            SetWindowPointerA(win,(struct TagItem *) tags);
    }
    return(r);
}


void ResetBusyPointer(EdDataPtr ed, struct Requester *r, struct Window *win)
{
   ULONG tags[5]={WA_Pointer,NULL,WA_BusyPointer,FALSE,TAG_DONE};

    if (IntuitionBase->lib_Version < 39)
        ClearPointer(win);
    else
        SetWindowPointerA(win,(struct TagItem *) tags);
    EndRequest(r,win);
    FreeMem(r,sizeof(struct Requester));
}

/*****************************************************************************/


#define NW_WIDTH     620
#define NW_HEIGHT    175
#define NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define NW_FLAGS     (WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,       EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_QUIT,       EC_CANCEL, 0},

    {NM_TITLE,  MSG_SETTINGS_MENU,      EC_NOP, 0},
      {NM_ITEM, MSG_SETTINGS_SET_HOST,  EC_SETHOST, 0},
      {NM_ITEM, MSG_SET_LOGIN,	    	EC_SETLOGIN, 0},

    {NM_END,    MSG_NOTHING,            EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {

    /* Main Panel */

    {BUTTON_KIND,   7,   146,  130, 14, MSG_USERS_ADD_USER,         EC_ADDUSER,           0},
    {BUTTON_KIND,   137, 146,  130, 14, MSG_USERS_REM_USER,	    EC_REMUSER,	          0},

    {BUTTON_KIND,   400, 4,    216, 14, MSG_USERS_SETHOST,	    EC_SETHOST,		  0},
    {TEXT_KIND,     400, 18,   216, 14, MSG_USERS_HOSTNAME,	    EC_NOP,		  PLACETEXT_LEFT},
    {STRING_KIND,   400, 48,   216, 14, MSG_USERS_AUTHPW,	    EC_AUTHPW,		  0},

    {STRING_KIND,   7,   0,    260, 14, 0,	                    EC_USERNAME,          0},
    {STRING_KIND,   400, 18,   216, 14, MSG_USERS_PASSWORD,	    EC_PASSWORD,	  0},

    {CHECKBOX_KIND, 588, 68,   26,  11, MSG_USERS_ADMINPRIVS,	    EC_ADMIN,		  0},
    {CHECKBOX_KIND, 588, 82,  26,  11, MSG_USERS_GROUPPRIVS,	    EC_ADMINGROUPS,	  0},
    {CHECKBOX_KIND, 588, 96,  26,  11, MSG_USERS_ADMINNAME,	    EC_ADMINNAME,	  0},
    {CHECKBOX_KIND, 588, 110,  26,  11, MSG_USERS_ADMINPASSWORD,    EC_ADMINPASSWORD,	  0},
    {CHECKBOX_KIND, 588, 124,  26,  11, MSG_USERS_NEEDPW,	    EC_ADMINNEEDPW,	  0},

    {LISTVIEW_KIND, 7,   18,   260, 128, MSG_USERS_USERLIST,	    EC_USERLIST,	  PLACETEXT_ABOVE},

    {BUTTON_KIND,   400, 34,   216, 14, MSG_SET_DEFGROUP,	    EC_SETGROUP,	  0},
    {TEXT_KIND,	    400, 50,   216, 14, MSG_PRIMARY_GROUP,          EC_NOP,		  0}
};

/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_LastAdded = NULL; /* CreateContext(&ed->ed_Gadgets); */

    RenderGadgets(ed);

    if ((ed->ed_LastAdded)
    &&  (ed->ed_Menus = CreatePrefsMenus(ed,EM))
    &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,  NW_WIDTH,
                                            WA_InnerHeight, NW_HEIGHT,
                                            WA_MinWidth,    NW_MINWIDTH,
                                            WA_MinHeight,   NW_MINHEIGHT,
                                            WA_MaxWidth,    NW_MAXWIDTH,
                                            WA_MaxHeight,   NW_MAXHEIGHT,
                                            WA_IDCMP,       NW_IDCMP,
                                            WA_Flags,       NW_FLAGS,
                                            WA_Zoom,        zoomSize,
                                            WA_AutoAdjust,  TRUE,
                                            WA_PubScreen,   ed->ed_Screen,
                                            WA_Title,       ed->ed_WindowTitle,
                                            WA_NewLookMenus,TRUE,
                                            WA_Gadgets,     ed->ed_Gadgets,
                                            TAG_DONE)))
    {
        return(TRUE);
    }

    DisposeDisplay(ed);
    return(FALSE);
}


/*****************************************************************************/


VOID DisposeDisplay(EdDataPtr ed)
{
    if (ed->ed_Window)
    {
        ClearMenuStrip(ed->ed_Window);
        CloseWindow(ed->ed_Window);
    }
    FreeMenus(ed->ed_Menus);
    FreeGadgets(ed->ed_Gadgets);
}


/*****************************************************************************/


VOID CenterLine(EdDataPtr ed, struct Window *wp, AppStringsID id,
                UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(wp->RPort,(w-TextLength(wp->RPort,str,len)) / 2 + wp->BorderLeft + x,wp->BorderTop+y);
    Text(wp->RPort,str,len);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}

/*****************************************************************************/

void ClearGadPtrs(EdDataPtr ed)
{
    ULONG *gad;
    for (gad = (ULONG *) &ed->ed_UserList; gad <= (ULONG *)&ed->ed_UserNeedsPassword; gad++)
    {
        *gad = NULL;
    }
}

/*****************************************************************************/

VOID RenderGadgets(EdDataPtr ed)
{
UWORD           i;
struct Node    *node;
BOOL able, checked;

    if(!ed->ed_LastAdded)
    {
    	ClearGadPtrs(ed);
    	ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
    }

    if(ed->ed_V39)
    {
        i = 0;
        node = ed->ed_PrefsWork.ep_UserList.lh_Head;
        while(node->ln_Succ)
        {
            if(node == (struct Node *)ed->ed_CurrentUser)
                break;
            i++;
            node = node->ln_Succ;
        }
        if(!node->ln_Succ)
            i=~0;

        ed->ed_UserList = DoPrefsGadget(ed,&EG[12],ed->ed_UserList,
                                          GTLV_Selected,     i,
                                          GTLV_ShowSelected, 0,
                                          GTLV_Labels,       &ed->ed_PrefsWork.ep_UserList,
                                          LAYOUTA_SPACING,   1,
                                          GTLV_ScrollWidth,  18,
                                          TAG_DONE);
    }
    else
    {
        ed->ed_UserList = DoPrefsGadget(ed,&EG[12],ed->ed_UserList,
                                          GTLV_Labels,       &ed->ed_PrefsWork.ep_UserList,
                                          LAYOUTA_SPACING,   1,
                                          GTLV_ScrollWidth,  18,
                                          TAG_DONE);
    }

    EG[5].eg_TopEdge = 18 + ed->ed_UserList->Height;

    EG[0].eg_TopEdge = EG[1].eg_TopEdge = EG[5].eg_TopEdge + 14;

    ed->ed_AddUser = DoPrefsGadget(ed,&EG[0],ed->ed_AddUser,
    					GA_Disabled, !(ed->ed_AuthUser.Flags & UFLAGF_AdminAll),
    					TAG_DONE);

    ed->ed_RemUser = DoPrefsGadget(ed,&EG[1],ed->ed_RemUser,
    					GA_Disabled, !(ed->ed_AuthUser.Flags & UFLAGF_AdminAll),
    					TAG_DONE);

//    DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);

//    ed->ed_HostName = DoPrefsGadget(ed,&EG[3],ed->ed_HostName,
//    					GTTX_Text,ed->ed_MachineName,
//    					GTTX_CopyText, TRUE,
//    					GTTX_Border, TRUE,
//    					TAG_DONE);


    able = FALSE;

    if(ed->ed_CurrentUser)
    {
        if(ed->ed_AuthUser.Flags & UFLAGF_AdminAll)
            able = TRUE;
        else if((ed->ed_AuthUser.UserID == ed->ed_CurrentUser->Data.UserID) &&
                (ed->ed_AuthUser.Flags & UFLAGF_AdminName))
            able = TRUE;
        stccpy(ed->ed_UserStr,ed->ed_CurrentUser->Data.UserName,32);
    }
    else
    {
        ed->ed_UserStr[0]=0;
    }
    ed->ed_UserName = DoPrefsGadget(ed,&EG[5],ed->ed_UserName,
                                    GTST_String, ed->ed_UserStr,
                                    GTST_MaxChars, 31,
                                    GA_Disabled, !able,
                                    TAG_DONE);

    able = FALSE;
    if(ed->ed_CurrentUser)
    {
        if(ed->ed_AuthUser.Flags & UFLAGF_AdminAll)
            able = TRUE;
        else if((ed->ed_AuthUser.UserID == ed->ed_CurrentUser->Data.UserID) &&
                (ed->ed_AuthUser.Flags & UFLAGF_AdminPassword) &&
                (ed->ed_AuthUser.Flags & UFLAGF_NeedsPassword))
            able = TRUE;
    }
    if(able)
    {
        stccpy(ed->ed_TmpPassStr,ed->ed_CurrentUser->Data.Password,32);
        stccpy(ed->ed_PassStr,GetString(&ed->ed_LocaleInfo,MSG_USERS_PWBULLETS),strlen(ed->ed_CurrentUser->Data.Password)+1);
    }
    else
    {
    	ed->ed_PassStr[0]=0;
        ed->ed_TmpPassStr[0]=0;
    }
    ed->ed_UserPassword = DoPrefsGadget(ed,&EG[6],ed->ed_UserPassword,
                                    GTST_String,ed->ed_PassStr,
                                    GTST_MaxChars, 31,
                                    GA_Disabled, !able,
                                    GTST_EditHook, (ULONG)&ed->ed_PWEditHook,
                                    TAG_DONE);
    able = checked = FALSE;
    if(ed->ed_CurrentUser)
    {
        if(ed->ed_AuthUser.Flags & UFLAGF_AdminAll)
            able = TRUE;
        checked = (ed->ed_CurrentUser->Data.Flags & UFLAGF_AdminAll);
    }

    ed->ed_UserIsAdmin = DoPrefsGadget(ed,&EG[7],ed->ed_UserIsAdmin,
                                       GTCB_Checked, checked,
                                       GA_Disabled, !able,
                                       TAG_DONE);

    if(ed->ed_CurrentUser)
    {
        checked = (ed->ed_CurrentUser->Data.Flags & UFLAGF_AdminGroups);
    }
    ed->ed_UserEdGroups = DoPrefsGadget(ed,&EG[8],ed->ed_UserEdGroups,
                                        GTCB_Checked, checked,
                                        GA_Disabled,  !able,
                                        TAG_DONE);

    if(ed->ed_CurrentUser)
        checked = (ed->ed_CurrentUser->Data.Flags & UFLAGF_AdminName);

    ed->ed_UserEdName = DoPrefsGadget(ed,&EG[9],ed->ed_UserEdName,
                                      GTCB_Checked, checked,
                                      GA_Disabled, !able,
                                      TAG_DONE);

    if(ed->ed_CurrentUser)
        checked = (ed->ed_CurrentUser->Data.Flags & UFLAGF_AdminPassword);

    ed->ed_UserEdPassword = DoPrefsGadget(ed,&EG[10],ed->ed_UserEdPassword,
                                          GTCB_Checked, checked,
                                          GA_Disabled, !able,
                                          TAG_DONE);

    if(ed->ed_CurrentUser)
        checked = (ed->ed_CurrentUser->Data.Flags & UFLAGF_NeedsPassword);

    ed->ed_UserNeedsPassword = DoPrefsGadget(ed,&EG[11],ed->ed_UserNeedsPassword,
                                             GTCB_Checked, checked,
                                             GA_Disabled, !able,
                                             TAG_DONE);


    ed->ed_GroupButton = DoPrefsGadget(ed,&EG[13],ed->ed_GroupButton,
    					  GA_Disabled, !able,
    					  TAG_DONE);

    if(ed->ed_CurrentUser)
        GetGroupName(ed);
    else
        ed->ed_GroupStr[0]='\0';

    ed->ed_GroupText = DoPrefsGadget(ed,&EG[14],ed->ed_GroupText,
                                     GTTX_Text, ed->ed_GroupStr,
                                     GTTX_CopyText, TRUE,
                                     GTTX_Border, TRUE,
                                     TAG_DONE);



}

/*****************************************************************************/

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
BOOL refresh,newhost=FALSE;
UWORD icode;
struct Node *node;
struct UserNode *user;
ULONG result;
struct Gadget *act = NULL;

    refresh = FALSE;

    icode = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
    	case EC_ADDUSER		: if(user = (struct UserNode *)AllocMem(sizeof(struct UserNode),MEMF_CLEAR|MEMF_PUBLIC))
    				  {
                                      stccpy(ed->ed_AdminUser.User.UserName,"NewUser",32);
                                      stccpy(ed->ed_AdminUser.User.Password,"Password",32);
                                      ed->ed_AdminUser.User.PrimaryGroupID = 0;
                                      ed->ed_AdminUser.User.Flags = 0;

                                      result = DoServerCmd(ed, ACMD_AddUser);
                                      if(!result)
                                      {
                                          stccpy(user->Data.UserName,ed->ed_AdminUser.User.UserName,32);
                                          stccpy(user->Data.Password,ed->ed_AdminUser.User.Password,32);
                                          user->Data.PrimaryGroupID = ed->ed_AdminUser.User.PrimaryGroupID;
                                          user->Data.UserID = ed->ed_AdminUser.User.UserID;
                                          user->Data.Flags = ed->ed_AdminUser.User.Flags;
                                          user->Link.ln_Name = user->Data.UserName;
                                          ed->ed_CurrentUser = user;
                                          DoPrefsGadget(ed,&EG[9],ed->ed_UserList,
                                          		GTLV_Labels, ~0,
                                          		TAG_DONE);
                                          AddNodeSorted(ed, &ed->ed_PrefsWork.ep_UserList,(struct Node *)user);
                                          refresh = TRUE;
                                      }
                                      else
                                      	  FreeMem(user,sizeof(struct UserNode));
                                  }
                                  break;

	case EC_REMUSER		: if(ed->ed_CurrentUser)
				  {
				      CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));

    				      result = DoServerCmd(ed, ACMD_RemUser);
                                      if(!result)
                                      {
                                          DoPrefsGadget(ed,&EG[9],ed->ed_UserList,
                                          		GTLV_Labels, ~0,
                                          		TAG_DONE);
                                          Remove((struct Node *)ed->ed_CurrentUser);
                                          FreeMem(ed->ed_CurrentUser,sizeof(struct UserNode));
                                          ed->ed_CurrentUser = NULL;
                                          refresh = TRUE;
                                      }
                                  }
                                  break;

	case EC_SETHOST: 	  newhost=TRUE;
	case EC_SETLOGIN:	  if(newhost || (!ed->ed_RemoteEntity))
				  {
                                      if(HostReq(ed, HREQ_Title, GetString(&ed->ed_LocaleInfo,MSG_USERS_HOSTREQ_TITLE),
                                      		     HREQ_Top, ed->ed_Window->TopEdge,
                                      		     HREQ_Left,ed->ed_Window->LeftEdge,
                                      		     HREQ_Width,ed->ed_Window->Width >> 1,
                                      		     HREQ_Height,ed->ed_Window->Height,
                                                     HREQ_Buffer, ed->ed_NewMachineName,
                                                     HREQ_BuffSize, 128,
                                                     MATCH_ENTITY,"Accounts Server",
                                                     TAG_DONE))
                                      {
                                          ed->ed_MachineStr = ed->ed_NewMachineName;
                                          if(ConnectToServer(ed))
                                          {
                                              stccpy(ed->ed_MachineName,ed->ed_NewMachineName,128);
                                              ed->ed_MachineStr = ed->ed_MachineName;
                                          }
                                          else
                                          {
                                              ed->ed_MachineStr = ed->ed_MachineName;
                                              ConnectToServer(ed);
                                          }
                                          GetUserList(ed);
                                          refresh = TRUE;
                                      }
				  }
				  sprintf(ed->ed_LoginStr,GetString(&ed->ed_LocaleInfo,MSG_USERS_LOGINREQ_TITLEFMT),ed->ed_MachineName);
				  if(LoginReq(ed, LREQ_Title, ed->ed_LoginStr,
						  LREQ_NameBuff, ed->ed_AuthUser.UserName,
						  LREQ_NameBuffLen, 32,
						  LREQ_PassBuff, ed->ed_AuthUser.Password,
						  LREQ_PassBuffLen, 32,
						  LREQ_Window, ed->ed_Window,
						  LREQ_OptimWindow, ed->ed_Window,
						  TAG_DONE))
				  {
				      LoginUser(ed);

				      node = ed->ed_PrefsWork.ep_UserList.lh_Head;
				      ed->ed_CurrentUser = NULL;
				      while(node->ln_Succ)
				      {
				          if(!Stricmp(node->ln_Name,ed->ed_AuthUser.UserName))
				          {
				              ed->ed_CurrentUser = (struct UserNode *) node;
				              break;
				          }
				          node = node->ln_Succ;
				      }
				      refresh = TRUE;
				  }
				  break;

	case EC_SETGROUP	: if(UserReq(ed, UGREQ_Title, GetString(&ed->ed_LocaleInfo,MSG_USERS_GROUPREQ_TITLE),
						 UGREQ_GroupBuff, ed->ed_NewGroupStr,
						 UGREQ_GroupBuffLen, 32,
						 UGREQ_Window,ed->ed_Window,
						 UGREQ_Left, ed->ed_Window->LeftEdge + 280,
						 UGREQ_Top, ed->ed_Window->TopEdge + ed->ed_Window->BorderTop,
						 UGREQ_Width, ed->ed_Window->Width - 280 - ed->ed_Window->BorderRight,
						 UGREQ_Height, ed->ed_Window->Height - ed->ed_Window->BorderTop - ed->ed_Window->BorderBottom,
						 UGREQ_GroupList,&ed->ed_PrefsWork.ep_GroupList,
						 TAG_DONE))
				  {
				      if(!GetGroupID(ed))
				      {
				      	  CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
				      	  ed->ed_AdminUser.User.PrimaryGroupID = ed->ed_NextGroup.GroupID;

				      	  result = DoServerCmd(ed, ACMD_AdminUser);
				      	  if(!result)
				      	  {
				      	      DoPrefsGadget(ed,&EG[9],ed->ed_UserList,
				      	      		    GTLV_Labels, ~0,
				      	      		    TAG_DONE);
				      	      ed->ed_CurrentUser->Data.PrimaryGroupID = ed->ed_NextGroup.GroupID;
				      	  }
				     }
				  }
				  refresh = TRUE;
				  break;

        case EC_USERNAME	: CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
        			  stccpy(ed->ed_AdminUser.User.UserName,((struct StringInfo *)ed->ed_UserName->SpecialInfo)->Buffer,32);

        			  result = DoServerCmd(ed, ACMD_AdminUser);
        			  if(!result)
        			  {
        			      DoPrefsGadget(ed,&EG[9],ed->ed_UserList,
        			      		    GTLV_Labels, ~0,
        			      		    TAG_DONE);
        			      Remove(ed->ed_CurrentUser);
        			      stccpy(ed->ed_CurrentUser->Data.UserName,ed->ed_AdminUser.User.UserName,32);
        			      if(ed->ed_CurrentUser->Data.UserID == ed->ed_AuthUser.UserID)
        			          CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AuthUser,sizeof(struct UserData));

        			      AddNodeSorted(ed,&ed->ed_PrefsWork.ep_UserList,(struct Node *)ed->ed_CurrentUser);
        			  }

        			  refresh = TRUE;
	        		  break;

        case EC_PASSWORD	: CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
        			  stccpy(ed->ed_AdminUser.User.Password,ed->ed_TmpPassStr,32);
        			  result = DoServerCmd(ed, ACMD_AdminUser);
        			  if(!result)
        			  {
        			      DoPrefsGadget(ed,&EG[9],ed->ed_UserList,
        			      		    GTLV_Labels, ~0,
        			      		    TAG_DONE);
        			      stccpy(ed->ed_CurrentUser->Data.Password,ed->ed_AdminUser.User.Password,32);
        			      if(ed->ed_CurrentUser->Data.UserID == ed->ed_AuthUser.UserID)
        			          CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AuthUser,sizeof(struct UserData));

        			  }

        			  refresh = TRUE;
	        		  break;


	case EC_ADMIN		: CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
				  if(ed->ed_UserIsAdmin->Flags & GFLG_SELECTED)
				  {
				      ed->ed_AdminUser.User.Flags |= UFLAGF_AdminAll;
				  }
				  else
				  {
				      ed->ed_AdminUser.User.Flags &= ~UFLAGF_AdminAll;
				  }
				  result = DoServerCmd(ed, ACMD_AdminUser);
				  if(!result)
				  {
				      CopyMem(&ed->ed_AdminUser.User,&ed->ed_CurrentUser->Data,sizeof(struct UserData));
				  }

				  refresh = TRUE;
                                  break;

	case EC_ADMINGROUPS	: CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
				  if(ed->ed_UserEdGroups->Flags & GFLG_SELECTED)
				  {
				      ed->ed_AdminUser.User.Flags |= UFLAGF_AdminGroups;
				  }
				  else
				  {
				      ed->ed_AdminUser.User.Flags &= ~UFLAGF_AdminGroups;
				  }
				  result = DoServerCmd(ed, ACMD_AdminUser);
				  if(!result)
				  {
				      CopyMem(&ed->ed_AdminUser.User,&ed->ed_CurrentUser->Data,sizeof(struct UserData));
				  }

				  refresh = TRUE;
                                  break;

	case EC_ADMINNAME	: CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
				  if(ed->ed_UserEdName->Flags & GFLG_SELECTED)
				  {
				      ed->ed_AdminUser.User.Flags |= UFLAGF_AdminName;
				  }
				  else
				  {
				      ed->ed_AdminUser.User.Flags &= ~UFLAGF_AdminName;
				  }
				  result = DoServerCmd(ed, ACMD_AdminUser);
				  if(!result)
				  {
				      CopyMem(&ed->ed_AdminUser.User,&ed->ed_CurrentUser->Data,sizeof(struct UserData));
				  }

				  refresh = TRUE;
                                  break;

	case EC_ADMINPASSWORD	: CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
				  if(ed->ed_UserEdPassword->Flags & GFLG_SELECTED)
				  {
				      ed->ed_AdminUser.User.Flags |= UFLAGF_AdminPassword;
				  }
				  else
				  {
				      ed->ed_AdminUser.User.Flags &= ~UFLAGF_AdminPassword;
				  }
				  result = DoServerCmd(ed, ACMD_AdminUser);
				  if(!result)
				  {
				      CopyMem(&ed->ed_AdminUser.User,&ed->ed_CurrentUser->Data,sizeof(struct UserData));
				  }

				  refresh = TRUE;
                                  break;

	case EC_ADMINNEEDPW	: CopyMem(&ed->ed_CurrentUser->Data,&ed->ed_AdminUser.User,sizeof(struct UserData));
				  if(ed->ed_UserNeedsPassword->Flags & GFLG_SELECTED)
				  {
				      ed->ed_AdminUser.User.Flags |= UFLAGF_NeedsPassword;
				  }
				  else
				  {
				      ed->ed_AdminUser.User.Flags &= ~UFLAGF_NeedsPassword;
				  }
				  result = DoServerCmd(ed, ACMD_AdminUser);
				  if(!result)
				  {
				      CopyMem(&ed->ed_AdminUser.User,&ed->ed_CurrentUser->Data,sizeof(struct UserData));
				  }

				  refresh = TRUE;
                                  break;

 	case EC_USERLIST	: node = ed->ed_PrefsWork.ep_UserList.lh_Head;
                                  while(icode--)
                                  {
                                      node = node->ln_Succ;
                                  }
                                  ed->ed_CurrentUser = (struct UserNode *)node;
                                  refresh = TRUE;
                                  break;

    }
    if (refresh)
        RenderGadgets(ed);

    if(act)
        ActivateGadget(act,window,NULL);
}

/*****************************************************************************/

ULONG DoServerCmd(EdDataPtr ed, UBYTE cmd)
{
    ed->ed_Transaction->trans_Command = cmd;
    ed->ed_Transaction->trans_RequestData = &ed->ed_AdminUser;
    ed->ed_Transaction->trans_ResponseData = &ed->ed_AdminUser;
    ed->ed_Transaction->trans_ReqDataActual = sizeof(struct AdminUserRequest);
    ed->ed_Transaction->trans_RespDataLength = sizeof(struct AdminUserRequest);

    stccpy(ed->ed_AdminUser.Authority.UserName,ed->ed_AuthUser.UserName,32);
    stccpy(ed->ed_AdminUser.Authority.Password,ed->ed_AuthUser.Password,32);

    DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);
    return(ed->ed_Transaction->trans_Error);
}

/*****************************************************************************/

VOID GetGroupName(EdDataPtr ed)
{
    struct GroupNode *group;

    group = (struct GroupNode *)ed->ed_PrefsWork.ep_GroupList.lh_Head;
    ed->ed_Transaction->trans_Error = 1;

    ed->ed_GroupStr[0]='\0';

    while(group->Link.ln_Succ)
    {
    	if(ed->ed_CurrentUser->Data.PrimaryGroupID == group->Data.GroupID)
    	{
    	    stccpy(ed->ed_GroupStr,group->Data.GroupName,32);
    	    ed->ed_Transaction->trans_Error = 0;
    	    break;
	}
	group = (struct GroupNode *)group->Link.ln_Succ;
    }
}

/*****************************************************************************/

ULONG GetGroupID(EdDataPtr ed)
{
    struct GroupNode *group;

    group = (struct GroupNode *)ed->ed_PrefsWork.ep_GroupList.lh_Head;
    ed->ed_Transaction->trans_Error = 1;

    while(group->Link.ln_Succ)
    {
    	if(!Stricmp(group->Link.ln_Name,ed->ed_NewGroupStr))
    	{
    	    ed->ed_NextGroup.GroupID = group->Data.GroupID;
    	    ed->ed_Transaction->trans_Error = 0;
    	    break;
	}
	group = (struct GroupNode *)group->Link.ln_Succ;
    }
    return(ed->ed_Transaction->trans_Error);
}

/*****************************************************************************/
void MakeNewDisplay(EdDataPtr ed)
{
    RemoveGList(window, ed->ed_Gadgets, -1);
    SetAPen(window->RPort, ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    RectFill(window->RPort, window->BorderLeft, window->BorderTop + 20, window->Width - window->BorderRight - 1, window->Height - window->BorderBottom - 25);
    FreeGadgets(ed->ed_Gadgets);

    ed->ed_Gadgets = NULL;
    ed->ed_LastAdded = NULL;
    RenderGadgets(ed);
    if (ed->ed_LastAdded)
    {
        AddGList (window, ed->ed_Gadgets, -1, -1, NULL);
        RefreshGList (ed->ed_Gadgets, window, NULL, -1);
        GT_RefreshWindow (window, NULL);
    }
}

/*****************************************************************************/

VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}

/*****************************************************************************/

BOOL ConnectToServer(EdDataPtr ed)
{
    BOOL status;

    struct Requester *req;

    req = SetBusyPointer(ed, ed->ed_Window);

    if(ed->ed_RemoteEntity)
    {
    	LoseEntity(ed->ed_RemoteEntity);
    }
    if(ed->ed_RemoteEntity = FindEntity(ed->ed_MachineStr,"Accounts Server",ed->ed_LocalEntity,NULL))
    {
	sprintf(ed->ed_WindowTitle,GetString(&ed->ed_LocaleInfo,MSG_USERS_TITLEFMT),ed->ed_MachineStr);
	SetWindowTitles(ed->ed_Window, ed->ed_WindowTitle, (UBYTE *)-1);
	status = TRUE;
    }
    else
    {
    	status = FALSE;
    }
    if(req)
        ResetBusyPointer(ed, req, ed->ed_Window);

    return(status);

}

/*****************************************************************************/

VOID LoginUser(EdDataPtr ed)
{
    struct Requester *req;

    req = SetBusyPointer(ed, ed->ed_Window);

    ed->ed_Transaction->trans_RequestData = &ed->ed_AuthUser;
    ed->ed_Transaction->trans_ResponseData = &ed->ed_AuthUser;
    ed->ed_Transaction->trans_ReqDataActual = sizeof(struct UserData);
    ed->ed_Transaction->trans_RespDataLength = sizeof(struct UserData);
    ed->ed_Transaction->trans_Timeout = 5;
    ed->ed_Transaction->trans_Command = ACMD_VerifyUser;

    DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);

    if(ed->ed_Transaction->trans_Error)
    {
        ed->ed_AuthUser.Flags = 0;
        ed->ed_AuthUser.UserID = 0;
    }
    if(req)
        ResetBusyPointer(ed, req, ed->ed_Window);
}

/*****************************************************************************/

struct Entity *CreateEnt(EdDataPtr ed, ULONG tag1, ...)
{
    return(CreateEntityA((struct TagItem *) &tag1));
}

/*****************************************************************************/

BOOL HostReq(EdDataPtr ed, ULONG tag1, ...)
{
    return(HostRequestA((struct TagItem *) &tag1));
}

/*****************************************************************************/

BOOL LoginReq(EdDataPtr ed, ULONG tag1, ...)
{
    return(LoginRequestA((struct TagItem *) &tag1));
}

/*****************************************************************************/

BOOL UserReq(EdDataPtr ed, ULONG tag1, ...)
{
    return(UserRequestA((struct TagItem *) &tag1));
}

/*****************************************************************************/

VOID GetUserList(EdDataPtr ed)
{
    struct UserNode *user;
    struct GroupNode *group;
    struct Requester *req;

    FreeList(ed, &ed->ed_PrefsWork.ep_UserList,sizeof(struct UserNode));
    FreeList(ed, &ed->ed_PrefsWork.ep_GroupList,sizeof(struct GroupNode));

    ed->ed_NextUser.UserID = 0L;
    ed->ed_NextGroup.GroupID = 0L;

    if(ed->ed_RemoteEntity)
    {
    	req = SetBusyPointer(ed, ed->ed_Window);

        while(TRUE)
        {
            ed->ed_Transaction->trans_RequestData = &ed->ed_NextUser;
            ed->ed_Transaction->trans_ResponseData = &ed->ed_NextUser;
            ed->ed_Transaction->trans_ReqDataActual = sizeof(struct UserData);
            ed->ed_Transaction->trans_RespDataLength = sizeof(struct UserData);
            ed->ed_Transaction->trans_Timeout = 5;
            ed->ed_Transaction->trans_Command = ACMD_NextUser;
	    ed->ed_Transaction->trans_Error = 0L;

            DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);
            if(!ed->ed_Transaction->trans_Error)
            {
                if(user = AllocMem(sizeof(struct UserNode),MEMF_CLEAR|MEMF_PUBLIC))
                {
                    CopyMem(&ed->ed_NextUser,&user->Data,sizeof(struct UserData));
                    user->Link.ln_Name = user->Data.UserName;
                    AddNodeSorted(ed, &ed->ed_PrefsWork.ep_UserList,(struct Node *)user);
                }
            }
            else
            {
                break;
            }
        }
        while(TRUE)
        {
            ed->ed_Transaction->trans_RequestData = &ed->ed_NextGroup;
            ed->ed_Transaction->trans_ResponseData = &ed->ed_NextGroup;
            ed->ed_Transaction->trans_ReqDataActual = sizeof(struct GroupData);
            ed->ed_Transaction->trans_RespDataLength = sizeof(struct GroupData);
            ed->ed_Transaction->trans_Timeout = 5;
            ed->ed_Transaction->trans_Command = ACMD_NextGroup;
	    ed->ed_Transaction->trans_Error = 0L;

            DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);
            if(!ed->ed_Transaction->trans_Error)
            {
                if(group = AllocMem(sizeof(struct GroupNode),MEMF_CLEAR|MEMF_PUBLIC))
                {
                    CopyMem(&ed->ed_NextGroup,&group->Data,sizeof(struct GroupData));
                    group->Link.ln_Name = group->Data.GroupName;
                    AddNodeSorted(ed, &ed->ed_PrefsWork.ep_GroupList,(struct Node *)group);
                }
            }
            else
            {
                break;
            }
        }
        if(req)
            ResetBusyPointer(ed, req, ed->ed_Window);
    }
}


/*****************************************************************************/

VOID AddNodeSorted(EdDataPtr ed, struct List *list, struct Node *node)
{
    struct Node *current;

    current = list->lh_TailPred;

    while(current->ln_Pred)
    {
    	if(Stricmp(current->ln_Name,node->ln_Name) <= 0)
    	{
    	    Insert(list, node, current);
    	    break;
    	}
    	current = current->ln_Pred;
   }
   if(!current->ln_Pred)
   	AddHead(list,node);
}

/*****************************************************************************/

ULONG __asm passwordHook(register __a0 struct Hook *hook,
			          register __a2 struct SGWork *sgw,
			          register __a1 ULONG *msg)
{
    ULONG return_code;
    UBYTE *pass_ptr;

    pass_ptr = (STRPTR)hook->h_SubEntry;

    return_code = ~0L;

    if(*msg == SGH_KEY)
    {
    	if((sgw->EditOp == EO_REPLACECHAR) ||
    	   (sgw->EditOp == EO_INSERTCHAR))
    	{
    	    pass_ptr[sgw->BufferPos - 1] = sgw->WorkBuffer[sgw->BufferPos - 1];
    	    sgw->WorkBuffer[sgw->BufferPos - 1] = '·';
    	}
    	else if(sgw->EditOp == EO_MOVECURSOR)
    	{
    	    sgw->Actions &= ~SGA_USE;
    	}
    	else if((sgw->EditOp == EO_DELBACKWARD) ||
    	        (sgw->EditOp == EO_DELFORWARD))
    	{
    	    pass_ptr[sgw->BufferPos] = 0;
    	}
    	else if(sgw->EditOp == EO_CLEAR)
    	{
    	    memset(pass_ptr,0,32);
    	}
    }
    else if(*msg == SGH_CLICK)
    {
    	sgw->BufferPos = sgw->NumChars;
    }
    else
    	return_code = 0;

    return(return_code);
}
