

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

#include "ppl:basicio/viewmgr.h"

#define MAX_PREFSLANG   15


/*----------------------------------------------------------------------*/

/* - DEFINES FOR THE PLAY LISTS - */

#define FRAMERATE       75
#define NENTRIES        100
#define TOCSIZE         (sizeof (struct CDTOC) * NENTRIES)

#define PLAYF_DISABLED  (1<<0)
#define PLAYF_BLANK     (1<<1)



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

#define BOXMOVE         0x8000  /* bitflag */

#define CURRBUTTONA             (1<<1)  /* in ButtonPos */
#define CURRBUTTONB             (1<<0)  


#define DIR_UP          0x8004
#define DIR_DOWN        0x8008
#define DIR_LEFT        0x8001
#define DIR_RIGHT       0x8002

/*----------------------------------------------------------------------*/
/* GADETS - PLAYER PREFS                                                */
/*----------------------------------------------------------------------*/

    /*----------------------------------*
     * structures for different gadgets *
     *----------------------------------*/

struct GADNumberInfo {

    WORD        xloc,yloc;      /* Number location on the button screen */
    WORD        width,height;   /* Dimentions */
    BYTE        xoffset,yoffset;/* offset from upper left corner */
    UBYTE       nwidth;         /* width between each digit */
    UBYTE       ndigits;        /* number of digits */
    UBYTE       offnum;         /* offnum equals */
    UBYTE       pad;
    UBYTE       onewidth,oneoffset; /* Both of these are used if */
    WORD        oneloc,blankoneloc; /* GNIF_LEADONE is set.      */
    ULONG       flags;
    };


#define GNIF_LEADONE        0x1     /* leading number can only be a one */
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
#define INS_ON_REDRAW   4  /* all of these are later implementaions */
#define INS_OFF_REDRAW  5
#define INS_ONSKIP      6
#define INS_OFFSKIP     7

#define INS_MAX         8


    /*----------------------------------*/


enum PLGads {

    PLGAD_COUNTER,
    PLGAD_DISK,
    PLGAD_TTRACK,
    PLGAD_STOP,
    PLGAD_REV,
    PLGAD_PLAY,
    PLGAD_FF,
    PLGAD_PAUSE,
    PLGAD_TIME,
    PLGAD_SHUFF,
    PLGAD_GCLR,
    PLGAD_INTRO,
    PLGAD_REPEAT,
    PLGAD_CDG,
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
    PLGAD_NL
    };


#define MAX_PLGAD       PLGAD_NL

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

#define GDFLAGS_BOXOVER         0x80

#define PLFLAGS_SCROLLUP        0x01
#define PLFLAGS_SCROLLDOWN      0x02
#define PLFLAGS_SCROLLLEFT      0x04
#define PLFLAGS_SCROLLRIGHT     0x08

#define PLAYF_DISABLED  (1<<0)
#define PLAYF_BLANK     (1<<1)

struct DisplayFrame {

    /* Only need to worry about these in WorkF and CurrentF */
    struct BMInfo       *bmi;
    struct BitMap       *bm;
    struct View         *view;
    struct ViewPort     *vp;
    struct RastPort     *rp;

    /* Worry About these in MasterF */
    struct GadDir       *GadList;
    struct BitMap       *bbm;
    UWORD               MaxGad;
    UBYTE               D0,D1,D2;
    UBYTE               UpDated; /* Set to 1 if still needs to be displayed */
    ULONG               GState1;

    /* Make the same */
    WORD                BoxNo;      /* Gadget is on box num */

        /* Gadget Info */
    UWORD               *gstate;

    UWORD               *cclist[32];    /* Copper color list */  
    };


struct Render2S_Info {

    APTR    boxinfo;
    UWORD   x,y,scr;
    };

/*---------------------------- PREFERENCES DEFINES ----------------------*/

enum PEGads {

    PEGAD_HOUR,
    PEGAD_MIN,
    PEGAD_AM,
    PEGAD_MONTH,
    PEGAD_DAY,
    PEGAD_YEAR,
    PEGAD_SCREEN,
    PEGAD_SCRSAVE,
    PEGAD_LACE,
    PEGAD_LANG,
    PEGAD_CLICK,
    PEGAD_PRPRAM,
    PEGAD_PRPCARD,
    PEGAD_CAUTION,
    PEGAD_NL,
    MAX_PEGAD
    };


#define MAX_PEGAD       PEGAD_NL

#define PEGAD_MAXLANGSLOTS      15

/*--------------------------- Prototypes -------------------------*/
VOID __stdargs ToggleFrame(VOID);
VOID __stdargs EndCDTVScreen(VOID);
char * __asm OpenLibs(register __a0 struct LibInfo *l);
void __asm  CloseLibs(register __a0 struct LibInfo *l);
ULONG __asm GetEvent(register __a1 struct InputData *id);
VOID __asm ClearEvent(register __a1 struct InputData *id);
__regargs StartCDTVScreen(struct GadDir *gl, UBYTE *maindata, UBYTE *buttondata, UBYTE initfade);
VOID __regargs LoadDisplayFrame(register struct CycleIntInfo *ci, register struct DisplayFrame *df);


UpDateDisplay(VOID);

UWORD __regargs GetBoxKey(register struct DisplayFrame *mf, UWORD event);
void __regargs BoxMove( register struct DisplayFrame *mf, UWORD event );


__regargs PowerTen( register UBYTE dig );

void ResetPlayList( int dogad, int clear );
/*--------------------------- Globals -----------------------------*/

extern struct DisplayFrame *CurrentF,*WorkF,MasterF;
extern struct CycleIntInfo  intdata;
extern struct CDTVInputInfo input_data;

extern struct CDTVPrefs         CDTVPrefs;


/*-----------------------------------------------------------------*/
/*-------------------------- STUFF FROM TEST.C --------------------*/


#define FRAMERATE   75

#define PLAYF_DISABLED  (1<<0)
#define PLAYF_BLANK (1<<1)

enum TimeModes {

    TIME_TRACK,
    TIME_TRACK_REMAIN,
    TIME_LIST,
    TIME_LIST_REMAIN,
    MAX_TIMEMODE
    };

enum WaitForTypes {

    WFT_NOTYPE,
    WFT_TRACK,
    WFT_POSITION,
    MAX_WFT
    };

enum NextTypes {

    NLI_START = 0,      /*  Initialize for entry 0.                             */
    NLI_MAKESANE,       /*  Re-confirm valid curlistidx.                        */
    NLI_PREVTRACK,      /*  Move to previous list entry.                        */
    NLI_PREVTRACKEND,   /*  Move to previous entry, end of track.               */
    NLI_NEXTTRACK,      /*  Move to next list entry.                            */
    NLI_TAKEOVER,       /*  takeover form of make sane list                     */
    MAX_NLI
    };

/*--------------------------- TIME ------------------------*/


struct TimeClock {

    WORD    year, month, day, hour, min;
    };

#define SECS_DAY        (60*60*24)

/*---------------------------Screen ----------------------*/

#define ADJMIN_X        -31
#define ADJMIN_Y        -11
#define ADJMAX_X        0
#define ADJMAX_Y        12

#define ADJSIZE_X       (ADJMAX_X - ADJMIN_X)
#define ADJSIZE_Y       (ADJMAX_Y - ADJMIN_Y)


struct CardStatus {

    UWORD   CardIn;
    UWORD   WasRemoved;
    UBYTE   CStatus;
    UBYTE   CPad;
    };



#define CLIP    #if 0

#define ENDCLIP #endif


