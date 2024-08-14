
#include <exec/memory.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <graphics/displayinfo.h>
#include <dos/dos.h>
#include <dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>
#include <clib/timer_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/timer_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "clock.h"
#include "menus.h"
#include "display.h"
#include "alarm.h"
#include "date.h"
#include "icon.h"
#include "clock_rev.h"


/*****************************************************************************/


#define TEMPLATE      "DIGITAL/S,LEFT/N,TOP/N,WIDTH/N,HEIGHT/N,24HOUR/S,SECONDS/S,DATE/S,FORMAT/N,PUBSCREEN/K" VERSTAG
#define OPT_DIGITAL   0
#define OPT_LEFT      1
#define OPT_TOP	      2
#define OPT_WIDTH     3
#define OPT_HEIGHT    4
#define OPT_24HOUR    5
#define OPT_SECONDS   6
#define OPT_DATE      7
#define OPT_FORMAT    8
#define OPT_PUBSCREEN 9
#define OPT_COUNT     10


/*****************************************************************************/


struct Library *SysBase;
struct Library *GadToolsBase;
struct Library *IconBase;
struct Library *UtilityBase;
struct Library *TimerBase;
struct Library *LocaleBase;
struct Library *GfxBase;
struct Library *DOSBase;
struct Library *LayersBase;
struct Library *IntuitionBase;

struct Screen      *sp;
struct Window      *wp;
struct RastPort    *rp;
struct DrawInfo    *di;
APTR                vi;
struct TextFont    *topazFont;
struct timerequest *timerReq;
struct MsgPort     *ioPort;
struct LocaleInfo  li;
struct Locale     *locale;

STRPTR dateFormat;
STRPTR timeFormat;
UBYTE  formatNum;

extern struct Window *awp;


/*****************************************************************************/


STRPTR screenName;
UBYTE  clockType;
UBYTE  doSeconds;
UBYTE  doDates;
UBYTE  doAlarm;
STRPTR timeSamples[7];
STRPTR timeFormats[7];

char sample1[25];
char sample2[25];

ULONG leftTag = TAG_IGNORE;
ULONG topTag  = TAG_IGNORE;

WORD Dims[2][4] =
{
    { 0, 0, -1, -1},	/* ANALOG */
    { 0, 0, 0, 0}	/* DIGITAL */
};

struct TextAttr topazAttr =
{
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};


/*****************************************************************************/


static BOOL DoClock(VOID);
static BOOL DoWindow(VOID);
static BOOL EventLoop(VOID);


/*****************************************************************************/


extern APTR  _BSSBAS;
extern ULONG _BSSLEN;
APTR  bssBas;
ULONG bssLen;


LONG main(VOID)
{
LONG               opts[OPT_COUNT];
struct RDArgs     *rdargs;
struct WBStartup  *WBenchMsg = NULL;
LONG               failureLevel = RETURN_FAIL;
struct Process    *process;
struct DiskObject *diskObj;
STRPTR            *ttype;
STRPTR             tvalue;
LONG               num;
BPTR               oldCD;

    geta4();

    /* kludge for SAS/C, to prevent it from generating data hunks that
     * are too large. Referencing _BSSBAS and _BSSLEN prevents the linker from
     * expanding the data hunk of the executable to include all BSS data. as
     * well. This reduces the on-disk size of the program by the size of the
     * BSS hunk.
     */
    bssBas = _BSSBAS;
    bssLen = _BSSLEN;

    SysBase = (*((struct Library **) 4));

    process = (struct Process *)FindTask(NULL);
    if (!process->pr_CLI)
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    screenName = NULL;
    clockType  = ANALOG;	/* analog/digital mode flag (bool) */
    doSeconds  = FALSE;		/* second hand enable flag (bool) */
    doDates    = FALSE;		/* show date? (bool) */
    doAlarm    = FALSE;		/* alarm on/off flag (bool) */
    formatNum  = 0;

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        if (GfxBase = OpenLibrary("graphics.library",37))
        {
            if (UtilityBase = OpenLibrary("utility.library",37))
            {
                if (IntuitionBase = OpenLibrary("intuition.library",37))
                {
                    if (GadToolsBase = OpenLibrary("gadtools.library",37))
                    {
                        if (IconBase = OpenLibrary("icon.library",37))
                        {
                            if (LayersBase = OpenLibrary("layers.library",37))
                            {
                                if (LocaleBase = OpenLibrary("locale.library",38))
                                {
                                    li.li_LocaleBase = LocaleBase;
                                    locale           = OpenLocale(NULL);
                                    li.li_Catalog    = OpenCatalogA(locale,"sys/utilities.catalog",NULL);
                                }

                                if (WBenchMsg)
                                {
                                    oldCD = CurrentDir(WBenchMsg->sm_ArgList->wa_Lock);
                                    if (diskObj = GetDiskObject(WBenchMsg->sm_ArgList->wa_Name))
                                    {
                                        ttype = diskObj->do_ToolTypes;

                                        if (FindToolType(ttype,"DIGITAL"))
                                            clockType = DIGITAL;

                                        if (FindToolType(ttype,"SECONDS"))
                                            doSeconds = TRUE;

                                        if (FindToolType(ttype,"DATE"))
                                            doDates = TRUE;

                                        if (tvalue = FindToolType(ttype,"LEFT"))
                                        {
                                            StrToLong(tvalue,&num);
                                            Dims[clockType][0] = num;
                                            leftTag = WA_Left;
                                        }

                                        if (tvalue = FindToolType(ttype,"TOP"))
                                        {
                                            StrToLong(tvalue,&num);
                                            Dims[clockType][1] = num;
                                            topTag = WA_Top;
                                        }

                                        if (tvalue = FindToolType(ttype,"WIDTH"))
                                        {
                                            StrToLong(tvalue,&num);
                                            Dims[clockType][2] = num;
                                        }

                                        if (tvalue = FindToolType(ttype,"HEIGHT"))
                                        {
                                            StrToLong(tvalue,&num);
                                            Dims[clockType][3] = num;
                                        }

                                        if (tvalue = FindToolType(ttype,"FORMAT"))
                                        {
                                            StrToLong(tvalue,&num);
                                            formatNum = num;
                                        }

                                        screenName = FindToolType(ttype,"PUBSCREEN");

                                        if (DoClock())
                                            failureLevel = RETURN_OK;

                                        FreeDiskObject(diskObj);
                                    }
                                    CurrentDir(oldCD);
                                }
                                else
                                {
                                    memset(opts,0,sizeof(opts));
                                    if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
                                    {
                                        if (opts[OPT_DIGITAL])
                                            clockType = DIGITAL;

                                        if (opts[OPT_LEFT])
                                        {
                                            Dims[clockType][0] = (WORD)(*(LONG *)opts[OPT_LEFT]);
                                            leftTag = WA_Left;
                                        }

                                        if (opts[OPT_TOP])
                                        {
                                            Dims[clockType][1] = (WORD)(*(LONG *)opts[OPT_TOP]);
                                            topTag = WA_Top;
                                        }

                                        if (opts[OPT_WIDTH])
                                            Dims[clockType][2] = (WORD)(*(LONG *)opts[OPT_WIDTH]);

                                        if (opts[OPT_HEIGHT])
                                            Dims[clockType][3] = (WORD)(*(LONG *)opts[OPT_HEIGHT]);

                                        if (opts[OPT_SECONDS])
                                            doSeconds = TRUE;

                                        if (opts[OPT_DATE])
                                            doDates = TRUE;

                                        if (opts[OPT_FORMAT])
                                            formatNum = (UBYTE)(*(LONG *)opts[OPT_FORMAT]);

                                        screenName = (STRPTR)opts[OPT_PUBSCREEN];

                                        if (DoClock())
                                            failureLevel = RETURN_OK;

                                        FreeArgs(rdargs);
                                    }
                                    else
                                    {
                                        PrintFault(IoErr(),NULL);
                                    }
                                }

                                if (LocaleBase)
                                {
                                    CloseLocale(locale);
                                    CloseCatalog(li.li_Catalog);
                                    CloseLibrary(LocaleBase);
                                }
                                CloseLibrary(LayersBase);
                            }
                            CloseLibrary(IconBase);
                        }
                        CloseLibrary(GadToolsBase);
                    }
                    CloseLibrary(IntuitionBase);
                }
                CloseLibrary(UtilityBase);
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


static VOID GetTimeFormats(VOID)
{
UWORD i;
UWORD maxFmt;
BOOL  found;
ULONG seconds;
UBYTE defaultFmt;

    if (LocaleBase)
    {
        seconds    = 60*60*16;
        defaultFmt = formatNum;

        timeSamples[0] = "16:00";
        timeFormats[0] = "%q:%M";

        timeSamples[1] = "4:00 PM";
        timeFormats[1] = "%Q:%M %p";

        timeSamples[2] = "16:00:00";
        timeFormats[2] = "%q:%M:%S";

        timeSamples[3] = "4:00:00 PM";
        timeFormats[3] = "%Q:%M:%S %p";

        maxFmt = 4;

        found = FALSE;
        i     = 0;
        while (i < maxFmt)
        {
            if (strcmp(timeFormats[i],locale->loc_ShortTimeFormat) == 0)
                found = TRUE;

            i++;
        }

        if (!found)
        {
            defaultFmt = maxFmt;
            timeFormats[maxFmt]   = locale->loc_ShortTimeFormat;
            timeSamples[maxFmt++] = sample1;
            GetTime(seconds,locale->loc_ShortTimeFormat,sample1);
        }

        found = FALSE;
        i     = 0;
        while (i < maxFmt)
        {
            if (strcmp(timeFormats[i],locale->loc_TimeFormat) == 0)
                found = TRUE;

            i++;
        }

        if (!found)
        {
            defaultFmt = maxFmt;
            timeFormats[maxFmt]   = locale->loc_TimeFormat;
            timeSamples[maxFmt++] = sample2;
            GetTime(seconds,locale->loc_TimeFormat,sample2);
        }

        timeFormats[maxFmt] = NULL;
        timeSamples[maxFmt] = NULL;

        if (formatNum >= maxFmt)
            formatNum = defaultFmt;

        if (formatNum >= maxFmt)
            formatNum = 0;

        dateFormat = locale->loc_ShortDateFormat;
        timeFormat = timeFormats[formatNum];
    }
    else
    {
        timeSamples[0] = NULL;
    }
}


/*****************************************************************************/


static BOOL DoClock(VOID)
{
BOOL result;

    result = FALSE;

    if (sp = LockPubScreen(screenName))
    {
        if (di = GetScreenDrawInfo(sp))
        {
            if (vi = GetVisualInfoA(sp,NULL))
            {
                if (topazFont = OpenFont(&topazAttr))
                {
                    if (ioPort = CreateMsgPort())
                    {
                        if (timerReq = CreateIORequest(ioPort,sizeof(struct timerequest)))
                        {
                            if (OpenDevice("timer.device",UNIT_VBLANK,timerReq,0) == 0)
                            {
                                TimerBase = timerReq->tr_node.io_Device;

                                GetTimeFormats();

                                result = DoWindow();

                                CloseAlarmWindow();

                                CloseDevice(timerReq);
                            }
                            DeleteIORequest(timerReq);
                        }
                        DeleteMsgPort(ioPort);
                    }
                    CloseFont(topazFont);
                }
                FreeVisualInfo(vi);
            }
            FreeScreenDrawInfo(sp,di);
        }
        UnlockPubScreen(NULL,sp);
    }

    return(result);
}


/*****************************************************************************/


static BOOL DoWindow(VOID)
{
BOOL               result;
WORD               width, height;
struct TextExtent  te;
BYTE               vBorder;
UWORD              zoomSize[4];
WORD               maxWidth, maxHeight;
WORD               minWidth, minHeight;

    vBorder   = sp->WBorTop + (sp->Font->ta_YSize + 1) - 11;
    maxWidth  = sp->Width;
    maxHeight = sp->Height;
    minWidth  = 65;
    minHeight = 65+vBorder;

    /* Get screen's aspect ratio, set default analog clock size */
    if (maxWidth > maxHeight)
    {
	height = maxHeight >> 2;
	width  = height * di->dri_Resolution.Y / di->dri_Resolution.X;
    }
    else
    {
	width  = maxWidth >> 2;
	height = width * di->dri_Resolution.X / di->dri_Resolution.Y;
    }

    if (Dims[ANALOG][2] < 0)
        Dims[ANALOG][2] = width + 12;

    if (Dims[ANALOG][3] < 0)
        Dims[ANALOG][3] = height + vBorder + 11 + 23;

    if (Dims[ANALOG][2] < minWidth)
        Dims[ANALOG][2] = minWidth;

    if (Dims[ANALOG][3] < minHeight)
        Dims[ANALOG][3] = minHeight;

    /* Get screen's font size, set digital clock size */
    TextExtent(&sp->RastPort, "00:00:00 AM", 11, &te);

    Dims[DIGITAL][2] = te.te_Width + 65;
    Dims[DIGITAL][3] = vBorder + 11;

    while (TRUE)
    {
        if (clockType == ANALOG)
        {
            zoomSize[0] = 0xffff;
            zoomSize[1] = 0xffff;
            zoomSize[2] = minWidth;
            zoomSize[3] = minHeight;

	    wp = OpenWindowTags(NULL, leftTag,         Dims[ANALOG][0],
                                      topTag,          Dims[ANALOG][1],
                                      WA_Width,        Dims[ANALOG][2],
                                      WA_Height,       Dims[ANALOG][3],
                                      WA_MinWidth,     minWidth,
                                      WA_MinHeight,    minHeight,
                                      WA_MaxWidth,     -1,
                                      WA_MaxHeight,    -1,
                                      WA_IDCMP,        IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW,
                                      WA_SizeBBottom,  TRUE,
                                      WA_DepthGadget,  TRUE,
                                      WA_DragBar,      TRUE,
                                      WA_SizeGadget,   TRUE,
                                      WA_CloseGadget,  TRUE,
                                      WA_SimpleRefresh,TRUE,
                                      WA_Activate,     TRUE,
                                      WA_Title,        GetString(&li,MSG_CLK_TITLE),
                                      WA_PubScreen,    sp,
                                      WA_NewLookMenus, TRUE,
                                      WA_AutoAdjust,   TRUE,
                                      WA_Zoom,         zoomSize,
                                      TAG_DONE);
        }
        else
        {
            wp = OpenWindowTags(NULL, leftTag,         Dims[DIGITAL][0],
                                      topTag,          Dims[DIGITAL][1],
                                      WA_Width,        Dims[DIGITAL][2],
                                      WA_Height,       Dims[DIGITAL][3],
                                      WA_IDCMP,        IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW,
                                      WA_DepthGadget,  TRUE,
                                      WA_DragBar,      TRUE,
                                      WA_CloseGadget,  TRUE,
                                      WA_SimpleRefresh,TRUE,
                                      WA_Activate,     TRUE,
                                      WA_PubScreen,    sp,
                                      WA_NewLookMenus, TRUE,
                                      WA_AutoAdjust,   TRUE,
                                      TAG_DONE);
        }

        if (!wp)
            return(FALSE);

        if (!InstallClockMenus())
        {
            CloseWindow(wp);
            return(FALSE);
        }

        rp = wp->RPort;
        SetFont(rp,topazFont);

        if (result = EventLoop())
        {
            Dims[1-clockType][0] = wp->LeftEdge;
            Dims[1-clockType][1] = wp->TopEdge;
            Dims[1-clockType][2] = wp->Width;
            Dims[1-clockType][3] = wp->Height;

            leftTag = WA_Left;
            topTag  = WA_Top;
        }

        RemoveClockMenus();
        CloseWindow(wp);

        if (!result)
            return(TRUE);
    }
}


/*****************************************************************************/


#define PC_NORMAL    0
#define PC_CLOSE     1
#define PC_NEWWINDOW 2

UBYTE ProcessCommand(enum ClockCommand cc)
{
    switch (cc)
    {
        case cc_Analog	      : if (clockType != ANALOG)
                                {
                                    clockType = ANALOG;
                                    return(PC_NEWWINDOW);
                                }
                                break;

        case cc_Digital	      : if (clockType != DIGITAL)
                                {
                                    clockType = DIGITAL;
                                    return(PC_NEWWINDOW);
                                }
                                break;

        case cc_Quit	      : return(PC_CLOSE);

        case cc_Seconds       : doSeconds = !doSeconds;
                                break;

        case cc_Date	      : doDates = !doDates;
                                break;

        case cc_DateFmt1      :
        case cc_DateFmt2      :
        case cc_DateFmt3      :
        case cc_DateFmt4      :
        case cc_DateFmt5      :
        case cc_DateFmt6      : formatNum  = cc - cc_DateFmt1;
                                timeFormat = timeFormats[formatNum];
                                break;

        case cc_SetAlarm      : OpenAlarmWindow();
                                break;

        case cc_Alarm	      : doAlarm = !doAlarm;
                                break;

        case cc_SaveSettings  : SaveClockIcon();
                                break;
    }

    return(PC_NORMAL);
}


/*****************************************************************************/


static BOOL EventLoop(VOID)
{
ULONG                sigs;
ULONG                mask;
struct IntuiMessage *intuiMsg;
UWORD                menuNum;
struct MenuItem     *menuItem;
UBYTE                num;
struct timeval       sysTime;

    AdjustClockMenus();

    timerReq->tr_node.io_Command = TR_ADDREQUEST;
    timerReq->tr_time.tv_secs    = 1;
    timerReq->tr_time.tv_micro   = 0;
    SendIO(timerReq);

    GetSysTime(&sysTime);

    if (clockType == ANALOG)
        DrawAnalog(sysTime.tv_secs,TRUE);
    else
        UpdateDigital(sysTime.tv_secs);

    while (TRUE)
    {
        mask = 1<<wp->UserPort->mp_SigBit | 1<<ioPort->mp_SigBit | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F;
        if (awp)
            mask |= (1<<awp->UserPort->mp_SigBit);

        sigs = Wait(mask);

        if (sigs & SIGBREAKF_CTRL_C)
        {
            AbortIO(timerReq);
            WaitIO(timerReq);
            return(FALSE);
        }

        if (sigs & SIGBREAKF_CTRL_F)
        {
            WindowToFront(wp);
            ActivateWindow(wp);
        }

        GetSysTime(&sysTime);

        if (CheckIO(timerReq) != NULL)
        {
            WaitIO(timerReq);
            timerReq->tr_node.io_Command = TR_ADDREQUEST;
            timerReq->tr_time.tv_secs    = 1;
            timerReq->tr_time.tv_micro   = 0;
            SendIO(timerReq);
            DoAlarm(sysTime.tv_secs);
        }

        if (awp && (mask & (1<<awp->UserPort->mp_SigBit)))
        {
            DoAlarm(sysTime.tv_secs);
            AdjustClockMenus();
        }

        while (intuiMsg = (struct IntuiMessage *)GetMsg(wp->UserPort))
        {
            switch (intuiMsg->Class)
            {
                case IDCMP_CLOSEWINDOW  : AbortIO(timerReq);
                                          WaitIO(timerReq);
                                          return(FALSE);

                case IDCMP_MENUPICK     : menuNum = intuiMsg->Code;
                                          while (menuNum != MENUNULL)
                                          {
                                              menuItem = ItemAddress(wp->MenuStrip,menuNum);
                                              num = ProcessCommand((enum ClockCommand)MENU_USERDATA(menuItem));
                                              if (num != PC_NORMAL)
                                              {
                                                  AbortIO(timerReq);
                                                  WaitIO(timerReq);
                                                  return((BOOL)(num == PC_NEWWINDOW));
                                              }
                                              menuNum = menuItem->NextSelect;
                                          }
                                          AdjustClockMenus();
                                          break;

                case IDCMP_REFRESHWINDOW:
                case IDCMP_NEWSIZE      : break;
            }
            ReplyMsg(intuiMsg);
        }

        if (clockType == ANALOG)
            DrawAnalog(sysTime.tv_secs,FALSE);
        else
            UpdateDigital(sysTime.tv_secs);
    }
}
