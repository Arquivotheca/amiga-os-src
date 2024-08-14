/* $Id: display.h,v 1.4 93/03/31 15:06:07 peter Exp $ */

#include "ppl:animate.h"

/* Copper changes color every COPPER_RATE lines */
#define COPPER_RATE	2


struct CycleRange
{
    UBYTE Enabled;	/* Go or not */
    UBYTE Length;	/* Length of range */
    UBYTE NumFrames;	/* How many frames should pass before we cycle */
    UBYTE Countdown;	/* Counts down from NumFrames */
    ULONG *LoadTable;
};

struct ColorChanger
{
    struct ViewPort *ViewPort;
    UWORD NumRanges;
    struct CycleRange *Ranges;
};

/* Encapsulates ScreenBuffer and other application stuff */
struct IBuffer
{
    /* Only need to worry about these in WorkF and CurrentF */
    struct Screen	*ib_Screen;
    struct BMInfo       *ib_BMInfo;
    struct ScreenBuffer *ib_ScreenBuffer;	/* Has BitMap, if I care */

    /* Worry About these in MasterF */
    struct GadDir       *ib_GadList;
    struct BitMap       *ib_ButtonBM;
    UWORD               ib_MaxGad;
    UBYTE               ib_TrackTableOffset;
    UBYTE               ib_Unused;
    UBYTE               ib_CurrentPlayingTrack;
    UBYTE               ib_Updated; /* Set to 1 if still needs to be displayed */

    /* Make the same */
    WORD                ib_BoxNo;      /* Gadget is on box num */
        /* Gadget Info */
    UWORD               *ib_GState;

    UWORD               *ig_cclist[32];    /* Copper color list */
};

#define IBUFFER_INUSE	0x80000000


#define NUM_COLORS 64
#define NUM_PLANES 6
#define NUM_FADE_STEPS 20

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

