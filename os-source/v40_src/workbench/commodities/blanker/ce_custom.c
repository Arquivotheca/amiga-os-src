
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <devices/inputevent.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <libraries/commodities.h>
#include <graphics/displayinfo.h>
#include <graphics/text.h>
#include <graphics/videocontrol.h>
#include <graphics/view.h>
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

static struct Gadget   *blankSecondsGad;
static struct Gadget   *cycleColorsGad;
static struct Gadget   *animationGad;
static struct Task     *parentTask;
static ULONG            cmd;
static struct Screen   *screen;
static WORD             screenWidth;
static WORD             screenHeight;
static ULONG            blankInterval = 600;
static ULONG            blankTimeOut;
static ULONG	        animTimeOut;
static BOOL             cycleColors;
static BOOL		doAnimation;
static ULONG            loopCount;
static ULONG            oldLoopCount;
static UWORD            stalled;
static WORD             mouseX;
static WORD             mouseY;
static WORD             maxPoints;
static LONG             oldPri;

extern struct Custom __far custom;

#define ANIMINTERVAL (600)   /* change animation type every 60 seconds */
#define MAXLINES     (30)
#define MAXPOINTS    (20)
#define STACKSIZE    (1500)
#define HALF(a)      ((*(a)+(a)[1])>>1)
#define FIX(x)       (((LONG)(x)) << 7)
#define FIXH(x)      (((LONG)((*(x)+(x)[1])>>1)) << 7)

static struct box
{
    WORD x[MAXPOINTS];
    WORD y[MAXPOINTS];
} store[MAXLINES];

static struct box *ptr;
static struct box *eptr;
static WORD numlines;
static WORD mdelta = -1;
static WORD maxlines = MAXLINES/2;
static WORD dx[MAXPOINTS],  dy[MAXPOINTS];
static WORD ox[MAXPOINTS],  oy[MAXPOINTS];
static WORD nx[MAXPOINTS],  ny[MAXPOINTS];
static WORD dr, dg, db;
static WORD nr, ng, nb;
static WORD closed;
static const STRPTR nextlegal[] = {"01458", "236", "01458", "236", "01458", "23", "01458", "", "0145"};
static const WORD advval[] = { 3, 2, 3, 2, 1, 0, 1, 0, 1 };
static char realfunc[14];

struct RastPort *rastPort;
LONG oldx, oldy;

VOID __asm DrawSpline(register __a0 LONG x1, register __a1 LONG y1,
                      register __a2 LONG x2, register __a3 LONG y2,
                      register __a5 LONG x3, register __a6 LONG y3,
                      register __d6 LONG x4, register __d7 LONG y4);


/*****************************************************************************/


#define CMD_NOP       0
#define CMD_BLANK     1
#define CMD_UNBLANK   2
#define CMD_HIDE      3
#define CMD_QUIT      4
#define CMD_SECONDS   5
#define CMD_CYCLE     6
#define CMD_ANIMATION 7


/*****************************************************************************/


static struct TextAttr __far topazAttr =
{
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};


/*****************************************************************************/


static const UWORD __chip           blankData[] = {0,0,0,0,0,0};
static struct SimpleSprite    blankMouse;

#define BlankMouse()   ChangeSprite(NULL,&blankMouse,(APTR)blankData)
#define UnblankMouse() RethinkDisplay();


/*****************************************************************************/


static VOID __stdargs __saveds BlankAction(CxMsg *cxm, CxObj *co)
{
struct InputEvent *ie;
UWORD              i;

    ie = (struct InputEvent *)CxMsgData(cxm);

    if (ie->ie_Class == IECLASS_TIMER)
    {
        if (screen)
        {
            animTimeOut++;

            if (loopCount == oldLoopCount)
            {
                stalled++;
                if (stalled == 40)
                {
                    Forbid();  /* make sure screen pointer doesn't become invalid */

                    if (screen)
                    {
                        for (i=0; i<32; i++)
                            SetRGB32(&screen->ViewPort,i,0,0,0);
                    }

                    Permit();
                }
            }
            else
            {
                oldLoopCount = loopCount;
                stalled      = 0;
            }
        }

        if (!blankTimeOut-- && (!(ie->ie_Qualifier & (IEQUALIFIER_LEFTBUTTON|IEQUALIFIER_RBUTTON|IEQUALIFIER_MIDBUTTON))))
        {
            cmd = CMD_BLANK;
            Signal(parentTask,(1L << cxSignal));
            blankTimeOut = blankInterval;
        }
    }
    else
    {
        if (screen)
        {
            cmd = CMD_UNBLANK;
            Signal(parentTask,(1L << cxSignal));
            SetTaskPri(parentTask,oldPri);
        }
        blankTimeOut = blankInterval;
    }
}


/*****************************************************************************/


static VOID NewIntervals(LONG blankSeconds)
{
    if (blankSeconds <= 0)
        blankSeconds = 1;
    else if (blankSeconds >= 10000)
        blankSeconds = 9999;

    blankInterval = blankSeconds*10;
    blankTimeOut  = blankInterval;
    animTimeOut   = ANIMINTERVAL;
}


/*****************************************************************************/


BOOL ProcessCustomArgs(struct WBStartup *wbMsg, struct DiskObject *diskObj,
                       ULONG *cliOpts)
{
STRPTR arg;
LONG   blankSeconds;

    blankSeconds = 60;
    cycleColors  = TRUE;
    doAnimation  = TRUE;
    loopCount    = 0;

    if (wbMsg)
    {
        if (arg = FindToolType(diskObj->do_ToolTypes,"SECONDS"))
            StrToLong(arg,&blankSeconds);

        if (arg = FindToolType(diskObj->do_ToolTypes,"CYCLECOLORS"))
            cycleColors = !MatchToolValue(arg,"NO");

        if (arg = FindToolType(diskObj->do_ToolTypes,"ANIMATION"))
            doAnimation = !MatchToolValue(arg,"NO");
    }
    else
    {
        if (cliOpts[OPT_SECONDS])
            blankSeconds = *(ULONG *)cliOpts[OPT_SECONDS];

        if (cliOpts[OPT_CYCLECOLORS])
            cycleColors = (Stricmp((STRPTR)cliOpts[OPT_CYCLECOLORS],"NO") != 0);

        if (cliOpts[OPT_ANIMATION])
            doAnimation = (Stricmp((STRPTR)cliOpts[OPT_ANIMATION],"NO") != 0);
    }

    NewIntervals(blankSeconds);

    return(TRUE);
}


/*****************************************************************************/


VOID ProcessCustomCxMsg(ULONG cmd)
{
}


/*****************************************************************************/


VOID ProcessCustomCxCmd(ULONG cmd)
{
}


/*****************************************************************************/


static BOOL ProcessCommand(UWORD cmd);

BOOL ProcessIntuiMsg(struct IntuiMessage *intuiMsg)
{
ULONG            class;
UWORD            icode;
struct Gadget   *gad;
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

    if (gad = CreateContext(&cxGadgets))
    {
        /* Some invariants */
        ng.ng_LeftEdge   = cxWindow->BorderLeft+200;
        ng.ng_Height     = 14;
        ng.ng_TextAttr   = &topazAttr;
        ng.ng_GadgetID   = 0;
        ng.ng_Flags      = NULL;
        ng.ng_VisualInfo = cxVisualInfo;

        ng.ng_TopEdge    = cxWindow->BorderTop+4;
        ng.ng_Width      = 52;
        ng.ng_GadgetText = GetString(MSG_BL_SECONDS_GAD);
        ng.ng_UserData   = (APTR)CMD_SECONDS;

        blankSecondsGad = CreateGadget(INTEGER_KIND,gad,&ng,GTIN_Number,   blankInterval/10,
                                                            GTIN_MaxChars, 4,
                                                            TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+21;
        ng.ng_Width      = 20;
        ng.ng_GadgetText = GetString(MSG_BL_CYCLECOLORS_GAD);
        ng.ng_UserData   = (APTR)CMD_CYCLE;

        cycleColorsGad = CreateGadget(CHECKBOX_KIND,blankSecondsGad,&ng,GTCB_Checked, cycleColors,
                                                                        TAG_DONE);

        ng.ng_TopEdge    = cxWindow->BorderTop+35;
        ng.ng_Width      = 20;
        ng.ng_GadgetText = GetString(MSG_BL_ANIMATION_GAD);
        ng.ng_UserData   = (APTR)CMD_ANIMATION;

        if (animationGad = CreateGadget(CHECKBOX_KIND,cycleColorsGad,&ng,GTCB_Checked, doAnimation,
                                                                         TAG_DONE))
        {
            AddGList(cxWindow,cxGadgets,-1,-1,NULL);
            RefreshGList(cxGadgets,cxWindow,NULL,-1);
            GT_RefreshWindow(cxWindow,NULL);
            ActivateGadget(blankSecondsGad,cxWindow,NULL);
            return(TRUE);
        }
        FreeGadgets(cxGadgets);
        cxGadgets = NULL;
    }
    return(FALSE);
}


/*****************************************************************************/


static struct NewMenu NM[] =
{
    {NM_TITLE,  NULL, 0, 0, 0, (APTR)CMD_NOP,  },
      {NM_ITEM, NULL, 0, 0, 0, (APTR)CMD_HIDE, },
      {NM_ITEM, NULL, 0, 0, 0, (APTR)CMD_QUIT, },

    {NM_END,    0,    0, 0, 0, (APTR)CMD_NOP,  }
};


static VOID DoItem(WORD itemNum, ULONG label)
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


VOID RefreshWindow(BOOL refreshEvent)
{
    if (refreshEvent)
    {
        GT_BeginRefresh(cxWindow);
        GT_EndRefresh(cxWindow,TRUE);
    }
}


/*****************************************************************************/


static LONG seed;

static WORD Random(WORD i)
{
LONG rseed;
LONG rval;

    rseed = seed;
    if  (rseed == 0)
        rseed = 323214521 + screen->MouseX + screen->MouseY;

    rseed = rseed * 123213 + 121;
    rval  = (rseed >> 5) & 65535;
    seed  = rseed;
    return ((WORD)((i * rval) >> 16));
}


/*****************************************************************************/


static VOID MakeFunc(VOID)
{
WORD i;
WORD goallen;
WORD sofar;
char *p;
char *nextpossib;

    closed = Random(4);
    switch (closed)
    {
        case 3: closed = 2;

        case 2: goallen = 3 + Random(4);
                break;

        case 1: goallen = 4 + Random(7);
                break;

        case 0: goallen = 2 + Random(8);
                break;
    }

    while (1)
    {
        if  (closed == 0)
            nextpossib = "0145";
        else
            nextpossib = "0123456";

        sofar   = 0;
        p = realfunc;
        while (sofar < goallen)
        {
            i = nextpossib[Random((WORD)strlen(nextpossib))]   - '0';
            *p++ = i;
            nextpossib = nextlegal[i];
            sofar   +=  advval[i];
        }

        if  (sofar == goallen)
        {
            if  (closed == 0)
            {
                if  (nextpossib[0]  ==  '0')
                    break;

            }
            else
            {
                if  (*nextpossib == '0' || realfunc[0] < 4  ||  *(p-1) <    4)  {
                    if  ((*nextpossib == '0') ?
                             ((realfunc[0]  & 2) != 0) : ((realfunc[0]  & 2) == 0)) {
                        if  (realfunc[0] != 5) {
                            realfunc[0] ^=  2;
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }

    *p  = 100;
    maxPoints = goallen;
    switch (closed)
    {
        case 2: for (i=0; i<p-realfunc; i++)
                    p[i] = realfunc[i];
                p[p-realfunc] = 100;
                break;

        case 1: break;

        case 0: maxPoints++;
                break;
    }
}


/*****************************************************************************/


static VOID Draw_S_F(WORD *xptr, WORD *yptr)
{
    oldx = HALF(xptr);
    oldy = HALF(yptr);

    DrawSpline(FIX(oldx),    FIX(oldy),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIXH(xptr+2), FIXH(yptr+2));
}


/*****************************************************************************/


static VOID Draw_SF(WORD *xptr, WORD *yptr)
{
    oldx = HALF(xptr);
    oldy = HALF(yptr);

    DrawSpline(FIX(oldx),    FIX(oldy),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIX(xptr[3]), FIX(yptr[3]));
}


/*****************************************************************************/


static VOID DrawS_F(WORD *xptr, WORD *yptr)
{
    oldx = *xptr;
    oldy = *yptr;

    DrawSpline(FIX(*xptr),   FIX(*yptr),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIXH(xptr+2), FIXH(yptr+2));
}


/*****************************************************************************/


static VOID DrawSF(WORD *xptr, WORD *yptr)
{
    oldx = *xptr;
    oldy = *yptr;

    DrawSpline(FIX(*xptr),   FIX(*yptr),
               FIX(xptr[1]), FIX(yptr[1]),
               FIX(xptr[2]), FIX(yptr[2]),
               FIX(xptr[3]), FIX(yptr[3]));
}


/*****************************************************************************/


static VOID Draw_LF(WORD *xptr, WORD *yptr)
{
    Move(rastPort,HALF(xptr),HALF(yptr));
    xptr++;
    yptr++;
    Draw(rastPort,*xptr,*yptr);
}


/*****************************************************************************/


static VOID DrawL_F(WORD *xptr, WORD *yptr)
{
    Move(rastPort, *xptr, *yptr);
    Draw(rastPort, HALF(xptr),HALF(yptr));
}


/*****************************************************************************/


static VOID DrawLF(WORD *xptr, WORD *yptr)
{
    Move(rastPort,*xptr, *yptr);
    xptr++;
    yptr++;
    Draw(rastPort,*xptr,*yptr);
}


/*****************************************************************************/


static VOID DrawnLF(WORD *dummya, WORD *dummyb)
{
}


/*****************************************************************************/


static VOID (*funcs[])(WORD *, WORD *) =
{
    DrawSF,
    DrawS_F,
    Draw_SF,
    Draw_S_F,
    DrawLF,
    DrawL_F,
    Draw_LF,
    NULL,
    DrawnLF
};


/*****************************************************************************/


static VOID DrawFunc(struct box *bptr)
{
LONG i;
WORD *x, *y;
char *p;

    switch (closed)
    {
        case 2: for (i=0, x=&(bptr->x[0]),  y=&(bptr->y[0]); i<maxPoints;   i++, x++, y++)
                {
                    x[maxPoints] =  screenWidth - 1 -   *x;
                    y[maxPoints] =  screenHeight -  1 - *y;
                }
setup:
                x[maxPoints] =  bptr->x[0];
                y[maxPoints] =  bptr->y[0];
                x++, y++;
                x[maxPoints] =  bptr->x[1];
                y[maxPoints] =  bptr->y[1];
                break;

        case 1: x = &(bptr->x[0]);
                y = &(bptr->y[0]);
                goto setup;
    }

    p = realfunc;
    x = &(bptr->x[0]);
    y = &(bptr->y[0]);
    while (*p < 10)
    {
        (funcs[*p])(x,  y);
        i = advval[*p];
        x += i;
        y += i;
        p++;
    }
}


/*****************************************************************************/


static VOID Adv(WORD *o, WORD *d, WORD *n, WORD w)
{
    *n = *o + *d;
    if (*n < 0)
    {
        *n = 0;
        *d = Random(6) + 1;
    }
    else if (*n >= w)
    {
        *n = w - 1;
        *d = - Random(6) - 1;
    }
}


/*****************************************************************************/


static VOID AdvanceLines(VOID)
{
WORD i;

    for (i=0; i<maxPoints; i++)
    {
        Adv(ox+i, dx+i, nx+i, screenWidth);
        Adv(oy+i, dy+i, ny+i, screenHeight);
    }
}


/*****************************************************************************/


static VOID DrawNew(VOID)
{
WORD       i;
struct box *bptr;

    while (numlines >= maxlines)
    {
        SetAPen(rastPort,0);
        bptr = eptr;
        DrawFunc(bptr);

        SetAPen(rastPort,1);
        numlines--;
        bptr++;

        if (bptr == store + MAXLINES)
            bptr = store;
        eptr = bptr;
    }

    bptr = ptr;
    for (i=0; i<maxPoints; i++)
    {
        bptr->x[i] = ox[i] = nx[i];
        bptr->y[i] = oy[i] = ny[i];
    }

    DrawFunc(bptr);
    numlines++;
    bptr++;

    if  (bptr == store + MAXLINES)
    {
        bptr = store;
        if  (mdelta == 1)
        {
            maxlines++;
            if (maxlines >= MAXLINES - 1)
                mdelta = -1;
        }
        else
        {
            maxlines--;
            if  (maxlines <= 2)
                mdelta = 1;
        }
    }
    ptr = bptr;
}


/*****************************************************************************/


static VOID StartLines(VOID)
{
WORD i;

    if (doAnimation)
    {
        ptr  = store;
        eptr = store;
        numlines = 0 ;
        if (dx[0] == 0)
        {
            for (i=0; i<MAXPOINTS; i++)
            {
                ox[i] = Random(screenWidth);
                oy[i] = Random(screenHeight);
                dx[i] = 2 + Random(3);
                dy[i] = 2 + Random(3);
            }
        }
    }

    nr = 53;
    ng = 33;
    nb = 35;
    dr = -3;
    dg = 5;
    db = 7;

    if (!cycleColors)
        SetRGB32(&screen->ViewPort,1,0xbbbbbbbb,0xbbbbbbbb,0xbbbbbbbb);
    else
        SetRGB32(&screen->ViewPort,1,nr << 24,ng << 24,nb << 24);

    BlankMouse();

    if (doAnimation)
    {
        for (i=0; i<maxlines; i++)
        {
            AdvanceLines();
            DrawNew();
        }
    }
}


/*****************************************************************************/


static VOID Colors(VOID)
{
WORD or, og, ob;
ULONG color;

    if (cycleColors)
    {
        or = nr;
        og = ng;
        ob = nb;
        Adv(&or, &dr, &nr, 128);
        Adv(&og, &dg, &ng, 128);
        Adv(&ob, &db, &nb, 128);

        color = 0;
        if (doAnimation)
            color = 1;

        SetRGB32(&screen->ViewPort,color,nr << 24,ng << 24,nb << 24);

        BlankMouse();
    }
}


/*****************************************************************************/


VOID HideInterface(VOID)
{
    if (cxWindow)
        NewIntervals(((struct StringInfo *)blankSecondsGad->SpecialInfo)->LongInt);

    DisposeWindow();
}


/*****************************************************************************/


struct ColorBunch
{
     UWORD NumColors;
     UWORD FirstColor;
     ULONG Colors[32*3];
     UWORD EndMarker;
};


static BOOL ProcessCommand(UWORD cmd)
{
ULONG                 lock;
ULONG                 modeID;
struct InputEvent     ie;
struct DimensionInfo  dimInfo;
WORD                  screenW, screenH, screenD;
struct Screen        *tempScreen;
struct TagItem        tags[2];
struct Rectangle      rect;
struct ViewPortExtra *extra;
struct ColorBunch     black;

    switch (cmd)
    {
        case CMD_HIDE     : HideInterface();
                            break;

        case CMD_QUIT     : return(FALSE);

        case CMD_SECONDS  : NewIntervals(((struct StringInfo *)blankSecondsGad->SpecialInfo)->LongInt);
                            break;

        case CMD_CYCLE    : cycleColors = (SELECTED & cycleColorsGad->Flags);
                            break;

        case CMD_ANIMATION: doAnimation = (SELECTED & animationGad->Flags);
                            break;

        case CMD_BLANK    : if (screen)
                            {
                                ScreenToFront(screen);
                            }
                            else
                            {
                                tags[0].ti_Tag = VTAG_VIEWPORTEXTRA_GET;
                                tags[1].ti_Tag = TAG_END;

			        lock = LockIBase(0);
			        if (IntuitionBase->FirstScreen)
			        {
                                    modeID  = GetVPModeID(&IntuitionBase->FirstScreen->ViewPort);
                                    VideoControl(IntuitionBase->FirstScreen->ViewPort.ColorMap,tags);
                                    extra   = (struct ViewPortExtra *)tags[0].ti_Data;
                                    rect    = extra->DisplayClip;
                                }
                                else
                                {
                                    modeID = LORES_KEY;
                                    QueryOverscan(LORES_KEY,&rect,OSCAN_TEXT);
                                }

                                mouseX  = IntuitionBase->MouseX;
                                mouseY  = IntuitionBase->MouseY;
                                screenW = rect.MaxX - rect.MinX + 1;
                                screenH = rect.MaxY - rect.MinY + 1;
                                screenD = 1;
                                UnlockIBase(lock);

                                if (!doAnimation)
                                {
                                    if (GetDisplayInfoData(FindDisplayInfo(modeID),(APTR)&dimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,INVALID_ID))
                                    {
                                        screenH = dimInfo.MinRasterHeight;
                                    }
                                }

                                if (screenW < 32)
                                    screenW = 32;

                                if (screenH < 8)
                                    screenH = 8;

                                /* Avoid external video showing through */
                                tags[0].ti_Tag  = VTAG_CHROMAKEY_SET;
                                tags[0].ti_Data = TRUE;
                                tags[1].ti_Tag  = TAG_DONE;

                                memset(&black,0,sizeof(black));
                                black.NumColors = 32;

                                if (screen = OpenScreenTags(NULL,SA_DisplayID,    modeID,
                                                                 SA_Depth,        screenD,
                                                                 SA_Width,        screenW,
                                                                 SA_Height,       screenH,
                                                                 SA_Quiet,        TRUE,
                                                                 SA_DClip,        &rect,
                                                                 SA_VideoControl, tags,
                                                                 SA_Colors32,     &black,
                                                                 TAG_DONE))
                                {
                                    BlankMouse();

                                    rastPort     = &screen->RastPort;
                                    screenWidth  = screen->Width;
                                    screenHeight = screen->Height;
                                    animTimeOut  = 0;

                                    SetAPen(rastPort,1);
                                    MakeFunc();
                                    StartLines();
                                    Colors();
                                }
                            }

                            if (screen && (doAnimation || cycleColors))
                            {
                                loopCount = 0;
                                stalled   = 0;
                                SetTaskPri(FindTask(NULL),-128);
                                while ((SetSignal(0,0) & (1<<cxSignal)) == 0)
                                {
                                    loopCount++;
                                    if (doAnimation)
                                    {
                                        AdvanceLines();
                                        DrawNew();
                                        Colors();

                                        if (animTimeOut >= ANIMINTERVAL)
                                        {
                                            animTimeOut = 0;
                                            MakeFunc();
                                            SetRast(rastPort,0);
                                            StartLines();
                                        }
                                    }
                                    else
                                    {
                                        WaitTOF();
                                        Colors();
                                    }
                                }
                                Signal(parentTask,(1 << cxSignal));
                                SetTaskPri(parentTask,oldPri);
                            }
                            break;

        case CMD_UNBLANK  : if (screen)
                            {
                                loopCount  = 0;
                                stalled    = 0;
                                tempScreen = screen;
                                screen     = NULL;         /* this is so input handler doesn't SetRGB32() into a closed screen */
                                CloseScreen(tempScreen);

                                ie.ie_NextEvent          = NULL;
                                ie.ie_Class              = IECLASS_POINTERPOS;
                                ie.ie_SubClass           = 0;
                                ie.ie_Code               = 0;
                                ie.ie_Qualifier          = 0;
                                ie.ie_X                  = mouseX;
                                ie.ie_Y                  = mouseY;
                                ie.ie_TimeStamp.tv_secs  = 0;
                                ie.ie_TimeStamp.tv_micro = 0;
                                AddIEvents(&ie);
                            }
                            break;


    }

    return(TRUE);
}


/*****************************************************************************/


VOID ProcessCustomCxSig(VOID)
{
    ProcessCommand(cmd);
}


/*****************************************************************************/


BOOL CreateCustomCx(CxObj *broker)
{
    parentTask = FindTask(NULL);
    oldPri = parentTask->tc_Node.ln_Pri;

    AttachCxObj(broker,CxCustom(BlankAction,NULL));
    return ((BOOL)(CxObjError(broker) == 0));
}


/*****************************************************************************/


VOID DisposeCustomCx(VOID)
{
    SetTaskPri(parentTask,oldPri);
    ProcessCommand(CMD_UNBLANK);
}