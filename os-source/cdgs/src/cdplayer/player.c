
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

#include "player_rev.h"

#define SysBase (*((struct ExecBase **)4))

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
extern struct Image TimeMode0I1;
extern struct Image TimeMode1I1;
extern struct Image TimeMode2I1;
extern struct Image TimeMode3I1;


struct Library *DOSBase;
struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *PlayerBase;
struct Library *IconBase;

#define TEMPLATE      "TOP/N,LEFT/N"
#define OPT_TOP       0
#define OPT_LEFT      1
#define OPT_COUNT     2

#define NUMTRACKS       40
#define BIGGADGETS      NUMTRACKS
#define GID_FREV        NUMTRACKS
#define GID_PLAYPAUSE   NUMTRACKS + 1
#define GID_FFWD        NUMTRACKS + 2
#define GID_STOP        NUMTRACKS + 3
#define GID_LOOPMODE    NUMTRACKS + 4
#define GID_INTROMODE   NUMTRACKS + 5
#define GID_TIMEMODE    NUMTRACKS + 6

#define MATRIXLEFT      155
#define MATRIXTOP       17

#define BIGBUTTONSIZE   26
#define MODEBUTTONSIZE  43
#define BIGBUTTONGAP    5

enum { OPT_LOOP, OPT_INTRO, OPT_TIME };

BYTE NextTimeMode[] = { 1, 3, 0, 2 };

struct Image *IconArray[][4] = {

    { &FREVI1,      &FREV_SI1 },
    { &PlayPauseI1, &PlayPause_PlayI1, &PlayPause_PauseI1 },
    { &FFWDI1,      &FFWD_SI1 },
    { &StopI1,      &Stop_SI1 },
    { &LoopI1,      &Loop_SI1 },
    { &IntroI1,     &Intro_SI1 },
    { &TimeMode0I1, &TimeMode1I1,      &TimeMode3I1,      &TimeMode2I1 }
    };


char TrackText[] = "01020304050607080910111213141516171819202122232425262728293031323334353637383940" VERSTAG;

struct TagItem  WinTags[] = {

    {WA_Title,(ULONG)"CD Audio Player"},
    {WA_CustomScreen,NULL},
    {WA_Top,0},
    {WA_Left,0},
    {WA_Height,MATRIXTOP+105},
    {WA_Width,MATRIXLEFT+245},
    {WA_Gadgets,NULL},
    {WA_Flags,WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_SMART_REFRESH\
             |WFLG_ACTIVATE|WFLG_RMBTRAP|WFLG_NOCAREREFRESH},

    {TAG_DONE,0}
    };



struct Window        *window;
struct MsgPort       *IPort;
struct MsgPort       *TPort;
struct timerequest   *TReq;
struct Gadget        *BigGadgets, *Gad_FREV, *Gad_PLAYPAUSE, *Gad_FFWD, *Gad_STOP;
struct Gadget        *Gad_LOOPMODE, *Gad_INTROMODE, *Gad_TIMEMODE;
struct PlayList      *PlayList;
struct PlayerState    PlayerState;
struct PlayerOptions  PlayerOptions;

int                   AudioDisk = 0, ColonLit = 1, MinusLit = 0;
int                   Selecting, Selected, Playing, Paused, CurrentIndex, HighlightedIndex;
int                   LoopMode, IntroMode, TimeMode, DisplayTracks;


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

        Gad->Flags      = GFLG_GADGHNONE|GFLG_GADGIMAGE;
        Gad->Activation = GACT_RELVERIFY|GACT_IMMEDIATE;
        Gad->GadgetType = GTYP_BOOLGADGET;
        Gad->GadgetID   = ID;

        Gad->GadgetRender = Image;

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

        Gad->Flags      = GFLG_GADGHNONE;
        Gad->Activation = GACT_RELVERIFY|GACT_IMMEDIATE;
        Gad->GadgetType = GTYP_BOOLGADGET;
        Gad->GadgetID   = ID;

        if (Gad->GadgetRender = Border = AllocMem(sizeof(struct Border), MEMF_PUBLIC|MEMF_CLEAR)) {

            Border->FrontPen = 2;
            Border->DrawMode = JAM1;
            Border->Count    = 3;

            if (Border->XY = Pairs = AllocMem(4*Border->Count, MEMF_PUBLIC)) {

                Pairs[0] = 0;     Pairs[1] = Height;
                Pairs[2] = 0;     Pairs[3] = 0;
                Pairs[4] = Width; Pairs[5] = 0;
                }

            if (Border->NextBorder = AllocMem(sizeof(struct Border), MEMF_PUBLIC|MEMF_CLEAR)) {

                Border = Border->NextBorder;

                Border->FrontPen = 1;
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
        }

    FreeMem((char *)Gad, sizeof(struct Gadget));
    }



void SelectIcon(struct Gadget *Gad, int Icon) {

    Gad->GadgetRender = IconArray[Gad->GadgetID-BIGGADGETS][Icon];

    RefreshGList(Gad, window, NULL, 1);
    }



void DisplayButton(struct Gadget *Gad, int Flag, char *DispText, int Len) {

    if (Flag && Selecting) { SetAPen(window->RPort, 3); SetBPen(window->RPort, 3); }
    else                   { SetAPen(window->RPort, 0); SetBPen(window->RPort, 0); }

    RectFill(window->RPort, Gad->LeftEdge + 1,
                            Gad->TopEdge  + 1,
                            Gad->LeftEdge + Gad->Width  - 1,
                            Gad->TopEdge  + Gad->Height - 1);
    if (DispText) {

        SetAPen(window->RPort, 1);
    
        Move(window->RPort, Gad->LeftEdge + 5, Gad->TopEdge + 11);
        if (DispText) Text(window->RPort, DispText, Len);
        }        
    }



void DisplayTrackButtons(int NumButtons) {

struct Gadget **GadP;
int             i;

    for (GadP=(struct Gadget **)&WIN_Gadgets,i=0; i<NumButtons; GadP=(struct Gadget **)&(*GadP)->NextGadget,i++)

        DisplayButton(*GadP, PlayList->Entry[(*GadP)->GadgetID] & PLEF_ENABLE, &TrackText[i*2], 2);
    }



void EnableButton(struct Gadget *Gad, int Flag, char *DispText, int Len) {

    while (!ModifyPlayList(1)) Delay(1);

    if (Flag) PlayList->Entry[Gad->GadgetID] |= PLEF_ENABLE;
    else      PlayList->Entry[Gad->GadgetID] &= PLEF_TRACK;

    ModifyPlayList(0);

    DisplayButton(Gad, Flag, DispText, Len);
    }



void EnableTrackButtons(int NumButtons, int Flag, int DispText) {

struct Gadget **GadP;
int             i;

    for (GadP=(struct Gadget **)&WIN_Gadgets,i=0; i<NumButtons; GadP=(struct Gadget **)&(*GadP)->NextGadget,i++)

        if (DispText) EnableButton(*GadP, Flag, &TrackText[i*2], 2);
        else          EnableButton(*GadP, Flag, NULL, 0);
    }





void HighlightButton(int Track, int Flag) {

    if (Track <= NUMTRACKS) {

        if (Flag) SetAPen(window->RPort, 1);
        else {

            if   (PlayList->Entry[Track] & PLEF_ENABLE) SetAPen(window->RPort, 3);
            else                                        SetAPen(window->RPort, 0);
            }

        Move(window->RPort, MATRIXLEFT + 2 + (Track % 8) * 30,      MATRIXTOP + 2 + (Track / 8) * 20);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 8) * 30 + 21, MATRIXTOP + 2 + (Track / 8) * 20);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 8) * 30 + 21, MATRIXTOP + 2 + (Track / 8) * 20 + 13);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 8) * 30,      MATRIXTOP + 2 + (Track / 8) * 20 + 13);
        Draw(window->RPort, MATRIXLEFT + 2 + (Track % 8) * 30,      MATRIXTOP + 2 + (Track / 8) * 20);
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

    if      ( LitSegments[DigitNum][Segment] && !Light) { SetAPen(window->RPort, 0); LitSegments[DigitNum][Segment] = 0; }
    else if (!LitSegments[DigitNum][Segment] &&  Light) { SetAPen(window->RPort, 1); LitSegments[DigitNum][Segment] = 1; }
    else return;

    BNDRYOFF(window);

    RectFill(window->RPort, SegPos[Segment][0] + DigitPos[DigitNum],
                            SegPos[Segment][1] + 20,
                            SegPos[Segment][2] + DigitPos[DigitNum],
                            SegPos[Segment][3] + 20);
    }




void DisplayDigit(int DigitNum, int Digit) {

int i;

    for (i=0; i!=7; i++) DisplaySegment(DigitNum, i, SegmentArray[Digit][i]);
    }





void DisplayTime(void) {

    if (Selected && ((TimeMode == 1) || (TimeMode == 3))) {

        if (!MinusLit) {

            MinusLit = 1;
            SetAPen(window->RPort, 1);
            RectFill(window->RPort, 20, 34, 26, 36);
            }
        }

    else {

        if (MinusLit) {

            MinusLit = 0;
            SetAPen(window->RPort, 0);
            RectFill(window->RPort, 20, 34, 26, 36);
            }
        }


    if (AudioDisk) {

        if (!(PlayerState.Minute/10)) DisplayDigit(0, 10);
        else                          DisplayDigit(0, PlayerState.Minute/10);

        DisplayDigit(1, PlayerState.Minute%10);

        if (!ColonLit) {

            ColonLit = 1;
            SetAPen(window->RPort, 1);
            RectFill(window->RPort, 80, 27, 82, 29);
            RectFill(window->RPort, 80, 42, 82, 44);
            }

        DisplayDigit(2, PlayerState.Second/10);
        DisplayDigit(3, PlayerState.Second%10);
        }

    else {

        DisplayDigit(0, 10);
        DisplayDigit(1, 10);

        if (ColonLit) {

            ColonLit = 0;
            SetAPen(window->RPort, 0);
            RectFill(window->RPort, 80, 27, 82, 29);
            RectFill(window->RPort, 80, 42, 82, 44);
            }

        DisplayDigit(2, 10);
        DisplayDigit(3, 10);
        }
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




void DoPlayer(int TOP, int LEFT) {

ULONG               Event;
int                 exit_code = 0;
int                 i, j, k;

struct Screen        *Screen;
struct IntuiMessage  *imsg;
struct Gadget       **GadP, *Gad, *TempGad, *LastDownGadget;

    PlayList = ObtainPlayList();

    memset(&LitSegments[0][0], 1, sizeof(LitSegments));

    Screen=LockPubScreen(NULL);
    UnlockPubScreen(NULL, Screen);

    WIN_Screen = (ULONG)Screen;
    WIN_Top    = TOP;
    WIN_Left   = LEFT;

    GadP = (struct Gadget **)&WIN_Gadgets;

    for (j=MATRIXTOP,k=0; j!=MATRIXTOP+100; j+=20) {

        for (i=MATRIXLEFT; i!=MATRIXLEFT+240; i+=30) {

           *GadP = MakeTrackGadget(i, j, 25, 17, k++);
            GadP = (struct Gadget **)&(*GadP)->NextGadget;
            }
        }

    *GadP = BigGadgets = Gad_FREV = MakeGadget(8,                    88, &FREVI1,      GID_FREV);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_PLAYPAUSE = MakeGadget(8+BIGBUTTONSIZE+BIGBUTTONGAP, 88, &PlayPauseI1, GID_PLAYPAUSE);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_FFWD = MakeGadget(2+BIGBUTTONSIZE*3+BIGBUTTONGAP*2,  88, &FFWDI1,      GID_FFWD);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_STOP = MakeGadget(2+BIGBUTTONSIZE*4+BIGBUTTONGAP*3,  88, &StopI1,      GID_STOP);


     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_LOOPMODE  = MakeGadget(8,                                 57, &LoopI1,  GID_LOOPMODE);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_INTROMODE = MakeGadget(8+MODEBUTTONSIZE+BIGBUTTONGAP,     57, &IntroI1, GID_INTROMODE);

     GadP = (struct Gadget **)&(*GadP)->NextGadget;
    *GadP = Gad_TIMEMODE  = MakeGadget(8+MODEBUTTONSIZE*2+BIGBUTTONGAP*2, 57, &IntroI1, GID_TIMEMODE);

    if (window = OpenWindowTagList(NULL, WinTags)) {

        window->UserPort = IPort;

        ModifyIDCMP(window, IDCMP_GADGETUP|IDCMP_MOUSEBUTTONS|IDCMP_GADGETDOWN|IDCMP_CLOSEWINDOW);

        SetAPen(window->RPort, 2);
        Move(window->RPort, 8, 54);
        Draw(window->RPort, 8, MATRIXTOP);
        Draw(window->RPort, 147, MATRIXTOP);
        SetAPen(window->RPort, 1);
        Draw(window->RPort, 147, 54);
        Draw(window->RPort, 8, 54);

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

        if ((DisplayTracks = PlayerState.Tracks) > 40) DisplayTracks = 40;

        if (AudioDisk) EnableTrackButtons(DisplayTracks, 1, 1);

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

                    if ((DisplayTracks = PlayerState.Tracks) > 40) DisplayTracks = 40;

                    if (AudioDisk) {

                        Selecting = 0;
                        EnableTrackButtons(DisplayTracks, 1, 1);
                        }

                    else EnableTrackButtons(NUMTRACKS, 0, 0);
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
                        }
                    }

                if ((PlayerState.PlayState >= PLS_SELECTED) != Selected) {

                    if (Selected = PlayerState.PlayState >= PLS_SELECTED) {

                        Selecting = 1;
                        DisplayTrackButtons(DisplayTracks);
                        HighlightButton(HighlightedIndex = CurrentIndex = PlayerState.ListIndex, 1);
                        }

                    else if (Selecting) {

                        HighlightButton(HighlightedIndex, 0);
                        Selecting = 0;
                        }
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

                if (imsg=(struct IntuiMessage *)GetMsg(IPort)) {

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

                                    if (!Selecting && !Selected && (TracksSelected() == ALLENA)) {

                                        Selecting = 1;
                                        EnableTrackButtons(DisplayTracks, 0, 1);
                                        }

                                    else Selecting = 1;

                                    EnableButton(Gad, !(PlayList->Entry[Gad->GadgetID] & PLEF_ENABLE),
                                                      &TrackText[Gad->GadgetID*2], 2);

                                    if (Selected && TracksSelected() == ALLENA) EnableTrackButtons(DisplayTracks, 0, 1);

                                    if (TracksSelected() == ALLCLR) {

                                        Selecting = 0;
                                        EnableTrackButtons(DisplayTracks, 1, 1);
                                        }

                                    SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                    SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                    }

                                else if (Gad->GadgetID == GID_PLAYPAUSE) SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_PRESS);
                                else if (Gad->GadgetID == GID_STOP) {

                                        if (Playing || (TracksSelected() == ALLENA)) {

                                            SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
                                            SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
                                            }

                                        if (!Selected || (TracksSelected() == ALLENA)) {

                                            Selecting = 0;
                                            EnableTrackButtons(DisplayTracks, 1, 1);
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

        Delay(5);

        while (!ModifyPlayList(1)) Delay(1);
        ModifyPlayList(0);

        WaitPort(TPort);
        while(GetMsg(TPort));

        window->UserPort = NULL;
        StripIntuiMessages(IPort);
        CloseWindow(window);
        }

    else printf("Could not open window\n");

    Gad = (struct Gadget *)WIN_Gadgets;

    while (Gad) {

        TempGad = Gad;
        Gad     = Gad->NextGadget;

        DeleteGadget(TempGad);
        }
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

                    if (GfxBase = OpenLibrary("graphics.library", 0)) {

                        if (PlayerBase = OpenLibrary("player.library", 0)) {

                            if (TPort = CreateMsgPort()) {

                                if (TReq = CreateIORequest(TPort, sizeof(struct timerequest))) {

                                    if (!OpenDevice("timer.device", 0, TReq, UNIT_MICROHZ)) {

                                        if (argc) {

                                            if (rdargs = ReadArgs(TEMPLATE, (LONG *)opts, NULL)) {

                                                if (opts[OPT_TOP])  TOP  = *(ULONG *)opts[OPT_TOP];
                                                if (opts[OPT_LEFT]) LEFT = *(ULONG *)opts[OPT_LEFT];

                                                FreeArgs(rdargs);
                                                }
                                            }

                                        else {

                                            argmsg = (struct WBStartup *)argv;
                                            wb_arg = argmsg->sm_ArgList;

                                            if ((*wb_arg->wa_Name) && (dobj = GetDiskObject(wb_arg->wa_Name))) {

                                                if (s = (char *)FindToolType(dobj->do_ToolTypes, "TOP"))  TOP  = val(s);
                                                if (s = (char *)FindToolType(dobj->do_ToolTypes, "LEFT")) LEFT = val(s);

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

                        CloseLibrary(GfxBase);
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


