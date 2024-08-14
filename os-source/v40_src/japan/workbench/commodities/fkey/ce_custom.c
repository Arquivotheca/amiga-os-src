
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
#include <dos/dostags.h>
#include <rexx/storage.h>
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
#include <clib/rexxsyslib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/commodities_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>

#include "ce_custom.h"
#include "ce_window.h"
#include "ce_strings.h"


/*****************************************************************************/


VOID kprintf(STRPTR,...);

struct KeySequence
{
    struct Node        fk_Node;
    UWORD	       fk_Cmd;
    STRPTR	       fk_Args;

    /* for use with CMD_INSERT command */
    struct InputEvent *fk_KeyEvents;
};


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
extern struct MsgPort       *cxPort;
extern CxObj                *cxBroker;
extern struct Gadget        *cxGadgets;
extern struct Menu          *cxMenus;
extern struct Window        *cxWindow;
extern APTR                  cxVisualInfo;
extern struct DrawInfo      *cxDrawInfo;

/* from IntuiMessage */
ULONG              class;
UWORD              icode;
struct Gadget     *gad;

struct Gadget      *keysGad;
struct Gadget      *addGad;
struct Gadegt      *removeGad;
struct Gadget      *seqGad;
struct Gadget      *cmdGad;
struct Gadget      *argGad;
STRPTR              cmdLabels[9];
struct List         keyList;
struct KeySequence *currentKey;
CxObj              *mainFilter;
struct DiskObject  *fkeyDiskObj;
struct Library     *RexxSysBase;


/*****************************************************************************/


struct InputXpression filterIX =
{
    IX_VERSION,
    IECLASS_RAWKEY,

    0,
    0,

    0,
    0,
    0
};


/*****************************************************************************/


/* UI commands */
#define CMD_NOP        0
#define CMD_HIDE       1
#define CMD_QUIT       2
#define CMD_KEYLIST    3
#define CMD_ADD        4
#define CMD_REMOVE     5
#define CMD_SEQUENCE   6
#define CMD_COMMAND    7
#define CMD_ARGS       8
#define CMD_SAVE       9


/*****************************************************************************/


/* Hot key commands */
#define CMD_CYCLEW   0
#define CMD_CYCLES   1
#define CMD_MAKEBIG  2
#define CMD_MAKESMAL 3
#define CMD_TOGGLE   4
#define CMD_INSERT   5
#define CMD_PROGRAM  6
#define CMD_AREXX    7


/*****************************************************************************/


/* Name of hot key commands, for icon tool types */
STRPTR Commands[] =
{
    "CYCLE",
    "CYCLESCREEN",
    "MAKEBIG",
    "MAKESMALL",
    "ZIPWINDOW",
    "INSERT",
    "RUN",
    "AREXX",
    NULL
};


/*****************************************************************************/


#define CONSOLE_SPEC "CON:0/25/640/150/FKey/AUTO/CLOSE/WAIT"


/*****************************************************************************/


struct TextAttr far topazAttr =
{
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};


/*****************************************************************************/


BOOL ProcessCommand(UWORD cmd);
struct InputEvent * __stdargs InvertString(STRPTR,APTR);
VOID __stdargs FreeIEvents(struct InputEvent *);


/*****************************************************************************/


VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)&list->lh_Head;
}


/*****************************************************************************/


struct Node *FindNum(struct List *list, UWORD number)
{
struct Node *node;

    node = list->lh_Head;
    while (node->ln_Succ && number--)
	node = node->ln_Succ;

    if (node->ln_Succ)
        return(node);

    return(NULL);
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


LONG WB2CLI(struct WBStartup *wbMsg, ULONG defaultStack)
{
struct Process		    *process;
struct CommandLineInterface *cli;
struct MsgPort		    *wbPort;
struct Process		    *wbProc;
struct CommandLineInterface *wbCLI;
ULONG			    *wbPath;
ULONG			    *lastPath;
ULONG			    *tmp;
STRPTR                       prompt;

    process = (struct Process *)FindTask(NULL);
    cli     = BADDR(process->pr_CLI);

    if (!cli && wbMsg && (cli = AllocDosObjectTagList(DOS_CLI,NULL)))
    {
        cli->cli_DefaultStack  = ((defaultStack+3) / 4);
        process->pr_CLI        = MKBADDR(cli);
        process->pr_Flags     |= PRF_FREECLI;

        Forbid();

        if (wbPort = wbMsg->sm_Message.mn_ReplyPort)
        {
            if ((wbPort->mp_Flags & PF_ACTION) == PA_SIGNAL)
            {
                if (wbProc = wbPort->mp_SigTask)
                {
                    if (wbProc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
                    {
                        if (wbCLI = BADDR(wbProc->pr_CLI))
                        {
			    prompt = BADDR(wbCLI->cli_Prompt);
                            if (prompt)
                                SetPrompt(&(prompt[1]));

                            wbPath   = BADDR(wbCLI->cli_CommandDir);
                            lastPath = &(cli->cli_CommandDir);
                            while (wbPath)
                            {
                                if (wbPath[1])
                                {
                                    if (tmp = AllocVec(8,MEMF_CLEAR))
                                    {
                                        if (!(tmp[1] = DupLock(wbPath[1])))
                                        {
                                            FreeVec(tmp);
                                            break;
                                        }
                                        lastPath[0] = MKBADDR(tmp);
                                        lastPath    = tmp;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }

                                wbPath = BADDR(wbPath[0]);
                            }
                        }
                    }
                }
            }
        }

        Permit();
    }

    return((LONG)cli);
}


/*****************************************************************************/


VOID CycleWindows(VOID)
{
LONG           lock;
struct Window *window;
struct Screen *screen;
struct Layer  *rearLayer = NULL;
struct Layer  *layer;
struct Window *layerWindow;

    lock = LockIBase(0);

    if (window = IntuitionBase->ActiveWindow)
    {
        screen = window->WScreen;

        /* find rearmost layer which is not a backdrop window,
         * nor the bar layer, nor a WBENCHWINDOW
         */
        for (layer = screen->LayerInfo.top_layer; layer; layer = layer->back)
        {
            layerWindow = (struct Window *) layer->Window;

            if ((layer != screen->BarLayer)
            && (!(layer->Flags & LAYERBACKDROP))
            && (layerWindow))
            {
                rearLayer = layer;
            }
        }
    }

    UnlockIBase(lock);

    if (rearLayer)
    {
        layerWindow = (struct Window *) rearLayer->Window;
        WindowToFront(layerWindow);
        ActivateWindow(layerWindow);
    }
}


/*****************************************************************************/


VOID CycleScreens(VOID)
{
    ScreenToBack(IntuitionBase->FirstScreen);
}


/*****************************************************************************/


VOID ToggleWindow(VOID)
{
struct Window *window;

    if (window = IntuitionBase->ActiveWindow)
        if (!(window->IDCMPFlags & IDCMP_SIZEVERIFY))
            if (window->Flags & WFLG_HASZOOM)
                ZipWindow(window);
}


/*****************************************************************************/


#define IMINWIDTH  40
#define IMINHEIGHT 30
#define MIN(A,B)   (((A)<(B))?(A):(B))
#define MAX(A,B)   (((A)>(B))?(A):(B))


VOID WindowSize(BOOL makeBig)
{
ULONG          lock;
struct Window *window;
struct Screen *screen;
SHORT          deltaw = 0;
SHORT          deltah = 0;
ULONG          sizing = FALSE;

    lock = LockIBase(0);

    if (window = IntuitionBase->ActiveWindow)
    {
        screen = window->WScreen;

        if (makeBig)
        {
            deltaw = MIN(screen->Width - window->LeftEdge, (unsigned) window->MaxWidth) - window->Width;
            deltah = MIN(screen->Height - window->TopEdge, (unsigned) window->MaxHeight) - window->Height;
        }
        else
        {
            deltaw = MAX(window->MinWidth, IMINWIDTH) - window->Width;
            deltah = MAX(window->MinHeight, IMINHEIGHT) - window->Height;
        }

        sizing = (window->Flags & WFLG_SIZEGADGET) && (!(window->IDCMPFlags & IDCMP_SIZEVERIFY));
    }

    UnlockIBase(lock);

    if (sizing)
        SizeWindow(window,deltaw,deltah);
}


/*****************************************************************************/


VOID StartProgram(STRPTR progName)
{
BPTR file;

    if (file = Open(CONSOLE_SPEC,MODE_NEWFILE))
    {
        if (SystemTags(progName,SYS_UserShell, TRUE,
                                SYS_Asynch,    TRUE,
                                SYS_Input,     file,
                                SYS_Output,    NULL,
                                TAG_DONE))
        {
            Close(file);
        }
    }
}


/*****************************************************************************/


VOID StartScript(STRPTR scriptName)
{
APTR            arg;
struct RexxMsg *msg;
struct MsgPort *rexx;
struct Process *process;

    process = (struct Process *)FindTask(NULL);

    if (rexx = FindPort("AREXX"))
    {
        if (RexxSysBase = OpenLibrary("rexxsyslib.library",36))
        {
            if (arg = CreateArgstring(scriptName,strlen(scriptName)))
            {
                if (msg = CreateRexxMsg(&process->pr_MsgPort,NULL,NULL))
                {
                    msg->rm_Action  = RXCOMM;
                    msg->rm_Args[0] = arg;
                    msg->rm_Stdin   = NULL;
                    msg->rm_Stdout  = NULL;

                    Forbid();
                    if (rexx = FindPort("AREXX"))
                    {
                        PutMsg(rexx,msg);
                        WaitPort(&process->pr_MsgPort);
                        GetMsg(&process->pr_MsgPort);
                    }
                    Permit();

                    DeleteRexxMsg(msg);
                }
                DeleteArgstring(arg);
            }
            CloseLibrary(RexxSysBase);
        }
    }
}


/*****************************************************************************/


struct KeySequence *GetSequence(STRPTR sequence, UWORD cmd, STRPTR args)
{
struct KeySequence *key;
UWORD               len;

    len = sizeof(struct KeySequence) + strlen(sequence) + 2;

    if (cmd >= CMD_INSERT)
        len += strlen(args);

    if (key = AllocVec(len,MEMF_CLEAR))
    {
        key->fk_Node.ln_Name = (APTR)((ULONG)key+sizeof(struct KeySequence));
        key->fk_Cmd          = cmd;
        key->fk_Args         = (APTR)((ULONG)key->fk_Node.ln_Name + strlen(sequence)+1);
        strcpy(key->fk_Node.ln_Name,sequence);

        if (cmd >= CMD_INSERT)
        {
            strcpy(key->fk_Args,args);

            if (cmd == CMD_INSERT)
                key->fk_KeyEvents = InvertString(args,NULL);
        }
    }

    return(key);
}


/*****************************************************************************/


VOID FreeSequence(struct KeySequence *key)
{
    FreeIEvents(key->fk_KeyEvents);
    FreeVec(key);
}


/*****************************************************************************/


struct KeySequence *UpdateSequence(struct KeySequence *key, STRPTR sequence,
                                   STRPTR args)
{
struct KeySequence *new;

    if (key)
    {
        if (new = GetSequence(sequence,key->fk_Cmd,args))
        {
            Insert(&keyList,new,key);
            Remove(key);
            FreeSequence(key);
            return(new);
        }
    }

    return(key);
}


/*****************************************************************************/


VOID SaveFKeyIcon(VOID)
{
STRPTR             *ttypes;
STRPTR             *new;
BOOL                allocated;
UWORD               i;
UWORD               len;
UWORD               cnt;
struct KeySequence *key;

    allocated = FALSE;
    if (!fkeyDiskObj)
    {
        fkeyDiskObj = GetDiskObject("PROGDIR:FKey");
        allocated = TRUE;
    }

    if (fkeyDiskObj)
    {
        i   = 0;
        key = (struct KeySequence *)keyList.lh_Head;
        while (key->fk_Node.ln_Succ)
        {
            i++;
            key = (struct KeySequence *)key->fk_Node.ln_Succ;
        }

        if (ttypes = fkeyDiskObj->do_ToolTypes)
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
            i   = 0;
            key = (struct KeySequence *)keyList.lh_Head;
            while (key->fk_Node.ln_Succ)
            {
                len = sizeof(struct KeySequence)+strlen(key->fk_Node.ln_Name)+strlen(key->fk_Args)+6;
                if (new[i] = AllocVec(len,MEMF_CLEAR))
                {
                    strcpy(new[i],"«");
                    strcat(new[i],key->fk_Node.ln_Name);
                    strcat(new[i],"» ");
                    strcat(new[i],Commands[key->fk_Cmd]);

                    if (key->fk_Cmd >= CMD_INSERT)
                    {
                        strcat(new[i]," ");
                        strcat(new[i],key->fk_Args);
                    }
                }
                i++;
                key = (struct KeySequence *)key->fk_Node.ln_Succ;
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

            fkeyDiskObj->do_ToolTypes = new;
            PutDiskObject("PROGDIR:FKey",fkeyDiskObj);
            fkeyDiskObj->do_ToolTypes = ttypes;

            i   = 0;
            key = (struct KeySequence *)keyList.lh_Head;
            while (key->fk_Node.ln_Succ)
            {
                FreeVec(new[i++]);
                key = (struct KeySequence *)key->fk_Node.ln_Succ;
            }
            FreeVec(new);
        }

        if (allocated)
        {
            FreeDiskObject(fkeyDiskObj);
            fkeyDiskObj = NULL;
        }
    }
}


/*****************************************************************************/


BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObject *diskObj,
                       ULONG *cliOpts)
{
STRPTR             *ptr;
STRPTR              name;
STRPTR              cmd;
UWORD               i,j;
UWORD               cmdCnt;
struct KeySequence *key;
struct Node        *node;
BOOL                allocated;

    NewList(&keyList);

    allocated = FALSE;
    if (!(fkeyDiskObj = diskObj))
    {
        fkeyDiskObj = GetDiskObject("PROGDIR:FKey");
        allocated = TRUE;
    }

    if (fkeyDiskObj && (ptr = fkeyDiskObj->do_ToolTypes))
    {
        while (name = *ptr)
        {
            if (name[0] == '«')
            {
                i = 0;
                name++;
                while (name[i] && (name[i] != '»'))
                    i++;

                if (name[i] == '»')
                {
                    name[i] = 0;

                    /* at this point, "name" contains the kbd sequence */

                    /* skip spaces */
                    j = 0;
                    cmd = &name[i+1];
                    while (cmd[j] && (cmd[j] == ' '))
                        j++;

                    cmd = &cmd[j];
                    while (cmd[j] && (cmd[j] != ' '))
                        j++;

                    cmd[j] = 0;

                    /* at this point, "cmd" has the name of the cmd to execute */

                    if (j)
                    {
                        cmdCnt = 0;
                        while (Commands[cmdCnt])
                        {
                            if (Stricmp(Commands[cmdCnt],cmd) == 0)
                            {
                                if (key = GetSequence(name,cmdCnt,&cmd[j+1]))
                                {
                                    node = keyList.lh_Head;
                                    while (node->ln_Succ)
                                    {
                                        if (Stricmp(node->ln_Name,key->fk_Node.ln_Name) >= 0)
                                            break;
                                        node = node->ln_Succ;
                                    }
                                    Insert(&keyList,(struct Node *)key,node->ln_Pred);
                                }
                                break;
                            }
                            cmdCnt++;
                        }
                        cmd[j] = ' ';
                    }
                    name[i] = '»';
                }
            }
            ptr++;
        }
    }

    if (fkeyDiskObj && allocated)
    {
        FreeDiskObject(fkeyDiskObj);
        fkeyDiskObj = NULL;
    }

    WB2CLI(wbMsg,4096);
    return(TRUE);
}


/*****************************************************************************/


VOID ProcessCustomCxMsg(ULONG cmd)
{
struct KeySequence *key;
struct KeySequence *check;

    key   = (struct KeySequence *)cmd;
    check = (struct KeySequence *)keyList.lh_Head;

    while (check->fk_Node.ln_Succ)
    {
        if (check == key)
        {
            switch (key->fk_Cmd)
            {
                case CMD_CYCLEW   : CycleWindows();
                                    break;

                case CMD_CYCLES   : CycleScreens();
                                    break;

                case CMD_MAKEBIG  : WindowSize(TRUE);
                                    break;

                case CMD_MAKESMAL : WindowSize(FALSE);
                                    break;

                case CMD_TOGGLE   : ToggleWindow();
                                    break;

                case CMD_PROGRAM  : StartProgram(key->fk_Args);
                                    break;

                case CMD_AREXX    : StartScript(key->fk_Args);
                                    break;
            }
            return;
        }
        check = (struct KeySequence *)check->fk_Node.ln_Succ;
    }
}


/*****************************************************************************/


VOID ProcessCustomCxSig()
{
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

    currentKey   = NULL;

    cmdLabels[0] = GetString(MSG_FK_CMD_CYCLEW);
    cmdLabels[1] = GetString(MSG_FK_CMD_CYCLES);
    cmdLabels[2] = GetString(MSG_FK_CMD_MAKEBIG);
    cmdLabels[3] = GetString(MSG_FK_CMD_MAKESMALL);
    cmdLabels[4] = GetString(MSG_FK_CMD_TOGGLE);
    cmdLabels[5] = GetString(MSG_FK_CMD_INSERT);
    cmdLabels[6] = GetString(MSG_FK_CMD_PROGRAM);
    cmdLabels[7] = GetString(MSG_FK_CMD_AREXX);
    cmdLabels[8] = NULL;

    if (gad = CreateContext(&cxGadgets))
    {
	/* Some invariants */
        ng.ng_TextAttr   = &topazAttr;
        ng.ng_GadgetID   = 0;
        ng.ng_Flags      = 0;
        ng.ng_VisualInfo = cxVisualInfo;

        ng.ng_TopEdge    = cxWindow->BorderTop+16;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+8;
        ng.ng_Width      = 260;
        ng.ng_Height     = 62;
        ng.ng_GadgetText = GetString(MSG_FK_DEFKEYS_GAD);
        ng.ng_UserData   = (APTR)CMD_KEYLIST;

        keysGad = CreateGadget(LISTVIEW_KIND,gad,&ng,GTLV_Labels,       &keyList,
                                                     LAYOUTA_SPACING,   1,
                                                     GTLV_ScrollWidth,  18,
                                                     GTLV_ShowSelected, NULL,
                                                     TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+74;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+8;
        ng.ng_Width      = 260;
        ng.ng_Height     = 14;
        ng.ng_GadgetText = NULL;
        ng.ng_UserData   = (APTR)CMD_SEQUENCE;

        seqGad = CreateGadget(STRING_KIND,keysGad,&ng,GA_Disabled,   TRUE,
                                                      GTST_MaxChars, 39,
                                                      TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+88;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+8;
        ng.ng_Width      = 130;
        ng.ng_Height     = 14;
        ng.ng_GadgetText = GetString(MSG_FK_ADDKEY_GAD);
        ng.ng_UserData   = (APTR)CMD_ADD;

        addGad = CreateGadget(BUTTON_KIND,seqGad,&ng,TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+88;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+138;
        ng.ng_Width      = 130;
        ng.ng_Height     = 14;
        ng.ng_GadgetText = GetString(MSG_FK_REMKEY_GAD);
        ng.ng_UserData   = (APTR)CMD_REMOVE;

        removeGad = CreateGadget(BUTTON_KIND,addGad,&ng,GA_Disabled, TRUE,
							TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+31;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+276;
        ng.ng_Width      = 240;
        ng.ng_Height     = 14;
        ng.ng_GadgetText = GetString(MSG_FK_CMD_GAD);
        ng.ng_Flags      = PLACETEXT_ABOVE;
        ng.ng_UserData   = (APTR)CMD_COMMAND;

        cmdGad = CreateGadget(CYCLE_KIND,removeGad,&ng,GTCY_Labels, &cmdLabels,
                                                       GA_Disabled, TRUE,
						       TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+67;
        ng.ng_LeftEdge   = cxWindow->BorderLeft+276;
        ng.ng_Width      = 240;
        ng.ng_Height     = 14;
        ng.ng_GadgetText = GetString(MSG_FK_PARMS_GAD);
        ng.ng_Flags      = PLACETEXT_ABOVE;
        ng.ng_UserData   = (APTR)CMD_ARGS;

        if (argGad = CreateGadget(STRING_KIND,cmdGad,&ng,GA_Disabled,   TRUE,
                                                         GTST_MaxChars, 255,
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
    {NM_TITLE,  NULL,        0, 0, 0, (APTR)CMD_NOP,  },
      {NM_ITEM, NULL,        0, 0, 0, (APTR)CMD_SAVE, },
      {NM_ITEM, NM_BARLABEL, 0, 0, 0, (APTR)CMD_NOP,  },
      {NM_ITEM, NULL,        0, 0, 0, (APTR)CMD_HIDE, },
      {NM_ITEM, NULL,        0, 0, 0, (APTR)CMD_QUIT, },

    {NM_END,    0,           0, 0, 0, (APTR)CMD_NOP,  }
};


/*****************************************************************************/


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
    DoItem(1,MSG_FK_SAVE);
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


VOID UpdateFilters(VOID)
{
struct KeySequence *key;
CxObj              *localFilter;

    DeleteCxObjAll(mainFilter);
    if (mainFilter = CxFilter(NULL))
    {
        key = (struct KeySequence *)keyList.lh_Head;
        while (key->fk_Node.ln_Succ)
        {
            if (localFilter = CxFilter(key->fk_Node.ln_Name))
            {
                if (key->fk_Cmd == CMD_INSERT)
                {
                    AttachCxObj(localFilter,CxTranslate(key->fk_KeyEvents));
                }
                else
                {
                    AttachCxObj(localFilter,CxSender(cxPort,(ULONG)key));
                    AttachCxObj(localFilter,CxTranslate(NULL));
                }
            }

            AttachCxObj(mainFilter,localFilter);
            key = (struct KeySequence *)key->fk_Node.ln_Succ;
        }

        AttachCxObj(cxBroker,mainFilter);
    }
}


/*
VOID PrintKeyList(STRPTR hdr)
{
struct KeySequence *key;

        kprintf("\n%s\n",hdr);
        key = (struct KeySequence *)keyList.lh_Head;
        while (key->fk_Node.ln_Succ)
        {
            kprintf("Sequence = %s, cmd = %ld, args = '%s', events = %lx\n",key->fk_Node.ln_Name,key->fk_Cmd,key->fk_Args,key->fk_KeyEvents);
            key = (struct KeySequence *)key->fk_Node.ln_Succ;
        }

}
*/


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


BOOL ProcessCommand(UWORD cmd)
{
struct KeySequence *key;
BOOL                refreshGads;
BOOL                updateFilters;
struct Gadget      *activate;
IX                  ix;
struct EasyStruct   ez;

    activate       = NULL;
    refreshGads    = FALSE;
    updateFilters  = FALSE;

    if (cxWindow)
    {
        GT_SetGadgetAttrs(keysGad,cxWindow,NULL,GTLV_Labels, ~0,
                                                TAG_DONE);

        if (((struct StringInfo *)seqGad->SpecialInfo)->Buffer[0])
        {
            currentKey = UpdateSequence(currentKey,((struct StringInfo *)seqGad->SpecialInfo)->Buffer,
                                                   ((struct StringInfo *)argGad->SpecialInfo)->Buffer);
        }
        else
        {
            if (currentKey)
            {
                Remove(currentKey);
                FreeSequence(currentKey);
                currentKey    = NULL;
                refreshGads   = TRUE;
                updateFilters = TRUE;
            }
        }
    }

    switch (cmd)
    {
        case CMD_SAVE       : UpdateFilters();
                              SaveFKeyIcon();
        		      break;

        case CMD_HIDE       : DisposeWindow();
                              UpdateFilters();
                              return(TRUE);
                              break;

        case CMD_QUIT       : return(FALSE);

        case CMD_KEYLIST    : currentKey  = (struct KeySequence *)FindNum(&keyList,icode);
                              refreshGads = TRUE;
                              activate    = seqGad;
                              break;

        case CMD_ADD        : if (key = GetSequence("",cmdGad->GadgetID,""))
                              {
                                  AddTail(&keyList,key);
                                  currentKey    = key;
                                  refreshGads   = TRUE;
                                  activate      = seqGad;
                                  updateFilters = TRUE;
                              }
                              break;

        case CMD_REMOVE     : if (currentKey)
                              {
                                  Remove(currentKey);
                                  FreeSequence(currentKey);
                                  currentKey    = NULL;
                                  refreshGads   = TRUE;
                                  updateFilters = TRUE;
                              }
                              break;

        case CMD_SEQUENCE   : if (currentKey)
                              {
                                  if (ParseIX(((struct StringInfo *)seqGad->SpecialInfo)->Buffer,&ix) != 0)
                                  {
                                      ez.es_StructSize   = sizeof(struct EasyStruct);
                                      ez.es_Flags        = 0;
                                      ez.es_Title        = GetString(MSG_FK_BADKEY_TITLE);
                                      ez.es_TextFormat   = GetString(MSG_FK_BADKEY_PROMPT);
                                      ez.es_GadgetFormat = GetString(MSG_FK_BADKEY_GAD);
                                      EasyRequestArgs(cxWindow,&ez,0,NULL);

                                      activate = seqGad;
                                  }
                                  else if (!(argGad->Flags & GFLG_DISABLED))
                                  {
                                      activate = argGad;
                                  }
                                  updateFilters = TRUE;
                              }
                              break;

        case CMD_COMMAND    : if (currentKey)
                              {
                                  GT_SetGadgetAttrs(argGad,cxWindow,NULL,GA_Disabled, icode < CMD_INSERT,
                                                                         TAG_DONE);
                                  cmdGad->GadgetID   = icode;
                                  currentKey->fk_Cmd = icode;
                                  updateFilters      = TRUE;

                                  currentKey = UpdateSequence(currentKey,((struct StringInfo *)seqGad->SpecialInfo)->Buffer,
                                                                         ((struct StringInfo *)argGad->SpecialInfo)->Buffer);
                              }
                              break;

        case CMD_ARGS       : if (currentKey)
                              {
                                  activate      = seqGad;
                                  updateFilters = TRUE;
                              }
                              break;
    }

    if (updateFilters)
        UpdateFilters();

    if (refreshGads)
    {
        if (currentKey)
        {
            GT_SetGadgetAttrs(removeGad,cxWindow,NULL,GA_Disabled, FALSE,
                                                      TAG_DONE);

            GT_SetGadgetAttrs(seqGad,cxWindow,NULL,GA_Disabled, FALSE,
                                                   GTST_String, currentKey->fk_Node.ln_Name,
                                                   TAG_DONE);

            GT_SetGadgetAttrs(cmdGad,cxWindow,NULL,GA_Disabled, FALSE,
                                                   GTCY_Active, currentKey->fk_Cmd,
                                                   TAG_DONE);

            GT_SetGadgetAttrs(argGad,cxWindow,NULL,GA_Disabled, currentKey->fk_Cmd < CMD_INSERT,
                                                   GTST_String, currentKey->fk_Args,
                                                   TAG_DONE);
        }
        else
        {
            GT_SetGadgetAttrs(removeGad,cxWindow,NULL,GA_Disabled,TRUE,
                                                      TAG_DONE);

            GT_SetGadgetAttrs(seqGad,cxWindow,NULL,GA_Disabled, TRUE,
                                                   GTST_String, "",
                                                   TAG_DONE);

            GT_SetGadgetAttrs(cmdGad,cxWindow,NULL,GA_Disabled,TRUE,
                                                   TAG_DONE);

            GT_SetGadgetAttrs(argGad,cxWindow,NULL,GA_Disabled, TRUE,
                                                   GTST_String, "",
                                                   TAG_DONE);
        }
    }

    if (activate)
        ActivateGadget(activate,cxWindow,NULL);

    if (cxWindow)
    {
        if (currentKey)
        {
            GT_SetGadgetAttrs(keysGad,cxWindow,NULL,GTLV_Labels,   &keyList,
                                                    GTLV_Selected, FindNodeNum((struct List *)&keyList,(struct Node *)currentKey),
                                                    TAG_DONE);
        }
        else
        {
            GT_SetGadgetAttrs(keysGad,cxWindow,NULL,GTLV_Labels, &keyList,
                                                    TAG_DONE);
        }
    }

    return(TRUE);
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
    mainFilter = NULL;
    UpdateFilters();

    return(TRUE);
}


/*****************************************************************************/


VOID DisposeCustomCx(VOID)
{
struct KeySequence *key;

    while (key = (struct KeySequence *)RemHead(&keyList))
        FreeSequence(key);
}
