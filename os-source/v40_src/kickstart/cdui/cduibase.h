#ifndef CDUIBASE_H
#define CDUIBASE_H


/*****************************************************************************/


#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/sprite.h>
#include <utility/hooks.h>
#include <cdtv/debox.h>
#include <dos/dos.h>

#include "cdui.h"
#include "animate.h"
#include "allocpatch.h"


/*****************************************************************************/


/* Setting this to TRUE prevents Intuition from making any gfx sprite calls,
 * thus ensuring that it lays off any sprites we are using.
 */
#define IBASE_NUKEPOINTER(ibase)          (*((char **)( ((char *)ibase) +0x68 )))


/*****************************************************************************/


#define NUM_PLANES           6
#define NUM_PLAYFIELD_COLORS (1 << NUM_PLANES)
#define NUM_SPRITE_COLORS    16
#define NUM_COLORS           (NUM_PLAYFIELD_COLORS + NUM_SPRITE_COLORS)

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


/*****************************************************************************/


/* Describes a color effect.  It might be a looping color cycle,
 * or a shooting one, or a waiter, or a fader.  Depending on the
 * type, the other fields may or may not be used.  Generally holds
 * source information for the effect.  Destination info (like what
 * colors in the target to affect) is held elsewhere.
 */
struct ColorEffect
{
    UWORD ce_Type;              /* Effect type */
    UWORD ce_Period;            /* Act every this number of vblanks */
    UWORD ce_FirstColor;        /* First source color */
    UWORD ce_LastColor;         /* Last source color */
    UWORD ce_Palette;           /* Where color source is */
};

/* NB: Ordering of these constants matter, because we do the effects in
 * three passes:
 * First pass: in-place color modifications (CE_LOOPER)
 * Second pass: color modifications from an alternate source (CE_SHOOTER)
 * Third pass: post-processing (CE_FADER) or independent effects (CE_WAITER)
 */

#define CE_LOOPER       1       /* Regular looping color cycle */
#define CE_SHOOTER      2       /* One-shot shoots specified colors through destination */
#define CE_WAITER       3       /* Delays ce_Period vblanks */
#define CE_INFADER      4       /* Fades from black */
#define CE_OUTFADER     5       /* Fades to black */

#define CEPALETTE_ALT1  0
#define CEPALETTE_ALT2  1

/* Describes the target component of a color cycling effect,
 * including which source effect we're using, how far we've
 * progressed, and where in our colormap to apply it.
 */
struct ColorCycle
{
    struct ColorEffect *cy_Effect;      /* Source effect */
    WORD                cy_Phase;                       /* How far into effect we've gone (can be negative) */
    UWORD               cy_Halted;
    UWORD               cy_NumColors;                   /* Number of colors in destination */
    UWORD               cy_ColorBase;
    UWORD               cy_CountDown;
    UWORD               cy_Termination;
};

#define CYTERM_DELAY    0       /* Transition to the delay effect */
#define CYTERM_HOLD     1       /* Hold the cycle in its end-state */

/* We currently have 9 cycles, namely:
 * 0: Disc rainbow
 * 1: Disc perimeter and stars
 * 2: Border around amiga logo
 * 3: Disc face
 * 4: Top aurora
 * 5: Bottom aurora
 * 6: Amiga-logo aurora
 * 7: Universal faders
 * 8: Universal faders
 * 9: Universal faders
 */

#define CYCLE_RAINBOW           0
#define CYCLE_PERIMETER         1
#define CYCLE_AMIGABORDER       2
#define CYCLE_FACE              3
#define CYCLE_AURORATOP         4
#define CYCLE_AURORABOTTOM      5
#define CYCLE_AURORAAMIGA       6
#define CYCLE_FADER1            7
#define CYCLE_FADER2            8
#define CYCLE_FADER3            9

#define NUM_CYCLES              10


/*****************************************************************************/


struct CDUILib
{
    struct Library     cb_Lib;
    UWORD              cb_Pad0;
    struct ExecBase   *cb_SysLib;
    struct GfxBase    *cb_GfxLib;
    struct Library    *cb_IntuitionLib;
    struct Library    *cb_DeBoxLib;
    struct Library    *cb_LowLevelLib;
    struct Library    *cb_NVLib;
    ULONG              cb_SegList;

    struct AllocPatch *cb_AllocPatch;

    struct Task       *cb_CDUITask;
    struct MsgPort    *cb_CDUIPort;

    // stuff used by all screens
    struct Screen     *cb_Screen;
    struct Window     *cb_Window;
    struct RastPort   *cb_RastPort;  /* shortcut for cb_Screen->RastPort */
    struct BitMap     *cb_BitMap;    /* shortcut for cb_RastPort->BitMap */
    struct ViewPort   *cb_ViewPort;  /* shortcut for cb_Screen->ViewPort */
    APTR               cb_Pointer;
    LONG               cb_XOffset;
    LONG               cb_YOffset;
    struct ExtSprite  *cb_Sprites[72];

    struct Screen     *cb_LowCostScreen;
    struct BitMap     *cb_LowCostBM;
    struct BMInfo     *cb_LowCostBMInfo;
    APTR               cb_LowCostColors;

    // stuff used by both the nram editing and the language selection screens
    struct ColorRange  cb_Range[160];
    struct ColorPack   cb_Colors;
    UWORD              cb_Pad1;

    // stuff used by the nram editing screen
    struct Task       *cb_ScrollerTask;
    struct Task       *cb_FaderTask;

    // stuff used by the language selection screen
    struct Task       *cb_SpinnerTask;
    struct Task       *cb_WaverTask;
    struct BitMap     *cb_SaveBM;
    struct BitMap     *cb_WorkBM;
    struct BitMap     *cb_CurrentFlagBM;
    struct SignalSemaphore cb_FlagLock;
    BYTE               cb_FlagToWave;
    BYTE               cb_DoBetterFlag;

    // stuff used by the startup anim
    struct BitMap     *cb_BackgroundBM;
    struct Task       *cb_FanfareTask;
    struct Task       *cb_CyclerTask;
    struct Task       *cb_FlipperTask;
    BOOL               cb_DidAnimation;
    WORD               cb_CDYOffset;
    UWORD              cb_AuroraIndex;
    WORD               cb_Pad3;
    struct BitMap     *cb_BgSaveBM;
    struct ColorCycle  cb_Cycles[NUM_CYCLES];
    APTR               cb_BitMapColors;
    APTR               cb_AuroraColors[2];
    APTR               cb_AuroraBM[2];

    // stuff used by the credit screen. Credit screen? What credit screen? :-)
    struct RastPort   *cb_CreditRP;
    struct BitMap     *cb_CreditBM;
    struct ViewPort   *cb_CreditVP;
};

#define SysBase       CDUIBase->cb_SysLib
#define GfxBase       CDUIBase->cb_GfxLib
#define IntuitionBase CDUIBase->cb_IntuitionLib
#define DeBoxBase     CDUIBase->cb_DeBoxLib
#define LowLevelBase  CDUIBase->cb_LowLevelLib
#define NVBase        CDUIBase->cb_NVLib

#define ASM           __stdargs __asm
#define REG(x)        register __ ## x
#define BOOLEAN       UBYTE            /* byte boolean */

kprintf(STRPTR,...);


/*****************************************************************************/


/* nonvolatile.library crap */
APTR GetCopyNV( STRPTR appName, STRPTR itemName );
void FreeNVData( APTR data );
UWORD StoreNV( STRPTR appName, STRPTR itemName, APTR data, unsigned long length );
BOOL DeleteNV( STRPTR appName, STRPTR itemName );
struct NVInfo *GetNVInfo( void );
struct MinList *GetNVList( STRPTR appName );
BOOL SetNVProtection( STRPTR appName, STRPTR itemName, long mask );

#pragma libcall NVBase GetCopyNV 1e 9802
#pragma libcall NVBase FreeNVData 24 801
#pragma libcall NVBase StoreNV 2a 0A9804
#pragma libcall NVBase DeleteNV 30 9802
#pragma libcall NVBase GetNVInfo 36 00
#pragma libcall NVBase GetNVList 3c 801
#pragma libcall NVBase SetNVProtection 42 29803

/* lowlevel.library crap */
ULONG ReadJoyPort( unsigned long port );
UBYTE GetLanguageSelection( void );
VOID SetLanguageSelection(long languageNum);
ULONG GetKey( void );
void QueryKeys( struct KeyQuery *queryArray, unsigned long bufferSize );
APTR AddKBInt( APTR intRoutine, APTR intData );
void RemKBInt( APTR intHandle );
ULONG SystemControlA( struct TagItem *tags );
ULONG SystemControl( unsigned long Tag1type, ... );
APTR AddTimerInt( APTR intRoutine, APTR intData );
void RemTimerInt( APTR intHandle );
void StopTimerInt( APTR intHandle );
void StartTimerInt( APTR intHandle, unsigned long timeInterval, long continuous );
ULONG ElapsedTime( struct EClockVal *context );
APTR AddVBlankInt( APTR intRoutine, APTR intData );
void RemVBlankInt( APTR intHandle );

#pragma libcall LowLevelBase ReadJoyPort 1e 001
#pragma libcall LowLevelBase GetLanguageSelection 24 00
#pragma libcall LowLevelBase SetLanguageSelection 2a 101
#pragma libcall LowLevelBase GetKey 30 00
#pragma libcall LowLevelBase QueryKeys 36 1802
#pragma libcall LowLevelBase AddKBInt 3c 9802
#pragma libcall LowLevelBase RemKBInt 42 901
#pragma libcall LowLevelBase SystemControlA 48 901
#pragma tagcall LowLevelBase SystemControl 48 901
#pragma libcall LowLevelBase AddTimerInt 4e 9802
#pragma libcall LowLevelBase RemTimerInt 54 901
#pragma libcall LowLevelBase StopTimerInt 5a 901
#pragma libcall LowLevelBase StartTimerInt 60 10903
#pragma libcall LowLevelBase ElapsedTime 66 801
#pragma libcall LowLevelBase AddVBlankInt 6c 9802
#pragma libcall LowLevelBase RemVBlankInt 72 901


/* debox.library crap */
BYTE CheckHeader(struct CompHeader *);
ULONG HeaderSize(struct CompHeader *);
void NextComp(struct CompHeader *, void *);
LONG DecompData(struct CompHeader *, void *, void *);
ULONG BMInfoSize(struct BMInfo *,struct CompHeader *, void *);
struct BMInfo *DecompBMInfo     (struct BMInfo *, struct CompHeader *, void *);
VOID FreeBMInfo (struct BMInfo *);
LONG DecompBitMap       (struct CompHeader *, void *, struct BitMap *, UBYTE *);
LONG MemSet(void *, BYTE, ULONG);
struct SuperView *CreateView(struct SuperView *, struct BitMap *,UWORD, UWORD, UWORD);
VOID DeleteView (struct SuperView *);
VOID CenterViewPort(struct View *,struct ViewPort *);
int CycleColors(struct BMInfo *, ULONG);
ULONG GetSpecialData    (ULONG, void * );
ULONG *GetBMInfoRGB32   (struct BMInfo *, UWORD, UWORD);
VOID FreeBMInfoRGB32    (ULONG *);
BOOL CycleRGB32 (struct BMInfo *,ULONG,ULONG *,ULONG *);

#pragma libcall DeBoxBase CheckHeader 30 801
#pragma libcall DeBoxBase HeaderSize 36 801
#pragma libcall DeBoxBase NextComp 3C 9802
#pragma libcall DeBoxBase DecompData 42 A9803
#pragma libcall DeBoxBase BMInfoSize 48 9802
#pragma libcall DeBoxBase DecompBMInfo 4E A9803
#pragma libcall DeBoxBase FreeBMInfo 54 801
#pragma libcall DeBoxBase DecompBitMap 5A BA9804
#pragma libcall DeBoxBase MemSet 60 10803
#pragma libcall DeBoxBase CreateView 72 2109805
#pragma libcall DeBoxBase DeleteView 78 801
#pragma libcall DeBoxBase CenterViewPort 7E 9802
#pragma libcall DeBoxBase CycleColors 84 0802
#pragma libcall DeBoxBase GetSpecialData 90 8002
#pragma libcall DeBoxBase GetBMInfoRGB32 96 10803
#pragma libcall DeBoxBase FreeBMInfoRGB32 9C 801
#pragma libcall DeBoxBase CycleRGB32 A2 A90804


/*****************************************************************************/


#endif /* CDUIBASE_H */
