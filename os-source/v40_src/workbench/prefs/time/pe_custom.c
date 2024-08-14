
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/keymap.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <resources/battclock.h>
#include <utility/date.h>
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
#include <clib/battclock_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/battclock_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "calendargclass.h"

#define SysBase ed->ed_SysBase


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
ULONG seconds;
ULONG micros;

    CurrentTime(&seconds,&micros);
    Amiga2Date(seconds,&ed->ed_PrefsDefaults.tp_ClockData);
    ed->ed_PrefsDefaults.tp_ClockData.sec = 0;

    ed->ed_PrefsDefaults.tp_Seconds = seconds;

    ed->ed_PrefsWork    = ed->ed_PrefsDefaults;
    ed->ed_PrefsInitial = ed->ed_PrefsDefaults;

    ed->ed_BestDay = ed->ed_PrefsWork.tp_ClockData.mday;

    BattClockBase = OpenResource(BATTCLOCKNAME);

    return(ES_NORMAL);
}


/*****************************************************************************/


EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
    return(ES_NORMAL);
}


/*****************************************************************************/


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
struct timerequest timerReq;

    if (name[0] == 'E')        /* saving to ENV:, means USE */
    {
        if (OpenDevice(TIMERNAME,0,&timerReq,0))
            return (FALSE);

        timerReq.tr_node.io_Message.mn_Node.ln_Type = NT_MESSAGE;
        timerReq.tr_node.io_Message.mn_Node.ln_Pri  = 0;
        timerReq.tr_node.io_Message.mn_ReplyPort    = NULL;
        timerReq.tr_node.io_Message.mn_Length       = sizeof(struct timerequest);
        timerReq.tr_node.io_Command                 = TR_SETSYSTIME;
        timerReq.tr_time.tv_secs                    = ed->ed_PrefsWork.tp_Seconds;
        timerReq.tr_time.tv_micro                   = 0;

        DoIO(&timerReq);
        CloseDevice(&timerReq);
    }
    else if (name[0] == 'A')   /* saving to ENVARC:, means SAVE */
    {
	WriteBattClock(ed->ed_PrefsWork.tp_Seconds);
    }

    return(ES_NORMAL);
}


/*****************************************************************************/


APTR NewPrefsObject(EdDataPtr ed,struct IClass *cl, STRPTR name, ULONG data,...)
{
    return (NewObjectA(cl,name, (struct TagItem *) &data));
}


/*****************************************************************************/


VOID SetGadgetAttrsP(EdDataPtr ed, struct Gadget * gad, ULONG tags,...)
{
    SetGadgetAttrsA(gad,ed->ed_Window,NULL,(struct TagItem *)&tags);
}


/*****************************************************************************/


#define NW_WIDTH     324
#define NW_HEIGHT    187
#define	NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define	NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {
    {BUTTON_KIND,   8,   170, 87,  14, MSG_SAVE_GAD,         EC_SAVE,    0},
    {BUTTON_KIND,   118, 170, 87,  14, MSG_USE_GAD,          EC_USE,     0},
    {BUTTON_KIND,   229, 170, 87,  14, MSG_CANCEL_GAD,       EC_CANCEL,  0},

    {INTEGER_KIND,  50,  4,   53,  14, MSG_NOTHING,          EC_YEAR,    0},
    {CYCLE_KIND,    138, 4,   178, 14, MSG_NOTHING,          EC_MONTH,   0},

    {SLIDER_KIND,   122, 126, 194, 11, MSG_TIME_HOURS_GAD,   EC_HOURS,   0},
    {SLIDER_KIND,   122, 138, 194, 11, MSG_TIME_MINUTES_GAD, EC_MINUTES, 0}
};

#define CALLEFT    8
#define CALTOP     25
#define CALWIDTH   311
#define CALHEIGHT  100

#define SAMPLELEFT   8
#define SAMPLETOP    152
#define SAMPLEWIDTH  308
#define SAMPLEHEIGHT 14


/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];
UWORD i;
ULONG firstDay;

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_Days[0] = "Sun";
    ed->ed_Days[1] = "Mon";
    ed->ed_Days[2] = "Tue";
    ed->ed_Days[3] = "Wed";
    ed->ed_Days[4] = "Thu";
    ed->ed_Days[5] = "Fri";
    ed->ed_Days[6] = "Sat";

    ed->ed_Months[0]  = "January";
    ed->ed_Months[1]  = "February";
    ed->ed_Months[2]  = "March";
    ed->ed_Months[3]  = "April";
    ed->ed_Months[4]  = "May";
    ed->ed_Months[5]  = "June";
    ed->ed_Months[6]  = "July";
    ed->ed_Months[7]  = "August";
    ed->ed_Months[8]  = "September";
    ed->ed_Months[9]  = "October";
    ed->ed_Months[10] = "Novemeber";
    ed->ed_Months[11] = "December";

    firstDay          = 0;

    if (ed->ed_LocaleBase)
    {
        ed->ed_Locale = OpenLocale(NULL);

        for (i=ABDAY_1; i<=ABDAY_7; i++)
            ed->ed_Days[i-ABDAY_1] = GetLocaleStr(ed->ed_Locale,i);

        for (i=MON_1; i<=MON_12; i++)
            ed->ed_Months[i-MON_1] = GetLocaleStr(ed->ed_Locale,i);

        firstDay = ed->ed_Locale->loc_CalendarType;
        if (firstDay > 6)
            firstDay = 0;
    }

    if (ed->ed_CalendarClass = InitCalendarGClass(ed))
    {
        ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
        DoPrefsGadget(ed,&EG[0],NULL,GA_Disabled, BattClockBase == NULL,
                                     TAG_DONE);
        DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
        DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);

        RenderGadgets(ed);

	/* Create the sketchpad gadget */
	if (ed->ed_LastAdded)
	{
	    ed->ed_LastAdded = ed->ed_Calendar = (struct Gadget *)
	     NewPrefsObject (ed,
			     ed->ed_CalendarClass, NULL,
			     GA_Left,              CALLEFT + ed->ed_Screen->WBorLeft,
                             GA_Top,               CALTOP + ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1,
                             GA_Width,             CALWIDTH,
                             GA_Height,            CALHEIGHT,
                             GA_RelVerify,         TRUE,
                             GA_ID,                EC_CALENDAR,
                             GA_Previous,          ed->ed_LastAdded,
                             BOA_ClockData,        &ed->ed_PrefsWork.tp_ClockData,
                             BOA_Day,              ed->ed_PrefsWork.tp_ClockData.mday,
                             BOA_FirstWeekday,     firstDay,
			     TAG_DONE);
	}

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
                                                WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_TIME_NAME),
                                                WA_NewLookMenus,TRUE,
                                                WA_Gadgets,     ed->ed_Gadgets,
                                                TAG_DONE)))
        {
            return(TRUE);
        }

        DisposeDisplay(ed);
    }

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
    FreeGadgets(ed->ed_Gadgets);
    FreeMenus(ed->ed_Menus);

    if (ed->ed_Calendar)
        DisposeObject(ed->ed_Calendar);

    FreeCalendarGClass(ed->ed_CalendarClass);

    if (LocaleBase)
        CloseLocale(ed->ed_Locale);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, struct RastPort *rp,
                          WORD x, WORD y,
                          WORD w, WORD h, ULONG tags, ...)
{
    DrawBevelBoxA(rp,x+ed->ed_Window->BorderLeft,
                     y+ed->ed_Window->BorderTop,
                     w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID __asm PutCh(register __a0 struct Hook *hook, register __a2 APTR loc,
                 register __a1 char c)
{
    *(char *)hook->h_Data = c;
    hook->h_Data = (APTR)((ULONG)hook->h_Data + 1);
}


/*****************************************************************************/


VOID RenderDisplay(EdDataPtr ed)
{
struct RastPort *rp;
char             sample[150];
struct Hook      hook;
struct DateStamp date;
UWORD            len,plen,x;

    rp = ed->ed_Window->RPort;

    DrawBB(ed,rp,SAMPLELEFT,SAMPLETOP,SAMPLEWIDTH,SAMPLEHEIGHT,
                 GT_VisualInfo, ed->ed_VisualInfo,
                 GTBB_Recessed, TRUE,
                 TAG_DONE);

    sample[0] = 0;
    if (ed->ed_Locale)
    {
        hook.h_Entry   = (APTR)PutCh;
        hook.h_Data    = sample;
        date.ds_Days   = ed->ed_PrefsWork.tp_Seconds / (60*60*24);
        date.ds_Minute = ed->ed_PrefsWork.tp_ClockData.hour*60 + ed->ed_PrefsWork.tp_ClockData.min;
        date.ds_Tick   = 0;

        FormatDate(ed->ed_Locale,ed->ed_Locale->loc_ShortDateTimeFormat,&date,&hook);
    }

    if ((strlen(sample) > 36) || (sample[0] == 0))
    {
        sprintf(sample,"%02ld:%02ld",ed->ed_PrefsWork.tp_ClockData.hour,ed->ed_PrefsWork.tp_ClockData.min);
    }

    len = strlen(sample);
    plen = TextLength(rp,sample,len);
    x = (SAMPLEWIDTH-plen) / 2 + SAMPLELEFT + window->BorderLeft;

    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    RectFill(rp,SAMPLELEFT+2+window->BorderLeft,
                SAMPLETOP+1+window->BorderTop,
                x-1,
                SAMPLETOP+SAMPLEHEIGHT-2+window->BorderTop);
    RectFill(rp,x+plen,
                SAMPLETOP+1+window->BorderTop,
                SAMPLELEFT+SAMPLEWIDTH-3+window->BorderLeft,
                SAMPLETOP+SAMPLEHEIGHT-2+window->BorderTop);

    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
    Move(rp,x,SAMPLETOP+9+window->BorderTop);
    Text(rp,sample,len);

/*
    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    RectFill(rp,SAMPLELEFT+2+window->BorderLeft,
                SAMPLETOP+1+window->BorderTop,
                SAMPLELEFT+SAMPLEWIDTH-3+window->BorderLeft,
                SAMPLETOP+SAMPLEHEIGHT-2+window->BorderTop);

    SetAPen(rp,ed->ed_DrawInfo->dri_Pens[TEXTPEN]);
    SetBPen(rp,ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    SetDrMd(rp,JAM2);
    CenterLine(ed,rp,sample,SAMPLELEFT,SAMPLETOP+9,SAMPLEWIDTH);
*/
}


/*****************************************************************************/


VOID FixDay(EdDataPtr ed)
{
UWORD maxDays;

    ed->ed_PrefsWork.tp_ClockData.mday = ed->ed_BestDay;

    maxDays = GetDaysInMonth(ed,ed->ed_PrefsWork.tp_ClockData.month,ed->ed_PrefsWork.tp_ClockData.year);
    if (ed->ed_PrefsWork.tp_ClockData.mday > maxDays)
        ed->ed_PrefsWork.tp_ClockData.mday = maxDays;
}


/*****************************************************************************/


VOID RenderGadgets(EdDataPtr ed)
{
    ed->ed_Year = DoPrefsGadget(ed,&EG[3],ed->ed_Year,GTIN_Number,   ed->ed_PrefsWork.tp_ClockData.year,
                                                      GTIN_MaxChars, 4,
                                                      TAG_DONE);

    if (!ed->ed_Month)
    {
        ed->ed_Month = DoPrefsGadget(ed,&EG[4],ed->ed_Month,GTCY_Active, ed->ed_PrefsWork.tp_ClockData.month-1,
                                                            GTCY_Labels, &ed->ed_Months,
                                                            TAG_DONE);

        ed->ed_Hours = DoPrefsGadget(ed,&EG[5],ed->ed_Hours,GTSL_Level,       ed->ed_PrefsWork.tp_ClockData.hour,
                                                            GTSL_Min,         0,
                                                            GTSL_Max,         23,
                                                            GA_RelVerify,     TRUE,
                                                            GA_Immediate,     TRUE,
                                                            TAG_DONE);

        ed->ed_Minutes = DoPrefsGadget(ed,&EG[6],ed->ed_Minutes,GTSL_Level,       ed->ed_PrefsWork.tp_ClockData.min,
                                                                GTSL_Min,         0,
                                                                GTSL_Max,         59,
                                                                GA_RelVerify,     TRUE,
                                                                GA_Immediate,     TRUE,
                                                                TAG_DONE);
    }

    FixDay(ed);

    if (ed->ed_Calendar)
        SetGadgetAttrsP(ed,ed->ed_Calendar,BOA_ClockData, &ed->ed_PrefsWork.tp_ClockData,
                                           BOA_Day,       ed->ed_PrefsWork.tp_ClockData.mday,
                                           TAG_DONE);
}


/*****************************************************************************/


VOID SyncTextGadgets(EdDataPtr ed)
{
LONG num;

    num = ((struct StringInfo *)ed->ed_Year->SpecialInfo)->LongInt;
    if (num < 1991)
    {
        num = 1991;
    }
    else if (num > 2099)
    {
        num = 2099;
    }

    ed->ed_PrefsWork.tp_ClockData.year = num;
    ed->ed_PrefsWork.tp_Seconds = Date2Amiga(&ed->ed_PrefsWork.tp_ClockData);
}


/*****************************************************************************/


VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
UWORD icode;
LONG  num;

    icode  = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
        case EC_CALENDAR: ed->ed_PrefsWork.tp_ClockData.mday = icode;
                          ed->ed_BestDay                     = icode;
                          ed->ed_PrefsWork.tp_Seconds = Date2Amiga(&ed->ed_PrefsWork.tp_ClockData);
                          break;

        case EC_MONTH   : ed->ed_PrefsWork.tp_ClockData.month = icode+1;
                          FixDay(ed);
                          SetGadgetAttrsP(ed,ed->ed_Calendar,BOA_ClockData, &ed->ed_PrefsWork.tp_ClockData,
                                                             BOA_Day,       ed->ed_PrefsWork.tp_ClockData.mday,
                                                             TAG_DONE);
                          ed->ed_PrefsWork.tp_Seconds = Date2Amiga(&ed->ed_PrefsWork.tp_ClockData);
                          break;

        case EC_YEAR    : SyncTextGadgets(ed);
                          num = ((struct StringInfo *)ed->ed_Year->SpecialInfo)->LongInt;
                          if (num != ed->ed_PrefsWork.tp_ClockData.year)
                              DisplayBeep(NULL);
                          RenderGadgets(ed);
                          break;


        case EC_MINUTES : ed->ed_PrefsWork.tp_ClockData.min = icode;
                          ed->ed_PrefsWork.tp_Seconds = Date2Amiga(&ed->ed_PrefsWork.tp_ClockData);
                          break;

        case EC_HOURS   : ed->ed_PrefsWork.tp_ClockData.hour = icode;
                          ed->ed_PrefsWork.tp_Seconds = Date2Amiga(&ed->ed_PrefsWork.tp_ClockData);
                          break;
    }
    RenderDisplay(ed);
}


/*****************************************************************************/


VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}
