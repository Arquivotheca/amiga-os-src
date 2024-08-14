
#include <exec/libraries.h>
#include <intuition/screens.h>
#include <graphics/gfxbase.h>
#include <internal/playerlibrary.h>
#include <devices/cd.h>

/* ColorRange - fill this structure for ACTAG_ColorRange.
 * Pass in ti_Data a pointer to an array of ColorRange structures. This array
 * can be used to change a number of pens on a line. Each line can
 * have an array of pens to change. The array is terminated when the cor_Pen at
 * the start of the line == -1
 *
 * Example:
 * struct ColorRange cr[] =
 * {
 *  {1, 0, 10, -1, 0, 0, NULL}, {2, 0, 0, 0, -1, 0, NULL},
 *  {1, 0, 11, 0x88888888, 0, 0, CORF_RESTORE, NULL}, {2, 0, 0, 0, 0x88888888, 0, 0, NULL},
 *      {-1, NULL},
 * };
 * This would change pen 1 on line 10 to red, pen 2 on line 10 to green,
 * pen 1 on line 11 to dark red, pen 2 on line 11 to dark green. At line 12, pen 1
 * is restored to its original value.
 */

struct ColorRange
{
    LONG cor_Pen;           /* Pen number to change */
    WORD cor_WAIT_X;        /* X Wait position - ignored for V40 */
    WORD cor_WAIT_Y;        /* Y Wait position */
    ULONG cor_Red;          /* 32 bit red value */
    ULONG cor_Green;        /* 32 bit green value */
    ULONG cor_Blue;         /* 32 bit blue value */
    ULONG cor_Flags;
    UBYTE cor_Private[36];  /* private */
};

struct CycleRange
{
    UBYTE Enabled;  /* Go or not */
    UBYTE Length;   /* Length of range */
    UBYTE NumFrames;    /* How many frames should pass before we cycle */
    UBYTE Countdown;    /* Counts down from NumFrames */
    ULONG *LoadTable;
};

struct ColorChanger
{
    struct ViewPort *ViewPort;
    UWORD NumRanges;
    struct CycleRange *Ranges;
};

#define CORB_ANIMATE 0      /* set to modify an existing ColorRange. If
                             * this bit is set in the first ColorRange in
                             * the array, then the hardware copperlist is
                             * modified with these new colours.
                             * THIS BIT SHOULD ONLY BE SET AFTER THE
                             * COLOUR RANGE HAS BEEN BUILT.
                             */
#define CORF_ANIMATE (1 << CORB_ANIMATE)
#define CORB_MODIFY 1       /* When used with CORB_ANIMATE, this colour
                             * entry is modified to the RGB value.
                             */
#define CORF_MODIFY (1 << CORB_MODIFY)
#define CORB_RESTORE 2      /* if set, then restore the pen number to its
                             * original value. This needs to be set only once
                             * for the pen number.
                             */
#define CORF_RESTORE (1 << CORB_RESTORE)

/* Tags for the various AnimationControl() features. (V40) */

#define ACTAG_ColorRange    0x80000001  /* ti_Data points to a
                         * struct ColorRange.
                         */
#define ACTAG_ScrollLines   0x80000002  /* ti_Data points to a
                         * struct ScrollLines.
                         */
#define ACTAG_RepeatLines   0x80000003  /* ti_Data points to a
                         * struct RepeatLines.
                         */



#define COPPER_RATE     2

#define NUM_COLORS      64
#define NUM_PLANES      6
#define NUM_FADE_STEPS  20

struct RGBEntry
{
   ULONG Red;
   ULONG Green;
   ULONG Blue;
};

struct ColorPack
{
    UWORD           cp_NumColors;
    UWORD           cp_FirstColor;
    struct RGBEntry cp_Colors[NUM_COLORS];
    UWORD           cp_EndMarker;              /* must be 0 */
};


struct XY {

    UWORD   X, Y;
    };


enum {

    B_TIMEMODE,
    B_FREV,
    B_PLAYPAUSE,
    B_FFWD,
    B_STOP,
    B_LOOP,
    B_INTRO,
    B_SHUFFLE,
    B_NUMBUTTONS,
    };

#define B_GRID B_NUMBUTTONS

struct ButtonInfo {

    BYTE        Button;
    UWORD       ScreenX;
    UWORD       ScreenY;
    UWORD       Width;
    UWORD       Height;
    struct  XY  SourcePos[5];
    };


struct V {

    struct Library       *DeBoxBase;
    struct Library       *IntuitionBase;
    struct GfxBase       *GfxBase;
    struct Library       *LowLevelBase;
    struct Library       *PlayerBase;

    struct IOStdReq      *CDIO;
    struct MsgPort       *CDReply;

    struct Screen        *Screen;
    struct Window        *Window;

    struct ColorPack      ColorPack;
    struct ColorRange    *ColorRange;

    struct BMInfo        *ScreenBMI;
    struct BitMap        *ScreenBM;

    struct BMInfo        *ButtonBMI;
    struct BitMap        *ButtonBM;

    struct BMInfo        *cdgBMI;
    struct BitMap        *cdgBM;

    struct BitMap        *PointerBM;
    APTR                  Pointer;

    struct PlayList      *PlayList;

    UWORD                 PlayerDisk;
    union  CDTOC          TOC[100];
    struct QCode          QCode;

    UWORD                 ButtonSelect[B_NUMBUTTONS], LastButtonSelect[B_NUMBUTTONS];
    UWORD                 Highlighted,                LastHighlighted;
    UWORD                 GridDisplayOffset,          LastEntryCount;
    UBYTE                 LastGrid[20];
    UWORD                 LastAudioDisk;
    ULONG                 CButtons[7];
    ULONG                 CDirection[4];

    UWORD                 HeadX, CurrentHeadX, LastHeadX, QCount;

    ULONG                 ControllerLock;
    BYTE                  LastMouseX, LastMouseY;

    struct PlayerState    PlayerState,    LastPlayerState;
    struct PlayerOptions  PlayerOptions;

    ULONG                 ButtonState,    LastButtonState;
    ULONG                 DirectionState, LastDirectionState;

    UBYTE                 KeyRepeat, RepeatCount;

    UWORD                 Random;

    BYTE                  SALevels[12];
    UWORD                 SABlank;

    UWORD                 CDGDisplaying;
    };

