
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <devices/audio.h>
#include <utility/hooks.h>
#include <stdio.h>
#include <string.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "texttable.h"
#include "alarm.h"


/*****************************************************************************/


extern struct Library    *GfxBase;
extern struct Library    *SysBase;
extern struct Library    *IntuitionBase;
extern struct Library    *GadToolsBase;
extern struct Library    *LocaleBase;
extern struct LocaleInfo  li;

extern struct Screen   *sp;
extern struct DrawInfo *di;
extern APTR             vi;
extern struct MsgPort  *ioPort;
extern UBYTE            doAlarm;


/*****************************************************************************/


struct Window  *awp;
struct Gadget  *agads;
UWORD           sampleTop;

UBYTE didAlarm;
WORD  alarmTime = 720;	/* 12:00 PM */
WORD  alarmHours;
WORD  alarmMins;


/*****************************************************************************/


#define GAD_USE     0
#define GAD_CANCEL  1
#define GAD_HOURS   2
#define GAD_MINUTES 3


/*****************************************************************************/


BYTE sines[] = {0, 49, 90, 117, 127, 117, 90, 49, 0, -49, -90, -117, -127, -117, -90, -49};
UBYTE allocMap[] = {1, 8, 2, 4};


/*****************************************************************************/


VOID __stdargs BeginIO(struct IORequest *io);


/*****************************************************************************/


static VOID __asm PutCh(register __a0 struct Hook *hook, register __a2 APTR loc,
                 register __a1 char c)
{
    *(char *)hook->h_Data = c;
    hook->h_Data = (APTR)((ULONG)hook->h_Data + 1);
}


/*****************************************************************************/


VOID DrawAlarmDisplay(VOID)
{
struct RastPort *rp;
char             sample[50];
struct Hook      hook;
struct DateStamp date;
UWORD            sLeft,sTop,sWidth,sHeight;
UWORD            len,plen;
UWORD            x;

    rp = awp->RPort;

    sLeft   = 8+awp->BorderLeft;
    sTop    = sampleTop;
    sWidth  = awp->Width-awp->BorderLeft-awp->BorderRight-16;
    sHeight = sp->RastPort.TxHeight+6;

    if (li.li_Locale)
    {
        hook.h_Entry   = (APTR)PutCh;
        hook.h_Data    = sample;
        date.ds_Days   = 0;
        date.ds_Minute = alarmHours * 60 + alarmMins;
        date.ds_Tick   = 0;

        FormatDate(li.li_Locale,li.li_Locale->loc_TimeFormat,&date,&hook);
    }
    else
    {
        sprintf(sample,"%02ld:%02ld",alarmHours,alarmMins);
    }

    DrawBevelBox(rp,sLeft,sTop,sWidth,sHeight,GT_VisualInfo, vi,
                                              GTBB_Recessed, TRUE,
                                              TAG_DONE);

    len = strlen(sample);
    plen = TextLength(rp,sample,len);
    x = (sWidth-plen) / 2 + sLeft;

    SetAPen(rp,di->dri_Pens[BACKGROUNDPEN]);
    RectFill(rp,sLeft+2,sTop+1,x-1,sTop+sHeight-2);
    RectFill(rp,x+plen,sTop+1,sLeft+sWidth-3,sTop+sHeight-2);

    SetAPen(rp,di->dri_Pens[TEXTPEN]);
    Move(rp,x,sTop+rp->TxBaseline+4);
    Text(rp,sample,len);
}


/*****************************************************************************/


VOID OpenAlarmWindow(VOID)
{
struct TextFont  *font;
UWORD             fontHeight;
UWORD             buttonWidth;
UWORD             width;
UWORD             height;
STRPTR            str;
UWORD             len, maxlen;
UWORD             mspace;
struct NewGadget  ng;
struct Gadget    *gad;

    if (awp)
    {
        WindowToFront(awp);
        ActivateWindow(awp);
        return;
    }

    alarmHours = alarmTime / 60;	/* Hours...   */
    alarmMins  = alarmTime % 60;	/* Minutes... */

    font       = sp->RastPort.Font;
    fontHeight = font->tf_YSize;
    mspace     = TextLength(&sp->RastPort,"m",1);

    height = fontHeight*4 + 4   /* border to hours gad      */
                          + 3   /* hours gad                */
                          + 2   /* hours gad to minutes gad */
                          + 3   /* minutes gad              */
                          + 4   /* minutes gad to sample    */
                          + 6   /* sample area              */
                          + 8   /* sample area to buttons   */
                          + 6   /* buttons                  */
                          + 4;  /* buttons to border        */

    str    = GetString(&li,MSG_CLK_ALARM_USE_GAD);
    maxlen = TextLength(&sp->RastPort,str,strlen(str)) + mspace;

    str = GetString(&li,MSG_CLK_ALARM_CANCEL_GAD);
    len = TextLength(&sp->RastPort,str,strlen(str)) + mspace;

    if (len > maxlen)
        maxlen = len;

    buttonWidth = maxlen+8;
    width = buttonWidth*2 + 8*3;

    str    = GetString(&li,MSG_CLK_ALARM_HOURS_GAD);
    maxlen = TextLength(&sp->RastPort,str,strlen(str));

    str = GetString(&li,MSG_CLK_ALARM_MINUTES_GAD);
    len = TextLength(&sp->RastPort,str,strlen(str));

    if (len > maxlen)
        maxlen = len;

    len = maxlen + 150 + 8 + 8 + 8;

    if (len > width)
        width = len;

    if (awp = OpenWindowTags(NULL,WA_InnerWidth,    width,
                                  WA_InnerHeight,   height,
                                  WA_AutoAdjust,    TRUE,
                                  WA_PubScreen,     sp,
                                  WA_Title,         GetString(&li,MSG_CLK_ALARM_TITLE),
                                  WA_DragBar,       TRUE,
                                  WA_DepthGadget,   TRUE,
                                  WA_SimpleRefresh, TRUE,
                                  WA_IDCMP,         IDCMP_REFRESHWINDOW | BUTTONIDCMP | SLIDERIDCMP,
                                  TAG_DONE))
    {
        gad = CreateContext(&agads);

        ng.ng_LeftEdge   = 8+awp->BorderLeft;
        ng.ng_TopEdge    = awp->Height-fontHeight-11;
        ng.ng_Width      = buttonWidth;
        ng.ng_Height     = fontHeight+6;
        ng.ng_GadgetText = GetString(&li,MSG_CLK_ALARM_USE_GAD);
        ng.ng_TextAttr   = sp->Font;
        ng.ng_GadgetID   = GAD_USE;
        ng.ng_Flags      = 0;
        ng.ng_VisualInfo = vi;
        gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

        ng.ng_LeftEdge   = awp->Width - ng.ng_Width - 8 - awp->BorderRight;
        ng.ng_GadgetText = GetString(&li,MSG_CLK_ALARM_CANCEL_GAD);
        ng.ng_GadgetID   = GAD_CANCEL;
        gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

        ng.ng_LeftEdge   = awp->Width - awp->BorderRight - 150 - 8,
        ng.ng_TopEdge    = awp->BorderTop+4;
        ng.ng_Width      = 150;
        ng.ng_Height     = fontHeight+3;
        ng.ng_GadgetText = GetString(&li,MSG_CLK_ALARM_HOURS_GAD);
        ng.ng_GadgetID   = GAD_HOURS;

        gad = CreateGadget(SLIDER_KIND, gad, &ng, GTSL_Min,     0,
                                                  GTSL_Max,     23,
                                                  GTSL_Level,   alarmHours,
                                                  GA_Immediate, TRUE,
                                                  GA_RelVerify, TRUE,
                                                  TAG_DONE);

        ng.ng_TopEdge    += fontHeight+3+2;
        ng.ng_GadgetText  = GetString(&li,MSG_CLK_ALARM_MINUTES_GAD);
        ng.ng_GadgetID    = GAD_MINUTES;
        gad = CreateGadget(SLIDER_KIND, gad, &ng, GTSL_Min,     0,
                                                  GTSL_Max,     59,
                                                  GTSL_Level,   alarmMins,
                                                  GA_Immediate, TRUE,
                                                  GA_RelVerify, TRUE,
                                                  TAG_DONE);

        if (gad)
        {
            sampleTop = ng.ng_TopEdge+fontHeight+3+2+2;

            AddGList(awp,agads,-1,-1,NULL);
            RefreshGList(agads,awp,NULL,-1);
            GT_RefreshWindow(awp,NULL);

            SetFont(awp->RPort,sp->RastPort.Font);
            SetBPen(awp->RPort,di->dri_Pens[BACKGROUNDPEN]);
            SetDrMd(awp->RPort,JAM2);

            DrawAlarmDisplay();
        }
        else
        {
            FreeGadgets(agads);
            CloseWindow(awp);
            awp   = NULL;
            agads = NULL;
        }
    }
}


/*****************************************************************************/


VOID CloseAlarmWindow(VOID)
{
    if (awp)
    {
        CloseWindow(awp);
        FreeGadgets(agads);
        awp   = NULL;
        agads = NULL;
    }
}


/*****************************************************************************/


VOID SoundAlarm(VOID)
{
struct IOAudio *audioReq;

    if (audioReq = CreateIORequest(ioPort,sizeof(struct IOAudio)))
    {
	audioReq->ioa_Request.io_Message.mn_Node.ln_Pri = 80;
        audioReq->ioa_Data   = allocMap;
        audioReq->ioa_Length = sizeof(allocMap);
	if (OpenDevice("audio.device",0,audioReq,0) == 0)
	{
            audioReq->ioa_Request.io_Command = CMD_WRITE;
            audioReq->ioa_Request.io_Flags   = ADIOF_PERVOL;

            if (audioReq->ioa_Data = AllocVec(sizeof(sines),MEMF_CHIP))
            {
                CopyMem(sines,audioReq->ioa_Data,sizeof(sines));
                audioReq->ioa_Length = sizeof(sines);
                audioReq->ioa_Period = 224;
                audioReq->ioa_Volume = 64;
                audioReq->ioa_Cycles = 3550000/(16*224);

                BeginIO(audioReq);
                WaitIO(audioReq);

                FreeVec(audioReq->ioa_Data);
	    }
	    CloseDevice(audioReq);
	}
	DeleteIORequest(audioReq);
    }
}


/*****************************************************************************/


VOID DoAlarm(ULONG seconds)
{
struct IntuiMessage *intuiMsg;
ULONG                class;
UWORD                icode;
struct Gadget       *gadget;

    if (awp)
    {
        while (intuiMsg = (struct IntuiMessage *)GT_GetIMsg(awp->UserPort))
        {
            class  = intuiMsg->Class;
            icode  = intuiMsg->Code;
            gadget = intuiMsg->IAddress;
            GT_ReplyIMsg(intuiMsg);

            switch (class)
            {
                case IDCMP_REFRESHWINDOW : GT_BeginRefresh(awp);
                                           DrawAlarmDisplay();
                                           GT_EndRefresh(awp,TRUE);
                                           break;

                case IDCMP_GADGETUP      :
                case IDCMP_GADGETDOWN    :
                case IDCMP_MOUSEMOVE     : switch (gadget->GadgetID)
                                           {
                                               case GAD_USE    : alarmTime = alarmHours * 60 + alarmMins;
                                                                 doAlarm = TRUE;

                                               case GAD_CANCEL : CloseAlarmWindow();
                                                                 return;

                                               case GAD_HOURS  : alarmHours = icode;
                                                                 DrawAlarmDisplay();
                                                                 break;

                                               case GAD_MINUTES: alarmMins = icode;
                                                                 DrawAlarmDisplay();
                                                                 break;
                                           }
                                           break;
            }

        }
    }
    else if (doAlarm)
    {
        if (alarmTime == ((seconds / 60) % (60*24)))
        {
            if (!didAlarm)
            {
                SoundAlarm();
                didAlarm = TRUE;
            }
        }
        else
        {
            didAlarm = FALSE;
        }
    }
}
