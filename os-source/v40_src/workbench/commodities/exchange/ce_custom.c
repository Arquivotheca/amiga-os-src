
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <devices/inputevent.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/commodities.h>
#include <graphics/displayinfo.h>
#include <graphics/text.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>
#include <clib/commodities_protos.h>
#include <clib/icon_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/commodities_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "ce_custom.h"
#include "ce_window.h"
#include "ce_strings.h"


/*****************************************************************************/


struct BrokerCopy
{
   struct Node     bc_Node;
   char            bc_Name[CBD_NAMELEN];
   char            bc_Title[CBD_TITLELEN];
   char            bc_Descr[CBD_DESCRLEN];
   struct Task    *bc_Task;
   struct MsgPort *bc_Port;
   WORD		   bc_Channel;
   LONG            bc_Flags;
};

#define COF_ENABLE 2


/*****************************************************************************/


extern struct Library       *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library       *CxBase;
extern struct Library       *IconBase;
extern struct Library       *GfxBase;
extern struct Library       *DOSBase;
extern struct Library       *GadToolsBase;
extern struct Library       *UtilityBase;
extern LONG                  cxSignal;
extern CxObj                *cxBroker;
extern struct Gadget        *cxGadgets;
extern struct Menu          *cxMenus;
extern struct Window        *cxWindow;
extern APTR                  cxVisualInfo;
extern struct DrawInfo      *cxDrawInfo;

/* from IntuiMessage */
ULONG             class;
UWORD             icode;
struct Gadget    *gad;

struct TextFont   *topazFont;
struct Gadget     *brokersGad;
struct Gadget     *stateGad;
struct Gadegt     *showUIGad;
struct Gadget	  *hideUIGad;
struct Gadget     *removeGad;
struct MinList     brokers;
char		   brokerName[CBD_NAMELEN+1];
struct BrokerCopy *currentBroker;
STRPTR             states[3];


/*****************************************************************************/


LONG BrokerCommand(STRPTR name,LONG longcommand );
LONG CopyBrokerList(struct List *blist );
VOID FreeBrokerList(struct List *list );

#pragma libcall CxBase CopyBrokerList ba 801
#pragma libcall CxBase FreeBrokerList c0 801
#pragma libcall CxBase BrokerCommand c6 802


/*****************************************************************************/


#define CMD_NOP           0
#define CMD_HIDE          1
#define CMD_QUIT          2
#define CMD_BROKERLIST    3
#define CMD_ACTIVATION    4
#define CMD_SHOWINTERFACE 5
#define CMD_HIDEINTERFACE 6
#define CMD_REMOVE        7


/*****************************************************************************/


struct TextAttr far topazAttr =
{
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};


/*****************************************************************************/


VOID RefreshGads(BOOL newBrokers);
BOOL ProcessCommand(UWORD cmd);


/*****************************************************************************/


BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObject *diskObj,
                       ULONG *cliOpts)
{
    return(TRUE);
}


/*****************************************************************************/


VOID ProcessCustomCxMsg(ULONG cmd)
{
}


/*****************************************************************************/


VOID ProcessCustomCxSig()
{
}


/*****************************************************************************/


VOID ProcessCustomCxCmd(ULONG cmd)
{
    if (cmd == CXCMD_LIST_CHG)
    {
        if (cxWindow)
        {
            RefreshGads(TRUE);
        }
    }
}


/*****************************************************************************/


BOOL ProcessIntuiMsg(struct IntuiMessage *intuiMsg)
{
BOOL             ok;
UWORD            menuNum;
struct MenuItem *menuItem;

    class = intuiMsg->Class;
    icode = intuiMsg->Code;
    gad   = intuiMsg->IAddress;
    GT_ReplyIMsg(intuiMsg);

    ok = TRUE;
    switch (class)
    {
        case IDCMP_CLOSEWINDOW  : ok = ProcessCommand(CMD_HIDE);
                                  break;

        case IDCMP_REFRESHWINDOW: RefreshWindow(TRUE);
                                  break;

        case IDCMP_MENUPICK     : menuNum = icode;
                                  while ((menuNum != MENUNULL) && (ok))
                                  {
                                      menuItem = ItemAddress(cxMenus,menuNum);
                                      ok = ProcessCommand((UWORD)MENU_USERDATA(menuItem));

                                      if (cxMenus == NULL)
                                          break;

                                      menuNum = menuItem->NextSelect;
                                  }
                                  break;

        case IDCMP_GADGETUP     :
        case IDCMP_GADGETDOWN   : ok = ProcessCommand((UWORD)gad->UserData);
                                  break;
    }

    return(ok);
}


/*****************************************************************************/


VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)&list->lh_Head;
}


/*****************************************************************************/


VOID GetBrokerList(VOID)
{
struct List  temp;
struct Node *node;
struct Node *new;

    NewList(&temp);

    FreeBrokerList((struct List *)&brokers);
    CopyBrokerList(&temp);

    while (new = RemHead(&temp))
    {
        node = (struct Node *)brokers.mlh_Head;
        while (node->ln_Succ)
        {
            if (Stricmp(node->ln_Name,new->ln_Name) >= 0)
                break;
            node = node->ln_Succ;
        }
        Insert((struct List *)&brokers,new,node->ln_Pred);
    }
}


/*****************************************************************************/


BOOL CreateCustomGadgets(VOID)
{
struct NewGadget  ng;
struct Gadget    *gad;

    SetFont(cxWindow->RPort,topazFont);

    GetBrokerList();
    brokerName[0] = 0;
    currentBroker = NULL;

    states[0] = GetString(MSG_EX_STATUS_ACTIVE);
    states[1] = GetString(MSG_EX_STATUS_INACTIVE);
    states[2] = NULL;

    if (gad = CreateContext(&cxGadgets))
    {
	/* Some invariants */
        ng.ng_TextAttr   = &topazAttr;
        ng.ng_GadgetID   = 0;
        ng.ng_Flags      = NULL;
        ng.ng_VisualInfo = cxVisualInfo;

        ng.ng_TopEdge    = cxWindow->BorderTop+16;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+8;
        ng.ng_Width      = 200;
        ng.ng_Height     = 64;
        ng.ng_GadgetText = GetString(MSG_EX_AVAIL_HDR);
        ng.ng_UserData   = (APTR)CMD_BROKERLIST;

        brokersGad = CreateGadget(LISTVIEW_KIND,gad,&ng,GTLV_Labels,       &brokers,
                                                        GTLV_ShowSelected, NULL,
                                                        LAYOUTA_SPACING,   1,
                                                        GTLV_ScrollWidth,  18,
                                                        TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+48;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+215;
        ng.ng_Width      = 171;
        ng.ng_Height     = 14;
        ng.ng_GadgetText = GetString(MSG_EX_SHOWINTERFACE_GAD);
        ng.ng_UserData   = (APTR)CMD_SHOWINTERFACE;

        showUIGad = CreateGadget(BUTTON_KIND,brokersGad,&ng,GA_Disabled, TRUE,
                                                            TAG_DONE);

        ng.ng_LeftEdge   = cxWindow->BorderLeft+388;
        ng.ng_GadgetText = GetString(MSG_EX_HIDEINTERFACE_GAD);
        ng.ng_UserData   = (APTR)CMD_HIDEINTERFACE;

        hideUIGad = CreateGadget(BUTTON_KIND,showUIGad,&ng,GA_Disabled, TRUE,
                                                           TAG_DONE);

        ng.ng_LeftEdge   = cxWindow->BorderLeft+215;
        ng.ng_TopEdge    = cxWindow->BorderTop+63;
        ng.ng_GadgetText = GetString(MSG_NOTHING);
        ng.ng_UserData   = (APTR)CMD_ACTIVATION;

        stateGad = CreateGadget(CYCLE_KIND,hideUIGad,&ng,GTCY_Labels, &states,
                                                         GA_Disabled, TRUE,
                                                         TAG_DONE);

        ng.ng_LeftEdge   = cxWindow->BorderLeft+388;
        ng.ng_GadgetText = GetString(MSG_EX_REMOVE_GAD);
        ng.ng_UserData   = (APTR)CMD_REMOVE;

        if (removeGad = CreateGadget(BUTTON_KIND,stateGad,&ng,GA_Disabled, TRUE,
                                                              TAG_DONE))
        {
            AddGList(cxWindow,cxGadgets,-1,-1,NULL);
            RefreshGList(cxGadgets,cxWindow,NULL,-1);
            GT_RefreshWindow(cxWindow,NULL);
            return(TRUE);
        }
        FreeGadgets(cxGadgets);
        cxGadgets = NULL;
    }
    return(FALSE);
}


/*****************************************************************************/


struct NewMenu NM[] =
{
    {NM_TITLE,  NULL, 0, 0, 0, (APTR)CMD_NOP,  },
      {NM_ITEM, NULL, 0, 0, 0, (APTR)CMD_HIDE, },
      {NM_ITEM, NULL, 0, 0, 0, (APTR)CMD_QUIT, },

    {NM_END,    0,    0, 0, 0, (APTR)CMD_NOP,  }
};



VOID DoItem(WORD itemNum, AppStringsID label)
{
    if (NM[itemNum].nm_Type != NM_TITLE)
    {
        NM[itemNum].nm_Label   = (APTR)((ULONG)GetString(label)+2);
        NM[itemNum].nm_CommKey = GetString(label);
    }
    else
    {
        NM[itemNum].nm_Label = GetString(label);
    }
}


/*****************************************************************************/


BOOL CreateCustomMenus(VOID)
{
    DoItem(0,MSG_PROJECT_MENU);
    DoItem(1,MSG_PROJECT_HIDE);
    DoItem(2,MSG_PROJECT_QUIT);

    if (cxMenus = CreateMenusA(NM,NULL))
    {
        if (LayoutMenus(cxMenus,cxVisualInfo,GTMN_NewLookMenus,TRUE,
                                             TAG_DONE))
        {
            SetMenuStrip(cxWindow,cxMenus);
            return(TRUE);
        }
        FreeMenus(cxMenus);
        cxMenus = NULL;
    }
    return(FALSE);
}


/*****************************************************************************/


struct Node *FindNum(struct List *list, UWORD number)
{
struct Node *node;

    node = list->lh_Head;
    while (number--)
	node = node->ln_Succ;

    return (node);
}


/*****************************************************************************/


ULONG FindNodeNum(struct List *list, struct Node *node)
{
struct Node *current;
ULONG        number;

    number  = 0;
    current = list->lh_Head;

    while (current != node)
    {
	current = current->ln_Succ;
	number++;
    }

    return(number);
}


/*****************************************************************************/


VOID RefreshGads(BOOL newBrokers)
{
LONG  active;
ULONG num;

    if (newBrokers)
    {
        GT_SetGadgetAttrs(brokersGad,cxWindow,NULL,GTLV_Labels, ~0,
                                                   TAG_DONE);

        GetBrokerList();

        if (currentBroker = (struct BrokerCopy *)FindName((struct List *)&brokers,brokerName))
        {
            num = FindNodeNum((struct List *)&brokers,(struct Node *)currentBroker);
        }
        else
        {
            num = ~0;
            brokerName[0] = 0;
        }


        GT_SetGadgetAttrs(brokersGad,cxWindow,NULL,GTLV_Labels,      &brokers,
                                                   GTLV_Selected,    num,
                                                   GTLV_MakeVisible, num,
                                                   TAG_DONE);
    }

    if (currentBroker)
    {
        active = 1;
        if (currentBroker->bc_Flags & COF_ENABLE)
            active = 0;

        GT_SetGadgetAttrs(stateGad,cxWindow,NULL,GA_Disabled, FALSE,
                                                 GTCY_Active, active,
                                                 TAG_DONE);

        GT_SetGadgetAttrs(showUIGad,cxWindow,NULL,GA_Disabled, !(currentBroker->bc_Flags & COF_SHOW_HIDE),
                                                  TAG_DONE);

        GT_SetGadgetAttrs(hideUIGad,cxWindow,NULL,GA_Disabled, !(currentBroker->bc_Flags & COF_SHOW_HIDE),
                                                  TAG_DONE);

        GT_SetGadgetAttrs(removeGad,cxWindow,NULL,GA_Disabled,FALSE,
                                                  TAG_DONE);
    }
    else
    {
        GT_SetGadgetAttrs(stateGad,cxWindow,NULL,GA_Disabled,TRUE,
                                                 TAG_DONE);

        GT_SetGadgetAttrs(showUIGad,cxWindow,NULL,GA_Disabled,TRUE,
                                                  TAG_DONE);

        GT_SetGadgetAttrs(hideUIGad,cxWindow,NULL,GA_Disabled,TRUE,
                                                  TAG_DONE);

        GT_SetGadgetAttrs(removeGad,cxWindow,NULL,GA_Disabled,TRUE,
                                                  TAG_DONE);
    }

    SetAPen(cxWindow->RPort,0);
    RectFill(cxWindow->RPort,217+cxWindow->BorderLeft,17+cxWindow->BorderTop,
                             217+304-5+cxWindow->BorderLeft,17+30-3+cxWindow->BorderTop);
    if (currentBroker)
    {
        SetAPen(cxWindow->RPort,cxDrawInfo->dri_Pens[TEXTPEN]);

        currentBroker->bc_Title[37] = 0;
        currentBroker->bc_Descr[37] = 0;

        Move(cxWindow->RPort,217+2+cxWindow->BorderLeft,17+10+cxWindow->BorderTop);
        Text(cxWindow->RPort,currentBroker->bc_Title,strlen(currentBroker->bc_Title));

        Move(cxWindow->RPort,217+2+cxWindow->BorderLeft,17+21+cxWindow->BorderTop);
        Text(cxWindow->RPort,currentBroker->bc_Descr,strlen(currentBroker->bc_Descr));
    }
}


/*****************************************************************************/


VOID CenterLine(struct RastPort *rp, STRPTR str, UWORD x, UWORD y, UWORD w)
{
UWORD  len;

    len = strlen(str);

    Move(rp,(w-TextLength(rp,str,len)) / 2 + cxWindow->BorderLeft + x,
            cxWindow->BorderTop+y);
    Text(rp,str,len);
}


/*****************************************************************************/


VOID RefreshWindow(BOOL refreshEvent)
{
    if (refreshEvent)
        GT_BeginRefresh(cxWindow);

    SetAPen(cxWindow->RPort,cxDrawInfo->dri_Pens[TEXTPEN]);
    SetBPen(cxWindow->RPort,0);

    CenterLine(cxWindow->RPort,GetString(MSG_EX_INFO_HDR),215,10,344);
    DrawBevelBox(cxWindow->RPort,215+cxWindow->BorderLeft,16+cxWindow->BorderTop,
                                 344,30,GT_VisualInfo, cxVisualInfo,
                                        GTBB_Recessed, TRUE,
                                        TAG_DONE);

    if (refreshEvent)
        GT_EndRefresh(cxWindow,TRUE);
}


/*****************************************************************************/


BOOL ProcessCommand(UWORD cmd)
{
    switch (cmd)
    {
        case CMD_HIDE       : DisposeWindow();
                              break;

        case CMD_QUIT       : return(FALSE);

        case CMD_BROKERLIST : currentBroker = (struct BrokerCopy *)FindNum((struct List *)&brokers,icode);
                              strcpy(brokerName,currentBroker->bc_Name);
                              RefreshGads(FALSE);
                              break;

        case CMD_ACTIVATION : if (brokerName[0])
                              {
                                  if (icode == 0)
                                      BrokerCommand(brokerName,CXCMD_ENABLE);
                                  else
                                      BrokerCommand(brokerName,CXCMD_DISABLE);
                              }
                              break;

        case CMD_SHOWINTERFACE: if (brokerName[0])
                                    BrokerCommand(brokerName,CXCMD_APPEAR);
                                break;

        case CMD_HIDEINTERFACE: if (brokerName[0])
                                    BrokerCommand(brokerName,CXCMD_DISAPPEAR);
                                break;

        case CMD_REMOVE     : if (brokerName[0])
                              {
                                  BrokerCommand(brokerName,CXCMD_KILL);
                              }
                              break;
    }

    return(TRUE);
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
    if (topazFont = OpenFont(&topazAttr))
    {
        NewList((struct List *)&brokers);
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID DisposeCustomCx(VOID)
{
    FreeBrokerList((struct List *)&brokers);
    NewList((struct List *)&brokers);
    CloseFont(topazFont);
}
