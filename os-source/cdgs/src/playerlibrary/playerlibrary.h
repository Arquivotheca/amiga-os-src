

#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#ifndef  CD_H
#include <gs:cd/cd.h>
#endif

/***********************************************************************/

struct PlayerOptions {

    BYTE    Loop;           /* 0 = Disabled, 1 = Enabled */

    BYTE    Intro;          /* 0 = Disabled, 1 = Enabled */

    BYTE    TimeMode;       /* 0 =  Track Relative       */
                            /* 1 = -Track Relative       */
                            /* 2 =  Disk Absolute        */
                            /* 3 = -Disk Absolute        */

    BYTE    Subcode;        /* 0 = Disabled, 1 = Enabled */
    };


/***********************************************************************/

struct PlayerState {

    BYTE    AudioDisk;          /*  1 = An Audio disk is present        */
                                /*  0 = No disk present                 */
                                /* -1 = Non audio disk is present       */
    UBYTE   Tracks;             /* Number of tracks on audio disk       */
    UBYTE   ListIndex;          /* Current position of player in list   */
                                /*   (values = 1-99 & 0 (not selected)) */
    UBYTE   LastModify;         /* Last to modify PlayList              */
                                /* (0 = 68000, 1 = internal player)     */
    UBYTE   PlayMode;           /* PLM_NORMAL, PLM_FFWD, PLM_FREV,      */
                                /*   PLM_SKIPFWD, PLM_SKIPREV           */
    UBYTE   PlayState;          /* PLS_STOPPED, PLS_SELECTED            */
                                /*   PLS_PLAYING, PLS_PAUSED            */
    
    UBYTE   Track;              /* Current value in TRACK field of VFD  */
    UBYTE   Minute;             /* Current value in MINUTE field of VFD */
    UBYTE   Second;             /* Current value in HOUR field of VFD   */
                                /*   (values = 0-99 & 100 (blank))      */
    UBYTE   Minus;              /* Current value in MINUS-SIGN field of */
                                /* VFD time display                     */
    };

#define PLM_NORMAL  0
#define PLM_FFWD    1
#define PLM_FREV    2
#define PLM_SKIPFWD 3
#define PLM_SKIPREV 4

#define PLS_STOPPED  0
#define PLS_SELECTED 1
#define PLS_NUMENTRY 2
#define PLS_PLAYING  3
#define PLS_PAUSED   4


/***********************************************************************/

#define PKSF_STROKEDIR  0x80    /* Mask for stroke direction (clear = down, set = up) */
#define PKSB_STROKEDIR  7
#define PKSF_PRESS      0x00	/* downstroke */
#define PKSF_RELEASE    0x80	/* upstroke */

#define PKSF_KEY        0x7F	/* Mask for key-code */

/* Keys that can be sent via SubmitKeyStroke(), and keys that are echoed
 * back by the player.library.
 *
 * Note that PKS_EJECT is co-opted to mean absolute play, which differs
 * from PKS_PLAYPAUSE in that it won't send the player into pause mode.
 *
 * Note also that the echoes are RAWKEYs with 0xFF in the high byte
 * of the code.  The audio player needs that to be able to distinguish
 * from keys that come from the game controller via the input handler
 * installable from lowlevel.library's SCON_AddCreateKeys.
 */
#define PKS_STOP        0x72
#define PKS_PLAYPAUSE   0x73
#define PKS_REVERSE     0x74
#define PKS_FORWARD     0x75
#define PKS_EJECT       0x76


/***********************************************************************/

struct PlayList {

    UBYTE   EntryCount;
    UBYTE   Entry[100];
    UBYTE   pad;
    };

#define PLEF_ENABLE     0x80
#define PLEF_TRACK      0x7F

#define PLEB_ENABLE     7

#endif
