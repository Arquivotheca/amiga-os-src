
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <workbench/startup.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <devices/timer.h>
#include <cdgs:include/internal/playerlibrary.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <cdgs:include/internal/player_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <cdgs:include/internal/player_pragmas.h>

#include "cdplayer_rev.h"

#define SysBase (*((struct ExecBase **)4))

extern UWORD volatile __far vhposr;

extern struct Image FREVI1;
extern struct Image FREV_SI1;
extern struct Image PlayPauseI1;
extern struct Image PlayPause_PlayI1;
extern struct Image PlayPause_PauseI1;
extern struct Image FFWDI1;
extern struct Image FFWD_SI1;
extern struct Image StopI1;
extern struct Image Stop_SI1;
extern struct Image LoopI1;
extern struct Image Loop_SI1;
extern struct Image IntroI1;
extern struct Image Intro_SI1;
extern struct Image RNDI1;
extern struct Image RND_SI1;
extern struct Image TimeMode0I1;
extern struct Image TimeMode1I1;
extern struct Image TimeMode2I1;
extern struct Image TimeMode3I1;

extern struct Image FREVTaI1;
extern struct Image FREVTsI1;
extern struct Image FREVTdI1;
extern struct Image PPTaI1;
extern struct Image PPTsI1;
extern struct Image PPTdI1;
extern struct Image FFWDTaI1;
extern struct Image FFWDTsI1;
extern struct Image FFWDTdI1;
extern struct Image STOPTaI1;
extern struct Image STOPTsI1;
extern struct Image STOPTdI1;

struct Image FREVT, PPT, FFWDT, STOPT;

struct Library *DOSBase;
struct Library *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *PlayerBase;
struct Library *IconBase;

#define TEMPLATE      "TOP/N,LEFT/N,SMALL/S"
#define OPT_TOP       0
#define OPT_LEFT      1
#define OPT_SMALL     2
#define OPT_COUNT     3

#define NUMTRACKS       20
#define BIGGADGETS      NUMTRACKS
#define GID_FREV        NUMTRACKS
#define GID_PLAYPAUSE   NUMTRACKS + 1
#define GID_FFWD        NUMTRACKS + 2
#define GID_STOP        NUMTRACKS + 3
#define GID_LOOPMODE    NUMTRACKS + 4
#define GID_INTROMODE   NUMTRACKS + 5
#define GID_TIMEMODE    NUMTRACKS + 6
#define GID_RND         NUMTRACKS + 7
#define GID_DRAGBAR     100

#define MATRIXLEFT      155
#define MATRIXTOP       7

#define BIGBUTTONSIZE   26
#define MODEBUTTONSIZE  33
#define MODEBUTTONGAP   3
#define BIGBUTTONGAP    5
#define ZOOMWIDTH       (167+12*DefaultFont->tf_XSize)

#define BigWindow (window->Width == WIN_Width)

struct TextFont *DefaultFont;

enum { OPT_LOOP, OPT_INTRO, OPT_TIME };

BYTE NextTimeMode[] = { 1, 3, 0, 2 };

char TrkTimeStr[30];

struct IntuiText IText = {

    1, 3,
    JAM2,
    8, 2,
    NULL,
    &TrkTimeStr[0],
    NULL
    };

struct Image *IconArray[][4] = {

    { &FREVI1,      &FREV_SI1 },
    { &PlayPauseI1, &PlayPause_PlayI1, &PlayPause_PauseI1 },
    { &FFWDI1,      &FFWD_SI1 },
    { &StopI1,      &Stop_SI1 },
    { &LoopI1,      &Loop_SI1 },
    { &IntroI1,     &Intro_SI1 },
    { &TimeMode0I1, &TimeMode1I1,      &TimeMode3I1,      &TimeMode2I1 },
    { &RNDI1,       &RND_SI1 },
    };


char TrackText[] = "01020304050607080910111213141516171819202122232425262728293031323334353637383940" VERSTAG;

WORD ZoomPos[] = { 0, 0, 0, 0 };

struct TagItem  WinTags[] = {

    {WA_Title,NULL},
    {WA_CustomScreen,NULL},
    {WA_Top,0},
    {WA_Left,0},
    {WA_Height,0},
    {WA_Width,0},
    {WA_Gadgets,NULL},
    {WA_Flags,WFLG_CLOSEGADGET|WFLG_DEPTHGADGET|WFLG_SMART_REFRESH|WFLG_ACTIVATE},
    {WA_Zoom, (ULONG)&ZoomPos},
    {TAG_DONE,0}
    };

#define DetailPen        DrawInfo->dri_Pens[DETAILPEN]
#define BlockPen         DrawInfo->dri_Pens[BLOCKPEN]
#define TextPen          DrawInfo->dri_Pens[TEXTPEN]
#define ShinePen         DrawInfo->dri_Pens[SHINEPEN]
#define ShadowPen        DrawInfo->dri_Pens[SHADOWPEN]
#define FillPen          DrawInfo->dri_Pens[FILLPEN]
#define FillTextPen      DrawInfo->dri_Pens[FILLTEXTPEN]
#define BackgroundPen    DrawInfo->dri_Pens[BACKGROUNDPEN]
#define HighlightTextPen DrawInfo->dri_Pens[HIGHLIGHTTEXTPEN]
#define BarDetailPen     DrawInfo->dri_Pens[BARDETAILPEN]
#define BarBlockPen      DrawInfo->dri_Pens[BARBLOCKPEN]
#define BarTrimPen       DrawInfo->dri_Pens[BARTRIMPEN]

struct Window        *window;
struct DrawInfo      *DrawInfo;
struct MsgPort       *IPort;
struct MsgPort       *TPort;
struct timerequest   *TReq;
struct Gadget       **FirstControlGadget, *BigGadgets, *TitleGadgets, *DragGadget;
struct Gadget        *Gad_FREV, *Gad_PLAYPAUSE, *Gad_FFWD, *Gad_STOP, *Gad_LOOPMODE, *Gad_INTROMODE, *Gad_RND, *Gad_TIMEMODE;
struct PlayList      *PlayList, UndoPlayList;
struct PlayerState    PlayerState;
struct PlayerOptions  PlayerOptions;

int                   AudioDisk = 0, ColonLit = 1, MinusLit = 0, STxtSize = 0, SMALL = 0;
int                   Selecting, Selected, Playing, Paused, CurrentIndex, HighlightedIndex;
int                   LoopMode, IntroMode, TimeMode, RND = 0, DisplayTracks;
ULONG                 randomseed;



#define WIN_Title   WinTags[0].ti_Data
#define WIN_Screen  WinTags[1].ti_Data
#define WIN_Top     WinTags[2].ti_Data
#define WIN_Left    WinTags[3].ti_Data
#define WIN_Height  WinTags[4].ti_Data
#define WIN_Width   WinTags[5].ti_Data
#define WIN_Gadgets WinTags[6].ti_Data




ULONG val(char *s) {

ULONG v;
char  c;

    v = 0;

    for (; *s; s++) {

        c = *s;

        if ((c>='0')&&(c<='9')) c-='0';
        v=v*10+c;
        }

    return(v);
    }



UWORD RandomNumber(ULONG range) {

ULONG iterations = range-1;
ULONG overflow   = 0;

    while (iterations) {

        if (randomseed & 0x80000000) overflow = 1;

        randomseed *= 2;

        if (overflow) randomseed ^= 0x1D872B41;

        iterations >>= 1;
        }

    return((UWORD)(((randomseed & 0xFFFF) * range) >> 16));
    }


void StripIntuiMessages(struct MsgPort *mp) {

struct IntuiMessage *msg;
struct Node         *succ;

    msg = (struct IntuiMessage *)mp->mp_MsgList.lh_Head;

    while (succ = msg->ExecMessage.mn_Node.ln_Succ) {

        if (msg->IDCMPWindow == window) {

            Remove(msg);
            ReplyMsg(msg);
            }
        
        msg = (struct IntuiMessage *)succ;
        }
    }



struct Gadget *MakeGadget(WORD LeftEdge, WORD TopEdge, struct Image *Image, UWORD ID) {

struct Gadget *Gad;

    if (Gad = (struct Gadget *)AllocMem(sizeof(struct Gadget), MEMF_PUBLIC|MEMF_CLEAR)) {

        Gad->LeftEdge = LeftEdge;
        Gad->TopEdge  = TopEdge;
        Gad->Width    = Image->Width;
        Gad->Height   = Image->Height;

        Gad->Flags        = GFLG_GADGHNONE|GFLG_GADGIMAGE;
        Gad->Activation   = GACT_RELVERIFY|GACT_IMMEDIATE;
        Gad->GadgetType   = GTYP_BOOLGADGET;
        Gad->GadgetID     = ID;
        Gad->GadgetRender = Image;

        return (Gad);
        }

    return (NULL);
    }


struct Gadget *MakeTitleGadget(WORD LeftEdge, WORD TopEdge, WORD Width, WORD Height, WORD Type,
                               struct Image *Image, struct Image *SImage, struct IntuiText *IText, UWORD ID) {

struct Gadget *Gad;

    if (Gad = (struct Gadget *)AllocMem(sizeof(struct Gadget), MEMF_PUBLIC|MEMF_CLEAR)) {

        Gad->LeftEdge = LeftEdge;
        Gad->TopEdge  = TopEdge;
        Gad->Width    = Width;
        Gad->Height   = Height;

        Gad->Flags        = GFLG_GADGHNONE;
        Gad->Activation   = GACT_RELVERIFY|GACT_IMMEDIATE;
        Gad->GadgetType   = Type;
        Gad->GadgetID     = ID;
        Gad->GadgetRender = Image;
        Gad->GadgetText   = IText;

        if (Gad->SelectRender = SImage) Gad->Flags  = GFLG_GADGHIMAGE;
        if (Image)                      Gad->Flags |= GFLG_GADGIMAGE;

        return (Gad);
        }

    return (NULL);
    }


struct Gadget *MakeTrackGadget(WORD LeftEdge, WORD TopEdge, WORD Width, WORD Height, UWORD ID) {

struct Gadget *Gad;
struct Border *Border;
UWORD         *Pairs;

    if (Gad = (struct Gadget *)AllocMem(sizeof(struct Gadget), MEMF_PUBLIC|MEMF_CLEAR)) {

        Gad->LeftEdge = LeftEdge;
        Gad->TopEdge  = TopEdge;
        Gad->Width    = Width;
        Gad->Height   = Height;

        Gad->Flags      = GFLG_GADGHIMAGE;
        Gad->Activation = GACT_RELVERIFY|GACT_IMMEDIATE;
        Gad->GadgetType = GTYP_BOOLGADGET;
        Gad->GadgetID   = ID;

        if (Gad->GadgetRender = Border = AllocMem(sizeof(struct Border), MEMF_PUBLIC|MEMF_CLEAR)) {

            Border->FrontPen = ShinePen;
            Border->DrawMode = JAM1;
            Border->Count    = 3;

            if (Border->XY = Pairs = AllocMem(4*Border->Count, MEMF_PUBLIC)) {

                Pairs[0] = 0;     Pairs[1] = Height;
                Pairs[2] = 0;     Pairs[3] = 0;
                Pairs[4] = Width; Pairs[5] = 0;
                }

            if (Border->NextBorder = AllocMem(sizeof(struct Border), MEMF_PUBLIC|MEMF_CLEAR)) {

                Border = Border->NextBorder;

                Border->FrontPen = ShadowPen;
                Border->DrawMode = JAM1;
                Border->Count    = 3;

                if (Border->XY = Pairs = AllocMem(4*Border->Count, MEMF_PUBLIC)) {

                    Pairs[0] = Width; Pairs[1] = 0;
                    Pairs[2] = Width; Pairs[3] = Height;
                    Pairs[4] = 0;     Pairs[5] = Height;
                    }
                }
            }

        if (Gad->SelectRender = Border = AllocMem(sizeof(struct Border), MEMF_PUBLIC|MEMF_CLEAR)) {

            Border->FrontPen = ShadowPen;
            Border->DrawMode = JAM1;
            Border->Count    = 3;

            if (Border->XY = Pairs = AllocMem(4*Border->Count, MEMF_PUBLIC)) {

                Pairs[0] = 0;     Pairs[1] = Height;
                Pairs[2] = 0;     Pairs[3] = 0;
                Pairs[4] = Width; Pairs[5] = 0;
                }

            if (Border->NextBorder = AllocMem(sizeof(struct Border), MEMF_PUBLIC|MEMF_CLEAR)) {

                Border = Border->NextBorder;

                Border->FrontPen = ShinePen;
                Border->DrawMode = JAM1;
                Border->Count    = 3;

                if (Border->XY = Pairs = AllocMem(4*Border->Count, MEMF_PUBLIC)) {

                    Pairs[0] = Width; Pairs[1] = 0;
                    Pairs[2] = Width; Pairs[3] = Height;
                    Pairs[4] = 0;     Pairs[5] = Height;
                    }
                }
            }

        return (Gad);
        }

    return (NULL);
    }


void DeleteGadget(struct Gadget *Gad) {

struct Border *Border, *TempBorder;

    if (!(Gad->Flags & GFLG_GADGIMAGE)) {

        if (Border = (struct Border *)Gad->GadgetRender) {

            while (Border) {

                TempBorder = Border;
                Border     = Border->NextBorder;
            
                if (TempBorder->XY) {

                    FreeMem((char *)(TempBorder->XY), TempBorder->Count*4);
                    }

                FreeMem((char *)TempBorder, sizeof(struct Border));
                }
            }

        if (Border = (struct Border *)Gad->SelectRender) {

            while (Border) {

                TempBorder = Border;
                Border     = Border->NextBorder;
            
                if (TempBorder->XY) {

                    FreeMem((char *)(TempBorder->XY), TempBorder->Count*4);
                    }

                FreeMem((char *)TempBorder, sizeof(struct Border));
                }
            }
        }

    FreeMem((char *)Gad, sizeof(struct Gadget));
    }



void SelectIcon(struct Gadget *Gad, int Icon) {

    if (BigWindow) {

        Gad->GadgetRender = IconArray[Gad->GadgetID-BIGGADGETS][Icon];
        RefreshGList(Gad, window, NULL, 1);
        }
    }


void MakeGadgets(void) {

int             i, j, k, l;
struct Gadget **GadP;
ULONG          *s, *d;

    if ((l = STxtSize + 1) < (k = FREVTaI1.Height)) {

        j = ((k - l - 1) / 2) * 2;

        FREVTaI1.Height =
        FREVTsI1.Height =
        FREVTdI1.Height =
        PPTaI1.Height   =
        PPTsI1.Height   =
        PPTdI1.Height   =
        FFWDTaI1.Height =
        FFWDTsI1.Height =
        FFWDTdI1.Height =
        STOPTaI1.Height =
        STOPTsI1.Height =
        STOPTdI1.Height = l; 

        FREVTaI1.ImageData += j;
        FREVTsI1.ImageData += j;
        FREVTdI1.ImageData += j;
        PPTaI1.ImageData   += j;
        PPTsI1.ImageData   += j;
        PPTdI1.ImageData   += j;
        FFWDTaI1.ImageData += j;
        FFWDTsI1.ImageData += j;
        FFWDTdI1.ImageData += j;
        STOPTaI1.ImageData += j;
        STOPTsI1.ImageData += j;
        STOPTdI1.ImageData += j;

        for (i=0,s=(ULONG *)&FREVTaI1.ImageData[k*2],
                 d=(ULONG *)&FREVTaI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&FREVTsI1.ImageData[k*2],
                 d=(ULONG *)&FREVTsI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&FREVTdI1.ImageData[k*2],
                 d=(ULONG *)&FREVTdI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&PPTaI1.ImageData[k*2],
                 d=(ULONG *)&PPTaI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&PPTsI1.ImageData[k*2],
                 d=(ULONG *)&PPTsI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&PPTdI1.ImageData[k*2],
                 d=(ULONG *)&PPTdI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&FFWDTaI1.ImageData[k*2],
                 d=(ULONG *)&FFWDTaI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&FFWDTsI1.ImageData[k*2],
                 d=(ULONG *)&FFWDTsI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&FFWDTdI1.ImageData[k*2],
                 d=(ULONG *)&FFWDTdI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&STOPTaI1.ImageData[k*2],
                 d=(ULONG *)&STOPTaI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&STOPTsI1.ImageData[k*2],
                 d=(ULONG *)&STOPTsI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        for (i=0,s=(ULONG *)&STOPTdI1.ImageData[k*2],
                 d=(ULONG *)&STOPTdI1.ImageData[l*2]; i!=l; i++) *d++ = *s++;
        }

    FFWDT = FFWDTaI1;
    PPT   = PPTaI1;
    FREVT = FREVTaI1;
    STOPT = STOPTaI1;

    GadP = (struct Gadget **)&WIN_Gadgets;

    for (j=STxtSize+MATRIXTOP,k=0; j!=STxtSize+MATRIXTOP+90; j+=18) {

        for (i=MATRIXLEFT; i!=MATRIXLEFT+160; i+=40) {

           *GadP = MakeTrackGadget(i, j, 35, 15, k++);
            GadP = (struct Gadget **)&(*GadP)->NextGadget;
            }
        }

    *GadP = DragGadget = MakeTitleGadget(19, 0, WIN_Width-64, STxtSize + 3, GTYP_WDRAGGING, NULL, NULL, NULL, GID_DRAGBAR);
     GadP = (struct Gadget **)&(*GadP)->NextGadget;

    FirstControlGadget = GadP;

    *GadP = BigGadgets = Gad_FREV = MakeGadget(8,                     STxtSize+68, &FREVI1,      GID_FREV);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_PLAYPAUSE = MakeGadget(8+BIGBUTTONSIZE+BIGBUTTONGAP, STxtSize+68, &PlayPauseI1, GID_PLAYPAUSE);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_FFWD = MakeGadget(3+BIGBUTTONSIZE*3+BIGBUTTONGAP*2,   STxtSize+68, &FFWDI1,      GID_FFWD);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_STOP = MakeGadget(3+BIGBUTTONSIZE*4+BIGBUTTONGAP*3,   STxtSize+68, &StopI1,      GID_STOP);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_LOOPMODE  = MakeGadget(8,                                  STxtSize+47, &LoopI1,  GID_LOOPMODE);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_INTROMODE = MakeGadget(8+MODEBUTTONSIZE+MODEBUTTONGAP,     STxtSize+47, &IntroI1, GID_INTROMODE);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_RND       = MakeGadget(8+MODEBUTTONSIZE*2+MODEBUTTONGAP*2, STxtSize+47, &RNDI1,   GID_RND);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_TIMEMODE  = MakeGadget(8+MODEBUTTONSIZE*3+MODEBUTTONGAP*3, STxtSize+47, &TimeMode0I1, GID_TIMEMODE);


     TitleGadgets = MakeTitleGadget(19, 0, 19, STxtSize + 3, GTYP_BOOLGADGET, &FREVT, &FREVTsI1, NULL, GID_FREV);

     GadP = (struct Gadget **)&TitleGadgets->NextGadget;
    *GadP =         MakeTitleGadget(38, 0, 29, STxtSize + 3, GTYP_BOOLGADGET, &PPT,   &PPTsI1,   NULL, GID_PLAYPAUSE);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP =         MakeTitleGadget(67, 0, 19, STxtSize + 3, GTYP_BOOLGADGET, &FFWDT, &FFWDTsI1, NULL, GID_FFWD);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP =         MakeTitleGadget(86, 0, 19, STxtSize + 3, GTYP_BOOLGADGET, &STOPT, &STOPTsI1, NULL, GID_STOP);
    }


void DeleteGadgets(void) {

struct Gadget        *Gad, *TempGad;

    Gad = (struct Gadget *)WIN_Gadgets;

    *FirstControlGadget = BigGadgets;

    while (Gad) {

        TempGad = Gad;
        Gad     = Gad->NextGadget;

        DeleteGadget(TempGad);
        }

    Gad = TitleGadgets;

    while (Gad) {

        TempGad = Gad;
        Gad     = Gad->NextGadget;

        DeleteGadget(TempGad);
        }
    }


void HighlightButton(int Track, int Flag) {

    if (Track < NUMTRACKS) {

        if (Flag) SetAPen(window->RPort, ShadowPen);
        else {

            if   (PlayList->Entry[Track] & PLEF_ENABLE) SetAPen(window->RPort, FillPen);
            else                                        SetAPen(window->RPort, BackgroundPen);
            }

        Move(window->RPort, MATRIXLEFT + 2 + (Track % 4) * 40,      MATRIXTOP + STxtSize + 2 + (Track / 4) * 18);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 4) * 40 + 31, MATRIXTOP + STxtSize + 2 + (Track / 4) * 18);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 4) * 40 + 31, MATRIXTOP + STxtSize + 2 + (Track / 4) * 18 + 11);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 4) * 40,      MATRIXTOP + STxtSize + 2 + (Track / 4) * 18 + 11);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 4) * 40,      MATRIXTOP + STxtSize + 2 + (Track / 4) * 18);
        }
    }



void DisplayButton(struct Gadget *Gad, int Flag, char *DispText, int Len) {

    if (Flag && Selecting) { SetAPen(window->RPort, FillPen);       SetBPen(window->RPort, FillPen);       }
    else                   { SetAPen(window->RPort, BackgroundPen); SetBPen(window->RPort, BackgroundPen); }

    RectFill(window->RPort, Gad->LeftEdge + 1,
                            Gad->TopEdge  + 1,
                            Gad->LeftEdge + Gad->Width  - 1,
                            Gad->TopEdge  + Gad->Height - 1);
    if (DispText) {

        SetAPen(window->RPort, TextPen);
    
        Move(window->RPort, Gad->LeftEdge + 10, Gad->TopEdge + 10);
        if (DispText) Text(window->RPort, DispText, Len);
        }
    }



void DisplayTrackButtons(int NumButtons) {

struct Gadget **GadP;
int             i;

    for (GadP=(struct Gadget **)&WIN_Gadgets,i=0; i<NumButtons; GadP=(struct Gadget **)&(*GadP)->NextGadget,i++) {

        DisplayButton(*GadP,              PlayList->Entry[(*GadP)->GadgetID] & PLEF_ENABLE,
                             &TrackText[((PlayList->Entry[(*GadP)->GadgetID] & PLEF_TRACK)-1)*2], 2);
        }
    }



void EnableButton(struct Gadget *Gad, int ID, int Flag, char *DispText, int Len) {

    while (!ModifyPlayList(1)) Delay(1);

    if (Flag) PlayList->Entry[ID] |= PLEF_ENABLE;
    else      PlayList->Entry[ID] &= PLEF_TRACK;

    ModifyPlayList(0);

    if (Gad) DisplayButton(Gad, Flag, DispText, Len);
    }



void EnableTrackButtons(int NumButtons, int Flag, int DispText) {

struct Gadget **GadP;
int             i;

    for (GadP=(struct Gadget **)&WIN_Gadgets,i=0; i<NumButtons; i++) {

        if (i < NUMTRACKS) {

            if (DispText) EnableButton(*GadP, (*GadP)->GadgetID, Flag,
                                       &TrackText[((PlayList->Entry[(*GadP)->GadgetID] & PLEF_TRACK)-1)*2], 2);
            else          EnableButton(*GadP, (*GadP)->GadgetID, Flag, NULL, 0);
            }

        else {

            EnableButton(NULL, i, Flag, NULL, 0);
            }

        if (GadP) GadP=(struct Gadget **)&(*GadP)->NextGadget;
        }
    }





void InvertOption(int Option) {

struct PlayerOptions Opts;
int                  i;

    if (!GetOptions(&Opts)) {

        for (i=0; i!=4; i++) {

            if (i != Option) ((BYTE *)&Opts)[i] = -1;
            else {

                if (Option != OPT_TIME) ((BYTE *)&Opts)[i] = !((BYTE *)&Opts)[i];
                else Opts.TimeMode = NextTimeMode[Opts.TimeMode];
                }
            }

        SetOptions(&Opts);
        GetOptions(&PlayerOptions);
        }
    }


char LitSegments[4][7];

char SegmentArray[11][7] = {

    { 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 0, 1, 1, 0, 1 },
    { 1, 1, 1, 1, 0, 0, 1 },
    { 0, 1, 1, 0, 0, 1, 1 },
    { 1, 0, 1, 1, 0, 1, 1 },
    { 1, 0, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 0, 0, 0 }
    };


char DigitPos[] = { 31, 56, 91, 116 };

char SegPos[7][4] = {

    {  4,  0, 10,  2 },
    { 12,  2, 14, 14 },
    { 12, 16, 14, 28 },
    {  4, 28, 10, 30 },
    {  0, 16,  2, 28 },
    {  0,  2,  2, 14 },
    {  4, 14, 10, 16 }
    };



void DisplaySegment(int DigitNum, int Segment, int Light) {

    if      ( LitSegments[DigitNum][Segment] && !Light) { SetAPen(window->RPort, BackgroundPen); LitSegments[DigitNum][Segment] = 0; }
    else if (!LitSegments[DigitNum][Segment] &&  Light) { SetAPen(window->RPort, TextPen);       LitSegments[DigitNum][Segment] = 1; }
    else return;

    BNDRYOFF(window);

    RectFill(window->RPort, SegPos[Segment][0] + DigitPos[DigitNum],
                            SegPos[Segment][1] + STxtSize + 10,
                            SegPos[Segment][2] + DigitPos[DigitNum],
                            SegPos[Segment][3] + STxtSize + 10);
    }




void DisplayDigit(int DigitNum, int Digit) {

int i;

    for (i=0; i!=7; i++) DisplaySegment(DigitNum, i, SegmentArray[Digit][i]);
    }





void DisplayTime(void) {

    if (Selected && ((TimeMode == 1) || (TimeMode == 3))) {

        if (!MinusLit) {

            MinusLit = 1;
            SetAPen(window->RPort, TextPen);
            RectFill(window->RPort, 20, 24 + STxtSize, 26, 26 + STxtSize);
            }
        }

    else {

        if (MinusLit) {

            MinusLit = 0;
            SetAPen(window->RPort, TextPen);
            RectFill(window->RPort, 20, 24 + STxtSize, 26, 26 + STxtSize);
            }
        }


    if (AudioDisk) {

        if (!(PlayerState.Minute/10)) DisplayDigit(0, 10);
        else                          DisplayDigit(0, PlayerState.Minute/10);

        DisplayDigit(1, PlayerState.Minute%10);

        if (!ColonLit) {

            ColonLit = 1;
            SetAPen(window->RPort, TextPen);
            RectFill(window->RPort, 80, 17 + STxtSize, 82, 19 + STxtSize);
            RectFill(window->RPort, 80, 32 + STxtSize, 82, 34 + STxtSize);
            }

        DisplayDigit(2, PlayerState.Second/10);
        DisplayDigit(3, PlayerState.Second%10);

        sprintf(&TrkTimeStr[0], " %02d  %c%02d:%02d ", PlayerState.Track,
                                                       (MinusLit ? '-':' '),
                                                       PlayerState.Minute,
                                                       PlayerState.Second);
        }

    else {

        DisplayDigit(0, 10);
        DisplayDigit(1, 10);

        if (ColonLit) {

            ColonLit = 0;
            SetAPen(window->RPort, BackgroundPen);
            RectFill(window->RPort, 80, 17 + STxtSize, 82, 19 + STxtSize);
            RectFill(window->RPort, 80, 32 + STxtSize, 82, 34 + STxtSize);
            }

        DisplayDigit(2, 10);
        DisplayDigit(3, 10);

        sprintf(&TrkTimeStr[0], "                      ");
        }

    RefreshGList(DragGadget, window, NULL, 1);
    }


void FlushTime(void) {

int i, j;

    ColonLit = MinusLit = 0;

    for (i=0; i!=4; i++) for (j=0; j!=7; j++) LitSegments[i][j] = 0;
    }



enum { ALLCLR, SOMEENA, ALLENA };


int TracksSelected(void) {

int i, SomeENA = 0, AllENA = 1;

    for (i=0; i<PlayerState.Tracks; i++) {

        if (PlayList->Entry[i] & PLEF_ENABLE) SomeENA = 1;
        else                                  AllENA  = 0;
        }

    return(SomeENA + AllENA);
    }




void SendTimerRequest(void) {

    TReq->tr_node.io_Command = TR_ADDREQUEST;
    TReq->tr_time.tv_secs    = 0;
    TReq->tr_time.tv_micro   = 200000;
    SendIO(TReq);
    }



void UpdateDisplay(void) {

    if (window->Width == WIN_Width) {

        DragGadget->LeftEdge   = 19;
        DragGadget->Width      = WIN_Width - 64;
        DragGadget->GadgetText = NULL;

       *FirstControlGadget = BigGadgets;
        window->Title = NULL;

        if (Playing) {

            if (Paused) SelectIcon(Gad_PLAYPAUSE, 2);
            else        SelectIcon(Gad_PLAYPAUSE, 1);

            SelectIcon(Gad_STOP,      0);
            }

        else {

            SelectIcon(Gad_PLAYPAUSE, 0);
            SelectIcon(Gad_STOP,      1);
            }

        RefreshWindowFrame(window);

        SetAPen(window->RPort, ShadowPen);
        Move(window->RPort,    8,   STxtSize+44);
        Draw(window->RPort,    8,   STxtSize+MATRIXTOP);
        Draw(window->RPort,    148, STxtSize+MATRIXTOP);
        SetAPen(window->RPort, ShinePen);
        Draw(window->RPort,    148, STxtSize+44);
        Draw(window->RPort,    8,   STxtSize+44);

        FlushTime();
        DisplayTime();
        DisplayTrackButtons(DisplayTracks);
        if (Selected) HighlightButton(HighlightedIndex, 1);
        }

    else {

        DragGadget->LeftEdge   = 105;
        DragGadget->Width      = ZOOMWIDTH - 64 - 86;
        DragGadget->GadgetText = &IText;
        SetAPen(window->RPort, IText.BackPen);
        RectFill(window->RPort, 105, 1, ZOOMWIDTH - 48, 1 + STxtSize);
        SetAPen(window->RPort, ShinePen);
        Move(window->RPort, 105, 1);
        Draw(window->RPort, 105, 1 + STxtSize);
        SetAPen(window->RPort, ShadowPen);
        Move(window->RPort, ZOOMWIDTH - 47, 1);
        Draw(window->RPort, ZOOMWIDTH - 47, 1 + STxtSize);

       *FirstControlGadget     = TitleGadgets;
        
        RefreshGadgets((struct Gadget *)WIN_Gadgets, window, NULL);
        }
    }


struct TextAttr TopazAttr = {

    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
    };


void DoPlayer(int TOP, int LEFT) {

ULONG                 Event;
int                   i, j, k, exit_code = 0;

struct Screen        *Screen;
struct IntuiMessage  *imsg;
struct Gadget        *Gad, *LastDownGadget;
struct TextFont      *Font;

    PlayList = ObtainPlayList();

    memset(&LitSegments[0][0], 1, sizeof(LitSegments));

    Screen=LockPubScreen(NULL);
    UnlockPubScreen(NULL, Screen);

    DefaultFont = OpenFont(Screen->Font);
    DrawInfo    = GetScreenDrawInfo(Screen);

    IText.FrontPen = FillTextPen;
    IText.BackPen  = FillPen;

    STxtSize = Screen->Font->ta_YSize;

    WIN_Screen = (ULONG)Screen;
    WIN_Top    = TOP;
    WIN_Left   = LEFT;
    WIN_Height = MATRIXTOP+STxtSize+94;
    WIN_Width  = MATRIXLEFT+165;

    MakeGadgets();
    
    ZoomPos[2] = ZOOMWIDTH;
    ZoomPos[3] = STxtSize + 3;

    if (window = OpenWindowTagList(NULL, WinTags)) {

        window->UserPort = IPort;

        ModifyIDCMP(window, IDCMP_GADGETUP|IDCMP_MOUSEBUTTONS|IDCMP_GADGETDOWN|IDCMP_CLOSEWINDOW
                            |IDCMP_CHANGEWINDOW|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW);

        if (SMALL) ZipWindow(window);

        randomseed = (-vhposr & 0x7fff) ^ 0x1D6539A3;

        if (Font = OpenFont(&TopazAttr)) SetFont(window->RPort, Font);

        SetAPen(window->RPort, ShadowPen);
        Move(window->RPort,    8,   STxtSize+44);
        Draw(window->RPort,    8,   STxtSize+MATRIXTOP);
        Draw(window->RPort,    148, STxtSize+MATRIXTOP);
        SetAPen(window->RPort, ShinePen);
        Draw(window->RPort,    148, STxtSize+44);
        Draw(window->RPort,    8,   STxtSize+44);

        DisplayTime();

        GetPlayerState(&PlayerState);
        AudioDisk = PlayerState.AudioDisk == 1;

        if   (Paused  = PlayerState.PlayState == PLS_PAUSED)  SelectIcon(Gad_PLAYPAUSE, 2);
        if   (Playing = PlayerState.PlayState >= PLS_PLAYING) SelectIcon(Gad_PLAYPAUSE, 1);
        else                                                  SelectIcon(Gad_STOP,      1);

        Selecting = Selected = PlayerState.PlayState >= PLS_SELECTED;

        GetOptions(&PlayerOptions);

        if (LoopMode  = PlayerOptions.Loop)  SelectIcon(Gad_LOOPMODE,  1);
        if (IntroMode = PlayerOptions.Intro) SelectIcon(Gad_INTROMODE, 1);

        SelectIcon(Gad_TIMEMODE, TimeMode = PlayerOptions.TimeMode);

        if ((DisplayTracks = PlayerState.Tracks) > NUMTRACKS) DisplayTracks = NUMTRACKS;

        if (AudioDisk) EnableTrackButtons(PlayerState.Tracks, 1, 1);

        SendTimerRequest();

//***** Event Loop **************************************************************************************

        while (!exit_code) {

            Event = Wait((1L << TPort->mp_SigBit) | (1L << IPort->mp_SigBit));

//********* Timer Event (update player states) **********************************************************

            if (Event & (1L << TPort->mp_SigBit)) {

                GetMsg(TPort);
                SendTimerRequest();                    

                GetPlayerState(&PlayerState);
                GetOptions(&PlayerOptions);

                if ((PlayerState.AudioDisk == 1) != AudioDisk) {

                    AudioDisk = PlayerState.AudioDisk == 1;

                    if ((DisplayTracks = PlayerState.Tracks) > NUMTRACKS) DisplayTracks = NUMTRACKS;

                    if (AudioDisk) {

                        Selecting = 0;
                        EnableTrackButtons(PlayerState.Tracks, 1, 1);
                        }

                    else {

                        EnableTrackButtons(NUMTRACKS, 0, 0);
                        RND = 0;
                        }
                    }

                if ((PlayerState.PlayState == PLS_PAUSED) != Paused) {

                    if   (Paused = PlayerState.PlayState == PLS_PAUSED) SelectIcon(Gad_PLAYPAUSE, 2);
                    else                                                SelectIcon(Gad_PLAYPAUSE, 1);
                    }

                if ((PlayerState.PlayState >= PLS_PLAYING) != Playing) {

                    if (Playing = PlayerState.PlayState >= PLS_PLAYING) {

                        SelectIcon(Gad_PLAYPAUSE, 1);
                        SelectIcon(Gad_STOP,      0);
                        }

                    else {

                        SelectIcon(Gad_PLAYPAUSE, 0);
                        SelectIcon(Gad_STOP,      1);
                        HighlightButton(HighlightedIndex, 0);

                        if (TracksSelected() == ALLENA) {

                            Selecting = 0;
                            EnableTrackButtons(PlayerState.Tracks, 1, 1);
                            }
                        }
                    }

                if ((PlayerState.PlayState >= PLS_SELECTED) != Selected) {

                    if (Selected = PlayerState.PlayState >= PLS_SELECTED) {

                        Selecting = 1;
                        DisplayTrackButtons(DisplayTracks);
                        HighlightButton(HighlightedIndex = CurrentIndex = PlayerState.ListIndex, 1);
                        }

                    else if (Selecting) HighlightButton(HighlightedIndex, 0);
                    }

                if (CurrentIndex != PlayerState.ListIndex) {

                    CurrentIndex = PlayerState.ListIndex;

                    if (Selected) {

                        HighlightButton(HighlightedIndex, 0);
                        HighlightButton(HighlightedIndex = CurrentIndex, 1);
                        }
                    }

                if (LoopMode != PlayerOptions.Loop) {

                    LoopMode = PlayerOptions.Loop;
                    SelectIcon(Gad_LOOPMODE, LoopMode);
                    }

                if (IntroMode != PlayerOptions.Intro) {

                    if ((IntroMode = PlayerOptions.Intro) && !Playing) {

                        SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_PRESS);
                        SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_RELEASE);
                        }

                    SelectIcon(Gad_INTROMODE, IntroMode);
                    }

                if (TimeMode != PlayerOptions.TimeMode) {

                    SelectIcon(Gad_TIMEMODE, TimeMode = PlayerOptions.TimeMode);
                    }

                DisplayTime();
                }

//********* Button Event ********************************************************************************

            if (Event & (1L << IPort->mp_SigBit)) {

                while (imsg=(struct IntuiMessage *)GetMsg(IPort)) {

                    if (imsg->IDCMPWindow == window) {

                        if (imsg->Class & IDCMP_CLOSEWINDOW) exit_code = 1;

                        else {

                            Gad = (struct Gadget *)imsg->IAddress;

                            if (imsg->Class & IDCMP_GADGETDOWN) {

                                LastDownGadget = Gad;

                                if (Gad->GadgetID < DisplayTracks) {

                                    if (Playing) {

                                        SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                        SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                        }

                                    if (!Selecting && !Selected && (TracksSelected() == ALLENA)) {      // Deselect all if going to select

                                        EnableTrackButtons(PlayerState.Tracks, 0, 1);
                                        }

                                    Selecting = 1;

                                    EnableButton(Gad, Gad->GadgetID, !(PlayList->Entry[Gad->GadgetID] & PLEF_ENABLE),
                                                          &TrackText[((PlayList->Entry[Gad->GadgetID] & PLEF_TRACK)-1)*2], 2);

                                    if (TracksSelected() == ALLCLR) {

                                        Selecting = 0;
                                        EnableTrackButtons(PlayerState.Tracks, 1, 1);
                                        }

                                    SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                    SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                    }

                                else if (Gad->GadgetID == GID_PLAYPAUSE) SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_PRESS);
                                else if (Gad->GadgetID == GID_STOP) {

                                    if (!Selected || (TracksSelected() == ALLENA)) {

                                        SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                        SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                        Selecting = 0;
                                        EnableTrackButtons(PlayerState.Tracks, 1, 1);
                                        }

                                    SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                    }

                                else if (Gad->GadgetID == GID_FREV) {

                                    if (AudioDisk) {

                                        SelectIcon(Gad, 1);
                                        SubmitKeyStroke(PKS_REVERSE|PKSF_PRESS);
                                        }
                                    }

                                else if (Gad->GadgetID == GID_FFWD) {

                                    if (AudioDisk) {

                                        SelectIcon(Gad, 1);
                                        SubmitKeyStroke(PKS_FORWARD|PKSF_PRESS);
                                        }
                                    }

                                else if (Gad->GadgetID == GID_LOOPMODE)  InvertOption(OPT_LOOP);
                                else if (Gad->GadgetID == GID_INTROMODE) InvertOption(OPT_INTRO);
                                else if (Gad->GadgetID == GID_TIMEMODE)  InvertOption(OPT_TIME);
                                else if (Gad->GadgetID == GID_RND && AudioDisk) {

                                    RND = (RND == 0);
                                    SelectIcon(Gad_RND, RND);

                                    if (RND) {

                                        if (Playing) {

                                            SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                            SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                            }

                                        UndoPlayList = *PlayList;

                                        EnableTrackButtons(PlayerState.Tracks, 0, 1);

                                        while (!ModifyPlayList(1)) Delay(1);

                                        for (i=0; i!=PlayerState.Tracks; i++) {

                                            k = RandomNumber(PlayerState.Tracks - i) + 1;

                                            for (j=0; j!=PlayerState.Tracks; j++) {

                                                if (!(PlayList->Entry[j] & PLEF_ENABLE)) --k;

                                                if (!k) {

                                                    PlayList->Entry[j] = (i+1) | PLEF_ENABLE;
                                                    break;
                                                    }
                                                }
                                            }

                                        for (i=0; i!=PlayerState.Tracks; i++) {

                                            PlayList->Entry[i] = UndoPlayList.Entry[(PlayList->Entry[i] & PLEF_TRACK) - 1];
                                            }

                                        ModifyPlayList(0);
                                        DisplayTrackButtons(DisplayTracks);

                                        SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_PRESS);
                                        SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_RELEASE);
                                        }

                                    else {

                                        SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                        SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);

                                        while (!ModifyPlayList(1)) Delay(1);

                                        for (i=0; i!=PlayerState.Tracks; i++) {

                                            UndoPlayList.Entry[(PlayList->Entry[i] & PLEF_TRACK) - 1]
                                                = PlayList->Entry[i];
                                            }

                                        *PlayList = UndoPlayList;

                                        ModifyPlayList(0);

                                        if (TracksSelected() == ALLENA) Selecting = 0;
                                        else                            Selecting = 1;

                                        SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                        SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                        DisplayTrackButtons(DisplayTracks);
                                        }
                                    }
                                }

                            else if (imsg->Class & IDCMP_GADGETUP
                                 || ((imsg->Class & IDCMP_MOUSEBUTTONS) && (imsg->Code == SELECTUP))) {

                                if (imsg->Class & IDCMP_MOUSEBUTTONS) Gad = LastDownGadget;

                                if      (Gad->GadgetID == GID_PLAYPAUSE) SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_RELEASE);
                                else if (Gad->GadgetID == GID_STOP)      SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                else if (Gad->GadgetID == GID_FREV) {

                                    if (AudioDisk) {

                                        SelectIcon(Gad, 0);
                                        SubmitKeyStroke(PKS_REVERSE|PKSF_RELEASE);
                                        }
                                    }

                                else if (Gad->GadgetID == GID_FFWD) {

                                    if (AudioDisk) {

                                        SelectIcon(Gad, 0);
                                        SubmitKeyStroke(PKS_FORWARD|PKSF_RELEASE);
                                        }
                                    }
                                }

                            if (imsg->Class & IDCMP_CHANGEWINDOW) UpdateDisplay();

                            if (imsg->Class & IDCMP_INACTIVEWINDOW) {

                                FFWDT = FFWDTdI1;
                                PPT   = PPTdI1;
                                FREVT = FREVTdI1;
                                STOPT = STOPTdI1;
                                IText.FrontPen = TextPen;
                                IText.BackPen  = BackgroundPen;

                                UpdateDisplay();
                                }

                            if (imsg->Class & IDCMP_ACTIVEWINDOW) {

                                FFWDT = FFWDTaI1;
                                PPT   = PPTaI1;
                                FREVT = FREVTaI1;
                                STOPT = STOPTaI1;
                                IText.FrontPen = FillTextPen;
                                IText.BackPen  = FillPen;

                                UpdateDisplay();
                                }
                            }

                        ReplyMsg((struct Message *)imsg);
                        }

                    else StripIntuiMessages(IPort);
                    }
                }
            }

//***** Close Down **************************************************************************************

        SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
        SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);

        while (!ModifyPlayList(1)) Delay(1);
        ModifyPlayList(0);

        WaitPort(TPort);
        while(GetMsg(TPort));

        if (Font) CloseFont(Font);

        window->UserPort = NULL;
        StripIntuiMessages(IPort);
        CloseWindow(window);
        }

    else printf("Could not open window\n");

    DeleteGadgets();

    CloseFont(DefaultFont);
    }






int main (int argc, char **argv) {

struct RDArgs     *rdargs;
char              *opts[OPT_COUNT];
struct WBStartup  *argmsg;
struct WBArg      *wb_arg;
struct DiskObject *dobj;
int                TOP = 0, LEFT = 0;
char              *s;

    memset(opts, 0, sizeof(opts));

    if (IconBase = OpenLibrary("icon.library", 0)) {

        if (IPort = CreateMsgPort()) {

            if (DOSBase = OpenLibrary("dos.library", 0)) {

                if (IntuitionBase = OpenLibrary("intuition.library", 0)) {

                    if (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0)) {

                        if (PlayerBase = OpenLibrary("player.library", 0)) {

                            if (TPort = CreateMsgPort()) {

                                if (TReq = CreateIORequest(TPort, sizeof(struct timerequest))) {

                                    if (!OpenDevice("timer.device", 0, TReq, UNIT_MICROHZ)) {

                                        if (argc) {

                                            if (rdargs = ReadArgs(TEMPLATE, (LONG *)opts, NULL)) {

                                                if (opts[OPT_TOP])   TOP  = ZoomPos[1] = *(ULONG *)opts[OPT_TOP];
                                                if (opts[OPT_LEFT])  LEFT = ZoomPos[0] = *(ULONG *)opts[OPT_LEFT];
                                                if (opts[OPT_SMALL]) SMALL = 1;

                                                FreeArgs(rdargs);
                                                }
                                            }

                                        else {

                                            argmsg = (struct WBStartup *)argv;
                                            wb_arg = argmsg->sm_ArgList;

                                            if ((*wb_arg->wa_Name) && (dobj = GetDiskObject(wb_arg->wa_Name))) {

                                                if (s = (char *)FindToolType(dobj->do_ToolTypes, "TOP"))   TOP  = ZoomPos[1] = val(s);
                                                if (s = (char *)FindToolType(dobj->do_ToolTypes, "LEFT"))  LEFT = ZoomPos[0] = val(s);
                                                if (s = (char *)FindToolType(dobj->do_ToolTypes, "SMALL")) SMALL = 1;

                                                FreeDiskObject(dobj);
                                                }
                                            }

                                        DoPlayer(TOP, LEFT);
                                        CloseDevice(TReq);
                                        }

                                    DeleteIORequest(TReq);
                                    }

                                DeleteMsgPort(TPort);
                                }

                            CloseLibrary(PlayerBase);
                            }

                        else printf("Could not open player.library\n");

                        CloseLibrary((struct Library *)GfxBase);
                        }

                    CloseLibrary(IntuitionBase);
                    }

                CloseLibrary(DOSBase);
                }

            DeleteMsgPort(IPort);
            }

        CloseLibrary(IconBase);
        }

    exit(0);
    }


