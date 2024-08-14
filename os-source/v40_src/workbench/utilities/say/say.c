
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <devices/narrator.h>
#include <devices/conunit.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/icon_protos.h>
#include <clib/wb_protos.h>
#include <clib/locale_protos.h>
#include <clib/translator_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/wb_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/translator_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "texttable.h"
#include "say_utils.h"
#include "speech.h"
#include "say_rev.h"


/*****************************************************************************/


#define TEMPLATE    "TEXT,-X=FILE/K,-M=MALE/S,-F=FEMALE/S,-N=NATURAL/S,-R=ROBOTIC/S,-P=PITCH/N/K,-S=SPEED/N/K,PUBSCREEN/K" VERSTAG
#define OPT_TEXT    0
#define OPT_FILE    1
#define OPT_MALE    2
#define OPT_FEMALE  3
#define OPT_NATURAL 4
#define OPT_ROBOTIC 5
#define OPT_PITCH   6
#define OPT_SPEED   7
#define OPT_SCREEN  8
#define OPT_COUNT   9


/*****************************************************************************/


VOID kprintf(STRPTR,...);
enum SayStatus SayFile(STRPTR fileName);
VOID DoUI(STRPTR screen);
enum SayStatus ProcessWBArgs(struct WBArg *wbArg, LONG argCnt, struct WBArg *nosay);
VOID RenderGadgets(VOID);


/*****************************************************************************/


struct Library     *SysBase;
struct Library     *GfxBase;
struct Library     *IconBase;
struct Library     *IntuitionBase;
struct Library     *DOSBase;
struct Library     *GadToolsBase;
struct Library     *WorkbenchBase = NULL;
struct WBStartup   *WBenchMsg = NULL;

struct Window      *wp;
struct Screen      *sp;
struct Gadget      *gadgets;
struct TextFont    *topazFont;
struct LocaleInfo   li;
APTR                vi;

extern struct MsgPort *narratorPort;
struct MsgPort        *intuiPort;
struct MsgPort        *appPort = NULL;
struct AppWindow      *appWindow = NULL;
BPTR                   console;
STRPTR                 screenName;
struct Gadget         *lastAdded = NULL;
struct Gadget         *pitchGad;
struct Gadget         *rateGad;
struct Gadget         *sexGad;
struct Gadget         *modeGad;
STRPTR                inputLine;

UWORD		    mode  = DEFMODE;
UWORD		    sex   = DEFSEX;
LONG                pitch = DEFPITCH;
LONG                rate  = DEFRATE;


/*****************************************************************************/


#define BUFSIZE     4096
#define LocaleBase  li.li_LocaleBase


/*****************************************************************************/


struct TextAttr topazAttr =
{
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};


/*****************************************************************************/


struct SayGadget far SG[] =
{
    {CYCLE_KIND,  132, 6,  162,  14, MSG_SAY_SEX_GAD,   SC_SEX,   0},
    {CYCLE_KIND,  132, 23, 162,  14, MSG_SAY_MODE_GAD,  SC_MODE,  0},
    {SLIDER_KIND, 132, 40, 162,  11, MSG_SAY_PITCH_GAD, SC_PITCH, 0},
    {SLIDER_KIND, 132, 54, 162,  11, MSG_SAY_RATE_GAD,  SC_RATE,  0}
};


/*****************************************************************************/


LONG main(VOID)
{
LONG            opts[OPT_COUNT];
struct RDArgs  *rdargs;
LONG            failureLevel = RETURN_FAIL;
struct Process *process;
enum SayStatus  result;

    geta4();
    SysBase = (*((struct Library **) 4));

    process = (struct Process *)FindTask(NULL);
    if (!process->pr_CLI)
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        if (GfxBase = OpenLibrary("graphics.library",37))
        {
            if (IntuitionBase = OpenLibrary("intuition.library",37))
            {
                if (GadToolsBase = OpenLibrary("gadtools.library",37))
                {
                    if (LocaleBase = OpenLibrary("locale.library",38))
                        li.li_Catalog = OpenCatalogA(NULL,"sys/utilities.catalog",NULL);

                    if ((result = OpenSpeech()) == SS_NORMAL)
                    {
                        if (WBenchMsg)
                        {
                            if (IconBase = OpenLibrary("icon.library",37))
                            {
                                failureLevel = RETURN_OK;
                                result = ProcessWBArgs(WBenchMsg->sm_ArgList,WBenchMsg->sm_NumArgs,WBenchMsg->sm_ArgList);

                                if (WBenchMsg->sm_NumArgs == 1)
                                    DoUI(screenName);

                                CloseLibrary(IconBase);
                            }
                        }
                        else
                        {
                            memset(opts,0,sizeof(opts));
                            if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
                            {
                                failureLevel = RETURN_OK;
                                if (opts[OPT_MALE])
                                    sex = MALE;

                                if (opts[OPT_FEMALE])
                                    sex = FEMALE;

                                if (opts[OPT_ROBOTIC])
                                    mode = ROBOTICF0;

                                if (opts[OPT_NATURAL])
                                    mode = NATURALF0;

                                if (opts[OPT_PITCH])
                                    pitch = *((ULONG *)opts[OPT_PITCH]);

                                if (opts[OPT_SPEED])
                                    rate = *((ULONG *)opts[OPT_SPEED]);

                                if (pitch < MINPITCH)
                                    pitch = MINPITCH;

                                if (pitch > MAXPITCH)
                                    pitch = MAXPITCH;

                                if (rate < MINRATE)
                                    rate = MINRATE;

                                if (rate > MAXRATE)
                                    rate = MAXRATE;

                                if (opts[OPT_TEXT])
                                    Say((STRPTR)opts[OPT_TEXT],sex,pitch,rate,mode);

                                if (opts[OPT_FILE])
                                    result = SayFile((STRPTR)opts[OPT_FILE]);

                                if (!opts[OPT_TEXT] && !opts[OPT_FILE])
                                    DoUI((STRPTR)opts[OPT_SCREEN]);

                                FreeArgs(rdargs);
                            }
                            else
                            {
                                PrintFault(IoErr(),NULL);
                            }
                        }
                        FreeVec(inputLine);

                        CloseSpeech();
                    }

                    if (result != SS_NORMAL)
                        ShowError(result);

                    if (LocaleBase)
                    {
                        CloseCatalog(li.li_Catalog);
                        CloseLibrary(LocaleBase);
                    }
                    CloseLibrary(GadToolsBase);
                }
                CloseLibrary(IntuitionBase);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}


/*****************************************************************************/


enum SayStatus ProcessWBArgs(struct WBArg *wbArg, LONG argCnt, struct WBArg *noSay)
{
BPTR               oldLock;
struct DiskObject *diskObj;
STRPTR             tvalue;
enum SayStatus     result = SS_NORMAL;

    while ((argCnt--) && (result == SS_NORMAL))
    {
        oldLock = CurrentDir(wbArg->wa_Lock);
        if (diskObj = GetDiskObject(wbArg->wa_Name))
        {
            if (FindToolType(diskObj->do_ToolTypes,"MALE"))
                sex = MALE;

            if (FindToolType(diskObj->do_ToolTypes,"FEMALE"))
                sex = FEMALE;

            if (FindToolType(diskObj->do_ToolTypes,"ROBOTIC"))
                mode = ROBOTICF0;

            if (FindToolType(diskObj->do_ToolTypes,"NATURAL"))
                mode = NATURALF0;

            if (tvalue = FindToolType(diskObj->do_ToolTypes,"PITCH"))
                StrToLong(tvalue,&pitch);

            if (tvalue = FindToolType(diskObj->do_ToolTypes,"SPEED"))
                StrToLong(tvalue,&rate);

            if (tvalue = FindToolType(diskObj->do_ToolTypes,"PUBSCREEN"))
                screenName = tvalue;

            if (pitch < MINPITCH)
                pitch = MINPITCH;

            if (pitch > MAXPITCH)
                pitch = MAXPITCH;

            if (rate < MINRATE)
                rate = MINRATE;

            if (rate > MAXRATE)
                rate = MAXRATE;

            if (wbArg != noSay)
            {
                RenderGadgets();
                result = SayFile(wbArg->wa_Name);
            }

            FreeDiskObject(diskObj);
        }
        CurrentDir(oldLock);
        wbArg++;
    }

    return(result);
}


/*****************************************************************************/


enum SayStatus SayFile(STRPTR fileName)
{
ULONG          sigs;
enum SayStatus result;
BPTR           file;

    if (!inputLine)
        inputLine = AllocVec(BUFSIZE,MEMF_PUBLIC);

    result = SS_NOMEMORY;
    if (inputLine)
    {
        result = SS_BADFILE;
        if (file = Open(fileName,MODE_OLDFILE))
        {
            result = SS_NORMAL;
            while (FGets(file,inputLine,BUFSIZE))
            {
                Say(inputLine,sex,pitch,rate,mode);

                sigs = Wait(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | (1<<narratorPort->mp_SigBit));

                if (SIGBREAKF_CTRL_C & sigs)
                {
                    result = SS_ABORTED;
                    StopSpeech();
                    break;
                }

                if (SIGBREAKF_CTRL_F & sigs)
                {
                    WindowToFront(wp);
                    ActivateWindow(wp);
                    if (wp->Flags & WFLG_ZOOMED)
                        ZipWindow(wp);
                }
            }

            /* should check for error */

            Close(file);
        }
    }

    return(result);
}


/*****************************************************************************/


VOID RenderDisplay(VOID)
{
    SetAPen(wp->RPort,1);
    CenterLine(MSG_SAY_INPUT_HDR,8,85,286);
    DrawSayBevelBox(8,90,286,68,GT_VisualInfo,vi,
                                GTBB_Recessed,  TRUE,
                                TAG_DONE);
    DrawSayBevelBox(6,89,290,70,GT_VisualInfo,vi,
                                TAG_DONE);
}


/*****************************************************************************/


VOID RenderGadgets(VOID)
{
    if (wp)
    {
        SetGadgetAttr(sexGad,GTCY_Active,sex,TAG_DONE);
        SetGadgetAttr(modeGad,GTCY_Active,mode,TAG_DONE);
        SetGadgetAttr(pitchGad,GTSL_Level,pitch,TAG_DONE);
        SetGadgetAttr(rateGad,GTSL_Level,rate,TAG_DONE);
    }
}


/*****************************************************************************/


VOID EventLoop(VOID)
{
ULONG                sigs,mask;
struct IntuiMessage *intuiMsg;
ULONG                class;
UWORD                icode;
struct Gadget       *gad;
UWORD                i;
struct AppMessage   *appMsg;

    mask = (1<<intuiPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F;
    if (appPort)
        mask |= (1<<appPort->mp_SigBit);

    while (TRUE)
    {
        sigs = Wait(mask);

        if (sigs & SIGBREAKF_CTRL_C)
            StopSpeech();

        if (sigs & SIGBREAKF_CTRL_F)
        {
            WindowToFront(wp);
            ActivateWindow(wp);
            if (wp->Flags & WFLG_ZOOMED)
                ZipWindow(wp);
        }

        while (intuiMsg = GT_GetIMsg(wp->UserPort))
        {
            class = intuiMsg->Class;
            icode = intuiMsg->Code;
            gad   = intuiMsg->IAddress;
            GT_ReplyIMsg(intuiMsg);

            switch(class)
            {
                case IDCMP_CLOSEWINDOW  : StopSpeech();
                                          return;

                case IDCMP_REFRESHWINDOW: GT_RefreshWindow(wp,NULL);
                                          RenderDisplay();
                                          GT_BeginRefresh(wp);
                                          GT_EndRefresh(wp,TRUE);
                                          break;

                case IDCMP_INTUITICKS   : while (WaitForChar(console,100))
                                          {
                                              i = 0;
                                              while (Read(console,&inputLine[i],1) && (inputLine[i] != '\n'))
                                                  i++;
                                              inputLine[i] = 0;

                                              Say(inputLine,sex,pitch,rate,mode);
                                          }
                                          break;

                case IDCMP_GADGETUP     :
                case IDCMP_GADGETDOWN   : switch((UWORD)gad->UserData)
                                          {
                                              case SC_PITCH: pitch = icode;
                                                             break;

                                              case SC_RATE : rate = icode;
                                                             break;

                                              case SC_MODE : mode = icode;
                                                             break;

                                              case SC_SEX  : sex = icode;
                                                             break;
                                          }
                                          break;

                default                 : break;
            }
        }

        if (appPort)
            while (appMsg = (struct AppMessage *)GetMsg(appPort))
            {
                ProcessWBArgs(appMsg->am_ArgList,appMsg->am_NumArgs,NULL);
                ReplyMsg(appMsg);
            }
    }
}


/*****************************************************************************/


VOID DoUI(STRPTR screen)
{
STRPTR             modeLabels[3];
STRPTR             sexLabels[3];
struct AppMessage *appMsg;
char               buffer[60];
BOOL               result = FALSE;
UWORD              zoomSize[4];

    modeLabels[0] = GetString(&li,MSG_SAY_NORMAL);
    modeLabels[1] = GetString(&li,MSG_SAY_ROBOTIC);
    modeLabels[2] = NULL;

    sexLabels[0]  = GetString(&li,MSG_SAY_MALE);
    sexLabels[1]  = GetString(&li,MSG_SAY_FEMALE);
    sexLabels[2]  = NULL;

    if (inputLine = AllocVec(BUFSIZE,MEMF_PUBLIC))
    {
        if (sp = LockPubScreen(screen))
        {
            if (vi = GetVisualInfoA(sp,NULL))
            {
                if (topazFont = OpenFont(&topazAttr))
                {
                    if (intuiPort = CreateMsgPort())
                    {
                        gadgets = CreateContext(&lastAdded);
                        sexGad = CreateSayGadget(&SG[0],GTCY_Labels, sexLabels,
                                                        GTCY_Active, sex,
                                                        TAG_DONE);

                        modeGad = CreateSayGadget(&SG[1],GTCY_Labels, modeLabels,
                                                         GTCY_Active, mode,
                                                         TAG_DONE);

                        pitchGad = CreateSayGadget(&SG[2],GTSL_Max,         MAXPITCH,
                                                          GTSL_Min,         MINPITCH,
                                                          GTSL_Level,       pitch,
                                                          GTSL_LevelFormat, "%3lu",
                                                          GTSL_MaxLevelLen, 3,
                                                          GA_RelVerify,     TRUE,
                                                          TAG_DONE);

                        rateGad = CreateSayGadget(&SG[3],GTSL_Max,         MAXRATE,
                                                         GTSL_Min,         MINRATE,
                                                         GTSL_Level,       rate,
                                                         GTSL_LevelFormat, "%3lu",
                                                         GTSL_MaxLevelLen, 3,
                                                         GA_RelVerify,     TRUE,
                                                         TAG_DONE);

                        if (rateGad)
                        {
                            zoomSize[0] = 0;
                            zoomSize[1] = 0;
                            zoomSize[2] = 200;
                            zoomSize[3] = sp->WBorTop + sp->Font->ta_YSize + 1;

                            if (wp = OpenSayWindow(WA_Title,       GetString(&li,MSG_SAY_TITLE),
                                                   WA_InnerWidth,  302,
                                                   WA_InnerHeight, 165,
                                                   WA_Flags,       WFLG_SIMPLE_REFRESH | WFLG_ACTIVATE | WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET,
                                                   WA_AutoAdjust,  TRUE,
                                                   WA_PubScreen,   sp,
                                                   WA_Zoom,        zoomSize,
                                                   TAG_DONE))
                            {
                                wp->UserPort = intuiPort;
                                ModifyIDCMP(wp,IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW | IDCMP_INTUITICKS | SLIDERIDCMP | CYCLEIDCMP);
                                SetFont(wp->RPort,topazFont);

                                sprintf(buffer,"CON://///WINDOW%lx",(ULONG)wp);
                                if (console = Open(buffer,MODE_NEWFILE))
                                {
                                    result = TRUE;
                                    sprintf(buffer,"\x1b[%ldx\x1b[%ldy\x1b[35u\x1b[8t",(ULONG)11+wp->BorderLeft,(ULONG)92+wp->BorderTop);
                                    Write(console,buffer,strlen(buffer));

                                    AddGList(wp,gadgets,-1,-1,NULL);
                                    RefreshGList(gadgets,wp,NULL,-1);
                                    GT_RefreshWindow(wp,NULL);
                                    RenderDisplay();

                                    if ((appPort = CreateMsgPort())
                                    && (WorkbenchBase = OpenLibrary("workbench.library",37)))
                                        appWindow = AddAppWindowA(0,NULL,wp,appPort,NULL);

                                    EventLoop();

                                    if (appWindow)
                                    {
                                        RemoveAppWindow(appWindow);
                                        while (appMsg = (struct AppMessage *)GetMsg(appPort))
                                            ReplyMsg(appMsg);
                                    }

                                    DeleteMsgPort(appPort);
                                    CloseLibrary(WorkbenchBase);
                                }

                                wp->UserPort = NULL;
                                ModifyIDCMP(wp,NULL);

                                if (console)
                                    Close(console);
                                else
                                    CloseWindow(wp);
                            }
                        }
                        FreeGadgets(gadgets);
                        DeleteMsgPort(intuiPort);
                    }
                    CloseFont(topazFont);
                }
                FreeVisualInfo(vi);
            }
            UnlockPubScreen(NULL,sp);
        }
    }

    if (!result)
        ShowError(SS_NOMEMORY);
}
