
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

#include <envoy/accounts.h>

#define SysBase ed->ed_SysBase


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

    NewList((struct List *)&ed->ed_Groups);
    NewList((struct List *)&ed->ed_Members);
    NewList((struct Lust *)&ed->ed_Users);

    ed->ed_CurrentGroup = NULL;
    ed->ed_CurrentMember = NULL;
    ed->ed_MachineStr = ed->ed_MachineName;
    ed->ed_LocalEntity = NULL;
    ed->ed_RemoteEntity = NULL;
    ed->ed_V39 = (ed->ed_IntuitionBase->lib_Version >= 39) ? TRUE : FALSE;

    if(ed->ed_Transaction = AllocTransactionA(NULL))
    {
	if(ed->ed_LocalEntity = CreateEnt(ed, ENT_Signal,ed->ed_EntitySignal,
					  TAG_DONE))
	{
	    error = ES_NORMAL;
	}
    }
    GetHostName(NULL,ed->ed_MachineName,127);
    sprintf(ed->ed_WindowTitle,GetString(&ed->ed_LocaleInfo,MSG_GROUPS_WINDOWFMT),ed->ed_MachineName);

    return(error);
}

void InitPrefs(struct ExtPrefs *prefs)
{
    NewList(&prefs->ep_UserList);
}

/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreeList(ed, (struct List *)&ed->ed_Groups, sizeof(struct GroupNode));
    FreeList(ed, (struct Lust *)&ed->ed_Members, sizeof(struct UserNode));

    FreeAslRequest(ed->ed_FileReq);

    if(ed->ed_RemoteEntity)
    	LoseEntity(ed->ed_RemoteEntity);
    if(ed->ed_LocalEntity)
    	DeleteEntity(ed->ed_LocalEntity);
    if(ed->ed_Transaction)
    	FreeTransaction(ed->ed_Transaction);

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

#define NW_WIDTH     630
#define NW_HEIGHT    160
#define NW_IDCMP     (IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define NW_FLAGS     (WFLG_ACTIVATE | WFLG_CLOSEGADGET | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},

    {NM_TITLE,  MSG_SETTINGS_GMENU,           EC_NOP, 0},
      {NM_ITEM, MSG_SETTINGS_GSET_HOST,      EC_SETHOST, 0},
      {NM_ITEM, MSG_SETTINGS_GSET_LOGIN,     EC_SETLOGIN, 0},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {

    {BUTTON_KIND,   8,   142,  150, 14, MSG_GROUPS_CREATE,          EC_CREATE,            0},
    {BUTTON_KIND,   158, 142,  150, 14, MSG_GROUPS_DELETE,	    EC_DELETE,	          0},
    {BUTTON_KIND,   318, 142,  150, 14, MSG_GROUPS_ADD,		    EC_ADD,	  	  0},
    {BUTTON_KIND,   468, 142,  150, 14, MSG_GROUPS_REMOVE,	    EC_REMOVE,		  0},

    {STRING_KIND,   96,  10,   214, 14, MSG_GROUPS_HOSTNAME,	    EC_HOSTNAME,	  0},

    {STRING_KIND,   434, 8,    176, 14, MSG_GROUPS_AUTHNAME,	    EC_AUTHNAME,	  0},
    {STRING_KIND,   434, 24,   176, 14, MSG_GROUPS_AUTHPW,	    EC_AUTHPW,		  0},
    {TEXT_KIND,     428, 22,   192, 14, MSG_GROUPS_ADMIN,	    EC_NOP,		  0},

    {STRING_KIND,   8,   125,  300, 14, 0,                          EC_GROUPNAME,	  0},
    {STRING_KIND,   318, 125,  300, 14, 0,                          EC_MEMBERNAME,	  0},

    {LISTVIEW_KIND, 8,   22,   300,106, MSG_GROUPS_LIST,	    EC_GROUPLIST,	  0},
    {LISTVIEW_KIND, 318, 54,   300, 86, MSG_GROUPS_MEMBERS,	    EC_MEMBERSLIST,	  0},

    {BUTTON_KIND,   428, 6,    192, 14, MSG_GROUPS_SELECTADMIN,	    EC_ADMIN,		  0}
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
     && (ed->ed_Menus = CreatePrefsMenus(ed,EM))
     && (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,  NW_WIDTH,
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
    for (gad = (ULONG *) &ed->ed_GroupList; gad <= (ULONG *)&ed->ed_AdminName; gad++)
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
ULONG num;

    if(!ed->ed_LastAdded)
    {
    	ClearGadPtrs(ed);
    	ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
    }

    if(ed->ed_V39)
    {
        i = 0;
        node = (struct Node *)ed->ed_Groups.mlh_Head;
        while(node->ln_Succ)
        {
            if(node == (struct Node *)ed->ed_CurrentGroup)
                break;
            i++;
            node = node->ln_Succ;
        }
        if(!node->ln_Succ)
            i=~0;

        ed->ed_GroupList = DoPrefsGadget(ed,&EG[10],ed->ed_GroupList,
                                          GTLV_Selected,     i,
                                          GTLV_ShowSelected, 0,
                                          GTLV_Labels,       (ULONG)&ed->ed_Groups,
                                          LAYOUTA_SPACING,   1,
                                          GTLV_ScrollWidth,  18,
                                          TAG_DONE);
    }
    else
    {
        ed->ed_GroupList = DoPrefsGadget(ed,&EG[10],ed->ed_GroupList,
                                          GTLV_Labels,       (ULONG)&ed->ed_Groups,
                                          LAYOUTA_SPACING,   1,
                                          GTLV_ScrollWidth,  18,
                                          TAG_DONE);
    }

    EG[8].eg_TopEdge = 22 + ed->ed_GroupList->Height;

    able = FALSE;
    if(ed->ed_CurrentGroup && ed->ed_RemoteEntity)
    {
    	if(ed->ed_AuthUser.Flags & UFLAGF_AdminAll)
    	    able = TRUE;

    	if(ed->ed_AuthUser.UserID == ed->ed_CurrentGroup->Data.AdminID)
    	    able = TRUE;
    	stccpy(ed->ed_GroupStr,ed->ed_CurrentGroup->Data.GroupName,32);
    }
    else
    	ed->ed_GroupStr[0]=0;

    ed->ed_GroupName = DoPrefsGadget(ed,&EG[8],ed->ed_GroupName,
    				     GTST_String, ed->ed_GroupStr,
    				     GTST_MaxChars, 31,
    				     GA_Disabled, !able);

    EG[0].eg_TopEdge = EG[1].eg_TopEdge = EG[8].eg_TopEdge + 14;

    able=FALSE;

    if((((ed->ed_AuthUser.Flags & UFLAGF_AdminAll) ||
         (ed->ed_AuthUser.Flags & UFLAGF_AdminGroups)) &&
          ed->ed_RemoteEntity))

    	able=TRUE;

    ed->ed_CreateGroup = DoPrefsGadget(ed,&EG[0],ed->ed_CreateGroup,
    					GA_Disabled,!able,
					TAG_DONE);

    ed->ed_DeleteGroup = DoPrefsGadget(ed,&EG[1],ed->ed_DeleteGroup,
    					GA_Disabled,!able,
    					TAG_DONE);

    i = 0;
    node = (struct Node *)ed->ed_Members.mlh_Head;
    while(node->ln_Succ)
    {
        if(node == (struct Node *)ed->ed_CurrentMember)
            break;
        i++;
        node = node->ln_Succ;
    }
    if(!node->ln_Succ)
        i=~0;

    if(!ed->ed_V39)
    	EG[11].eg_TopEdge =60;

    ed->ed_MemberList = DoPrefsGadget(ed,&EG[11],ed->ed_MemberList,
                                      GTLV_Selected,     i,
                                      GTLV_ShowSelected, 0,
                                      GTLV_Labels,       (ULONG)&ed->ed_Members,
                                      LAYOUTA_SPACING,   1,
                                      GTLV_ScrollWidth,  18,
                                      TAG_DONE);


    able=FALSE;
    if(ed->ed_CurrentGroup && ed->ed_RemoteEntity)
    {
    	if((ed->ed_AuthUser.Flags & UFLAGF_AdminAll) ||
    	   (ed->ed_AuthUser.UserID == ed->ed_CurrentGroup->Data.AdminID))
    	   able = TRUE;
    }

    ed->ed_AdminName = DoPrefsGadget(ed,&EG[7],ed->ed_AdminName,
    				 GTTX_Text, ed->ed_AdminUser.UserName,
    				 GTTX_CopyText, TRUE,
    				 GTTX_Border, TRUE,
    				 TAG_DONE);

    ed->ed_AdminSelect = DoPrefsGadget(ed,&EG[12],ed->ed_AdminSelect,
    				 GA_Disabled, !able,
    				 TAG_DONE);

    EG[2].eg_TopEdge = EG[3].eg_TopEdge = EG[11].eg_TopEdge + ed->ed_MemberList->Height;
    if(!ed->ed_V39)
    {
    	EG[2].eg_TopEdge += 12;
    	EG[3].eg_TopEdge += 12;
    }

    ed->ed_AddMember = DoPrefsGadget(ed,&EG[2],ed->ed_AddMember,
    				     GA_Disabled,!able,
    				     TAG_DONE);

    ed->ed_RemMember = DoPrefsGadget(ed,&EG[3],ed->ed_RemMember,
    				     GA_Disabled,!able,
    				     TAG_DONE);


}

/*****************************************************************************/

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
BOOL refresh;
UWORD icode;
struct Node *node;
struct UserNode *user;
struct GroupNode *group;
ULONG num,result;
struct Gadget *act = NULL;

    refresh = FALSE;

    icode = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
    	case EC_CREATE		: if(group = (struct GroupNode *)AllocMem(sizeof(struct GroupNode),MEMF_CLEAR|MEMF_PUBLIC))
    				  {
                                      stccpy(ed->ed_ModifyGroup.Group.GroupName,"NewGroup",32);
                                      ed->ed_ModifyGroup.Group.AdminID = ed->ed_AuthUser.UserID;
                                      ed->ed_ModifyGroup.Group.Flags = 0;

                                      result = DoServerCmd(ed, ACMD_AddGroup);
                                      if(!result)
                                      {
                                          stccpy(group->Data.GroupName,ed->ed_ModifyGroup.Group.GroupName,32);
                                          group->Link.ln_Name = group->Data.GroupName;
                                          group->Data.AdminID = ed->ed_ModifyGroup.Group.AdminID;
                                          group->Data.GroupID = ed->ed_ModifyGroup.Group.GroupID;
                                          group->Data.Flags = ed->ed_ModifyGroup.Group.Flags;

                                          DoPrefsGadget(ed,&EG[10],ed->ed_GroupList,
                                          		GTLV_Labels, ~0,
                                          		TAG_DONE);
                                          AddNodeSorted(ed,(struct List *)&ed->ed_Groups,(struct Node *)group);
                                          ed->ed_CurrentGroup = group;
                                          ed->ed_CurrentMember = NULL;
                                          FreeList(ed,&ed->ed_Members,sizeof(struct UserNode));
                                          GetMemberList(ed, ed->ed_CurrentGroup);
                                          refresh = TRUE;
                                      }
                                      else
                                      	  FreeMem(group,sizeof(struct GroupNode));
                                  }
                                  break;

	case EC_DELETE		: if(ed->ed_CurrentGroup)
				  {
				      CopyMem(&ed->ed_CurrentGroup->Data,&ed->ed_ModifyGroup.Group,sizeof(struct GroupData));

    				      result = DoServerCmd(ed, ACMD_RemGroup);
                                      if(!result)
                                      {
                                          DoPrefsGadget(ed,&EG[9],ed->ed_GroupList,
                                          		GTLV_Labels, ~0,
                                          		TAG_DONE);
                                          Remove((struct Node *)ed->ed_CurrentGroup);
                                          FreeMem(ed->ed_CurrentGroup,sizeof(struct GroupNode));
                                          ed->ed_CurrentMember = NULL;
                                          ed->ed_CurrentGroup = NULL;
                                          ed->ed_AdminUser.UserName[0]='\0';

                                          FreeList(ed,&ed->ed_Members,sizeof(struct UserNode));
                                          refresh = TRUE;
                                      }
                                  }
                                  break;

	case EC_SETHOST		: if(HostReq(ed, HREQ_Title, GetString(&ed->ed_LocaleInfo,MSG_GROUPS_SELECTHOST),
                                                 HREQ_Top, ed->ed_Window->TopEdge,
                                                 HREQ_Left,ed->ed_Window->LeftEdge,
                                                 HREQ_Width,ed->ed_Window->Width >> 1,
                                                 HREQ_Height,ed->ed_Window->Height,
                                                 HREQ_Buffer, ed->ed_NewMachineName,
                                                 HREQ_BuffSize, 128,
                                                 MATCH_ENTITY,"Accounts Server",
						 TAG_DONE))
				  {
				      BOOL brk = FALSE;

				      ed->ed_MachineStr = ed->ed_NewMachineName;
				      if(ConnectToServer(ed))
				      {
				      	  stccpy(ed->ed_MachineName,ed->ed_NewMachineName,128);
				      	  ed->ed_MachineStr = ed->ed_MachineName;
				      	  sprintf(ed->ed_WindowTitle,GetString(&ed->ed_LocaleInfo,MSG_GROUPS_WINDOWFMT),ed->ed_MachineName);
				      	  SetWindowTitles(ed->ed_Window,ed->ed_WindowTitle,(struct Requester *)-1);
				      }
				      else
				      {
				      	  ed->ed_MachineStr = ed->ed_MachineName;
				      	  ConnectToServer(ed);
				      	  brk = TRUE;
				      }
				      GetGroupList(ed);
				      refresh = TRUE;
				      if(brk)
				      	  break;
				  }

	case EC_SETLOGIN	: sprintf(ed->ed_LoginTitle,GetString(&ed->ed_LocaleInfo,MSG_GROUPS_LOGINFMT),ed->ed_MachineName);
				  if(LoginReq(ed, LREQ_Title, ed->ed_LoginTitle,
						  LREQ_Window, ed->ed_Window,
						  LREQ_NameBuff, ed->ed_AuthUser.UserName,
						  LREQ_NameBuffLen, 32,
						  LREQ_PassBuff, ed->ed_AuthUser.Password,
						  LREQ_PassBuffLen, 32,
						  LREQ_OptimWindow, ed->ed_Window,
						  TAG_DONE))
				  {
				      ConnectToServer(ed);
//				      GetGroupList(ed);
				      refresh = TRUE;
				  }
				  break;

/*
	case EC_AUTHNAME	: stccpy(ed->ed_AuthUser.UserName,((struct StringInfo *)ed->ed_AuthName->SpecialInfo)->Buffer,32);
				  ConnectToServer(ed);
				  GetGroupList(ed);
				  refresh = TRUE;
				  break;

	case EC_AUTHPW		: stccpy(ed->ed_AuthUser.Password,((struct StringInfo *)ed->ed_AuthPassword->SpecialInfo)->Buffer,32);
				  ConnectToServer(ed);
				  GetGroupList(ed);
				  refresh = TRUE;
				  break;
*/
        case EC_GROUPNAME	: CopyMem(&ed->ed_CurrentGroup->Data,&ed->ed_ModifyGroup.Group,sizeof(struct GroupData));
        			  stccpy(ed->ed_ModifyGroup.Group.GroupName,((struct StringInfo *)ed->ed_GroupName->SpecialInfo)->Buffer,32);

        			  result = DoServerCmd(ed, ACMD_ModifyGroup);
        			  if(!result)
        			  {
        			      DoPrefsGadget(ed,&EG[9],ed->ed_GroupList,
        			      		    GTLV_Labels, ~0,
        			      		    TAG_DONE);
        			      stccpy(ed->ed_CurrentGroup->Data.GroupName,ed->ed_ModifyGroup.Group.GroupName,32);
        			  }

        			  refresh = TRUE;
	        		  break;
/*
        case EC_MEMBERNAME	: stccpy(ed->ed_MemberStr,((struct StringInfo *)ed->ed_MemberName->SpecialInfo)->Buffer,32);
				  ed->ed_CurrentMember = NULL;
				  refresh = TRUE;
				  break;
*/
	case EC_ADMIN		: if(ed->ed_CurrentGroup)
				  {
				      if(UserReq(ed, UGREQ_Title, GetString(&ed->ed_LocaleInfo,MSG_GROUPS_SELECTADMINFMT),
				      		     UGREQ_Window,ed->ed_Window,
                                                     UGREQ_UserBuff, ed->ed_NextUser.UserName,
                                                     UGREQ_UserBuffLen, 32,
                                                     UGREQ_Left, ed->ed_Window->LeftEdge + 310,
                                                     UGREQ_Top, ed->ed_Window->TopEdge + ed->ed_Window->BorderTop,
                                                     UGREQ_Width, ed->ed_Window->Width - 310 - ed->ed_Window->BorderRight,
                                                     UGREQ_Height, ed->ed_Window->Height - ed->ed_Window->BorderTop - ed->ed_Window->BorderBottom,
                                                     UGREQ_UserList, &ed->ed_Users,
                                                     TAG_DONE))
                                      {
                                          ed->ed_Transaction->trans_RequestData = &ed->ed_NextUser;
                                          ed->ed_Transaction->trans_ResponseData = &ed->ed_NextUser;
                                          ed->ed_Transaction->trans_ReqDataActual = sizeof(struct UserData);
                                          ed->ed_Transaction->trans_RespDataLength = sizeof(struct UserData);
                                          ed->ed_Transaction->trans_Timeout = 15;
                                          ed->ed_Transaction->trans_Command = ACMD_NameToUser;
                                          DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);
                                          if(!ed->ed_Transaction->trans_Error)
                                          {
                                              CopyMem(&ed->ed_CurrentGroup->Data,&ed->ed_ModifyGroup.Group,sizeof(struct GroupData));
                                              ed->ed_ModifyGroup.Group.AdminID = ed->ed_NextUser.UserID;
                                              result=DoServerCmd(ed, ACMD_ModifyGroup);
                                              if(!result)
                                              {
                                                  CopyMem(&ed->ed_NextUser,&ed->ed_AdminUser,sizeof(struct UserData));
                                                  CopyMem(&ed->ed_ModifyGroup.Group,&ed->ed_CurrentGroup->Data,sizeof(struct GroupData));
                                              }
                                          }
                                      }
                                  }
                                  refresh = TRUE;
                                  break;

	case EC_ADD		: if(ed->ed_CurrentGroup)
				  {
				      sprintf(ed->ed_LoginTitle,GetString(&ed->ed_LocaleInfo,MSG_GROUPS_ADDUSER_TITLE),ed->ed_CurrentGroup->Data.GroupName);
				      if(UserReq(ed, UGREQ_Title, ed->ed_LoginTitle,
                                                     UGREQ_UserBuff, ed->ed_ModifyGroup.User.UserName,
                                                     UGREQ_UserBuffLen, 32,
                                                     UGREQ_Window,ed->ed_Window,
                                                     UGREQ_Left, ed->ed_Window->LeftEdge + 310,
                                                     UGREQ_Top, ed->ed_Window->TopEdge + ed->ed_Window->BorderTop,
                                                     UGREQ_Width, ed->ed_Window->Width - 310 - ed->ed_Window->BorderRight,
                                                     UGREQ_Height, ed->ed_Window->Height - ed->ed_Window->BorderTop - ed->ed_Window->BorderBottom,
                                                     UGREQ_UserList,&ed->ed_Users,
                                                     TAG_DONE))
                                      {
                                      	  CopyMem(&ed->ed_CurrentGroup->Data,&ed->ed_ModifyGroup.Group,sizeof(struct GroupData));
                                      	  ed->ed_ModifyGroup.User.UserID = 0;
                                      	  result = DoServerCmd(ed, ACMD_AddMember);
                                      	  if(!result)
                                      	  {
                                      	      DoPrefsGadget(ed,&EG[10],ed->ed_MemberList,
                                      	      		    GTLV_Labels, ~0,
                                      	      		    TAG_DONE);
                                      	      GetMemberList(ed, ed->ed_CurrentGroup);
                                      	      refresh = TRUE;
                                      	  }
                                      	  else
                                      	      DisplayBeep(ed->ed_Screen);
                                      }
                                  }
                                  break;

	case EC_REMOVE		: if(ed->ed_CurrentMember && ed->ed_CurrentGroup)
				  {
				      CopyMem(&ed->ed_CurrentGroup->Data,&ed->ed_ModifyGroup.Group,sizeof(struct GroupData));
				      CopyMem(&ed->ed_CurrentMember->Data,&ed->ed_ModifyGroup.User,sizeof(struct UserData));
				      result = DoServerCmd(ed, ACMD_RemoveMember);
				      if(!result)
				      {
				      	  DoPrefsGadget(ed,&EG[10],ed->ed_MemberList,
				      	  		GTLV_Labels, ~0,
				      	  		TAG_DONE);
				      	  GetMemberList(ed, ed->ed_CurrentGroup);
				      	  refresh = TRUE;
				      }
				  }
				  break;

	case EC_GROUPLIST	: node = (struct Node *) ed->ed_Groups.mlh_Head;
				  while(icode--)
				  {
				      node = node->ln_Succ;
				  }
				  ed->ed_CurrentGroup = (struct GroupNode *)node;
				  GetMemberList(ed, ed->ed_CurrentGroup);
				  refresh = TRUE;
				  break;

 	case EC_MEMBERSLIST	: node = (struct Node *) ed->ed_Members.mlh_Head;
                                  while(icode--)
                                  {
                                      node = node->ln_Succ;
                                  }
                                  ed->ed_CurrentMember = (struct MemberNode *)node;
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
    ed->ed_Transaction->trans_RequestData = &ed->ed_ModifyGroup;
    ed->ed_Transaction->trans_ResponseData = &ed->ed_ModifyGroup;
    ed->ed_Transaction->trans_ReqDataActual = sizeof(struct ModifyGroupRequest);
    ed->ed_Transaction->trans_RespDataLength = sizeof(struct ModifyGroupRequest);
    stccpy(ed->ed_ModifyGroup.Authority.UserName,ed->ed_AuthUser.UserName,32);
    stccpy(ed->ed_ModifyGroup.Authority.Password,ed->ed_AuthUser.Password,32);

    DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);
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
    struct Requester *req;
    BOOL status;

    req = SetBusyPointer(ed, ed->ed_Window);

	ed->ed_CurrentGroup = NULL;
	ed->ed_CurrentMember = NULL;
	ed->ed_GroupStr[0]=0;
	ed->ed_MemberStr[0]=0;

    if(ed->ed_RemoteEntity)
    {
    	LoseEntity(ed->ed_RemoteEntity);
    }
    if(ed->ed_RemoteEntity = FindEntity(ed->ed_MachineStr,"Accounts Server",ed->ed_LocalEntity,NULL))
    {
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

struct Entity *CreateEnt(EdDataPtr ed, ULONG tag1, ...)
{
   return(CreateEntityA((struct TagItem *) &tag1));
}

/*****************************************************************************/

VOID GetMemberList(EdDataPtr ed, struct GroupNode *group)
{
    struct UserNode *user;
    struct Requester *req;

    FreeList(ed, (struct List *)&ed->ed_Members,sizeof(struct UserNode));

    ed->ed_CurrentMember = NULL;
    ed->ed_MemberStr[0]=0;
    ed->ed_NextMember.User.UserID = 0L;

    if(ed->ed_RemoteEntity)
    {
    	req = SetBusyPointer(ed, ed->ed_Window);

    	if(ed->ed_MemberList)
            DoPrefsGadget(ed,&EG[9],ed->ed_MemberList,
                  GTLV_Labels, ~0,
                  TAG_DONE);
    	CopyMem(&group->Data,&ed->ed_NextMember.Group,sizeof(struct GroupData));

        while(TRUE)
        {
            ed->ed_Transaction->trans_RequestData = &ed->ed_NextMember;
            ed->ed_Transaction->trans_ResponseData = &ed->ed_NextMember;
            ed->ed_Transaction->trans_ReqDataActual = sizeof(NextMemberReq);
            ed->ed_Transaction->trans_RespDataLength = sizeof(NextMemberReq);
            ed->ed_Transaction->trans_Timeout = 15;
            ed->ed_Transaction->trans_Command = ACMD_NextMember;

            DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);
            if(!ed->ed_Transaction->trans_Error)
            {
                if(user = AllocMem(sizeof(struct UserNode),MEMF_CLEAR|MEMF_PUBLIC))
                {
                    CopyMem(&ed->ed_NextMember.User,&user->Data,sizeof(struct UserData));
                    user->Link.ln_Name = user->Data.UserName;
                    AddNodeSorted(ed,(struct List *)&ed->ed_Members,(struct Node *)user);
                }
            }
            else
            {
                break;
            }
        }
        ed->ed_AdminUser.UserID = group->Data.AdminID;
        ed->ed_Transaction->trans_RequestData = &ed->ed_AdminUser;
        ed->ed_Transaction->trans_ResponseData = &ed->ed_AdminUser;
        ed->ed_Transaction->trans_ReqDataActual = sizeof(struct UserData);
        ed->ed_Transaction->trans_RespDataLength = sizeof(struct UserData);
        ed->ed_Transaction->trans_Timeout = 15;
        ed->ed_Transaction->trans_Command = ACMD_IDToUser;
        DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);

        if(req)
            ResetBusyPointer(ed, req, ed->ed_Window);

    }
}

/*****************************************************************************/
#ifdef FOOBAR
VOID GetGroupList(EdDataPtr ed)
{
    struct GroupNode *group;

    FreeList(ed, (struct List *)&ed->ed_Groups,sizeof(struct GroupNode));

    ed->ed_NextGroup.GroupID = 0L;

    if(ed->ed_RemoteEntity)
    {
    	if(ed->ed_GroupList)
            DoPrefsGadget(ed,&EG[9],ed->ed_GroupList,
                  GTLV_Labels, ~0,
                  TAG_DONE);
        while(TRUE)
        {
            ed->ed_Transaction->trans_RequestData = &ed->ed_NextGroup;
            ed->ed_Transaction->trans_ResponseData = &ed->ed_NextGroup;
            ed->ed_Transaction->trans_ReqDataActual = sizeof(struct GroupData);
            ed->ed_Transaction->trans_RespDataLength = sizeof(struct GroupData);
            ed->ed_Transaction->trans_Timeout = 5;
            ed->ed_Transaction->trans_Command = ACMD_NextGroup;

            DoTransaction(ed->ed_RemoteEntity,ed->ed_LocalEntity,ed->ed_Transaction);
            if(!ed->ed_Transaction->trans_Error)
            {
                if(group = AllocMem(sizeof(struct GroupNode),MEMF_CLEAR|MEMF_PUBLIC))
                {
                    CopyMem(&ed->ed_NextGroup,&group->Data,sizeof(struct GroupData));
                    group->Link.ln_Name = group->Data.GroupName;
                    AddNodeSorted(ed, (struct List *)&ed->ed_Groups,(struct Node *)group);
                }
            }
            else
            {
                break;
            }
        }
    }
}
#endif
/*****************************************************************************/

VOID GetGroupList(EdDataPtr ed)
{
    struct UserNode *user;
    struct GroupNode *group;
    struct Requester *req;

    FreeList(ed, (struct List *)&ed->ed_Groups,sizeof(struct GroupNode));
    FreeList(ed, (struct List *)&ed->ed_Users, sizeof(struct UserNode));

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
                    AddNodeSorted(ed, (struct List *)&ed->ed_Users,(struct Node *)user);
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
                    AddNodeSorted(ed, (struct List *)&ed->ed_Groups,(struct Node *)group);
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

