/* $Id: playerprefs.h,v 1.11 93/04/07 16:51:38 peter Exp $ */

#ifndef EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif

#ifndef EXEC_IO_H
#include <exec/io.h>
#endif

#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif

#ifndef CDTVPREFS_H
#include <cdtv/cdtvprefs.h>
#endif

#include <libraries/lowlevel.h>

#include "ppl:basicio/viewmgr.h"

#define MAX_PREFSLANG   15


/*----------------------------------------------------------------------*/

/* Number of vblanks to flash a button for (Intro, Shuffle, etc.) */

#define FLASHLENGTH	20

/* Number of vblanks after the last time the user moved around inside
 * the track table before it becomes allowed for the player to scroll
 * the track table to keep the currently-playing track in view.
 */
#define NOSCROLLTRACKS_TICKS 300

/*----------------------------------------------------------------------*/

/* - DEFINES FOR THE PLAY LISTS - */

/* NENTRIES must be 100, to match amounts in player.library */
#define NENTRIES        100
#define TOCSIZE         (sizeof (struct CDTOC) * NENTRIES)



/*----------------------------------------------------------------------*/

struct CDTV_SEvent {

    UWORD           Qualifier;
    UWORD           Code;
    };

struct CDTVEvent {

    struct Message      cdtve_Msg;
    struct CDTV_SEvent  cdtve_SEvent;
    };


struct CDTVInputInfo {

    struct Interrupt    Inter;
    struct MsgPort      MsgPort;
    
    LONG                MouseWait;
    UWORD               MouseTDist;
    struct timeval      mousetime;
    UWORD               mousemove;
    UBYTE               mousedir;
    UBYTE               CurrButtonPos;  /* button pos based on input events */
    UWORD               mousequal;
    
    struct IOStdReq     *input_io;
    struct MsgPort      returnport;
        
    WORD                MouseXmove, MouseYmove;
    };

/*----------------------------------------------------------------------*/
/* GADGETS - PLAYER PREFS                                                */
/*----------------------------------------------------------------------*/

    /*----------------------------------*
     * structures for different gadgets *
     *----------------------------------*/

struct GADNumberInfo {

    WORD        xloc,yloc;      /* Number location on the button screen */
    WORD        width,height;   /* Dimensions */
    BYTE        xoffset,yoffset;/* offset from upper left corner */
    UBYTE       nwidth;         /* width between each digit */
    UBYTE       ndigits;        /* number of digits */
    UBYTE       offnum;         /* offnum equals */
    UBYTE       pad;
    UBYTE       onewidth,oneoffset; /* Both of these are used if */
    WORD        oneloc,blankoneloc; /* GNIF_LEADONE is set.      */
    ULONG       flags;
    };


#define GNIF_LEADONE    0x1     /* leading number can only be a one */
#define GNIF_HALF       0x8000  /* also in gstate */
#define GNIF_BLANKZERO  0x2     /* zero blank     */
#define GNIF_BLANKLEAD  0x4     /* blank leading zeros */
#define GNIF_BLANKMAX   0x8     /* blank if >= max */


    /*----------------------------------*
     * structures for different boxes   *
     *----------------------------------*/

struct GADImageLoc {

    UBYTE  map; /* 0 - main, 1 - bbm */
    BYTE  mask; /* 0 - main, 1 - bbm, <0 - no mask */ 
    UWORD  xloc,yloc;
    };

struct GADSTDBox {

    WORD    xoffset,yoffset;
    WORD    width,height;
    struct  GADImageLoc offi,oni;
    };


#define INS_END         0
#define INS_PEN         1
#define INS_MOVE        2
#define INS_DRAW        3
#if 0 /* unused */
	#define INS_ON_REDRAW   4  /* all of these are later implementaions */
#endif

#define INS_OFF_REDRAW  5
#if 0 /* unused */
	#define INS_ONSKIP      6
	#define INS_OFFSKIP     7
#endif

#define INS_MAX         8

/* States for the grid gadgets PLGAD_1 - PLGAD_20 (that make up the track table)
 * GRID_CURRENT denotes the current selection in the grid (has a small
 *	white box around it)
 * GRID_DIMMED denotes that the current track is disabled and should
 *	be dimmed.  This corresponds to entries that are not PLEF_ENABLE
 *	in the PlayList.
 */
#define GRID_CURRENT	0x4000
#define GRID_DIMMED	0x8000

/* States for the disk head (PLGAD_COUNTER).
 * COUNTER_LASERON denotes that the laser should be drawn.
 * COUNTER_FLYING denotes a state where the head is travelling vertically,
 *	as part of the disk insertion/removal animation.
 * COUNTER_TRACKNUM is a mask to extract the track number portion
 */
#define COUNTER_LASERON	0x4000
#define COUNTER_FLYING	0x8000
#define COUNTER_TRACKNUM	0x0FFF


    /*----------------------------------*/


/* NB: Order of gadgets IS significant.  The laser from PLGAD_COUNTER can
 * overwrite the disk from PLGAD_DISK, thus PLGAD_DISK must follow
 * PLGAD_COUNTER.  Also, PLGAD_TTRACK intentionally writes over PLGAD_COUNTER,
 * so it too must follow.
 */
enum PLGads {

    PLGAD_DISK,
    PLGAD_COUNTER,
    PLGAD_TTRACK,

    PLGAD_REV,
    PLGAD_PLAY,
    PLGAD_FF,
    PLGAD_STOP,

    PLGAD_GOCDG,
    PLGAD_TIME,
    PLGAD_INTRO,
    PLGAD_SHUFF,
    PLGAD_REPEAT,

    PLGAD_1,
    PLGAD_2,
    PLGAD_3,
    PLGAD_4,
    PLGAD_5,
    PLGAD_6,
    PLGAD_7,
    PLGAD_8,
    PLGAD_9,
    PLGAD_10,
    PLGAD_11,
    PLGAD_12,
    PLGAD_13,
    PLGAD_14,
    PLGAD_15,
    PLGAD_16,
    PLGAD_17,
    PLGAD_18,
    PLGAD_19,
    PLGAD_20,

    PLGAD_MIN,
    PLGAD_SEC,
    PLGAD_NEG,
    PLGAD_CDG,
    PLGAD_COLON,

    PLGAD_NL
    };


#define MAX_PLGAD    		PLGAD_NL
#define PLGAD_FIRSTBUTTON	PLGAD_REV
#define PLGAD_LASTBUTTON	PLGAD_20

/* Function return values:
 * BoxFunc() returns zero if the highlight was drawn/erased normally.
 * 	It returns non-zero if the highlight was suppressed
 *	(see GDFLAGS_DISABLABLE)
 * RenderFunc() returns non-zero if the gadget just drawn is highlightable.
 */
struct GadDir {

    UBYTE       ID,Flags,KeyTrans,routine;
    UWORD       MaxStates;
    UBYTE       Up,Down,Left,Right;
    UWORD       LeftEdge,TopEdge,RightEdge,BottomEdge;
    int         (*BoxFunc)();
        VOID            *BoxData;
    int         (*RenderFunc)();
    VOID        *RenderData;
    };

/* GDFLAGS_SELECTABLE means we support putting the twinkling cursor-box
 * 	over this gadget.
 * GDFLAGS_DISABLABLE means that we can disable this gadget, therefore
 *	disallowing the twinkling cursor box.  Here's how it works:
 *	state = MaxStates-1 means enabled, fully normal.
 *	state = 0 means disabled, and DoBoxMove() must "pass over"
 *		us instead of stopping with the highlight on us.
 *	state between 0 and MaxStates-1 means we're enabled (as far
 *		as DoBoxMove() knows), but the highlight shouldn't be
 *		drawn because we're in a transitionary rendering.
 * ZZZ: Not all infinite-looping conditions are handled, i.e.
 * You can't make a whole row of gadgets have this feature!
 */
#define GDFLAGS_SELECTABLE      0x80
#define GDFLAGS_DISABLABLE	0x40

#define PLFLAGS_SCROLLUP        0x01
#define PLFLAGS_SCROLLDOWN      0x02
#define PLFLAGS_SCROLLLEFT      0x04
#define PLFLAGS_SCROLLRIGHT     0x08

/*----------------------------------------------------------------------*/

/* The RAWKEY_xxx definitions are the raw key codes of those particular
 * keys of interest.
 */
#define RAWKEY_F1	0x50
#define RAWKEY_F2	0x51
#define RAWKEY_F3	0x52
#define RAWKEY_F4	0x53
#define RAWKEY_F5	0x54
#define RAWKEY_F6	0x55
#define RAWKEY_F7	0x56
#define RAWKEY_F8	0x57
#define RAWKEY_F9	0x58
#define RAWKEY_F10	0x59

#define RAWKEY_ENTER	0x43	/* (Numpad) */
#define RAWKEY_RETURN	0x44
#define RAWKEY_ESC	0x45

#define RAWKEY_ARROWUP	0x4C
#define RAWKEY_ARROWDN	0x4D
#define RAWKEY_ARROWRT	0x4E
#define RAWKEY_ARROWLT	0x4F

/* The actual code makes reference to KEY_xxx, GC0_xxx, and GC1_xxx
 * definitions, where these definitions are named by program function,
 * not by actual physical key or controller button.  Thus, this file
 * defines the mapping of physical key/button to function.
 * KEY_xxx = keyboard control.
 * GC0_xxx = game-controller 0 control
 * GC1_xxx = game-controller 0 control
 */

/* Transport control functions:
 * ..._BACK	 = track backward/seek backward
 * ..._PLAYPAUSE = toggle play/pause
 * ..._FWD	 = track forward/seek forward
 * ..._STOP	 = stop
 */
#define KEY_BACK	RAWKEY_F1
#define GC0_BACK	RAWKEY_PORT0_BUTTON_REVERSE
#define GC1_BACK	RAWKEY_PORT1_BUTTON_REVERSE
#define KEY_PLAYPAUSE	RAWKEY_F2
#define GC0_PLAYPAUSE	RAWKEY_PORT0_BUTTON_PLAY
#define GC1_PLAYPAUSE	RAWKEY_PORT1_BUTTON_PLAY
#define KEY_FWD		RAWKEY_F3
#define GC0_FWD		RAWKEY_PORT0_BUTTON_FORWARD
#define GC1_FWD		RAWKEY_PORT1_BUTTON_FORWARD
#define KEY_STOP	RAWKEY_F4
#define GC0_STOP	RAWKEY_PORT0_BUTTON_BLUE
#define GC1_STOP	RAWKEY_PORT1_BUTTON_BLUE

/* Feature controls:
 * ..._GOCDG	 = Re-enter CD+G display
 * ..._TIMEMODE	 = cycle through time modes
 * ..._INTRO	 = toggle Intro mode
 * ..._SHUFFLE	 = initiate shuffle play
 * ..._REPEAT	 = toggle Repeat mode
 */
#define KEY_GOCDG	RAWKEY_F5
#define KEY_TIMEMODE	RAWKEY_F6
#define KEY_INTRO	RAWKEY_F7
#define KEY_SHUFFLE	RAWKEY_F8
#define GC0_SHUFFLE	RAWKEY_PORT0_BUTTON_GREEN
#define GC1_SHUFFLE	RAWKEY_PORT1_BUTTON_GREEN
#define KEY_REPEAT	RAWKEY_F9
#define GC0_REPEAT	RAWKEY_PORT0_BUTTON_YELLOW
#define GC1_REPEAT	RAWKEY_PORT1_BUTTON_YELLOW

/* Navigation controls:
 * ..._SELECT	 = "click" on current user-interface object
 * ..._UP	 = move up
 * ..._DOWN	 = move down
 * ..._RIGHT	 = move right
 * ..._LEFT	 = move left
 * ..._GBUTTON	 = toggle selected track in track table
 * ..._ADVANCE	 = move to next track in track table
 */
#define KEY_SELECT	RAWKEY_RETURN
#define GC0_SELECT	RAWKEY_PORT0_BUTTON_RED
#define GC1_SELECT	RAWKEY_PORT1_BUTTON_RED

#define KEY_UP		RAWKEY_ARROWUP
#define GC0_UP		RAWKEY_PORT0_JOY_UP
#define GC1_UP		RAWKEY_PORT1_JOY_UP
#define KEY_DOWN	RAWKEY_ARROWDN
#define GC0_DOWN	RAWKEY_PORT0_JOY_DOWN
#define GC1_DOWN	RAWKEY_PORT1_JOY_DOWN
#define KEY_RIGHT	RAWKEY_ARROWRT
#define GC0_RIGHT	RAWKEY_PORT0_JOY_RIGHT
#define GC1_RIGHT	RAWKEY_PORT1_JOY_RIGHT
#define KEY_LEFT	RAWKEY_ARROWLT
#define GC0_LEFT	RAWKEY_PORT0_JOY_LEFT
#define GC1_LEFT	RAWKEY_PORT1_JOY_LEFT

#define KEY_GBUTTON	RAWKEY_F10
#define KEY_ADVANCE	RAWKEY_ENTER


/* Player.library echoes keys to us after we submit keystrokes to it.
 * The only echoes we care about are the reverse and forward keys.
 * We _almost_ can do without the echoes entirely, but player.library
 * won't process (hence echo) a key while another key is held.
 *
 * For the CDGS, we modified player.library to put 0xFF in the high
 * byte of the RAWKEY code, to distinguish these keys from similar
 * numbers that lowlevel.library sends when we use the game-controller
 * input-event generator feature.
 */
#define ECHOKEY_PREVTRAK 0xFF74
#define ECHOKEY_NEXTTRAK 0xFF75



/*---------------------------- PREFERENCES DEFINES ----------------------*/

char * __asm OpenLibs(register __a0 struct LibInfo *l);
void __asm  CloseLibs(register __a0 struct LibInfo *l);


UpDateDisplay(VOID);

UWORD GetBoxKey(struct IBuffer *ib, UWORD event);
void BoxMove( struct IBuffer *ib, UWORD event );


/*--------------------------- Globals -----------------------------*/

extern struct CDTVInputInfo input_data;

extern struct CDTVPrefs         CDTVPrefs;


/*-----------------------------------------------------------------*/
/*-------------------------- STUFF FROM TEST.C --------------------*/


enum TimeModes {

    TIME_TRACK,
    TIME_TRACK_REMAIN,
    TIME_LIST,
    TIME_LIST_REMAIN,
    MAX_TIMEMODE
    };

/*--------------------------- TIME ------------------------*/


struct TimeClock {

    WORD    year, month, day, hour, min;
    };

#define SECS_DAY        (60*60*24)
