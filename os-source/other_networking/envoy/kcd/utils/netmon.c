#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#define INTUITION_IOBSOLETE_H TRUE
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <devices/scsidisk.h>
#include "work:appn/nipc/nipcinternal.h"

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>
/* #include <clib/alib_protos.h> */
#include <clib/nipc_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/nipc_pragmas.h>

#define MIN_VERSION 36L

#define IPLABEL_ID           0
#define ARPLABEL_ID          1
#define RDPLABEL_ID          2
#define UDPLABEL_ID          3
#define MEMLABLE_ID          4
#define IPIN_ID              5
#define IPOUT_ID             6
#define IPFORWARD_ID         7
#define IPKEPT_ID            8
#define IPBYTESIN_ID         9
#define IPBYTESOUT_ID       10
#define IPBYTESFORWARD_ID   11
#define IPBYTESKEPT_ID      12
#define ARPREQSENT_ID       13
#define ARPRPLRECV_ID       14
#define ARPREQRECV_ID       15
#define ARPRPLSENT_ID       16
#define RDPIN_ID            17
#define RDPOUT_ID           18
#define RDPBYTESIN_ID       19
#define RDPBYTESOUT_ID      20
#define UDPIN_ID            21
#define UDPOUT_ID           22
#define UDPBYTESIN_ID       23
#define UDPBYTESOUT_ID      24
#define CHIPSTART_ID        25
#define AVAILCHIP_ID        26
#define CHIPDIFF_ID         27
#define FASTSTART_ID        28
#define AVAILFAST_ID        29
#define FASTDIFF_ID         30
#define MAX_GADGETS         31

struct Gadget *GadgetArray[MAX_GADGETS];

UBYTE *GadgetLabels[]={ "IP", "ARP", "RDP", "UDP", "Memory Free",
                        "Packets Sent      ",
                        "Packets Received  ",
                        "Packets Forwarded ",
                        "Packets Kept      ",
                        "Bytes Sent        ",
                        "Bytes Received    ",
                        "Bytes Forwarded   ",
                        "Bytes Kept        ",
                        "Requests Sent     ",
                        "Replies Received  ",
                        "Requests Received ",
                        "Replies Sent      ",
                        "Packets Sent      ",
                        "Packets Received  ",
                        "Bytes Sent        ",
                        "Bytes Received    ",
                        "Packets Sent      ",
                        "Packets Received  ",
                        "Bytes Sent        ",
                        "Bytes Received    ",
                        "Initial Chip      ",
                        "Current Chip      ",
                        "Difference        ",
                        "Initial Fast      ",
                        "Current Fast      ",
                        "Difference        " };

UWORD GadgetPosInfo[]= {230,  0, 20, 10,
                        230, 60, 20, 10,
                        230,100, 20, 10,
                        230,140, 20, 10,
                        230,180, 20, 10,

                        150, 12, 80, 10,
                        150, 22, 80, 10,
                        150, 32, 80, 10,
                        150, 42, 80, 10,
                        400, 12, 80, 10,
                        400, 22, 80, 10,
                        400, 32, 80, 10,
                        400, 42, 80, 10,

                        150, 72, 80, 10,
                        150, 82, 80, 10,
                        400, 72, 80, 10,
                        400, 82, 80, 10,

                        150,112, 80, 10,
                        150,122, 80, 10,
                        400,112, 80, 10,
                        400,122, 80, 10,

                        150,152, 80, 10,
                        150,162, 80, 10,
                        400,152, 80, 10,
                        400,162, 80, 10,

                        150,192, 80, 10,
                        150,202, 80, 10,
                        150,212, 80, 10,
                        400,192, 80, 10,
                        400,202, 80, 10,
                        400,212, 80, 10 };

UBYTE *StatStrings[] = { NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0",
                         "         0" };

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *GadToolsBase;
struct Library *NIPCBase;
extern struct Library *SysBase;
struct Screen *screen = NULL;
struct Window *window = NULL;
struct Gadget *glist = NULL;
struct List list;
struct Remember *rmem = NULL;
void *vi = NULL;
struct NIPCMonitorInfo *MonitorInfo = NULL;

UBYTE StatFmt[]="%10ld";

struct TextAttr topaz80 = { "topaz.font",8,0,0 };

struct NewWindow new_window = {
    50, 20, 506, 232,
    -1, -1,
    IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS,
    WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIMPLE_REFRESH | WFLG_RMBTRAP,
    NULL,
    NULL,
    "NIPC Monitor Utility Alpha",
    NULL,
    NULL,
    0, 0, 0, 0,
    WBENCHSCREEN
};

void abort(char *);
void setup(void);
void closedown(void);
void create_gadgets(void);
void handle_input(void);
void exit(int);
void RefreshWindow(void);
void UpdateStats(void);

int
_main(void)
{
   setup();
   create_gadgets();
   SetTaskPri(FindTask(0L),-125);
   handle_input();
   return(0);
}

void abort(txt)
char *txt;
{
   closedown();
   exit(0);
}

void setup(void)
{
   IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",37L);
   if (!IntuitionBase) abort("Couldn't open V37 Intuition or higher.\n");

   GadToolsBase = (struct Library *) OpenLibrary("gadtools.library",37L);
   if (!GadToolsBase) abort("Couldn't open V37 GadTools or higher.\n");

   GfxBase = (struct GfxBase *) OpenLibrary("graphics.library",37L);
   if (!GfxBase) abort("Couldn't open V37 Graphics or higher.\n");

   NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
   if (!NIPCBase) abort("Couldn't open NIPC library.\n");

   MonitorInfo = StartMonitor();
   if (!MonitorInfo) abort("Someone already monitoring NIPC.\n");

   screen = LockPubScreen(NULL);
   if (!screen) abort("Couldn't lock the default public screen.\n");

   vi = GetVisualInfoA(screen, NULL);
   if (!vi) abort("Couldn't get the default public screens's VisualInfo.\n");

   new_window.Height += (screen->WBorTop + screen->Font->ta_YSize + 1);
   window = OpenWindowTags(&new_window,
            WA_PubScreen, screen,
            TAG_DONE);
   if (!window) abort("Couldn't open the new window.\n");
}

void closedown(void)
{
   if (window) CloseWindow(window);
   if (glist) FreeGadgets(glist);
   if (vi) FreeVisualInfo(vi);
   if (screen) UnlockPubScreen(NULL, screen);
   if (rmem) FreeRemember(&rmem, TRUE);
   if (GadToolsBase) CloseLibrary(GadToolsBase);
   if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
   if (GfxBase) CloseLibrary((struct Library *)GfxBase);
   if (MonitorInfo) StopMonitor();
   if (NIPCBase) CloseLibrary(NIPCBase);
}

void create_gadgets()
{
   UWORD top, left;
   struct NewGadget ng;
   struct Gadget *gad;
   int index;

   /* Quit Program */
   top = window->BorderTop + 2;
   left = window->BorderLeft + 10;
   gad = CreateContext(&glist);

    for(index = 0;index < MAX_GADGETS; index++)
    {
        ng.ng_LeftEdge = GadgetPosInfo[index * 4] + left;
        ng.ng_TopEdge  = GadgetPosInfo[index * 4 + 1] + top;
        ng.ng_Width    = GadgetPosInfo[index * 4 + 2];
        ng.ng_Height   = GadgetPosInfo[index * 4 + 3];
        ng.ng_GadgetText = GadgetLabels[index];
        ng.ng_GadgetID = index;
        ng.ng_Flags = ((index<5)?PLACETEXT_IN|NG_HIGHLABEL:PLACETEXT_LEFT);
        ng.ng_TextAttr = &topaz80;
        ng.ng_VisualInfo = vi;
        gad = CreateGadget(TEXT_KIND, gad, &ng,
                           GTTX_Text, StatStrings[index],
                           TAG_DONE);
        GadgetArray[index] = gad;
    }

   if(!gad) abort("Couldn't allocate the Gadget List.");

   AddGList(window, glist, (UWORD)-1,(UWORD)-1, NULL);
   RefreshGList(glist, window, NULL, (UWORD)-1);
   RefreshWindow();
   UpdateStats();

}

void RefreshWindow(void)
{
    UWORD left, top, right, bottom;
    left = window->BorderLeft;
    right = window->BorderRight;
    top = window->BorderTop;
    bottom = window->BorderBottom;

    GT_BeginRefresh(window);

    GT_RefreshWindow(window, NULL);

    GT_EndRefresh(window, TRUE);

    DrawBevelBox(window->RPort, left, top, window->Width - (left + right), 60,
                 GT_VisualInfo, vi, TAG_DONE);
    DrawBevelBox(window->RPort, left + 4, top + 12, window->Width - (left + right + 8), 44,
                 GTBB_Recessed, TRUE,
                 GT_VisualInfo, vi, TAG_DONE);

    DrawBevelBox(window->RPort, left, top + 60, window->Width - (left + right), 40,
                 GT_VisualInfo, vi, TAG_DONE);
    DrawBevelBox(window->RPort, left + 4, top + 72, window->Width - (left + right + 8), 24,
                 GTBB_Recessed, TRUE,
                 GT_VisualInfo, vi, TAG_DONE);

    DrawBevelBox(window->RPort, left, top + 100, window->Width - (left + right), 40,
                 GT_VisualInfo, vi, TAG_DONE);
    DrawBevelBox(window->RPort, left + 4, top + 112, window->Width - (left + right + 8), 24,
                 GTBB_Recessed, TRUE,
                 GT_VisualInfo, vi, TAG_DONE);

    DrawBevelBox(window->RPort, left, top + 140, window->Width - (left + right), 40,
                 GT_VisualInfo, vi, TAG_DONE);
    DrawBevelBox(window->RPort, left + 4, top + 152, window->Width - (left + right + 8), 24,
                 GTBB_Recessed, TRUE,
                 GT_VisualInfo, vi, TAG_DONE);

    DrawBevelBox(window->RPort, left, top + 180, window->Width - (left + right), 50,
                 GT_VisualInfo, vi, TAG_DONE);
    DrawBevelBox(window->RPort, left + 4, top + 192, window->Width - (left + right + 8), 34,
                 GTBB_Recessed, TRUE,
                 GT_VisualInfo, vi, TAG_DONE);

    return;
}

void handle_input(void)
{
    struct IntuiMessage *message;
    struct Gadget *gadget;
    ULONG monmask,winmask, sigs;
    ULONG class;
    UWORD code;

    monmask = (1 << MonitorInfo->nmi_SigBitNum);
    winmask = (1 << window->UserPort->mp_SigBit);

    while(1)
    {
        sigs=Wait(monmask|winmask);

        if(sigs & winmask)
        {
            while(message=GT_GetIMsg(window->UserPort))
            {
                class = message->Class;
                code = message->Code;
                gadget = (struct Gadget *)message->IAddress;
                GT_ReplyIMsg(message);

                switch(class)
                {
                    case IDCMP_CLOSEWINDOW:   closedown();
                                              exit(0);

                    case IDCMP_REFRESHWINDOW: RefreshWindow();
                                              break;

                    case IDCMP_MOUSEBUTTONS:
                                              if(code == MENUDOWN)
                                              {
                                                Forbid();
                                                StopMonitor();
                                                MonitorInfo = StartMonitor();
                                                Permit();
                                                UpdateStats();
                                              }

                                              break;
                }
            }
        }
        if(sigs & monmask)
            UpdateStats();
    }
}

void UpdateStats()
{
    UWORD index;
    ULONG init, curr, diff;
    sprintf(StatStrings[IPIN_ID],StatFmt,MonitorInfo->nmi_IPIn);
    sprintf(StatStrings[IPOUT_ID],StatFmt,MonitorInfo->nmi_IPOut);
    sprintf(StatStrings[IPFORWARD_ID],StatFmt,MonitorInfo->nmi_IPForwarded);
    sprintf(StatStrings[IPKEPT_ID],StatFmt,MonitorInfo->nmi_IPKept);
    sprintf(StatStrings[IPBYTESIN_ID],StatFmt,MonitorInfo->nmi_IPBytesIn);
    sprintf(StatStrings[IPBYTESOUT_ID],StatFmt,MonitorInfo->nmi_IPBytesOut);
    sprintf(StatStrings[IPBYTESFORWARD_ID],StatFmt,MonitorInfo->nmi_IPBytesForwarded);
    sprintf(StatStrings[IPBYTESKEPT_ID],StatFmt,MonitorInfo->nmi_IPBytesKept);

    init = MonitorInfo->nmi_AvailChipStart;
    curr = AvailMem(MEMF_CHIP);
    diff = (init > curr)?(init - curr):(curr - init);

    sprintf(StatStrings[CHIPSTART_ID],StatFmt,init);
    sprintf(StatStrings[AVAILCHIP_ID],StatFmt,curr);
    sprintf(StatStrings[CHIPDIFF_ID],StatFmt,diff);

    init = MonitorInfo->nmi_AvailFastStart;
    curr = AvailMem(MEMF_FAST);
    diff = (init > curr)?(init - curr):(curr - init);

    sprintf(StatStrings[FASTSTART_ID],StatFmt,init);
    sprintf(StatStrings[AVAILFAST_ID],StatFmt,curr);
    sprintf(StatStrings[FASTDIFF_ID],StatFmt,diff);

    for(index = IPIN_ID; index < MAX_GADGETS; index++)
        GT_SetGadgetAttrs(GadgetArray[index],window,NULL,
                          GTTX_Text, StatStrings[index],
                          TAG_DONE);

}

