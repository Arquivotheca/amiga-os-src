
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <devices/inputevent.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/commodities.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <dos/exall.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/commodities_protos.h>
#include <clib/icon_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/commodities_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "ce_custom.h"
#include "ce_window.h"
#include "ce_strings.h"
#include "crossdos.h"


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
extern BOOL                  cxPopUp;

/* from IntuiMessage */
ULONG             class;
UWORD             icode;
struct Gadget    *gad;

struct TextFont   *topazFont;
struct Gadget     *deviceGad;
struct Gadget     *filterGad;
struct Gadget     *transGad;
struct Gadget     *transListGad;

STRPTR      *transArray;
struct List  transList;
struct List  handlerList;
struct Node  dummyNode;

struct Node *currentHandler;
BOOL         doFilter;
BOOL         doTrans;
STRPTR       transType;

struct DiskObject *cdcDiskObj;


/*****************************************************************************/


#define CMD_NOP       0
#define CMD_SAVE      1
#define CMD_HIDE      2
#define CMD_QUIT      3
#define CMD_DEVICES   4
#define CMD_FILTER    5
#define CMD_TRANS     6
#define CMD_TRANSLIST 7
#define CMD_NEWDISK   8

#define CROSSDOS_TRANS "L:FileSystem_Trans"
#define CROSSDOS_NAME  "« CrossDOS »"


/*****************************************************************************/


struct TextAttr far topazAttr =
{
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};


/*****************************************************************************/


VOID RefreshGads(VOID);
BOOL ProcessCommand(UWORD cmd);
VOID ProcessHandlerList(BOOL initial);
BOOL BuildTransList(STRPTR directory, STRPTR pattern, WORD strip);
VOID HandlerInfo(STRPTR handlerName, BOOL *doFilter, BOOL *doTrans, STRPTR *transType);
VOID NewList(struct List *list);
VOID kprintf(STRPTR,...);
VOID HandlerControl(STRPTR handlerName, BOOL doFilter, BOOL doTrans, STRPTR transType);
VOID Terminate(VOID);
VOID EmptyList(struct List *list);


/*****************************************************************************/


BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObject *diskObj,
                       ULONG *cliOpts)
{
STRPTR *ptr;
STRPTR  name;
UWORD   i,j;
char    handler[50];
char    transName[50];
BOOL    allocated;

    NewList(&handlerList);
    NewList(&transList);
    transArray = NULL;

    allocated = FALSE;
    if (!(cdcDiskObj = diskObj))
    {
        cdcDiskObj = GetDiskObject("PROGDIR:CrossDOS");
        allocated = TRUE;
    }

    if (cdcDiskObj && (ptr = cdcDiskObj->do_ToolTypes))
    {
        while (name = *ptr)
        {
            if (name[0] == '«')
            {
                i = 1;
                while ((name[i] != ',') && (name[i]))
                {
                    handler[i-1] = name[i];
                    i++;
                }
                handler[i-1] = 0;

                if (name[i] == ',')
                {
                    i++;
                    if (name[i] == ',')
                    {
                        doFilter = FALSE;
                    }
                    else
                    {
                        doFilter = TRUE;
                        while (name[i] && (name[i] != '»') && (name[++i] != ','))
                        {
                        }
                    }

                    if (name[i])
                    {
                        i++;
                        if (name[i] == ',')
                        {
                            doTrans = FALSE;
                        }
                        else
                        {
                            doTrans = TRUE;
                            while (name[i] && (name[i] != '»') && (name[++i] != ','))
                            {
                            }
                        }

                        j = 0;
                        if (name[i] == ',')
                        {
                            i++;
                            while (name[i] && (name[i] != '»'))
                                transName[j++] = name[i++];
                        }
                        transName[j] = 0;

                        HandlerControl(handler,doFilter,doTrans,transName);
                    }
                }
            }
            ptr++;
        }
    }

    if (cdcDiskObj && allocated)
    {
        FreeDiskObject(cdcDiskObj);
        cdcDiskObj = NULL;
    }

    return(TRUE);
}


/*****************************************************************************/


VOID ProcessCustomCxMsg(ULONG cmd)
{
}


/*****************************************************************************/


VOID ProcessCustomCxSig()
{
    if (currentHandler)
    {
        HandlerInfo(currentHandler->ln_Name,&doFilter,&doTrans,&transType);
        RefreshGads();
    }
}


/*****************************************************************************/


VOID ProcessCustomCxCmd(ULONG cmd)
{
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

        case IDCMP_DISKINSERTED : ok = ProcessCommand(CMD_NEWDISK);
                                  break;

        case IDCMP_GADGETUP     :
        case IDCMP_GADGETDOWN   : ok = ProcessCommand((UWORD)gad->UserData);
                                  break;
    }

    return(ok);
}


/*****************************************************************************/


BOOL CreateCustomGadgets(VOID)
{
struct NewGadget  ng;
struct Gadget    *gad;

    SetFont(cxWindow->RPort,topazFont);

    currentHandler = NULL;
    transType      = NULL;
    doFilter       = FALSE;
    doTrans        = FALSE;

    ProcessHandlerList(TRUE);
    if (BuildTransList(CROSSDOS_TRANS,"#?.crossdos",9))
    {
        if (gad = CreateContext(&cxGadgets))
        {
            /* Some invariants */
            ng.ng_TextAttr   = &topazAttr;
            ng.ng_GadgetID   = 0;
            ng.ng_Flags      = 0;
            ng.ng_VisualInfo = cxVisualInfo;

            ng.ng_LeftEdge   = cxWindow->BorderLeft+8;
            ng.ng_TopEdge    = cxWindow->BorderTop+6;
            ng.ng_Width      = 110;
            ng.ng_Height     = 53;
            ng.ng_GadgetText = NULL;
            ng.ng_UserData   = (APTR)CMD_DEVICES;

            deviceGad = CreateGadget(LISTVIEW_KIND,gad,&ng,GTLV_Labels, &handlerList,
                                                           GTLV_ShowSelected, NULL,
                                                           LAYOUTA_SPACING,   1,
                                                           GTLV_ScrollWidth,  18,
                                                           TAG_DONE);

            ng.ng_LeftEdge   = cxWindow->BorderLeft+131;
            ng.ng_TopEdge    = cxWindow->BorderTop+5;
            ng.ng_Width      = 16;
            ng.ng_Height     = 16;
            ng.ng_GadgetText = GetString(MSG_CD_FILTER_GAD);
            ng.ng_Flags      = PLACETEXT_RIGHT;
            ng.ng_UserData   = (APTR)CMD_FILTER;

            filterGad = CreateGadget(CHECKBOX_KIND,deviceGad,&ng,GTCB_Checked,FALSE,
                                                                 GA_Disabled, TRUE,
                                                                 TAG_DONE);

            ng.ng_TopEdge    = cxWindow->BorderTop+18;
            ng.ng_Width      = 16;
            ng.ng_Height     = 16;
            ng.ng_GadgetText = GetString(MSG_CD_TRANS_GAD);
            ng.ng_UserData   = (APTR)CMD_TRANS;

            transGad = CreateGadget(CHECKBOX_KIND,filterGad,&ng,GTCB_Checked,FALSE,
                                                                GA_Disabled, TRUE,
                                                                TAG_DONE);

            ng.ng_TopEdge    = cxWindow->BorderTop+45;
            ng.ng_Width      = 178;
            ng.ng_Height     = 14;
            ng.ng_GadgetText = GetString(MSG_CD_TRANSTYPE_GAD);
            ng.ng_Flags      = PLACETEXT_ABOVE;
            ng.ng_UserData   = (APTR)CMD_TRANSLIST;

            transListGad = CreateGadget(CYCLE_KIND,transGad,&ng,GTCY_Labels, transArray,
                                                                GA_Disabled, currentHandler == NULL,
                                                                TAG_DONE);

            if (transListGad)
            {
                AddGList(cxWindow,cxGadgets,-1,-1,NULL);
                RefreshGList(cxGadgets,cxWindow,NULL,-1);
                GT_RefreshWindow(cxWindow,NULL);
                return(TRUE);
            }

            FreeGadgets(cxGadgets);
            cxGadgets = NULL;
        }
    }

    Terminate();

    return(FALSE);
}


/*****************************************************************************/


struct NewMenu NM[] =
{
    {NM_TITLE,  NULL,        0, 0, 0, (APTR)CMD_NOP,  },
      {NM_ITEM, NULL,        0, 0, 0, (APTR)CMD_SAVE, },
      {NM_ITEM, NM_BARLABEL, 0, 0, 0, (APTR)CMD_NOP,  },
      {NM_ITEM, NULL,        0, 0, 0, (APTR)CMD_HIDE, },
      {NM_ITEM, NULL,        0, 0, 0, (APTR)CMD_QUIT, },

    {NM_END,    0,           0, 0, 0, (APTR)CMD_NOP,  }
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
    DoItem(1,MSG_CD_PROJECT_SAVE);
    DoItem(3,MSG_PROJECT_HIDE);
    DoItem(4,MSG_PROJECT_QUIT);

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


VOID SaveCDCIcon(VOID)
{
STRPTR             *ttypes;
STRPTR             *new;
BOOL                allocated;
UWORD               i;
UWORD               len;
UWORD               cnt;
struct Node        *node;
BOOL                doFilter;
BOOL                doTrans;
STRPTR              transType;

    allocated = FALSE;
    if (!cdcDiskObj)
    {
        cdcDiskObj = GetDiskObject("PROGDIR:CrossDOS");
        allocated = TRUE;
    }

    if (cdcDiskObj)
    {
        i    = 0;
        node = handlerList.lh_Head;
        while (node->ln_Succ)
        {
            i++;
            node = node->ln_Succ;
        }

        if (ttypes = cdcDiskObj->do_ToolTypes)
        {
            cnt = 0;
            while (ttypes[cnt])
            {
                if (ttypes[cnt][0] != '«')
                    i++;
                cnt++;
            }
        }

        if (new = AllocVec((i+1)*4,MEMF_CLEAR))
        {
            i    = 0;
            node = handlerList.lh_Head;
            while (node->ln_Succ)
            {
                HandlerInfo(node->ln_Name,&doFilter,&doTrans,&transType);
                len = strlen(node->ln_Name) + 7 + 7 + 4;
                if (transType)
                    len += strlen(transType) + 1;

                if (new[i] = AllocVec(len,MEMF_CLEAR))
                {
                    strcpy(new[i],"«");
                    strcat(new[i],node->ln_Name);
                    strcat(new[i],",");
                    if (doFilter)
                        strcat(new[i],"FILTER");
                    strcat(new[i],",");
                    if (doTrans)
                        strcat(new[i],"TRANS");
                    strcat(new[i],",");
                    if (transType)
                        strcat(new[i],transType);
                    strcat(new[i],"»");
                }
                i++;
                node = node->ln_Succ;
            }

            if (ttypes)
            {
                cnt = 0;
                while (ttypes[cnt])
                {
                    if (ttypes[cnt][0] != '«')
                    {
                        new[i++] = ttypes[cnt];
                    }
                    cnt++;
                }
            }

            cdcDiskObj->do_ToolTypes = new;
            PutDiskObject("PROGDIR:CrossDOS",cdcDiskObj);
            cdcDiskObj->do_ToolTypes = ttypes;

            i    = 0;
            node = handlerList.lh_Head;
            while (node->ln_Succ)
            {
                FreeVec(new[i++]);
                node = node->ln_Succ;
            }
            FreeVec(new);
        }
    }

    if (cdcDiskObj && allocated)
    {
        FreeDiskObject(cdcDiskObj);
        cdcDiskObj = NULL;
    }
}


/*****************************************************************************/


VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)&list->lh_Head;
}


/*****************************************************************************/


struct Node *FindNameNC(struct List *list, STRPTR name)
{
struct Node *node;
WORD         result;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
        result = Stricmp(name,node->ln_Name);
        if (result == 0)
            return(node);

	node = node->ln_Succ;
    }

    return(NULL);
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


ULONG FindNodeNum(struct List *list, STRPTR name)
{
struct Node *node;
struct Node *current;
ULONG        number;

    if (!name)
        name = dummyNode.ln_Name;

    node = FindNameNC(list,name);

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


VOID HandlerControl(STRPTR handlerName, BOOL doFilter, BOOL doTrans, STRPTR transType)
{
struct CrossDOSLock    *sem;
struct CrossDOSHandler *hdlr;
struct CrossDOSTrans   *trans;
struct DosList         *dl;
struct MsgPort         *replyPort;
struct DosPacket       *packet;
BPTR                    file;
char                    transName[60];
UWORD                   flags;
BOOL                    result;
UWORD                   len;
BOOL                    sent;
BOOL                    changed;

    Forbid();

    if (!(sem = (struct CrossDOSLock *)FindSemaphore(CROSSDOS_NAME)))
    {
        if (sem = AllocVec(sizeof(struct CrossDOSLock)+strlen(CROSSDOS_NAME)+1,MEMF_CLEAR))
        {
            sem->cdl_StructSize = sizeof(struct CrossDOSLock);
            InitSemaphore(sem);
            sem->cdl_Lock.ss_Link.ln_Name = (STRPTR)((ULONG)sem+sizeof(struct CrossDOSLock));
            strcpy(sem->cdl_Lock.ss_Link.ln_Name,CROSSDOS_NAME);
            NewList(&sem->cdl_Handlers);
            NewList(&sem->cdl_TransTables);
            AddSemaphore(sem);
        }
    }

    if (sem)
        ObtainSemaphore(sem);

    Permit();

    if (sem)
    {
        changed = FALSE;

        if (hdlr = (struct CrossDOSHandler *)FindNameNC(&sem->cdl_Handlers,handlerName))
        {
            trans = NULL;
            if (transType && (transType != dummyNode.ln_Name))
            {
                strcpy(transName,transType);
                len = strlen(transName);
                if ((len < 9) || Stricmp(&transName[len-9],".crossdos"))
                    strcat(transName,".crossdos");

                if (!(trans = (struct CrossDOSTrans *)FindNameNC(&sem->cdl_TransTables,transName)))
                {
                    if (trans = AllocVec(sizeof(struct CrossDOSTrans)+strlen(transName)+1,MEMF_CLEAR))
                    {
                        /* Once added to the list of translation
                         * tables, this memory should never be deallocated
                         * as CrossDOS can refer to it async
                         */

                        trans->cdt_StructSize = sizeof(struct CrossDOSTrans);
                        trans->cdt_Link.ln_Name = (STRPTR)((ULONG)trans+sizeof(struct CrossDOSTrans));
                        strcpy(trans->cdt_Link.ln_Name,transName);

                        strcpy(transName,CROSSDOS_TRANS);
                        AddPart(transName,trans->cdt_Link.ln_Name,sizeof(transName));

                        result = FALSE;
                        if (file = Open(transName,MODE_OLDFILE))
                        {
                            if (Read(file,trans->cdt_AToM,512) == 512)
                            {
                                /* The above Read() call fills in both cdt_AToM and cdt_MToA */
                                AddTail(&sem->cdl_TransTables,trans);
                                result = TRUE;
                            }
                            Close(file);
                        }

                        if (!result)
                        {
                            FreeVec(trans);
                            trans = NULL;
                        }
                    }
                }
            }

            flags = 0;
            if (doFilter)
                flags |= CDF_FILTER;

            if (doTrans)
                flags |= CDF_TRANSLATE;

            if ((hdlr->cdh_Flags != flags) || (hdlr->cdh_TransTable != trans))
                changed = TRUE;

            hdlr->cdh_Flags      = flags;
            hdlr->cdh_TransTable = trans;
        }

        ReleaseSemaphore(sem);

        if (changed)
        {
            if (replyPort = CreateMsgPort())
            {
                if (packet = AllocDosObject(DOS_STDPKT,0))
                {
                    sent = FALSE;

                    dl = LockDosList(LDF_DEVICES|LDF_READ);
                    if (dl = FindDosEntry(dl,handlerName,LDF_DEVICES | LDF_READ))
                    {
                        if (dl->dol_Task)
                        {
                            packet->dp_Type = ACTION_EVENT;
                            SendPkt(packet,dl->dol_Task,replyPort);
                            sent = TRUE;
                        }
                    }
                    UnLockDosList(LDF_DEVICES|LDF_READ);

                    if (sent)
                        WaitPort(replyPort);

                    FreeDosObject(DOS_STDPKT,packet);
                }
                DeleteMsgPort(replyPort);
            }
        }
    }
}


/*****************************************************************************/


VOID HandlerInfo(STRPTR handlerName, BOOL *doFilter, BOOL *doTrans, STRPTR *transType)
{
struct CrossDOSLock    *sem;
struct CrossDOSHandler *hdlr;

    *doFilter  = FALSE;
    *doTrans   = FALSE;
    *transType = NULL;

    Forbid();

    if (sem = (struct CrossDOSLock *)FindSemaphore(CROSSDOS_NAME))
        ObtainSemaphoreShared(sem);

    Permit();

    if (sem)
    {
        if (hdlr = (struct CrossDOSHandler *)FindNameNC(&sem->cdl_Handlers,handlerName))
        {
            if (CDF_FILTER & hdlr->cdh_Flags)
                *doFilter = TRUE;

            if (CDF_TRANSLATE & hdlr->cdh_Flags)
                *doTrans = TRUE;

            if (hdlr->cdh_TransTable)
                *transType = hdlr->cdh_TransTable->cdt_Link.ln_Name;
        }

        ReleaseSemaphore(sem);
    }
}


/*****************************************************************************/


VOID ProcessHandlerList(BOOL initial)
{
struct CrossDOSLock    *sem;
struct CrossDOSHandler *hdlr;
struct Node            *new;
struct Node            *node;
struct Task            *task;

    Forbid();

    if (sem = (struct CrossDOSLock *)FindSemaphore(CROSSDOS_NAME))
        ObtainSemaphore(sem);

    Permit();

    if (sem)
    {
        task = FindTask(NULL);

        hdlr = (struct CrossDOSHandler *)sem->cdl_Handlers.lh_Head;
        while (hdlr->cdh_Link.ln_Succ)
        {
            if (initial)
            {
                if (new = AllocVec(sizeof(struct Node) + strlen(hdlr->cdh_Link.ln_Name) + 1,MEMF_CLEAR))
                {
                    new->ln_Name = (STRPTR)((ULONG)new + sizeof(struct Node));
                    strcpy(new->ln_Name,hdlr->cdh_Link.ln_Name);

                    node = handlerList.lh_Head;
                    while (node->ln_Succ)
                    {
                        if (Stricmp(node->ln_Name,new->ln_Name) >= 0)
                            break;
                        node = node->ln_Succ;
                    }
                    Insert(&handlerList,(struct Node *)new,node->ln_Pred);
                }

                if (!hdlr->cdh_NotifyTask)
                {
                    hdlr->cdh_NotifyTask   = task;
                    hdlr->cdh_NotifySigBit = cxSignal;
                }
            }
            else
            {
                if (hdlr->cdh_NotifyTask == task)
                {
                    hdlr->cdh_NotifyTask   = NULL;
                    hdlr->cdh_NotifySigBit = 0;
                }
            }

            hdlr = (struct CrossDOSHandler *)hdlr->cdh_Link.ln_Succ;
        }

        ReleaseSemaphore(sem);
    }
}


/*****************************************************************************/


BOOL BuildTransList(STRPTR directory, STRPTR pattern, WORD strip)
{
UBYTE                exAllBuffer[512];
struct ExAllControl *eac;
struct ExAllData    *ead;
BPTR                 lock;
BOOL                 more;
char                 pat[20];
BOOL                 ok;
UWORD                len;
STRPTR		     name;
struct Node         *node;
struct Node         *new;
UWORD                cnt;

    dummyNode.ln_Name = "ASCII-7";
    AddHead(&transList,&dummyNode);

    cnt = 2;  /* 1 for the dummy node, 1 for the empty slot at the end of the array */

    ok = FALSE;
    if (eac = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL,0))
    {
        ParsePatternNoCase(pattern,pat,20);
        eac->eac_LastKey     = 0;
        eac->eac_MatchString = pat;

        ok = TRUE;
        if (lock = Lock(directory,ACCESS_READ))
        {
            do
            {
                more = ExAll(lock,(struct ExAllData *)exAllBuffer,sizeof(exAllBuffer),ED_TYPE,eac);
                if ((!more) && (IoErr() != ERROR_NO_MORE_ENTRIES))
                {
                    ok = FALSE;
                    break;
                }

                if (eac->eac_Entries > 0)
                {
                    ead = (struct ExAllData *) exAllBuffer;
                    do
                    {
                        if (ead->ed_Type < 0)
                        {
		            name = (STRPTR)ead->ed_Name;
		    	    len  = strlen(name) - strip;

			    if (new = AllocVec(sizeof(struct Node)+len+1,MEMF_CLEAR))
		            {
			        new->ln_Name = (STRPTR)((ULONG)new + sizeof(struct Node));
                                CopyMem(name,new->ln_Name,len);

                                node = transList.lh_Head;
                                while (node->ln_Succ)
                                {
                                    if (Stricmp(node->ln_Name,new->ln_Name) >= 0)
                                        break;
                                    node = node->ln_Succ;
                                }
                                Insert(&transList,new,node->ln_Pred);
                                cnt++;
			    }
                            else
                            {
                                ok = FALSE;
                                break;
                            }
                        }
                        ead = ead->ed_Next;
                    }
                    while (ead);
                }
            }
            while (more);

            UnLock(lock);
        }
        else
        {
            if (IoErr() != ERROR_OBJECT_NOT_FOUND)
                ok = FALSE;
        }

        FreeDosObject(DOS_EXALLCONTROL,eac);
    }

    if (ok && (transArray = AllocVec(cnt*4,MEMF_CLEAR)))
    {
        node = transList.lh_Head;
        cnt  = 0;
        while (node->ln_Succ)
        {
            transArray[cnt++] = node->ln_Name;
            node = node->ln_Succ;
        }
    }
    else
    {
        ok = FALSE;
    }

    return(ok);
}


/*****************************************************************************/


VOID EmptyList(struct List *list)
{
struct Node *node;

    while (node = RemHead(list))
    {
        if (node != &dummyNode)
            FreeVec(node);
    }
}


/*****************************************************************************/


VOID Terminate(VOID)
{
    ProcessHandlerList(FALSE);
    EmptyList(&handlerList);
    EmptyList(&transList);
    FreeVec(transArray);
    transArray = NULL;
    currentHandler = NULL;
}


/*****************************************************************************/


VOID RefreshGads(VOID)
{
char transName[32];

    GT_SetGadgetAttrs(filterGad,cxWindow,NULL,GTCB_Checked, doFilter,
                                              GA_Disabled,  currentHandler == NULL,
                                              TAG_DONE);

    GT_SetGadgetAttrs(transGad,cxWindow,NULL,GTCB_Checked, doTrans,
                                             GA_Disabled,  currentHandler == NULL,
                                             TAG_DONE);

    if (transType)
    {
        strcpy(transName,transType);
        transName[strlen(transName)-9] = 0;
    }
    else
    {
        strcpy(transName,dummyNode.ln_Name);
    }

    GT_SetGadgetAttrs(transListGad,cxWindow,NULL,GTCY_Active, FindNodeNum(&transList,transName),
                                                 GA_Disabled, currentHandler == NULL,
                                                 TAG_DONE);
}


/*****************************************************************************/


VOID RefreshWindow(BOOL refreshEvent)
{
    if (refreshEvent)
    {
        GT_BeginRefresh(cxWindow);
        GT_EndRefresh(cxWindow,TRUE);
    }
}


/*****************************************************************************/


VOID HideInterface(VOID)
{
    DisposeWindow();
    Terminate();
}


/*****************************************************************************/


BOOL ProcessCommand(UWORD cmd)
{
struct Node *node;
char         temp[50];

    switch (cmd)
    {
        case CMD_HIDE       : HideInterface();
                              break;

        case CMD_QUIT       : return(FALSE);

        case CMD_SAVE       : SaveCDCIcon();
                              break;

        case CMD_DEVICES    : currentHandler = FindNum(&handlerList,icode);
                              HandlerInfo(currentHandler->ln_Name,&doFilter,&doTrans,&transType);
                              RefreshGads();
                              break;

        case CMD_FILTER     : if (currentHandler)
                              {
                                  doFilter = (GFLG_SELECTED & filterGad->Flags);
                                  HandlerControl(currentHandler->ln_Name,doFilter,doTrans,transType);
                              }
                              break;

        case CMD_TRANS      : if (currentHandler)
                              {
                                  doTrans = (GFLG_SELECTED & transGad->Flags);
                                  HandlerControl(currentHandler->ln_Name,doFilter,doTrans,transType);
                              }
                              break;

        case CMD_TRANSLIST  : if (currentHandler)
                              {
                                  node = FindNum(&transList,icode);
                                  transType = node->ln_Name;
                                  HandlerControl(currentHandler->ln_Name,doFilter,doTrans,transType);
                              }
                              break;

        case CMD_NEWDISK    : temp[0] = 0;
                              if (currentHandler)
                              {
                                  stccpy(temp,currentHandler->ln_Name,sizeof(temp));
                                  currentHandler = NULL;
                              }

                              GT_SetGadgetAttrs(deviceGad,cxWindow,NULL,GTLV_Labels, ~0,
                                                                        TAG_DONE);

                              ProcessHandlerList(FALSE);
                              EmptyList(&handlerList);
                              ProcessHandlerList(TRUE);

                              if (currentHandler = FindNameNC(&handlerList,temp))
                                  HandlerInfo(currentHandler->ln_Name,&doFilter,&doTrans,&transType);

                              GT_SetGadgetAttrs(deviceGad,cxWindow,NULL,GTLV_Labels,   &handlerList,
                                                                        GTLV_Selected, FindNodeNum(&handlerList,temp),
                                                                        TAG_DONE);

                              RefreshGads();
                              break;
    }

    return(TRUE);
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
    if (topazFont = OpenFont(&topazAttr))
    {
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID DisposeCustomCx(VOID)
{
    CloseFont(topazFont);
    Terminate();
}
