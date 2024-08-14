

#include <exec/types.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <exec/memory.h>

#include <graphics/gfxbase.h>

#include <devices/input.h>
#include <devices/inputevent.h>

#include <gs:cd/cd.h>

#include <cdtv/cdtvprefs.h>
#include <cdtv/debox.h>

#include "/screensaver/screensaver.h"
#include "/basicio/keyclick.h"

#include "/cdtvkeys.h"
#include "/playerprefs.h"
#include "/playerprefsbase.h"

#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"

#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "clib/graphics_protos.h"
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <cdtv/debox_pragmas.h>

#include "cdtv/cdg_cr_pragmas.h"
#include "cdtv/cdg_cr_prefs.h"

#include "cdgs:src/game/gamebase.h"
#include "cdgs:src/game/readjoyport.h"
#include "cdgs:src/game/game_pragmas.h"
#include "cdgs:src/game/game_protos.h"

#include "cdgs:include/internal/playerlibrary.h"
#include "cdgs:include/internal/player_protos.h"
#include "cdgs:include/internal/player_pragmas.h"

#include <string.h>

/*************************** External Functions ****************************/

extern BOOL             CDGBegin();
extern VOID             CDGEnd();
extern VOID             CDGChannel(ULONG);
extern void             UpdateGadgets(void);
extern VOID __stdargs   EndCDTVScreen(VOID);
extern VOID             InitVarBase(register struct PlayerPrefsBase *ppb);
extern                  StartPPScreen(struct GadDir *gl, struct BuildBG_Info *bg, UBYTE *buttondata, UBYTE initfade);
extern UWORD           *DoAnimation();
extern void             PlayerNumPad(int);

/********************* External Structures/Variables ***********************/

extern struct GfxBase          *GfxBase;
extern struct DeBoxBase        *DeBoxBase;
extern void                    *SysBase;
extern struct PlayerPrefsBase  *PlayerPrefsBase;

extern struct CycleIntInfo      intdata;
extern UWORD volatile __far     vhposr;                     /* Custom chip register. */
extern char __far               TimerDeviceName[];

extern UBYTE __far              scrdata[];
extern UBYTE __far              playerstrip[];

/******************************* Functions *********************************/

LONG __asm DoPlayer(register __a6 struct PlayerPrefsBase *);

void  PlayerLoop(struct CDGPrefs *);
void  DoKeyStroke(UWORD);
void  UpdateGadgets(void);
void  SetupPlayer();
void  ShutDown(void);
void  DoReset(void);
void  disableclick(int);
void  click(void);
UBYTE inlist(UBYTE);
void  AbortQCode(void);
UWORD GetJoyPortEvent(void);

LONG __asm changeintr(register __a1 struct changedata *);

/************************* Structures/Variables ****************************/

struct BuildBg_Info __far PlayerBg = {

    NULL, playerstrip, NULL, scrdata,
    352, 296, 6,
    14, 54,
    NULL
    };

UBYTE  LastTrack = 0;
UWORD  IntroCount;
UBYTE  LastPlayState, LastPlayMode;

union  CDTOC         *TOC;
struct QCode          qcode, copyqcode;
int                   qvalid, qoutstanding, toutstanding;
struct MsgPort       *cdreply, *qcodereply, *TimeReplyPort;
struct IOStdReq      *cdio, *cdqcode;
struct timerequest   *TimeReq;

struct PlayerOptions  PlayerOptions, OptionsSet;
struct PlayerState    PlayerState;
struct PlayList      *PlayList;
struct PlayList      *UndoPlayList;

struct Library       *CDGBase;
struct PlayerBase    *PlayerBase;
struct GameBase      *GameBase;

int    CDGAvail = 0, CDGPlaying = 0, CDGDisplaying = 0;
int    ScreenFront = 0;
int    LastEntryCount = -1, NumericPadEnabled = 1;

short  changesigbit = -1;
BYTE   keyclick;
UBYTE  channel;
WORD   blank;
ULONG  currsec;

UBYTE  KeyPressed = PKSF_RELEASE;

UWORD  JoyPortEvent = 0, JoyPortBits = 0;
UWORD  JoyPortEvents[] = {

    KEY_PAUSE,
    KEY_BACK,
    KEY_FWD,
    KEY_PLAY,
    KEY_STOP
    };


/********************************* Tables **********************************/

UWORD IntroData[] = {

    1, PLGAD_DISK,38, PLGAD_MIN, 100, PLGAD_SEC, 100, 0xffff,
    1, PLGAD_DISK,37, 0xffff,
    1, PLGAD_DISK,36, 0xffff,
    1, PLGAD_DISK,35, 0xffff,
    1, PLGAD_DISK,34, 0xffff,
    1, PLGAD_DISK,33, 0xffff,
    1, PLGAD_DISK,32, 0xffff,
    1, PLGAD_DISK,31, 0xffff,
    1, PLGAD_DISK,30, 0xffff,
    1, PLGAD_DISK,29, 0xffff,
    1, PLGAD_DISK,28, 0xffff,
    1, PLGAD_DISK,27, 0xffff,
    1, PLGAD_DISK,26, 0xffff,
    1, PLGAD_DISK,25, 0xffff,
    1, PLGAD_DISK,24, 0xffff,
    1, PLGAD_DISK,23, PLGAD_COUNTER, 0x8000+ 0, 0xffff,
    1, PLGAD_DISK,22, PLGAD_COUNTER, 0x8000+ 1, 0xffff,
    1, PLGAD_DISK,21, PLGAD_COUNTER, 0x8000+ 2, 0xffff,
    1, PLGAD_DISK,20, PLGAD_COUNTER, 0x8000+ 3, 0xffff,
    1, PLGAD_DISK,19, PLGAD_COUNTER, 0x8000+ 4, 0xffff,
    1, PLGAD_DISK,18, PLGAD_COUNTER, 0x8000+ 5, 0xffff,
    1, PLGAD_DISK,17, PLGAD_COUNTER, 0x8000+ 6, 0xffff,
    1, PLGAD_DISK,16, PLGAD_COUNTER, 0x8000+ 7, 0xffff,
    1, PLGAD_DISK,15, PLGAD_COUNTER, 0x8000+ 8, 0xffff,
    1, PLGAD_DISK,14, PLGAD_COUNTER, 0x8000+ 9, 0xffff,
    1, PLGAD_DISK,13, PLGAD_COUNTER, 0x8000+10, 0xffff,
    1, PLGAD_DISK,12, PLGAD_COUNTER, 0x8000+11, 0xffff,
    1, PLGAD_DISK,11, PLGAD_COUNTER, 0x8000+12, 0xffff,
    1, PLGAD_DISK,10, PLGAD_COUNTER, 0x8000+13, 0xffff,
    1, PLGAD_DISK, 9, PLGAD_COUNTER, 0x8000+14, 0xffff,
    1, PLGAD_DISK, 8, PLGAD_COUNTER, 0x8000+15, 0xffff,
    1, PLGAD_DISK, 7, PLGAD_COUNTER, 0x8000+16, 0xffff,
    1, PLGAD_DISK, 6, PLGAD_COUNTER, 0x8000+17, 0xffff,
    1, PLGAD_DISK, 5, PLGAD_COUNTER, 0x8000+18, 0xffff,
    1, PLGAD_DISK, 4, PLGAD_COUNTER, 0x8000+19, 0xffff,
    1, PLGAD_DISK, 3, PLGAD_COUNTER, 0x8000+20, 0xffff,
    1, PLGAD_DISK, 2, PLGAD_COUNTER, 0x8000+21, 0xffff,
    1, PLGAD_DISK, 1, PLGAD_COUNTER, 0,         0xffff,        
    1, PLGAD_DISK, 1, 0xffff,
    0xffff
    };


UWORD ShutdownAnim[] = {

    0,PLGAD_STOP,0,PLGAD_REV,0,PLGAD_PLAY,0,PLGAD_FF,0,PLGAD_PAUSE,0,
      PLGAD_SHUFF,0,PLGAD_GCLR,0,PLGAD_CDG,0,0xffff,
    0, PLGAD_20, 0, 0xffff,
    0, PLGAD_19, 0, 0xffff,
    0, PLGAD_18, 0, 0xffff,
    0, PLGAD_17, 0, 0xffff,
    0, PLGAD_16, 0, 0xffff,
    0, PLGAD_15, 0, 0xffff,
    0, PLGAD_14, 0, 0xffff,
    0, PLGAD_13, 0, 0xffff,
    0, PLGAD_12, 0, 0xffff,
    0, PLGAD_11, 0, 0xffff,
    0, PLGAD_10, 0, 0xffff,
    0, PLGAD_9 , 0, 0xffff,
    0, PLGAD_8 , 0, 0xffff,
    0, PLGAD_7 , 0, 0xffff,
    0, PLGAD_6 , 0, 0xffff,
    0, PLGAD_5 , 0, 0xffff,
    0, PLGAD_4 , 0, 0xffff,
    0, PLGAD_3 , 0, 0xffff,
    0, PLGAD_2 , 0, 0xffff,
    0, PLGAD_1 , 0, 0xffff,
    0xffff
    };


UWORD DiskOutAnim[] = {

    1, PLGAD_DISK, 1, 0xffff,
    1, PLGAD_DISK, 2, 0xffff,
    1, PLGAD_DISK, 3, 0xffff,
    1, PLGAD_DISK, 4, 0xffff,
    1, PLGAD_DISK, 5, 0xffff,
    1, PLGAD_DISK, 6, 0xffff,
    1, PLGAD_DISK, 7, 0xffff,
    1, PLGAD_DISK, 8, 0xffff,
    1, PLGAD_DISK, 9, 0xffff,
    1, PLGAD_DISK,10, 0xffff,
    1, PLGAD_DISK,11, 0xffff,
    1, PLGAD_DISK,12, 0xffff,
    1, PLGAD_DISK,13, 0xffff,
    1, PLGAD_DISK,14, 0xffff,
    1, PLGAD_DISK,15, 0xffff,
    1, PLGAD_DISK,16, PLGAD_COUNTER,0x8000+21, 0xffff,
    1, PLGAD_DISK,17, PLGAD_COUNTER,0x8000+20, 0xffff, 
    1, PLGAD_DISK,18, PLGAD_COUNTER,0x8000+19, 0xffff,
    1, PLGAD_DISK,19, PLGAD_COUNTER,0x8000+18, 0xffff,
    1, PLGAD_DISK,20, PLGAD_COUNTER,0x8000+17, 0xffff,
    1, PLGAD_DISK,21, PLGAD_COUNTER,0x8000+16, 0xffff,
    1, PLGAD_DISK,22, PLGAD_COUNTER,0x8000+15, 0xffff,
    1, PLGAD_DISK,23, PLGAD_COUNTER,0x8000+14, 0xffff,
    1, PLGAD_DISK,24, PLGAD_COUNTER,0x8000+13, 0xffff,
    1, PLGAD_DISK,25, PLGAD_COUNTER,0x8000+12, 0xffff,
    1, PLGAD_DISK,26, PLGAD_COUNTER,0x8000+11, 0xffff,
    1, PLGAD_DISK,27, PLGAD_COUNTER,0x8000+10, 0xffff,
    1, PLGAD_DISK,28, PLGAD_COUNTER,0x8000+ 9, 0xffff,
    1, PLGAD_DISK,29, PLGAD_COUNTER,0x8000+ 8, 0xffff,
    1, PLGAD_DISK,30, PLGAD_COUNTER,0x8000+ 7, 0xffff,
    1, PLGAD_DISK,31, PLGAD_COUNTER,0x8000+ 6, 0xffff,
    1, PLGAD_DISK,32, PLGAD_COUNTER,0x8000+ 5, 0xffff,
    1, PLGAD_DISK,33, PLGAD_COUNTER,0x8000+ 4, 0xffff,
    1, PLGAD_DISK,34, PLGAD_COUNTER,0x8000+ 3, 0xffff,
    1, PLGAD_DISK,35, PLGAD_COUNTER,0x8000+ 2, 0xffff,
    1, PLGAD_DISK,36, PLGAD_COUNTER,0x8000+ 1, 0xffff,
    1, PLGAD_DISK,37, PLGAD_COUNTER,0x8000+ 0, 0xffff,
    1, PLGAD_DISK,38, 0xffff,        
    0xffff
    };


/****** playerprefs.library/DoPlayer ****************************************
*
*   NAME   
*   DoPlayer -- Start CD audio control panel.
*
*   SYNOPSIS
*   DoPlayer();
*
*   VOID DoPlayer(void);
*
*   FUNCTION
*   Starts the CD audio control panel.  This function never returns.
*
*   RESULT
*   Like the fabled DETONATE instruction, the function returning
*   indicates an error.
*
*   EXAMPLE
*   DoPlayer();
*   error_exit();
*
*   NOTES
*
*   BUGS
*   Yes.
*
*   SEE ALSO
*
*****************************************************************************
*/


LONG __asm DoPlayer(register __a6 struct PlayerPrefsBase *ppb) {

extern struct GadDir                PlayGADList[];
extern UBYTE  __far                 scrbuttons[];
extern struct BuildBg_Info __far    PlayerBg;

struct CDTVPrefs    prefs;
struct CDGPrefs     cdp;
int                 i;

    InitVarBase(ppb);                                                                                           /* Initilize the varables */

    srand((-vhposr & 0x7fff) ^ 0x1D6539A3);

    if (!StartPPScreen(PlayGADList, &PlayerBg, scrbuttons, 15)) {
    
        for (i=PLGAD_STOP; i<=PLGAD_20; i++) WorkF->gstate[i] = CurrentF->gstate[i] = -1;

        if (cdreply = (struct MsgPort *)CreatePort(0, 0)) {

            if (qcodereply = (struct MsgPort *)CreatePort(0, 0)) {

                if (cdio = (struct IOStdReq *)CreateStdIO(cdreply)) {

                    if (cdqcode = (struct IOStdReq *)CreateStdIO(qcodereply)) {

                        if (!OpenDevice("cd.device", 0, cdio, 0)) {

                            CopyMem(cdio, cdqcode, sizeof(*cdio));

                            cdio->io_Message.mn_ReplyPort   = cdreply;
                            cdqcode->io_Message.mn_ReplyPort = qcodereply;
                            qoutstanding = toutstanding = qvalid = 0;

                            FillCDTVPrefs(&prefs);

                            keyclick = (prefs.Flags & CDTVPF_KEYCLICK) != 0;

                            if (TOC = AllocMem(sizeof(union CDTOC) * 100, MEMF_PUBLIC)) {

                                if (UndoPlayList = AllocMem(sizeof(struct PlayList), MEMF_PUBLIC)) {

                                    InstallKeyClick();
                                    InstallJoyMouse();

                                    if (TimeReplyPort = CreateMsgPort()) {

                                        if (TimeReq = (struct timerequest *)
                                            CreateExtIO(TimeReplyPort, sizeof(struct timerequest))) {

                                            if (!OpenDevice("timer.device", UNIT_VBLANK, TimeReq, 0)) {

                                                if (PlayerBase = OpenLibrary("player.library", NULL)) {

                                                    if (GameBase = (struct GameBase *)OpenLibrary("game.library", 0L)) {

                                                        if (CDG_Open(&cdp)) {

                                                            if (UnpackSprites(&cdp)) {

                                                                PlayerLoop(&cdp);
                                                                CDG_Close();
                                                                }

                                                            }

                                                        CloseLibrary(GameBase);
                                                        }

                                                    CloseLibrary(PlayerBase);
                                                    }

                                                CloseDevice(TimeReq);
                                                }

                                            DeleteExtIO(TimeReq);
                                            }

                                        DeleteMsgPort(TimeReplyPort);
                                        }

                                    RemoveJoyMouse();
                                    KeyClickCommand(CLKCMD_DIE, 0, 0, 0, 0, 0);
                                    FreeMem(UndoPlayList, sizeof(struct PlayList));
                                    }

                                FreeMem(TOC, sizeof(union CDTOC) * 100);
                                }

                            AbortQCode();

                            CloseDevice(cdio);
                            }

                        DeleteStdIO(cdqcode);
                        }

                    DeleteStdIO(cdio);
                    }

                DeletePort(qcodereply);
                }

            DeletePort(cdreply);
            }

        EndCDTVScreen();
        }

    return(20);
    }




/************************ Main Player Event Loop ***************************/

void PlayerLoop(struct CDGPrefs *cdp) {

ULONG               event;

    SetupPlayer();

    CDGAvail = CDGBegin(cdp);

    while(1) {

        while(event = GetEvent(&input_data)) DoKeyStroke(event); 

        while(event = GetJoyPortEvent())     DoKeyStroke(event);

        if (!PlayerState.AudioDisk) {

            ShutDown();
            DoReset();
            }

        UpdateGadgets();

        if (!CDGPlaying) {

            if (CDGAvail && PlayerState.PlayState >= PLS_PLAYING)
                if ((CDGDraw(CDGF_GRAPHICS|CDGF_MIDI)) & CDGF_GRAPHICS) {

                CDGPlaying = channel = 1;
                CDGClearScreen();

                if (PlayerState.PlayState == PLS_PAUSED) CDGPause();
                else                                     CDGPlay(0);

                if (!MasterF.gstate[PLGAD_CDG]) {

                    CDGDisplaying = 1;

                    CDGFront();

                    if (MasterF.gstate[PLGAD_FF])       CDGFastForward();
                    else if (MasterF.gstate[PLGAD_REV]) CDGRewind();
                    }

                else CDGDisplaying = 0;
                }
            }

        else CDGDraw(CDGF_GRAPHICS|CDGF_MIDI);

        if (!CDGDisplaying) {

            UpdateDisplay();

            if (ScreenFront) {

                if (!--ScreenFront) {

                    if (intdata.cintd_View) {

                        LoadView((struct View *)intdata.cintd_View);
                        WaitTOF();
                        }

                    intdata.cintd_Bump = 1;
                    }
                }
            }

        WaitTOF();
        }
    }





/*********************** Interpret KeyStroke Event *************************/

void DoKeyStroke(UWORD event) {

register short  up;
register UWORD  dir;

    if (event & BOXMOVE) {

        click();
        DoBoxMove(event);
        return;
        }

    if (!CDGDisplaying) {

        blank = 0;

        if ((event == IECODE_LBUTTON) || (event == (IECODE_LBUTTON+IECODE_UP_PREFIX))
            || (event == RAWKEY_RETURN) || (event == (RAWKEY_RETURN | IECODE_UP_PREFIX)))
            event = GetBoxKey(&MasterF, event);

        if (!SubmitKeys(event)) switch (event) {

            case RAWKEY_ARROWUP: dir = DIR_UP;      DoBoxMove(dir);  break;
    
            case RAWKEY_ARROWDN: dir = DIR_DOWN;    DoBoxMove(dir);  break;
    
            case RAWKEY_ARROWLT: dir = DIR_LEFT;    DoBoxMove(dir);  break;
    
            case RAWKEY_ARROWRT: dir = DIR_RIGHT;   DoBoxMove(dir);  break;
    
            case KEY_GBUTTON:

                click();
                togglegnum();
                return;

            case KEY_GCLR:

                click();
                DoGCLRGad();
                return;
    
            case KEY_SHUFFLE:

                click();
                ShuffleGrid();                                                                                      /* Call zero for not quick */
                return;
    
            case KEY_TIMEMODE:                                                                                      /* Change time display mode. */

                GetOptions(&OptionsSet);
                OptionsSet.TimeMode = (OptionsSet.TimeMode + 1) & 0x03;
                OptionsSet.Loop = OptionsSet.Intro = OptionsSet.Subcode = -1;
                SetOptions(&OptionsSet);
                break;
    
            case KEY_REPEAT:                                                                                        /* A/B loop */

                GetOptions(&OptionsSet);
                OptionsSet.Loop = !OptionsSet.Loop;
                OptionsSet.TimeMode = OptionsSet.Intro = OptionsSet.Subcode = -1;
                SetOptions(&OptionsSet);
                break;

            case KEY_INTRO:                                                                                         /* Intro mode */

                GetOptions(&OptionsSet);
                OptionsSet.Intro = !OptionsSet.Intro;
                OptionsSet.Loop = OptionsSet.TimeMode = OptionsSet.Subcode = -1;
                SetOptions(&OptionsSet);

                LastTrack  = 0;
                IntroCount = -1;

                if (OptionsSet.Intro && PlayerState.PlayState == PLS_STOPPED) {

                    SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_PRESS);
                    SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_RELEASE);
                    }

                break;

            case RAWKEY_PREVTRAK:

                MasterF.gstate[PLGAD_REV] = 1;
                if (CDGPlaying) CDGClearScreen();
                return;

            case RAWKEY_NEXTTRAK:

                MasterF.gstate[PLGAD_FF] = 1;
                if (CDGPlaying) CDGClearScreen();
                return;

            case (RAWKEY_PREVTRAK | IECODE_UP_PREFIX):

                MasterF.gstate[PLGAD_REV] = 0;
                break;

            case (RAWKEY_NEXTTRAK | IECODE_UP_PREFIX):
    
                MasterF.gstate[PLGAD_FF] = 0;
                break;

            case KEY_CDG:

                if (MasterF.gstate[PLGAD_CDG] = (MasterF.gstate[PLGAD_CDG] ? 0:1)) CDGDisplaying = 0;
                break;
    
            case IECODE_RBUTTON:

                if (!MasterF.gstate[PLGAD_CDG] && CDGPlaying) {

                    CDGDisplaying = 1;
                    CDGFront();
                    if (PlayerState.PlayState == PLS_PAUSED) CDGPause();

                    if (MasterF.gstate[PLGAD_FF])       CDGFastForward();
                    else if (MasterF.gstate[PLGAD_REV]) CDGRewind();
                    }

                break;
    
            case RAWKEY_ESC:

                if (MasterF.BoxNo >= PLGAD_1 && MasterF.BoxNo <= PLGAD_20) {

                    if (PlayList->EntryCount) ResetPlayList(0, 1);
                    else                      ResetPlayList(0, 0);

                    UpdatePlayTime();
                    }

                break;

            case RAWKEY_ENTER:                                                                                      /* Numeric keypad */

                if (MasterF.BoxNo >= PLGAD_1 && MasterF.BoxNo <= PLGAD_20) DoBoxMove(DIR_RIGHT);

                break;
    
            default:

                if ((up = IsNumKey(event)) >= 0) {

                    enternum(up);
                    break;
                    }

                return;
            }
        }

    else if (!SubmitKeys(event)) switch (event) {

        case RAWKEY_PREVTRAK:

            CDGRewind();
            break;

        case RAWKEY_NEXTTRAK:

            CDGFastForward();
            break;

        case (RAWKEY_PREVTRAK | IECODE_UP_PREFIX):
        case (RAWKEY_NEXTTRAK | IECODE_UP_PREFIX):

            if (PlayerState.PlayState == PLS_PAUSED) CDGPause();
            else                                     CDGPlay(0);

            break;

        case IECODE_RBUTTON:

            CDGDisplaying = 0;
            CDGBack();

            SetTrackCounter(1);
            MasterF.gstate[PLGAD_REV] = MasterF.gstate[PLGAD_FF]  = 0;
            ScreenFront = 20;

            break;
    
        case IECODE_LBUTTON:

            CDGChannel((channel & 0x0f));
            if (++channel > 15) channel = 1;

            break;

        case (IECODE_LBUTTON | IECODE_UP_PREFIX):

            if ((KeyPressed & PKSF_STROKEDIR) == PKSF_PRESS)
                SubmitKeyStroke(KeyPressed = ((KeyPressed & PKSF_KEY) | PKSF_RELEASE));

            break;
        }

    if (!(event & IECODE_UP_PREFIX)) {

        click();
        }
    }
    



int SubmitKeys(UWORD event) {

    switch(event) {

        case KEY_PAUSE:                                                                                         /* Pause */

            if (PlayerState.PlayState > PLS_SELECTED) {

                SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_PRESS);
                SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_RELEASE);
                }

            return(1);

        case KEY_PLAY:                                                                                          /* Play */

            SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_PRESS);
            SubmitKeyStroke(KeyPressed = PKS_PLAYPAUSE|PKSF_RELEASE);
            return(1);

        case KEY_STOP:                                                                                          /*  Stop  */

            SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_PRESS);
            SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_RELEASE);
            return(1);

        case KEY_BACK:                                                                                          /* Back track */

            MasterF.gstate[PLGAD_REV]  = 1;
            SubmitKeyStroke(KeyPressed = PKS_REVERSE|PKSF_PRESS);
            return(1);

        case KEY_FWD:                                                                                           /* Next track */

            MasterF.gstate[PLGAD_FF]   = 1;
            SubmitKeyStroke(KeyPressed = PKS_FORWARD|PKSF_PRESS);
            return(1);

        case (KEY_BACK|IECODE_UP_PREFIX):

            MasterF.gstate[PLGAD_REV]  = 0;
            SubmitKeyStroke(KeyPressed = PKS_REVERSE|PKSF_RELEASE);
            AbortQCode();
            return(1);

        case (KEY_FWD|IECODE_UP_PREFIX):

            MasterF.gstate[PLGAD_FF]   = 0;
            SubmitKeyStroke(KeyPressed = PKS_FORWARD|PKSF_RELEASE);
            AbortQCode();
            return(1);

        default: return(0);
        }
    }


/************** Update Screen Gadgets Based on Current State ***************/

void UpdateGadgets() {

    GetPlayerState(&PlayerState);

    if (PlayerState.PlayState >= PLS_PLAYING) {

        if (toutstanding) {

            if (CheckIO(TimeReq)) {

                if (!qoutstanding) {

                    SendIO(cdqcode);
                    qoutstanding = 1;
                    }

                if (CheckIO(cdqcode)) {

                    WaitIO(cdqcode);
                    qoutstanding = 0;

                    if (!cdqcode->io_Error) {

                        copyqcode = qcode;
                        qvalid = 1;
                        }

                    WaitIO(TimeReq);

                    if (TimeReq->tr_node.io_Error) kprintf("Error %d\n", TimeReq->tr_node.io_Error);

                    TimeReq->tr_time.tv_secs    = 1;
                    TimeReq->tr_time.tv_micro   = 0;
                    SendIO(TimeReq);
                    }
                }
            }
        }

    if (CDGPlaying) {

        if (CDGDisplaying && LastPlayMode == PlayerState.PlayMode && LastPlayState == PlayerState.PlayState) return;

        if (LastPlayState != PlayerState.PlayState) switch (PlayerState.PlayState) {

            case PLS_STOPPED:

                if (CDGDisplaying) ScreenFront = 20;

                CDGStop();
                CDGBack();
                CDGPlaying = CDGDisplaying = 0;
                MasterF.gstate[PLGAD_REV] = MasterF.gstate[PLGAD_FF]  = 0;
                break;

            case PLS_PLAYING: CDGPlay(1); break;
            case PLS_PAUSED:  CDGPause(); break;
            }
        }

    switch (PlayerState.PlayState) {

        case PLS_STOPPED:

            AbortQCode();
            break;

        case PLS_PLAYING:
        case PLS_PAUSED:

            if (!toutstanding) {

                TimeReq->tr_time.tv_secs    = 1;
                TimeReq->tr_time.tv_micro   = 0;
                SendIO(TimeReq);
                toutstanding = 1;
                }

            break;
        }

    if (PlayerState.PlayState == PLS_STOPPED) MasterF.gstate[PLGAD_TTRACK] = 100;

    else {

        MasterF.gstate[PLGAD_TTRACK] = PlayerState.Track;

        if (PlayerState.Track != LastTrack) {

            if (PlayerState.PlayMode == PLM_SKIPREV) --IntroCount;
            else                                     IntroCount++;

            LastTrack = PlayerState.Track;
            }
        }

    if (PlayerState.AudioDisk) {

        MasterF.gstate[PLGAD_MIN]    = PlayerState.Minute;
        MasterF.gstate[PLGAD_SEC]    = PlayerState.Second;
        }

    if (PlayerState.PlayMode == PLM_NORMAL) {

        MasterF.gstate[PLGAD_STOP]  =  (PlayerState.PlayState == PLS_STOPPED)
                                    || (PlayerState.PlayState == PLS_SELECTED)
                                    || (PlayerState.PlayState == PLS_NUMENTRY);

        MasterF.gstate[PLGAD_PLAY]  =  (PlayerState.PlayState == PLS_PLAYING)
                                    || (PlayerState.PlayState == PLS_PAUSED);

        MasterF.gstate[PLGAD_PAUSE] =   PlayerState.PlayState == PLS_PAUSED;
        }

    if (NumericPadEnabled && PlayList->EntryCount == LastEntryCount+1) MasterF.D0 = 80;

    LastEntryCount = PlayList->EntryCount;

    GetOptions(&PlayerOptions);

    MasterF.gstate[PLGAD_TIME]   = PlayerOptions.TimeMode;
    MasterF.gstate[PLGAD_REPEAT] = PlayerOptions.Loop;
    MasterF.gstate[PLGAD_INTRO]  = PlayerOptions.Intro * ((0x03 & IntroCount) + 1);

    if (PlayerState.LastModify) MasterF.gstate[PLGAD_SHUFF] = 0;

    SetTrackCounter(0);

    if (PlayerState.AudioDisk) DisplayGrid();

    LastPlayMode  = PlayerState.PlayMode;
    LastPlayState = PlayerState.PlayState;
    }




/**************** Initialize Structures/Variables/Gadgets ******************/

void SetupPlayer() {

extern UWORD     IntroData[], *DoAnimation();
register UWORD  *anim;
int              i, j, k, l;

    PlayList = ObtainPlayList();

    OptionsSet.Loop     = -1;
    OptionsSet.Intro    = -1;
    OptionsSet.TimeMode = -1;
    OptionsSet.Subcode  = 1;
    SetOptions(&OptionsSet);

    MasterF.BoxNo                = PLGAD_PLAY;
    MasterF.gstate[PLGAD_STOP]   = 1;
    MasterF.gstate[PLGAD_TIME]   = 0;
    MasterF.gstate[PLGAD_REPEAT] = 0;
    MasterF.gstate[PLGAD_SHUFF]  = 0;

    cdio->io_Command = CD_ATTENUATE;                                                                                /* Turn on the speakers; full volume, right now(sorta). */
    cdio->io_Offset  = 0x7FFF;
    cdio->io_Length  = 0;
    DoIO(cdio);

    PlayerNumPad(1);

    cdqcode->io_Command  = CD_QCODELSN;                                                                             /* Initialize the qcode packet. */
    cdqcode->io_Data     = (APTR)&qcode;

    TimeReq->tr_node.io_Command = TR_ADDREQUEST;                                                                    /* Create timer request */
    TimeReq->tr_time.tv_secs    = 1;
    TimeReq->tr_time.tv_micro   = 0;

    GetPlayerState(&PlayerState);

    if (PlayerState.PlayMode == PLM_NORMAL) {

        MasterF.gstate[PLGAD_STOP]  =  (PlayerState.PlayState == PLS_STOPPED)
                                    || (PlayerState.PlayState == PLS_SELECTED);

        MasterF.gstate[PLGAD_PLAY]  =  (PlayerState.PlayState == PLS_PLAYING)
                                    || (PlayerState.PlayState == PLS_PAUSED);

        MasterF.gstate[PLGAD_PAUSE] =   PlayerState.PlayState == PLS_PAUSED;
        }

    GetOptions(&PlayerOptions);

    MasterF.gstate[PLGAD_TIME]   = PlayerOptions.TimeMode;
    MasterF.gstate[PLGAD_REPEAT] = PlayerOptions.Loop;
    MasterF.gstate[PLGAD_INTRO]  = PlayerOptions.Intro * ((0x03 & IntroCount) + 1);

    LastPlayMode  = PlayerState.PlayMode;
    LastPlayState = PlayerState.PlayState;

    j = 1; k = l = 0;

    if (PlayList->EntryCount == TOC[0].Summary.LastTrack && !PlayerState.LastModify) {

        for (i=0; i!=PlayList->EntryCount; i++) {

            if (!inlist(i+1))                             j = 0;                                                    /* all list entries must be present */
            if ((PlayList->Entry[i] & PLEF_TRACK) != i+1) k = 1;                                                    /* at least 1 entry must be out of order */
            if (PlayList->Entry[i] & PLEF_ENABLE)         l = 1;                                                    /* at least 1 entry must be enabled */
            }
        }

    if (j && k && l) {

        MasterF.gstate[PLGAD_SHUFF] = 1;

        UndoPlayList->EntryCount = PlayList->EntryCount;

        for (i=0; i!=PlayList->EntryCount; i++) UndoPlayList->Entry[i] = inlist(i+1);
        }

    UpdateDisplay();

    FadeTo(0);

    anim = IntroData; while (anim) anim = DoAnimation(anim, NULL);

    ClearEvent(&input_data);                                                                                        /* Remove any queued events. */

    disableclick(1);

    GetPlayerState(&PlayerState);
    GetOptions(&PlayerOptions);

    cdio->io_Command  = CD_TOCLSN;                                                                                  /* Gimme summary. */
    cdio->io_Offset   = 0;
    cdio->io_Length   = 100;
    cdio->io_Data     = (APTR)TOC;
    DoIO(cdio);

    if (!cdio->io_Error) InitTrackCounter();

    else {

        ShutDown();
        DoReset();
        }

    currsec = TOC[PlayerState.Track].Entry.Position.LSN;
    }


/***************************** Shutdown Routine ****************************/

void ShutDown() {

register UWORD *anim;
    
    anim = ShutdownAnim;
    MasterF.BoxNo = -1;

    while ((MasterF.gstate[PLGAD_COUNTER] != 0) || anim) {

        if (MasterF.gstate[PLGAD_COUNTER] != 0) MasterF.gstate[PLGAD_COUNTER]--;

        if (anim) anim = DoAnimation(anim);

        else UpdateDisplay();
        }

    anim = DiskOutAnim;
    while(anim = DoAnimation(anim));
    }



/****************************** Fade and Reset *****************************/

void DoReset() {

    FadeTo(15);
    ColdReboot();
    }


/*************************** Make Keyclick Sound ***************************/

void click() {

    if (keyclick && (PlayerState.PlayState < PLS_PLAYING)) {

        disableclick(0);
        KeyClickCommand(CLKCMD_CLICK, 0, 0, 0, 0, 0);
        disableclick(1);
        }
    }


/******************* Disable/Enable Automatic Keyclick *********************/

void disableclick(int disable) {

    if (keyclick) KeyClickCommand(CLKCMD_DISABLE, 0, disable, 0, 0, 0);
    }



/**************** Determine if a Track is in the PlayList ******************/

UBYTE inlist(UBYTE track) {

int i;

    for (i=0; i!=PlayList->EntryCount; i++)
        if ((PlayList->Entry[i] & PLEF_TRACK) == track) return(PlayList->Entry[i]);

    return(0);
    }



/*********************** Abort Q-Code related requests *********************/

void AbortQCode(void) {

    if (qoutstanding) {

        AbortIO(cdqcode);
        WaitIO(cdqcode);
        qoutstanding = 0;
        }

    if (toutstanding) {

        AbortIO(TimeReq);
        WaitIO(TimeReq);
        toutstanding = 0;
        }

    qvalid = 0;
    }



/************* Simulate a keypress when using Game Controller **************/

UWORD GetJoyPortEvent(void) {

ULONG TempJP0, TempJP1, Temp = 0;
UWORD i, j, k;

    TempJP0 = ReadJoyPort(0);
    TempJP1 = ReadJoyPort(1);

    if ((TempJP0 & JP_TYPE_MASK) == JP_TYPE_GAMECTLR) Temp  = TempJP0;
    if ((TempJP1 & JP_TYPE_MASK) == JP_TYPE_GAMECTLR) Temp |= TempJP1;

    Temp &= (JPF_BTN3|JPF_BTN4|JPF_BTN5|JPF_BTN6|JPF_BTN7);
    Temp >>= JPB_BTN7;

    for (i=j=0,k=Temp; i!=5; i++,k>>=1) if (k & 1) j++;

    if (j <= 1) {

        if (Temp != JoyPortBits) {

            if (JoyPortEvent) {

                i = JoyPortEvent;
                JoyPortEvent = 0;
                JoyPortBits  = Temp;

                return(i | IECODE_UP_PREFIX);
                }

            if (Temp) {

                for (i=0,k=Temp; (i!=5) && !(k&1); i++,k>>=1);

                JoyPortBits  = Temp;
                JoyPortEvent = JoyPortEvents[i];

                return(JoyPortEvent);
                }
            }
        }

    return(0);
    }
