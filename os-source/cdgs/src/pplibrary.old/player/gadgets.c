
#include <exec/types.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <exec/memory.h>

#include <hardware/blit.h>

#include <devices/timer.h>
#include <devices/input.h>
#include <devices/inputevent.h>

#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <graphics/view.h>

#include <gs:cd/cd.h>
#include <cdtv/debox.h>

#include <libraries/dos.h>

#include "/playerprefs.h"
#include "/playerprefsbase.h"

#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"
#include "/cdtvkeys.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <cdtv/debox_pragmas.h>

#include "cdgs:include/internal/playerlibrary.h"
#include "cdgs:include/internal/player_protos.h"
#include "cdgs:include/internal/player_pragmas.h"


/******************************** Defines **********************************/

#define TCOUNTMAX   59                                                      /* Max pos for track counter */

/*************************** External Functions ****************************/

extern void UpdateGadgets(void);

extern GADInsDraw(struct RastPort *, struct GadDir *, int, UWORD);
extern GADSTDBox(struct RastPort *, struct GadDir *, int, UWORD);

extern GADNumber(struct RastPort *, struct BitMap *, struct GadDir *, WORD, WORD);
extern RNormal(struct RastPort *, struct BitMap *, struct GadDir *, WORD, WORD);

/********************* External Structures/Variables ***********************/

extern void                    *SysBase;
extern struct GfxBase          *GfxBase;
extern struct DeBoxBase        *DeBoxBase;
extern struct DOSBase          *DOSBase;
extern struct PlayerPrefsBase  *PlayerPrefsBase;
extern struct PlayerBase       *PlayerBase;

extern struct CycleIntInfo      intdata;
extern struct CDTVInputInfo     input_data;
extern struct DisplayFrame     *CurrentF, *WorkF, MasterF;

extern struct PlayerState       PlayerState;
extern struct PlayList         *PlayList;
extern struct PlayList         *UndoPlayList;

extern union  CDTOC            *TOC;
extern struct QCode             qcode, copyqcode;
extern int                      qvalid;
extern struct IOStdReq         *cdio;

extern int                      NumericPadEnabled;
extern UBYTE                    KeyPressed;
extern ULONG                    currsec;
extern UWORD volatile __far     vhposr;                                     /* Custom chip register. */

/******************************* Functions *********************************/

static NOOP();
static GADTrackNum(), GADGridNumber(), RDisk(), RCounter();

void   SetNumPad(void);
void   PlayerNumPad(int);

/************************* Structures/Variables ****************************/

static LONG diskstart;
static LONG disksec;
static LONG secperpixel;

static WORD enttrack, orgtrack;
static WORD enttrackpos = -1;

/*============================= Gadget Data ===============================*/

static UWORD d_STOP[]   = {  0, 28,   0, 51 };
static UWORD d_REV[]    = { 25, 28,  25, 51 };
static UWORD d_PLAY[]   = { 50, 28,  50, 51 };
static UWORD d_FF[]     = { 75, 28,  75, 51 };
static UWORD d_PAUSE[]  = {100, 28, 100, 51 };

static UWORD d_TIME[]   = { 0,  74, 48,  74, 96, 74, 144, 74 };
static UWORD d_SHUFF[]  = { 0, 101, 48, 101 };
static UWORD d_GCLR[]   = { 0, 128, 48, 128 };
static UWORD d_REPEAT[] = { 0, 155, 48, 155, 155, 155, 203, 155 };
static UWORD d_INTRO[]  = { 0, 182, 48, 182,  96, 182, 144, 182, 192, 182 };
static UWORD d_CDG[]    = { 0, 209, 48, 209 };

/*============================= Number Data ===============================*/

struct GADNumberInfo d_GridNumberData = {

    0,0,8,9,
    4,4,10,2,
    20,0,
    0,0,0,0,
    GNIF_HALF | GNIF_BLANKZERO
    };

struct GADNumberInfo d_TimerNumberData = {

    0,9,14,19,
    0,0,18,2,
    10,0,
    0,0,0,0,
    GNIF_BLANKMAX
    };

struct GADNumberInfo d_TrackNumberData = {

    176,29,16,23,
    20,79,24,2,
    10,0,
    0,0,0,0,
    GNIF_BLANKZERO | GNIF_BLANKMAX
    };


/*======================== Gadget Highlight Data ==========================*/

struct GADSTDBox d_GridBoxInfo = {

    -4,-3,33,23,
    { 1,1, 192,97 },
    { 1,1, 226,97 }
    };


BYTE d_GADBigButtons[] = {

    INS_OFF_REDRAW,
    INS_PEN,    34,21,
    INS_MOVE,   2,1,
    INS_DRAW,   45,1,
    INS_PEN,    34,22,
    INS_DRAW,   45,25,
    INS_PEN,    34,23,
    INS_DRAW,   2,25,
    INS_PEN,    34,24,
    INS_DRAW,   2,1,
    INS_END
    };

BYTE d_GADSmallButtons[] = {

    INS_OFF_REDRAW,
    INS_PEN,    34,21,
    INS_MOVE,   3,2,
    INS_DRAW,   18,2,
    INS_PEN,    34,22,
    INS_MOVE,   19,3,
    INS_DRAW,   19,17,
    INS_PEN,    34,23,
    INS_MOVE,   18,18,
    INS_DRAW,   3,18,
    INS_PEN,    34,24,
    INS_MOVE,   2,17,
    INS_DRAW,   2,3,
    INS_END
    };

/*============================= Gadget List ===============================*/

struct GadDir PlayGADList[MAX_PLGAD + 1] = {

    { PLGAD_COUNTER, 0x00, 0,            0, TCOUNTMAX, PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     293, 214, 321, 234, NOOP,       NULL,               RCounter,      NULL               },
    { PLGAD_DISK,    0x00, 0,            0, 38,        PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     293, 214, 321, 234, NOOP,       NULL,               RDisk,         NULL               },
    { PLGAD_TTRACK,  0x00, 0,            0, 100,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     293, 214, 321, 234, NOOP,       NULL,               GADTrackNum,   &d_TrackNumberData },
    { PLGAD_STOP,    0x80, KEY_STOP,     0,   2,       PLGAD_NL,     PLGAD_NL,     PLGAD_GCLR,   PLGAD_REV,    24,  211, 49,  234, GADInsDraw, &d_GADSmallButtons, RNormal,       &d_STOP   },
    { PLGAD_REV,     0x80, KEY_BACK,     0,   2,       PLGAD_NL,     PLGAD_NL,     PLGAD_STOP,   PLGAD_PLAY,   49,  211, 74,  234, GADInsDraw, &d_GADSmallButtons, RNormal,       &d_REV    },
    { PLGAD_PLAY,    0x80, KEY_PLAY,     0,   2,       PLGAD_NL,     PLGAD_NL,     PLGAD_REV,    PLGAD_FF,     74,  211, 99,  234, GADInsDraw, &d_GADSmallButtons, RNormal,       &d_PLAY   },
    { PLGAD_FF,      0x80, KEY_FWD,      0,   2,       PLGAD_NL,     PLGAD_NL,     PLGAD_PLAY,   PLGAD_PAUSE,  99,  211, 124, 234, GADInsDraw, &d_GADSmallButtons, RNormal,       &d_FF     },
    { PLGAD_PAUSE,   0x80, KEY_PAUSE,    0,   2,       PLGAD_NL,     PLGAD_NL,     PLGAD_FF,     PLGAD_INTRO,  124, 211, 150, 234, GADInsDraw, &d_GADSmallButtons, RNormal,       &d_PAUSE  },
    { PLGAD_TIME,    0x80, KEY_TIMEMODE, 0,   4,       PLGAD_16,     PLGAD_INTRO,  PLGAD_CDG,    PLGAD_SHUFF,  169, 169, 216, 195, GADInsDraw, &d_GADBigButtons,   RNormal,       &d_TIME   },
    { PLGAD_SHUFF,   0x80, KEY_SHUFFLE,  0,   2,       PLGAD_18,     PLGAD_REPEAT, PLGAD_TIME,   PLGAD_GCLR,   231, 169, 278, 195, GADInsDraw, &d_GADBigButtons,   RNormal,       &d_SHUFF   },
    { PLGAD_GCLR,    0x80, KEY_GCLR,     0,   2,       PLGAD_20,     PLGAD_CDG,    PLGAD_SHUFF,  PLGAD_STOP,   283, 169, 330, 195, GADInsDraw, &d_GADBigButtons,   RNormal,       &d_GCLR   },
    { PLGAD_INTRO,   0x80, KEY_INTRO,    0,   5,       PLGAD_TIME,   PLGAD_1,      PLGAD_PAUSE,  PLGAD_REPEAT, 169, 208, 216, 234, GADInsDraw, &d_GADBigButtons,   RNormal,       &d_INTRO  }, 
    { PLGAD_REPEAT,  0x80, KEY_REPEAT,   0,   4,       PLGAD_SHUFF,  PLGAD_3,      PLGAD_INTRO,  PLGAD_CDG,    221, 208, 268, 234, GADInsDraw, &d_GADBigButtons,   RNormal,       &d_REPEAT },
    { PLGAD_CDG,     0x80, KEY_CDG,      0,   2,       PLGAD_GCLR,   PLGAD_5,      PLGAD_REPEAT, PLGAD_TIME,   283, 208, 330, 234, GADInsDraw, &d_GADBigButtons,   RNormal,       &d_CDG    },
    { PLGAD_1,       0x05, KEY_GBUTTON,  0, 255,       PLGAD_INTRO,  PLGAD_6,      PLGAD_CDG,    PLGAD_2,      169, 57,  193, 73,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_2,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_INTRO,  PLGAD_7,      PLGAD_1,      PLGAD_3,      203, 57,  227, 73,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_3,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_REPEAT, PLGAD_8,      PLGAD_2,      PLGAD_4,      237, 57,  261, 73,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_4,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_CDG,    PLGAD_9,      PLGAD_3,      PLGAD_5,      271, 57,  295, 73,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_5,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_CDG,    PLGAD_10,     PLGAD_4,      PLGAD_6,      305, 57,  329, 73,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_6,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_1,      PLGAD_11,     PLGAD_5,      PLGAD_7,      169, 80,  193, 96,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_7,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_2,      PLGAD_12,     PLGAD_6,      PLGAD_8,      203, 80,  227, 96,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_8,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_3,      PLGAD_13,     PLGAD_7,      PLGAD_9,      237, 80,  261, 96,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_9,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_4,      PLGAD_14,     PLGAD_8,      PLGAD_10,     271, 80,  295, 96,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_10,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_5,      PLGAD_15,     PLGAD_9,      PLGAD_11,     305, 80,  329, 96,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_11,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_6,      PLGAD_16,     PLGAD_10,     PLGAD_12,     169, 103, 193, 119, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_12,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_7,      PLGAD_17,     PLGAD_11,     PLGAD_13,     203, 103, 227, 119, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_13,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_8,      PLGAD_18,     PLGAD_12,     PLGAD_14,     237, 103, 261, 119, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_14,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_9,      PLGAD_19,     PLGAD_13,     PLGAD_15,     271, 103, 295, 119, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_15,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_10,     PLGAD_20,     PLGAD_14,     PLGAD_16,     305, 103, 329, 119, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_16,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_11,     PLGAD_TIME,   PLGAD_15,     PLGAD_17,     169, 126, 193, 142, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_17,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_12,     PLGAD_TIME,   PLGAD_16,     PLGAD_18,     203, 126, 227, 142, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_18,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_13,     PLGAD_SHUFF,  PLGAD_17,     PLGAD_19,     237, 126, 261, 142, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_19,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_14,     PLGAD_SHUFF,  PLGAD_18,     PLGAD_20,     271, 126, 295, 142, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_20,      0x0A, KEY_GBUTTON,  0, 255,       PLGAD_15,     PLGAD_GCLR,   PLGAD_19,     PLGAD_TIME,   305, 126, 329, 142, GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_MIN,     0x00, 0,            0, 101,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     50,  173, 79,  191, NOOP,       NULL,               GADNumber,     &d_TimerNumberData },
    { PLGAD_SEC,     0x00, 0,            0, 101,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     90,  173, 119, 191, NOOP,       NULL,               GADNumber,     &d_TimerNumberData },

    { PLGAD_NL,      0x00, 0,         0xff, 0xff,      0xff,         0xff,         0,            0xff,         0,   0,   0,   0,   NULL,       NULL,               NULL }
    };





/******************************* No Function *******************************/

static NOOP() {

    return(0);
    }



/*=================== Head Gadget Positioning/Clipping ====================*/

static void __regargs getcounterbase(WORD loc, WORD *x, WORD *y, WORD *dpos, WORD *dsize) {

static WORD pos[] = {

                -47,-34,-22,-11,-1,8,17,25,32,38,43,47,51,55,58,61,64,66,68,69,70,71
                };

    if (loc & 0x8000) {                                                                             /* Vertical movememt */

        *x = 20;
        if (loc >= 20)  *y = 43;
        else            *y = pos[loc & 0xfff];
        }

    else {                                                                                          /* Horizontal movement */

        loc &= 0xfff;

        *y = 71;
        *x = 20+loc;
        }

    *dpos  = 101;                                                                                   /* Default gadget dimensions */
    *dsize = 45;

    if (*y<0) {                                                                                     /* Clip the gadget if off screen */

        *dpos -= *y;
        *dsize += *y;
        *y = 0;     
        }
    }




/*************************** Head Gadget Movement **************************/

static RCounter(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

extern struct BitMap    *bm;
extern UBYTE            *bmask;
WORD                     xloc, yloc, dpos, dsize;

    getcounterbase(old, &xloc, &yloc, &dpos, &dsize);                                                                   /* erase old */

    if (old & 0x4000) {                                                                                                 /* old laser is on */

        dsize = 57;
        BltBitMap(bbm, 96, 209, rp->BitMap, 52, 128, 80, 16 ,0xc0, 0xff, NULL);
        }

    if (dsize > 0) BltBitMap(bm, xloc, yloc, rp->BitMap, xloc, yloc, 80, dsize, 0xc0, 0xff, NULL);

    getcounterbase(new, &xloc, &yloc, &dpos, &dsize);

    if (dsize > 0) {

        BltMaskBitMapRastPort(bbm, 96, dpos, rp, xloc, yloc, 96, dsize, (ABC|ABNC|ANBC), bmask);

        if (new & 0x4000) BltMaskBitMapRastPort(bbm, 97, 147, rp, xloc+12, yloc+37, 50, 34, (ABC|ABNC|ANBC), bmask);    /* new laser is on */
        }
        
    WorkF->gstate[PLGAD_TTRACK] = CurrentF->gstate[PLGAD_TTRACK] = 0;
    return(0);
    }





/*************************** Disk Gadget Movement **************************/

static RDisk(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

extern struct BitMap   *bm;
extern UBYTE           *bmask;

WORD                    xloc,dpos,dwidth;

static WORD             nums[] = { 

                            0,52,51,50,47,44,40,35,30,24,17,10,2,-7,-17,-28,
                            -43,-58,-74,-91,-112,-134,-157,-181,-206
                            };  

register int            overlen;

    BltBitMap(bm, 0, 128, rp->BitMap, 0, 128, 267, 18, 0xc0, 0xff, NULL);

    if (new) {
    
        dpos   = 96;
        dwidth = 207;

        if (new < 24) {
    
            xloc = nums[new];
    
            if (xloc < 0) {

                dpos -= xloc;
                dwidth += xloc; 
                xloc = 0;
                }
    
            BltMaskBitMapRastPort(bbm, dpos, 209, rp, xloc, 128, dwidth, 18, (ABC|ABNC|ANBC), bmask);

            overlen = xloc+dwidth - 165;
    
            if (overlen > 0) BltMaskBitMapRastPort(bbm, 192,74, rp, 165,123, overlen, 23, (ABC|ABNC|ANBC), bmask);
            }
        }

    if (MasterF.gstate[PLGAD_16]) GADNumber(rp, bbm, &MasterF.GadList[PLGAD_16], -1, MasterF.gstate[PLGAD_16]);         /* Restore destroyed grid */
    if (MasterF.gstate[PLGAD_17]) GADNumber(rp, bbm, &MasterF.GadList[PLGAD_17], -1, MasterF.gstate[PLGAD_17]);
    if (MasterF.gstate[PLGAD_18]) GADNumber(rp, bbm, &MasterF.GadList[PLGAD_18], -1, MasterF.gstate[PLGAD_18]);

    return(0);
    }




/************************** Update Digits in Head **************************/

static GADTrackNum(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

struct GadDir gad;
UWORD state;
    
    state = MasterF.gstate[PLGAD_COUNTER];

    if (state & 0x8000) return(0);

    gad.LeftEdge   = 20+(state & 0xfff);
    gad.TopEdge    = 0;
    gad.RenderData = g->RenderData;

    GADNumber(rp, bbm, &gad, old, new);

    return(g->Flags & GDFLAGS_BOXOVER);
    }




/************************** Update Grid Selection **************************/

static GADGridNumber(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

struct GadDir gad;

static BYTE gbox[] = {

    INS_PEN,    43,9,
    INS_MOVE,   2,0,
    INS_DRAW,   22,0,
    INS_DRAW,   24,2,
    INS_DRAW,   24,14,
    INS_DRAW,   22,16,
    INS_DRAW,   2,16,
    INS_DRAW,   0,14,
    INS_DRAW,   0,2,
    INS_DRAW,   2,0,
    INS_END
    };

    if ((old ^ new) & 0x4000) {

        CopyMem((char *)g, (char *)&gad, sizeof(struct GadDir));
        gad.BoxData = gbox;
        GADInsDraw(rp, &gad, (new & 0x4000 ? 1:0), new);
        }

    return(GADNumber(rp, bbm, g, old,new));
    }



/***************************************************************************/
/*********************** Miscellaneous Functions ***************************/
/***************************************************************************/


/**************** Stop Play and Display Total Time on Disk *****************/


void UpdatePlayTime() {

    SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_PRESS);
    SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_RELEASE);
    WaitTOF();
    }




/************************* Get Gadget's Keycode ****************************/

UWORD __regargs GetBoxKey(register struct DisplayFrame *mf, UWORD event) {

register UWORD ans;

    if (mf->BoxNo >= 0) {

        ans = mf->GadList[mf->BoxNo].KeyTrans + (event & IECODE_UP_PREFIX);
        return(ans);
        }

    return(0);
    }





/*================ Scroll Up/Down Functions for DoBoxMove =================*/

ScrollGridUp() {

UBYTE orgpos;

    orgpos = MasterF.D0;

    MasterF.D0 += 5;

    UpdateGadgets();

    return((MasterF.D0 == orgpos ? 0:1));
    }



ScrollGridDown() {

int ans = 1;

    if (MasterF.D0 >= 5) MasterF.D0 -= 5;
    else                 ans = 0;

    UpdateGadgets();

    return(ans);
    }




/*************************** Move Box Selection ****************************/

void DoBoxMove(UWORD event) {

register UBYTE f;
register struct GadDir *g;

    if ((KeyPressed & PKSF_STROKEDIR) == PKSF_PRESS) return;

    g = &MasterF.GadList[MasterF.BoxNo];
    f = g->Flags;

    if (MasterF.D1) {
        if (SetNum()) return;                                                                                      /* Error in setting number don't move */
        UpdatePlayTime();
        }

    if ((event == DIR_LEFT) && (MasterF.BoxNo == PLGAD_1)) if (ScrollGridDown()) {

        MasterF.BoxNo = PLGAD_5;
        SetNumPad();
        return;
        }

    if ((event == DIR_RIGHT) && (MasterF.BoxNo == PLGAD_20)) if (ScrollGridUp()) {

        MasterF.BoxNo = PLGAD_16;
        SetNumPad();
        return;
        }

    if ((event == DIR_UP) && (f & PLFLAGS_SCROLLUP)) if (ScrollGridDown()) {

        SetNumPad();
        return;
        }

    if ((event == DIR_DOWN) && (f & PLFLAGS_SCROLLDOWN)) if (ScrollGridUp()) {

        SetNumPad();
        return;
        }

    BoxMove(&MasterF, event);
    SetNumPad();
    return;
    }





/************************** Update Digits in Grid **************************/

VOID DisplayGrid() {

register WORD   pos,i;
register UBYTE *pe;
register UWORD *gs;                                                                                         /* D0 is the position */
    
    pos = (MasterF.D0/5) * 5;

    if (pos + 19 >= PlayList->EntryCount) {                                                                 /* Scroll upwards until the 20th posisition is filled with a blank */

        while((pos + 14 >= PlayList->EntryCount) && (pos > 0)) pos-=5;

        if (pos < 0)  pos = 0;
        if (pos > 80) pos = 80;                                                                             /* Limit the hundred area */
        }

    MasterF.D0 = pos;

    for(i=0,pe=&PlayList->Entry[pos],gs=&MasterF.gstate[PLGAD_1];
        (i<20)&&(pos<PlayList->EntryCount); i++,pe++,gs++,pos++) {

        if (!*pe)                   *gs = 0;
        else if (*pe & PLEF_ENABLE) *gs = *pe & PLEF_TRACK;
        else                        *gs = 0x8000 + (*pe & PLEF_TRACK);
        }

    for(; i<20; i++) *gs++ = 0;

    MasterF.D2 = 0;                                                                                         /* Set the lit number if it is playing */

    if (PlayerState.PlayState == PLS_SELECTED || PlayerState.PlayState >= PLS_PLAYING)
        if ((PlayerState.ListIndex >= MasterF.D0) && (PlayerState.ListIndex < MasterF.D0+20))
            MasterF.gstate[(MasterF.D2 = PLGAD_1 + PlayerState.ListIndex - MasterF.D0)] |= 0x4000;
    }   






/*================== Randomize Function for ShuffleGrid ===================*/

UWORD rnd(int num) {

    return(rand() % num);
    }




/**************************** Randomize Tracks *****************************/

void ShuffleGrid() {

WORD    i, j, d, r;

    UpdatePlayTime(); while (!ModifyPlayList(1));

    for(i=PLGAD_1; i<=PLGAD_20; i++) MasterF.gstate[i] = 0x0000;                                                        /* Blank grid before we display our changes */
    
    MasterF.D0 = 0;
    MasterF.gstate[PLGAD_SHUFF] ^= 1;                                                                                   /* Toggle */
    UpdateDisplay();

    if (MasterF.gstate[PLGAD_SHUFF]) {                                                                                  /* Here we need to save the list and reshuffle the grid.  If we started in shuffle mode, then this is a re-shuffle, so we don't copy to the undo buffer in that case. */

        CopyMem(PlayList, UndoPlayList, sizeof(struct PlayList));

        PlayList->EntryCount = 0;
        for (i=0; i!=100; i++) PlayList->Entry[i] = 0;
        PlayList->EntryCount = UndoPlayList->EntryCount;

        for(i=0; i<UndoPlayList->EntryCount; i++) {                                                                     /* Generate random list from 'undo' list. */

            j=0;

            do r = rnd(UndoPlayList->EntryCount); while(++j<10 && PlayList->Entry[r]);

            if (j == 10) {

                d = rnd(2);

                while (PlayList->Entry[r]) {                                                                            /* Find the r'th unused entry. */

                    if (d) {

                        if (--r < 0) r = UndoPlayList->EntryCount - 1;
                        }

                    else if (++r >= UndoPlayList->EntryCount) r = 0;
                    }
                }

            PlayList->Entry[r] = UndoPlayList->Entry[i];

            WaitTOF(); WaitTOF(); WaitTOF();

            UpdateGadgets();
            UpdateDisplay();
            }
        }

    else {

        UpdateDisplay();                                                                                                /* Restore the play list */

        PlayList->EntryCount = 0;
        for (i=0; i!=100; i++) PlayList->Entry[i] = UndoPlayList->Entry[i];
        PlayList->EntryCount = UndoPlayList->EntryCount;
        }

    ModifyPlayList(0);
    UpdateGadgets();
    UpdateDisplay(); 
    ClearEvent(&input_data);
    }





/*********************** Initialize Head Positioner ************************/

void InitTrackCounter() {

    disksec     = TOC[0].Summary.LeadOut.LSN;

    secperpixel = disksec/TCOUNTMAX;

    if (secperpixel == 0) secperpixel = 1;
    }




/************************ Head Positioning Function ************************/

void SetTrackCounter(int now) {

WORD  loc, pos, move;

    if (PlayerState.PlayState == PLS_NUMENTRY);

    else if ((PlayerState.PlayState == PLS_SELECTED
            || PlayerState.PlayMode == PLM_SKIPFWD
            || PlayerState.PlayMode == PLM_SKIPREV)
            && MasterF.gstate[PLGAD_TTRACK] != 100) {

        currsec = TOC[PlayerState.Track].Entry.Position.LSN;
        }

    else if (PlayerState.PlayState >= PLS_PLAYING) {

        if (qvalid) currsec = copyqcode.DiskPosition.LSN;
        }

    else currsec = 0;

    pos = TCOUNTMAX - (currsec/secperpixel);

    if (pos >= TCOUNTMAX) pos = TCOUNTMAX-1;
    if (pos < 0)          pos = 0;
    
    if (now) loc = pos;

    else {

        loc = MasterF.gstate[PLGAD_COUNTER] & 0xfff;
        pos -= loc;

        if (pos < 0) {

            move = -2;
            if (pos >= -2) move = pos;
            if (pos < -5)  move = -3;
            if (pos < -10) move = -4;
            }

        else {

            move = 2;
            if (pos <= 2) move = pos;
            if (pos > 5)  move = 3;
            if (pos > 10) move = 4;
            }

        loc += move;
        }

    MasterF.gstate[PLGAD_COUNTER] = loc + (PlayerState.PlayState >= PLS_PLAYING ? 0x4000:0);
    }





/******************** Set Grid Entry to Entered Number *********************/

SetNum() {

    UpdatePlayTime(); while (!ModifyPlayList(1));

    MasterF.gstate[PLGAD_SHUFF] = 0;

    MasterF.D1 = 0;

    if (enttrackpos == -1) return(0);

    else {

        if ((enttrack <= TOC->Summary.LastTrack) && (enttrack >= 0)) {

            if (enttrack) {

                PlayList->Entry[enttrackpos] = enttrack | PLEF_ENABLE;
                }

            else {

                PlayList->Entry[enttrackpos] = 0;

                if (PlayList->EntryCount == enttrackpos + 1)                                                            /* Blanked track is at the end of list - Reduce playlistlen */ 
                    while(PlayList->EntryCount-1 >= 0 && !PlayList->Entry[PlayList->EntryCount-1])
                        --PlayList->EntryCount;
                }
            }

        else {                                                                                                          /* an error */

            PlayList->Entry[enttrackpos] = 0;
            ModifyPlayList(0);
            return(1);
            }
        }

    ModifyPlayList(0);
    return(0);
    }




/********************** Enter a Digit into Position ************************/

void enternum(int no) {

int i;

    if ((MasterF.BoxNo >= PLGAD_1) && (MasterF.BoxNo <= PLGAD_20)) {       

        if (MasterF.D1) enttrack = (enttrack * 10 + no) % 100;

        else {

            enttrack = no;
            enttrackpos = MasterF.D0 + (MasterF.BoxNo - PLGAD_1);
            if (enttrackpos > 99) return;
            MasterF.D1 = 1;                                                                                             /* do something when moved */

            if (enttrackpos+1 >= PlayList->EntryCount) {

                UpdatePlayTime(); while (!ModifyPlayList(1));

                for (i=PlayList->EntryCount; i<=enttrackpos+1; i++) PlayList->Entry[i] = 0;

                PlayList->EntryCount = enttrackpos+1;
                ModifyPlayList(0);
                }

            UpdatePlayTime();
            }

        PlayList->Entry[enttrackpos] = enttrack;
        }
    }




/******************* Highlight/Unhighlight a Grid Entry ********************/

void togglegnum() {

int pos;

    pos = MasterF.D0 + (MasterF.BoxNo - PLGAD_1);

    if (pos >= PlayList->EntryCount) return;

    if (PlayList->Entry[pos] & PLEF_TRACK) {

        MasterF.gstate[PLGAD_SHUFF] = 0;
        
        if (MasterF.D1) SetNum();

        PlayList->Entry[pos] ^= PLEF_ENABLE;

        UpdatePlayTime();
        UpdateGadgets();
        }
    }




/********************** Clear/Reset Default PlayList ***********************/

void ResetPlayList(int dogad, int clear) {

register WORD           i;

    MasterF.gstate[PLGAD_SHUFF] = 0;
    MasterF.D0 = 0;                                                                                                 /* Set playlist back to zero */

    if (dogad) {

        intdata.cintd_VCountDown = 5;
        MasterF.gstate[PLGAD_GCLR] = 1; 
        UpdateDisplay(); 
        }

    UpdatePlayTime(); while (!ModifyPlayList(1));

    if (clear) PlayList->EntryCount = 0;

    else {

        for (i=0; i<TOC[0].Summary.LastTrack; i++) PlayList->Entry[i] = (UBYTE)PLEF_ENABLE | (i+1);

        PlayList->EntryCount = TOC[0].Summary.LastTrack;
        }

    ModifyPlayList(0);

    UpdateGadgets();
    
    if (dogad) {

        while(intdata.cintd_VCountDown) WaitTOF();
        MasterF.gstate[PLGAD_GCLR] = 0;
        UpdateDisplay();
        }
    }




/******************** Highlight/Unhighligh Entire Grid *********************/

void BlankPlayList(int dogad, int clear) {

register WORD   i;

    MasterF.gstate[PLGAD_SHUFF] = 0;

    if (dogad) {

        intdata.cintd_VCountDown = 5;
        MasterF.gstate[ PLGAD_GCLR ] = 1; 
        UpdateDisplay(); 
        }

    UpdatePlayTime(); while (!ModifyPlayList(1));

    if (clear) for (i=0; i<PlayList->EntryCount; i++) PlayList->Entry[i] &= (UBYTE)PLEF_TRACK;
    
    else       for (i=0; i<PlayList->EntryCount; i++) PlayList->Entry[i] |= (UBYTE)PLEF_ENABLE;
    
    ModifyPlayList(0);

    UpdateGadgets();

    if (dogad) {

        while(intdata.cintd_VCountDown) WaitTOF();
        MasterF.gstate[PLGAD_GCLR] = 0;
        UpdateDisplay();
        }
    }




/***************** Determine/Execute CLR Gadget Function *******************/

void DoGCLRGad() {

register WORD   i;

    if (PlayList->EntryCount) {

        for(i=0; i<PlayList->EntryCount && (PlayList->Entry[i] & PLEF_ENABLE); i++);

        if (i == PlayList->EntryCount) BlankPlayList(1, 1);
        else {

            for(i=0; i<PlayList->EntryCount && !(PlayList->Entry[i] & PLEF_ENABLE); i++);

            if (i == PlayList->EntryCount) ResetPlayList(1, 1);
            else                           BlankPlayList(1, 0);
            }
        }

    else ResetPlayList(1, 0);

    UpdatePlayTime();
    }





/********* Enable/Disable Numeric Keypad Based on Cursor Position **********/

void SetNumPad() {

    if (MasterF.BoxNo >= PLGAD_1 && MasterF.BoxNo <= PLGAD_20) PlayerNumPad(0);
    else                                                       PlayerNumPad(1);
    }




/************************ Configure Numeric Keypad *************************/

void PlayerNumPad(int on) {

#if 0
    cdio->io_Command = CDTV_FRONTPANEL;
    cdio->io_Data    = NULL;
    cdio->io_Offset  = 0;
    cdio->io_Length  = CDTV_PANEL_ENABLED | CDTV_PANEL_PLAY_EJECT | (on ? CDTV_PANEL_NUMERIC:0);
    DoIO(cdio);
#endif
    NumericPadEnabled = on;
    }


