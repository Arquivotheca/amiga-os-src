/* $Id: gadgets.c,v 1.18 93/04/07 16:57:31 peter Exp $ */

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

#include <cd/cd.h>
#include <libraries/debox.h>

#include <libraries/dos.h>

/* ZZZ: for ScreenBuffer structure */
#include <intuition/screens.h>

#include "/screencoords.h"
#include "/playerprefs.h"
#include "/display.h"
#include "/playerprefsbase.h"

#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/debox_pragmas.h>

#include <internal/playerlibrary.h>
#include <clib/player_protos.h>
#include <pragmas/player_pragmas.h>


/******************************** Defines **********************************/

/* Max pos for track counter */
#define TCOUNTMAX   59

/*************************** External Functions ****************************/

extern void UpdateGadgets(void);
extern int UpdateDisplay( void );

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

extern struct CDTVInputInfo     input_data;
extern struct IBuffer          *CurrentIB;
extern struct IBuffer          *WorkIB;
extern struct IBuffer          *MasterIB;

extern struct PlayerState       PlayerState;
extern struct PlayList         *PlayList;
extern struct PlayList         *UndoPlayList;
extern WORD noscrolltracks;

extern union  CDTOC            *TOC;
extern struct QCode             qcode, copyqcode;
extern int                      qvalid;
extern struct IOStdReq         *cdio;

extern UBYTE                    KeyPressed;
extern ULONG                    currsec;

extern WORD XOFF;
extern WORD YOFF;
extern ULONG NumSelectedTracks;

/******************************* Functions *********************************/

int NOOP(void);

static GADTrackNum(), GADGridNumber(), RDisk(), RGoCDG(), RCounter();
static UWORD randomNumber( ULONG range );

void   PlayerNumPad(int);

/************************* Structures/Variables ****************************/

static LONG secperpixel = 1;
ULONG randomseed;

/*============================= Gadget Data ===============================*/

/* These arrays are used to render the states of each gadget.
 * Each array is a list of coordinate pairs, one pair per state,
 * of the Left/TopEdge in the scrbuttons.pic picture of that
 * state's imagery.  The Width/Height are inferred from the GadDir.
 */
static UWORD d_STOP[]   = { BTN_STOP_OFF, BTN_STOP_ON };
static UWORD d_REV[]    = { BTN_REV_OFF, BTN_REV_ON };
static UWORD d_PLAY[]   = { BTN_PLAY_OFF, BTN_PLAY_PAUSE, BTN_PLAY_PLAY };
static UWORD d_FF[]     = { BTN_FF_OFF, BTN_FF_ON };

static UWORD d_TIME[]   = { BTN_TIME_TRACK, BTN_TIME_TRACKTOGO, BTN_TIME_DISC, BTN_TIME_DISCTOGO };
static UWORD d_NEG[]	= { BTN_NEG_OFF, BTN_NEG_ON };
static UWORD d_SHUFF[]  = { BTN_SHUFF_OFF, BTN_SHUFF_ON };
static UWORD d_REPEAT[] = { BTN_REPEAT_OFF, BTN_REPEAT_ON };
static UWORD d_INTRO[]  = { BTN_INTRO_OFF, BTN_INTRO_1, BTN_INTRO_2, BTN_INTRO_3, BTN_INTRO_4, BTN_INTRO_ON };
static UWORD d_CDG[]    = { BTN_CDG_OFF, BTN_CDG_GFX, BTN_CDG_MIDI, BTN_CDG_BOTH };
static UWORD d_Colon[]  = { BTN_COLON_OFF, BTN_COLON_ON };

/*============================= Number Data ===============================*/

struct GADNumberInfo d_GridNumberData =
{
    NUM_GRID_XY, NUM_GRID_DIMS,
    4,4,10,2,
    20,0,
    0,0,0,0,
    GNIF_HALF | GNIF_BLANKZERO
};

struct GADNumberInfo d_MinNumberData =
{
    NUM_TIME_XY, NUM_TIME_DIMS,
    0,0,18,2,
    10,0,
    0,0,0,0,
    GNIF_BLANKMAX|GNIF_BLANKLEAD,
};

struct GADNumberInfo d_SecNumberData =
{
    NUM_TIME_XY, NUM_TIME_DIMS,
    0,0,18,2,
    10,0,
    0,0,0,0,
    GNIF_BLANKMAX
};

struct GADNumberInfo d_TrackNumberData =
{
    NUM_TRACK_XY, NUM_TRACK_DIMS,
    26,25,24,2,
    10,0,
    0,0,0,0,
    GNIF_BLANKZERO | GNIF_BLANKMAX
};


/*======================== Gadget Highlight Data ==========================*/

/* Used by GADStdBox() to do the highlighting of the entries in the
 * track table.
 */
struct GADSTDBox d_GridBoxInfo =
{
    -4,-3,33,23,		/* xoffset,yoffset/width/height */
    { 1,1, 192,97 },		/* offi: dispbm=ButtonBM, mask=bmask, coords */
    { 1,1, 226,97 }		/* oni:  dispbm=ButtonBM, mask=ButtonBM, coords */
};


/* These are data tables indicating how to draw the box-highlight
 * for different sized boxes, used by GADInsDraw().
 * d_GADBigButtons[] is for the Play/Pause and regular buttons like Intro, Repeat, etc.
 * d_GADMedButtons[] is for the CDG and Time button.
 * d_GADSmallButtons[] is for the other transport controller buttons.
 */
BYTE d_GADBigButtons[] =
{
    INS_OFF_REDRAW,

    INS_PEN,    34,21,
    INS_MOVE,   3,1,
    INS_DRAW,   44,1,

    INS_PEN,    34,22,
    INS_MOVE,   45,2,
    INS_DRAW,   45,24,

    INS_PEN,    34,23,
    INS_MOVE,   44,25,
    INS_DRAW,   3,25,

    INS_PEN,    34,24,
    INS_MOVE,   2,24,
    INS_DRAW,   2,2,

    INS_END
};

BYTE d_GADSmallButtons[] =
{
    INS_OFF_REDRAW,

    INS_PEN,    34,21,
    INS_MOVE,   3,1,
    INS_DRAW,   26,1,

    INS_PEN,    34,22,
    INS_MOVE,   27,2,
    INS_DRAW,   27,24,

    INS_PEN,    34,23,
    INS_MOVE,   26,25,
    INS_DRAW,   3,25,

    INS_PEN,    34,24,
    INS_MOVE,   2,24,
    INS_DRAW,   2,2,

    INS_END
};

BYTE d_GADMedButtons[] =
{
    INS_OFF_REDRAW,

    INS_PEN,    34,21,
    INS_MOVE,   4,2,
    INS_DRAW,   44,2,

    INS_PEN,    34,22,
    INS_MOVE,   45,3,
    INS_DRAW,   45,20,

    INS_PEN,    34,23,
    INS_MOVE,   44,21,
    INS_DRAW,   4,21,

    INS_PEN,    34,24,
    INS_MOVE,   3,20,
    INS_DRAW,   3,3,

    INS_END
};

/*============================= Gadget List ===============================*/

/* ZZZ: Left/Top/Right/Bottom of PLGAD_COUNTER, DISK, TTRACK are unused */
/* NB: Order of gadgets IS significant.  The laser from PLGAD_COUNTER can
 * overwrite the disk from PLGAD_DISK, thus PLGAD_DISK must follow
 * PLGAD_COUNTER.  Also, PLGAD_TTRACK intentionally writes over PLGAD_COUNTER,
 * so it too must follow.
 *
 * NB: Legal states run from 0..MaxStates-1.
 */

struct GadDir PlayGADList[MAX_PLGAD + 1] =
{
/*  { ID,            Flags,KeyTrans,routine,MaxStates, Up,           Down,         Left,         Right,        Lft, Top, Rt,  Btm, BoxFunc,    BoxData,            RenderFunc,    RenderData }, */

    { PLGAD_DISK,    0x00, 0,            0, 25,        PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     279, 160, 307, 180, NOOP,       NULL,               RDisk,         NULL               },
    { PLGAD_COUNTER, 0x00, 0,            0, TCOUNTMAX, PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     279, 160, 307, 180, NOOP,       NULL,               RCounter,      NULL               },
    { PLGAD_TTRACK,  0x00, 0,            0, 100,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     279, 160, 307, 180, NOOP,       NULL,               GADTrackNum,   &d_TrackNumberData },

    { PLGAD_REV,     0x80, KEY_BACK,     0,   2,       PLGAD_16,     PLGAD_INTRO,  PLGAD_STOP,   PLGAD_PLAY,   CELL_REV,           GADInsDraw, &d_GADSmallButtons, RNormal,       &d_REV    },
    { PLGAD_PLAY,    0x80, KEY_PLAYPAUSE,0,   3,       PLGAD_17,     PLGAD_SHUFF,  PLGAD_REV,    PLGAD_FF,     CELL_PLAY,          GADInsDraw, &d_GADBigButtons,   RNormal,       &d_PLAY   },
    { PLGAD_FF,      0x80, KEY_FWD,      0,   2,       PLGAD_19,     PLGAD_SHUFF,  PLGAD_PLAY,   PLGAD_STOP,   CELL_FF,            GADInsDraw, &d_GADSmallButtons, RNormal,       &d_FF     },
    { PLGAD_STOP,    0x80, KEY_STOP,     0,   2,       PLGAD_20,     PLGAD_REPEAT, PLGAD_FF,     PLGAD_REV,    CELL_STOP,          GADInsDraw, &d_GADSmallButtons, RNormal,       &d_STOP   },

    { PLGAD_GOCDG,   0xC0, KEY_GOCDG, 0,GOCDGSTATES+1, PLGAD_REV,    PLGAD_1,      PLGAD_REPEAT, PLGAD_TIME,   CELL_GOCDG,         GADInsDraw, &d_GADMedButtons,   RGoCDG,        NULL      },
    { PLGAD_TIME,    0x80, KEY_TIMEMODE, 0,   4,       PLGAD_REV,    PLGAD_1,      PLGAD_GOCDG,  PLGAD_INTRO,  CELL_TIME,          GADInsDraw, &d_GADMedButtons,   RNormal,       &d_TIME   },
    { PLGAD_INTRO,   0x80, KEY_INTRO,    0,   6,       PLGAD_REV,    PLGAD_1,      PLGAD_TIME,   PLGAD_SHUFF,  CELL_INTRO,         GADInsDraw, &d_GADBigButtons,   RNormal,       &d_INTRO  },
    { PLGAD_SHUFF,   0x80, KEY_SHUFFLE,  0,   2,       PLGAD_PLAY,   PLGAD_3,      PLGAD_INTRO,  PLGAD_REPEAT, CELL_SHUFF,         GADInsDraw, &d_GADBigButtons,   RNormal,       &d_SHUFF  },
    { PLGAD_REPEAT,  0x80, KEY_REPEAT,   0,   2,       PLGAD_STOP,   PLGAD_5,      PLGAD_SHUFF,  PLGAD_GOCDG,  CELL_REPEAT,        GADInsDraw, &d_GADBigButtons,   RNormal,       &d_REPEAT },

    { PLGAD_1,       0x05, KEY_GBUTTON,  0, 255,       PLGAD_INTRO,  PLGAD_6,      PLGAD_NL,     PLGAD_2,      155, 3,   179, 19,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_2,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_INTRO,  PLGAD_7,      PLGAD_1,      PLGAD_3,      189, 3,   213, 19,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_3,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_SHUFF,  PLGAD_8,      PLGAD_2,      PLGAD_4,      223, 3,   247, 19,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_4,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_REPEAT, PLGAD_9,      PLGAD_3,      PLGAD_5,      257, 3,   281, 19,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_5,       0x01, KEY_GBUTTON,  0, 255,       PLGAD_REPEAT, PLGAD_10,     PLGAD_4,      PLGAD_6,      291, 3,   315, 19,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_6,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_1,      PLGAD_11,     PLGAD_5,      PLGAD_7,      155, 26,  179, 42,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_7,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_2,      PLGAD_12,     PLGAD_6,      PLGAD_8,      189, 26,  213, 42,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_8,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_3,      PLGAD_13,     PLGAD_7,      PLGAD_9,      223, 26,  247, 42,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_9,       0x00, KEY_GBUTTON,  0, 255,       PLGAD_4,      PLGAD_14,     PLGAD_8,      PLGAD_10,     257, 26,  281, 42,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_10,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_5,      PLGAD_15,     PLGAD_9,      PLGAD_11,     291, 26,  315, 42,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_11,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_6,      PLGAD_16,     PLGAD_10,     PLGAD_12,     155, 49,  179, 65,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_12,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_7,      PLGAD_17,     PLGAD_11,     PLGAD_13,     189, 49,  213, 65,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_13,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_8,      PLGAD_18,     PLGAD_12,     PLGAD_14,     223, 49,  247, 65,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_14,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_9,      PLGAD_19,     PLGAD_13,     PLGAD_15,     257, 49,  281, 65,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_15,      0x00, KEY_GBUTTON,  0, 255,       PLGAD_10,     PLGAD_20,     PLGAD_14,     PLGAD_16,     291, 49,  315, 65,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_16,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_11,     PLGAD_REV,    PLGAD_15,     PLGAD_17,     155, 72,  179, 88,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_17,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_12,     PLGAD_PLAY,   PLGAD_16,     PLGAD_18,     189, 72,  213, 88,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_18,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_13,     PLGAD_PLAY,   PLGAD_17,     PLGAD_19,     223, 72,  247, 88,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_19,      0x02, KEY_GBUTTON,  0, 255,       PLGAD_14,     PLGAD_FF,     PLGAD_18,     PLGAD_20,     257, 72,  281, 88,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },
    { PLGAD_20,      0x0A, KEY_GBUTTON,  0, 255,       PLGAD_15,     PLGAD_STOP,   PLGAD_19,     PLGAD_NL,     291, 72,  315, 88,  GADSTDBox,  &d_GridBoxInfo,     GADGridNumber, &d_GridNumberData  },

    { PLGAD_MIN,     0x00, 0,            0, 101,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     CELL_MIN,           NOOP,       NULL,               GADNumber,     &d_MinNumberData },
    { PLGAD_SEC,     0x00, 0,            0, 101,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     CELL_SEC,           NOOP,       NULL,               GADNumber,     &d_SecNumberData },
    { PLGAD_NEG,     0x00, 0,            0,   2,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     CELL_NEG,           NOOP,       NULL,               RNormal,       &d_NEG },
    { PLGAD_CDG,     0x80, 0,            0,   4,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     CELL_CDG,           NOOP,       NULL,               RNormal,       &d_CDG },
    { PLGAD_COLON,   0x80, 0,            0,   2,       PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     PLGAD_NL,     CELL_COLON,         NOOP,       NULL,               RNormal,       &d_Colon },

    { PLGAD_NL,      0x00, 0,         0xff, 0xff,      0xff,         0xff,         0,            0xff,         0,   0,   0,   0,   NULL,       NULL,               NULL }
};


/******************************* No Function *******************************/

int
NOOP( void )
{
    return(0);
}



/*=================== Head Gadget Positioning/Clipping ====================*/

/* Based on state, returns x, y, height, and source-y in button bitmap.
 * The state bit of COUNTER_LASERON means the laser is on, and is ignored
 * by this routine.  The state bit of COUNTER_FLYING means that the head-gadget
 * entry/exit motion is in progress, and the y-value comes from
 * the embedded table, while x=20.
 * If COUNTER_FLYING is not set, the state value reflects the desired head
 * position left/right.
 */
static void
getcounterbase(WORD loc, WORD *x, WORD *y, WORD *dpos, WORD *dsize)
{
static WORD pos[] =
{
    -101, -88, -76, -65, -55,
     -46, -37, -29, -22, -16,
     -11,  -7,  -3,   1,   4,
       7,  10,  12,  14,  15,
      16,  17,
};

    if ( loc & COUNTER_FLYING )
    {
	/* Vertical movememt */
	*x = 6+XOFF;
	if (loc >= 20)  *y = -11+YOFF;
	else            *y = pos[loc & COUNTER_TRACKNUM] + YOFF;
    }

    else
    {
	/* Horizontal movement */
	loc &= COUNTER_TRACKNUM;

	*y = 17+YOFF;
	*x = 6+XOFF+loc;
    }

    /* Default gadget dimensions */
    *dpos  = 101;
    *dsize = 45;

    /* Clip the gadget if off screen */
    if (*y<0)
    {
	*dpos -= *y;
	*dsize += *y;
	*y = 0;
    }
}




/*************************** Head Gadget Movement **************************/

static
RCounter(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new)
{
extern UBYTE            *bmask;
WORD                     xloc, yloc, dpos, dsize;

    /* erase old */
    getcounterbase(old, &xloc, &yloc, &dpos, &dsize);

    if ( old & COUNTER_LASERON )
    {
	/* COUNTER_LASERON means laser is on in old state.  Since the laser
	 * overwrites the CD, we will have to blit the CD back into place to
	 * erase that part of the laser.  We also blit part of the background
	 * near the CD's left end, since the laser could have been there
	 * too.  We pump up dsize so that when we erase the head gadget,
	 * we obliterate the part of the laser that's in the air between
	 * the head and the CD.
	 */
	dsize = 67;
    }

    if (dsize > 0)
    {
	/* This erases the head gadget and any part of the laser which is
	 * in the air below the head
	 */
	SetAPen( rp, 0 );
	RectFill( rp, xloc, yloc, xloc + 79, yloc + dsize - 1 );
    }
    if ( old & COUNTER_LASERON )
    {
	/* Erase the part of the laser which is over the CD by redrawing
	 * part of the CD itself.
	 */
	BltBitMapRastPort( bbm, 96, 209, rp, 38+XOFF, 74+YOFF, 106, 16, 0xC0 );
    }

    getcounterbase(new, &xloc, &yloc, &dpos, &dsize);

    if (dsize > 0)
    {
	/* This draws the laser head */
	BltBitMapRastPort(bbm, 96, dpos, rp, xloc, yloc, 80, dsize, 0xC0 );

	if ( new & COUNTER_LASERON )
	{
	    /* COUNTER_LASERON means laser is on in new state.  This blit draws
	     * the laser.
	     */
	    BltMaskBitMapRastPort(bbm, 97, 147, rp, xloc+12, yloc+37, 50, 34, (ABC|ABNC|ANBC), bmask);
	}
    }

    /* Blitting the disk head blew away the numbers inside.  Thus,
     * we have to mark the PLGAD_TTRACK state as blank.
     */
    CurrentIB->ib_GState[PLGAD_TTRACK] = WorkIB->ib_GState[PLGAD_TTRACK] = 0;

    return(0);
}





/*************************** Disk Gadget Movement **************************/

/* arg rp = ViewPort's RastPort
 * arg bbm = Button bitmap
 * arg GadDir = "gadget" description
 * arg old = old state
 * arg new = new state
 * global bmask = Button bitmap's mask plane (DPaint stencil)
 */

static
RDisk(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new)
{
extern UBYTE           *bmask;

WORD                    xloc,dpos,dwidth;
WORD                    oldxloc, olddwidth;
int                     overlen = 0;

static WORD             nums[] =
{
    /* ZZZ: first entry is unused */
       0,   38,   37,   36,   33,
      30,   26,   21,   16,   10,
       3,   -4,  -12,  -21,  -31,
     -42,  -57,  -72,  -88, -105,
    -126, -148, -171, -195,
};


    /* Work out the span of the disk in its old and new positions,
     * so we know what to erase.
     *
     * To handle the case of (old==0), we choose initial numbers
     * that will produce no rendering.
     */
    olddwidth = -20000;
    oldxloc = 10000;
    if ( ( old ) && ( old < 24 ) )
    {
	olddwidth = 207;

	oldxloc = nums[old]+XOFF;

	if (oldxloc < 0)
	{
	    olddwidth += oldxloc;
	    oldxloc = 0;
	}
	overlen = oldxloc + olddwidth - (151+XOFF);
    }

    xloc = 0;
    dwidth = 0;
    if ( ( new ) && ( new < 24 ) )
    {
	dpos   = 96;
	dwidth = 207;

	xloc = nums[new]+XOFF;

	if (xloc < 0)
	{
	    dpos -= xloc;
	    dwidth += xloc;
	    xloc = 0;
	}
	/* Draw the CD sliding in or out */
	BltBitMapRastPort(bbm, dpos, 209, rp, xloc, 74+YOFF, dwidth, 18, 0xC0 );

	if ( xloc+dwidth - (151+XOFF) > overlen )
	{
	    overlen = xloc+dwidth - (151+XOFF);
	}
    }

    SetAPen( rp, 0 );
    /* If the disk is moving to the right, we may have stuff
     * to erase on the left-hand-side
     */
    if ( xloc > oldxloc )
    {
	RectFill( rp, oldxloc, 74+YOFF, xloc-1, 91+YOFF );
    }
    /* If the disk is moving to the left, we may have stuff
     * to erase on the right-hand-side.  This is more complex,
     * for one reason.  If you look closely at the "plastic"
     * in the track table, you can see the background gradient
     * lensing through.  What we _should_ do is blit the area
     * from the original bitmap (giving us track table plus
     * gradient, then do a masked blit of the CD, superposing
     * that, then a masked blit of the special repair copy
     * of the track, which puts the CD behind.
     * Now we want to avoid the masked blit of the CD, for speed,
     * so we cheat.
     *
     * So what we do here is blit from the original bitmap, which
     * restores the grid and the lensing.  The reason this is
     * a cheat is that the lensing gradient is only visible in
     * the pixels to the right of the extreme of the CD, whereas
     * because the CD is non-rectangular, we should have more
     * of the gradient visible than that.  However, since the
     * anim is fast, who cares?
     */
    if ( oldxloc + olddwidth > xloc + dwidth )
    {
/*ZZZ shouldn't have Intuition refs here? */
	BltBitMapRastPort(MasterIB->ib_ScreenBuffer->sb_BitMap,
	    xloc + dwidth, 74+YOFF,
	    rp, xloc + dwidth, 74+YOFF,
	    oldxloc + olddwidth - ( xloc + dwidth ), 18, 0xC0 );
    }
    /* The CD might have been rendered over part of the track table.
     * We keep a spare masked copy on the scrbuttons screen, and
     * right here, we blit it back over so the CD goes behind,
     * as we wished.
     */
    if (overlen > 0)
    {
	BltMaskBitMapRastPort(bbm, 192,79, rp, 151+XOFF, 74+YOFF,
	    overlen, 18, (ABC|ABNC|ANBC), bmask);

	GADNumber(rp, bbm, &MasterIB->ib_GadList[PLGAD_16], -1, MasterIB->ib_GState[PLGAD_16]);
	GADNumber(rp, bbm, &MasterIB->ib_GadList[PLGAD_17], -1, MasterIB->ib_GState[PLGAD_17]);
	GADNumber(rp, bbm, &MasterIB->ib_GadList[PLGAD_18], -1, MasterIB->ib_GState[PLGAD_18]);
    }
    return(0);
}


/************************** Update Digits in Head **************************/

static
GADTrackNum(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new)
{
struct GadDir gad;
UWORD counterstate;

    counterstate = MasterIB->ib_GState[PLGAD_COUNTER];

    /* When the head is flying, we want the counter to be blank.
     * For speed, we don't keep re-rendering the blankness.
     */
    if ( counterstate & COUNTER_FLYING )
    {
	return(0);
    }

    gad.LeftEdge   = (counterstate & COUNTER_TRACKNUM);
    gad.TopEdge    = 0;
    gad.RenderData = g->RenderData;

    GADNumber(rp, bbm, &gad, old, new);

    return(g->Flags & GDFLAGS_SELECTABLE);
}




/************************** Update Grid Selection **************************/

static
GADGridNumber(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new)
{
struct GadDir gad;

static BYTE gbox[] =
{
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

    if ( ( old ^ new ) & GRID_CURRENT )
    {
	CopyMem((char *)g, (char *)&gad, sizeof(struct GadDir));
	gad.BoxData = gbox;
	GADInsDraw( rp, &gad, ( new & GRID_CURRENT ? 1 : 0 ), new );
    }

    return(GADNumber(rp, bbm, g, old,new));
}

/* Handle GOCDG button imagery:
 * arg rp = ViewPort's RastPort
 * arg bbm = Button bitmap
 * arg GadDir = "gadget" description
 * arg old = old state
 * arg new = new state
 * global bmask = Button bitmap's mask plane (DPaint stencil)
 */

static
RGoCDG(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new)
{
    if ( CELL_GOCDG_HEIGHT - new )
    {
	BltBitMapRastPort( bbm,
	    BTN_GOCDG_OFF,
	    rp, CELL_GOCDG_LEFT+XOFF, CELL_GOCDG_TOP+YOFF+new,
	    CELL_GOCDG_WIDTH, CELL_GOCDG_HEIGHT - new, 0xC0 );
    }
    if ( new )
    {
	BltBitMapRastPort( bbm,
	    BTN_GOCDG_ON,
	    rp, CELL_GOCDG_LEFT+XOFF, CELL_GOCDG_TOP+YOFF,
	    CELL_GOCDG_WIDTH, new, 0xC0 );
    }
    return( 1 );
}


/***************************************************************************/
/*********************** Miscellaneous Functions ***************************/
/***************************************************************************/

/**************** Stop Play and Display Total Time on Disk *****************/


void
UpdatePlayTime( void )
{
    SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_PRESS);
    SubmitKeyStroke(KeyPressed = PKS_STOP|PKSF_RELEASE);
    WaitTOF();
}


/************************* Get Gadget's Keycode ****************************/

UWORD
GetBoxKey(struct IBuffer *ib, UWORD event)
{
UWORD ans;

    if (ib->ib_BoxNo >= 0)
    {
	ans = ib->ib_GadList[ib->ib_BoxNo].KeyTrans + (event & IECODE_UP_PREFIX);
	return(ans);
    }

    return(0);
}





/*================ Scroll Up/Down Functions for DoBoxMove =================*/

static int
ScrollGridUp( void )
{
UBYTE orgpos;

    orgpos = MasterIB->ib_TrackTableOffset;

    MasterIB->ib_TrackTableOffset += 5;

    UpdateGadgets();

    return((MasterIB->ib_TrackTableOffset == orgpos ? 0:1));
}



static int
ScrollGridDown( void )
{
int ans = 1;

    if (MasterIB->ib_TrackTableOffset >= 5) MasterIB->ib_TrackTableOffset -= 5;
    else                 ans = 0;

    UpdateGadgets();

    return(ans);
}




/*************************** Move Box Selection ****************************/

static void
DoBoxMoveGrunt(UWORD event)
{
UBYTE f;
struct GadDir *g;

    if ((KeyPressed & PKSF_STROKEDIR) == PKSF_PRESS) return;

    g = &MasterIB->ib_GadList[MasterIB->ib_BoxNo];
    f = g->Flags;

    if ((event == KEY_LEFT) && (MasterIB->ib_BoxNo == PLGAD_1)) if (ScrollGridDown())
    {
	MasterIB->ib_BoxNo = PLGAD_5;
	return;
    }

    if ((event == KEY_RIGHT) && (MasterIB->ib_BoxNo == PLGAD_20)) if (ScrollGridUp())
    {
	MasterIB->ib_BoxNo = PLGAD_16;
	return;
    }

    if ((event == KEY_UP) && (f & PLFLAGS_SCROLLUP)) if (ScrollGridDown())
    {
	return;
    }

    if ((event == KEY_DOWN) && (f & PLFLAGS_SCROLLDOWN)) if (ScrollGridUp())
    {
	return;
    }

    BoxMove( MasterIB, event );
    return;
}

/* noscrolltracks is a shot counter, which is the timeout during which
 * the player will refuse to automatically scroll so as to put the
 * currently playing track into view.
 * If we're currently in the track table or we end up in the track table
 * as a result of DoBoxMoveGrunt(), we want to set this counter to the
 * countdown value, to enable scrolling.  We need this enabled before
 * DoBoxMoveGrunt(), since some scrolling may result.  We need also
 * to test afterward, since the new position of the highlight matters too.
 */
void DoBoxMove(UWORD event)
{
    UWORD beforebox;
    UWORD afterbox;

    afterbox = MasterIB->ib_BoxNo;
    if ( ( MasterIB->ib_BoxNo >= PLGAD_1 ) && ( MasterIB->ib_BoxNo <= PLGAD_20 ) &&
	( PlayerState.PlayState >= PLS_PLAYING ) )
    {
	noscrolltracks = NOSCROLLTRACKS_TICKS;
    }

    /* We know support gadgets which resist the highlight box when
     * their state is zero.  So iterate on DoBoxMoveGrunt() until
     * either:
     * - we fail to move, or
     * - the new gadget does not have this property, or
     * - the new gadget's state is non-zero
     *
     * ZZZ: Not all infinite-looping conditions are caught by this!
     * You can't make a whole row of gadgets have this feature!
     */
    do
    {
	beforebox = afterbox;
	DoBoxMoveGrunt(event);
	afterbox = MasterIB->ib_BoxNo;
    } while ( ( afterbox != beforebox ) &&
	( MasterIB->ib_GadList[afterbox].Flags & GDFLAGS_DISABLABLE ) &&
	( MasterIB->ib_GState[afterbox] == 0 ) );

    /* If we're moving in the track table, suppress scrolling of the
     * track-table if we're playing
     */
    if ( ( MasterIB->ib_BoxNo >= PLGAD_1 ) && ( MasterIB->ib_BoxNo <= PLGAD_20 ) &&
	( PlayerState.PlayState >= PLS_PLAYING ) )
    {
	noscrolltracks = NOSCROLLTRACKS_TICKS;
    }
}





/************************** Update Digits in Grid **************************/

VOID
DisplayGrid( void )
{
WORD   pos,maxpos,i;
UBYTE *pe;
UWORD *gs;

    /* D0 is the position */
    /* Should be unnecessary as ib_TrackTableOffset should already be an exact
     * multiple of 5
     */
    pos = (MasterIB->ib_TrackTableOffset/5) * 5;

    /* No lower than zero */
    if (pos < 0)
    {
	pos = 0;
    }
    /* Don't let pos be so high that we see any blank rows.
     * The maximum position is one which is 20 less than the
     * number of tracks rounded up to the next multiple of 5.
     */
    maxpos = 5 * ( ( PlayList->EntryCount + 4 ) / 5 ) - 20;
    if ( maxpos < 0 )
    {
	maxpos = 0;
    }

    if ( pos > maxpos )
    {
	pos = maxpos;
    }

    /* Position in playlist of track denoted by first gadget
     * of track table
     */
    MasterIB->ib_TrackTableOffset = pos;

    pe = &PlayList->Entry[pos];
    gs = &MasterIB->ib_GState[PLGAD_1];

    for ( i = 0; i < 20; i++ )
    {
	if ( ( pos >= PlayList->EntryCount ) || ( !*pe ) ) 
	{
	    /* Entries past the end of the PlayList, or entries of
	     * zero should produce a blank gadget
	     */
	    *gs = 0;
	}
	else if (*pe & PLEF_ENABLE)
	{
	    /* The gadget of an enabled track has state = track-number */
	    *gs = *pe & PLEF_TRACK;
	}
	else
	{
	    /* The gadget of a disabled track has state = GRID_DIMMED|track-number */
	    *gs = GRID_DIMMED + (*pe & PLEF_TRACK);
	}

	pe++;
	gs++;
	pos++;
    }

    /* Set the lit number if it is playing */
    MasterIB->ib_CurrentPlayingTrack = 0;

    if ( ( PlayerState.PlayState == PLS_SELECTED ) ||
	( PlayerState.PlayState >= PLS_PLAYING ) )
    {
	UWORD current;

	if ( !noscrolltracks )
	{
	    /* Light up the currently playing track, and ensure that
	     * it's scrolled into view.
	     */
	    while ( PlayerState.ListIndex < MasterIB->ib_TrackTableOffset )
	    {
		MasterIB->ib_TrackTableOffset -= 5;
	    }
	    while ( PlayerState.ListIndex >= MasterIB->ib_TrackTableOffset + 20 )
	    {
		MasterIB->ib_TrackTableOffset += 5;
	    }
	}
	current = PlayerState.ListIndex - MasterIB->ib_TrackTableOffset;
	if ( ( current >= 0 ) && ( current < 20 ) )
	{
	    MasterIB->ib_CurrentPlayingTrack = PLGAD_1 + current;
	    MasterIB->ib_GState[ MasterIB->ib_CurrentPlayingTrack ] |= GRID_CURRENT;
	}
    }
}




/**************************** Randomize Tracks *****************************/

/* We're called from inside UpdateGadgets(), so we don't
 * need to do any refresh of our own
 */
void
UnShuffleGrid( void )
{
WORD i;

    UpdatePlayTime(); while (!ModifyPlayList(1));

    MasterIB->ib_TrackTableOffset = 0;

    PlayList->EntryCount = 0;
    NumSelectedTracks = 0;
    for ( i = 0; i < UndoPlayList->EntryCount; i++ )
    {
	PlayList->Entry[i] = UndoPlayList->Entry[i];
	if ( PlayList->Entry[i] & PLEF_ENABLE )
	{
	    NumSelectedTracks++;
	}
    }
    PlayList->EntryCount = UndoPlayList->EntryCount;

    ModifyPlayList(0);
}

void
ShuffleGrid( LONG alreadyshuffled )
{
WORD    i, j, d, r;

    UpdatePlayTime();
    while (!ModifyPlayList(1));

    /* Blank grid before we display our changes */
    for(i=PLGAD_1; i<=PLGAD_20; i++) MasterIB->ib_GState[i] = 0x0000;

    MasterIB->ib_TrackTableOffset = 0;
    MasterIB->ib_GState[PLGAD_SHUFF] = 1;
    UpdateDisplay();

    if ( !alreadyshuffled )
    {
	CopyMem(PlayList, UndoPlayList, sizeof(struct PlayList));
    }
    PlayList->EntryCount = 0;
    for ( i = 0; i < UndoPlayList->EntryCount; i++ )
    {
	PlayList->Entry[i] = 0;
    }
    PlayList->EntryCount = UndoPlayList->EntryCount;
    NumSelectedTracks = 0;

     /* Generate random list from 'undo' list. */
    for(i=0; i<UndoPlayList->EntryCount; i++)
    {
	j=0;

	do
	{
	    r = randomNumber( UndoPlayList->EntryCount );
	}
	while ( ++j < 10 && PlayList->Entry[r] );

	if (j == 10)
	{
	    d = randomNumber(2);

	    /* Find the r'th unused entry. */
	    while (PlayList->Entry[r])
	    {
		if (d)
		{
		    if (--r < 0) r = UndoPlayList->EntryCount - 1;
		}

		else if (++r >= UndoPlayList->EntryCount) r = 0;
	    }
	}

	PlayList->Entry[r] = UndoPlayList->Entry[i];
	if ( PlayList->Entry[r] & PLEF_ENABLE )
	{
	    NumSelectedTracks++;
	}

	WaitTOF(); WaitTOF(); WaitTOF();

	UpdateGadgets();
	UpdateDisplay();
    }

    ModifyPlayList(0);
    /* Ensure the shuffle button has been lit for FLASHLENGTH frames,
     * knowing that we got 3 WaitTOF()s for every entry
     * in the UndoPlayList
     */
    for ( i = UndoPlayList->EntryCount * 3; i < FLASHLENGTH; i++ )
    {
	WaitTOF();
    }

    MasterIB->ib_GState[PLGAD_SHUFF] = 0;

    UpdateGadgets();
    UpdateDisplay();
}


/*********************** Initialize Head Positioner ************************/

void
InitTrackCounter( void )
{
    secperpixel = TOC[0].Summary.LeadOut.LSN/TCOUNTMAX;

    if (secperpixel == 0) secperpixel = 1;
}




/************************ Head Positioning Function ************************/

void
SetTrackCounter(int now)
{
WORD  loc, pos, move;

    if (PlayerState.PlayState == PLS_NUMENTRY);

    else if ((PlayerState.PlayState == PLS_SELECTED
	    || PlayerState.PlayMode == PLM_SKIPFWD
	    || PlayerState.PlayMode == PLM_SKIPREV)
	    && (PlayerState.PlayState != PLS_STOPPED ))
    {
	currsec = TOC[PlayerState.Track].Entry.Position.LSN;
    }

    else if (PlayerState.PlayState >= PLS_PLAYING)
    {
	if (qvalid) currsec = copyqcode.DiskPosition.LSN;
    }

    else currsec = 0;

    if ( !( MasterIB->ib_GState[PLGAD_COUNTER] & COUNTER_FLYING ) )
    {
	pos = TCOUNTMAX - (currsec/secperpixel);

	if (pos >= TCOUNTMAX) pos = TCOUNTMAX-1;
	if (pos < 0)          pos = 0;

	if (now) loc = pos;

	else
	{
	    loc = MasterIB->ib_GState[PLGAD_COUNTER] & COUNTER_TRACKNUM;
	    pos -= loc;

	    if (pos < 0)
	    {
		move = -2;
		if (pos >= -2) move = pos;
		if (pos < -5)  move = -3;
		if (pos < -10) move = -4;
	    }

	    else
	    {
		move = 2;
		if (pos <= 2) move = pos;
		if (pos > 5)  move = 3;
		if (pos > 10) move = 4;
	    }

	    loc += move;
	}

	MasterIB->ib_GState[PLGAD_COUNTER] = loc;
	if ( PlayerState.PlayState >= PLS_PLAYING )
	{
	    MasterIB->ib_GState[PLGAD_COUNTER] |= COUNTER_LASERON;
	}
    }
}


/******************* Highlight/Unhighlight a Grid Entry ********************/

void
togglegnum( void )
{
int pos;

    pos = MasterIB->ib_TrackTableOffset + (MasterIB->ib_BoxNo - PLGAD_1);

    if ( ( MasterIB->ib_BoxNo < PLGAD_1 ) || ( MasterIB->ib_BoxNo > PLGAD_20 ) ||
	( pos >= PlayList->EntryCount ) )
    {
	return;
    }

    if (PlayList->Entry[pos] & PLEF_TRACK)
    {
	MasterIB->ib_GState[PLGAD_SHUFF] = 0;

	PlayList->Entry[pos] ^= PLEF_ENABLE;
	if ( PlayList->Entry[pos] & PLEF_ENABLE )
	{
	    NumSelectedTracks++;
	}
	else
	{
	    NumSelectedTracks--;
	}

	/* Suppress scrolling of the track-table if we're playing */
	if ( PlayerState.PlayState >= PLS_PLAYING )
	{
	    noscrolltracks = NOSCROLLTRACKS_TICKS;
	}

	UpdatePlayTime();
	UpdateGadgets();
    }
}




/* This is nothing other than an upcoding of RangeRand() from
 * amiga.lib.  We upcode it to avoid problems with accessing
 * the global "randomseed" variable, which is a pain because
 * we're using the blink library-generation stuff.
 */
static UWORD
randomNumber( ULONG range )
{
    ULONG iterations = range-1;

    while ( iterations )
    {
	ULONG overflow = 0;
	if ( randomseed & 0x80000000 )
	{
	    overflow = 1;
	}
	randomseed *= 2;
	if ( overflow )
	{
	    randomseed ^= 0x1D872B41;
	}
	iterations >>= 1;
    }
    return( ( ( randomseed & 0xFFFF ) * range ) >> 16 );
}

