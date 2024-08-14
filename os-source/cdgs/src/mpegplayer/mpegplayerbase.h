#ifndef MPEGPLAYERBASE_H
#define MPEGPLAYERBASE_H


/*****************************************************************************/


#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <intuition/screens.h>
#include <graphics/text.h>
#include <cdtv/debox.h>
#include <libraries/cdgprefs.h>
#include <dos.h>

#include "defs.h"
#include "splines.h"


/*****************************************************************************/


#define DO_TEXTDISPLAY 0      // make this "1" to get credits and lyrics
#define CLEAN_EXIT     0      // make this "0" to remove the termination code
#define DOUBLE_BUFFER  1      // make this "0" to disable double-buffering


/*****************************************************************************/


/* Setting this to TRUE prevents Intuition from making any gfx sprite calls,
 * thus ensuring that it lays off any sprites we are using.
 */
#define IBASE_NUKEPOINTER(ibase)	  (*((char **)( ((char *)ibase) +0x68 )))


/*****************************************************************************/

/* for spline display */
#define MAXLINES     (30)
#define MAXPOINTS    (20)


#define NUM_COLORS 128
#define NUM_PLANES 7


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
    UWORD           cp_Pad;
};

struct Box
{
    WORD x[MAXPOINTS];
    WORD y[MAXPOINTS];
};

typedef void __asm (*DEPTHFUNC)(register __a0 struct Screen *,
                                register __d0 ULONG,
                                register __a1 ULONG,
                                register __a6 APTR);


struct MPEGPlayerLib
{
    struct Library       mp_Lib;
    UWORD                mp_Pad0;
    struct ExecBase     *mp_SysLib;
    struct GfxBase      *mp_GfxLib;
    struct Library      *mp_IntuitionLib;
    struct Library      *mp_DeBoxLib;
    struct Library      *mp_LowLevelLib;
    struct Library      *mp_VideoCDLib;
    struct Library      *mp_UtilityLib;
    struct Library      *mp_CDGLib;
    ULONG	         mp_SegList;
    DEPTHFUNC            mp_ScreenDepth;

    // Intuition stuff
    struct Screen       *mp_Screen;
    struct Window       *mp_Window;
    APTR                 mp_Pointer;
    struct RastPort     *mp_RenderRP;

    // our beloved background...
    struct ColorPack     mp_BgColors;
    struct BMInfo       *mp_BgBMInfo;
    struct BitMap       *mp_BgBitMap;

    // double-buffering stuff
    struct BitMap       *mp_DBBitMap[2];
    struct RastPort      mp_DBRastPort[2];
    struct ScreenBuffer *mp_DBBuffers[2];
    struct MsgPort      *mp_DBNotify;
    ULONG                mp_DBCurrentBuffer;

    struct BMInfo       *mp_IconBMInfo;
    struct BitMap       *mp_IconBitMap;

    struct BMInfo       *mp_ArrowBMInfo;
    struct BitMap       *mp_ArrowBitMap;

    struct TextFont      mp_Compact24;
    struct TextFont      mp_Compact31;

    struct Task         *mp_MainTask;       // task to ping
    LONG                 mp_RemovalSigBit;  // ping on disk removal/insertion
    LONG                 mp_TimerSigBit;
    struct timerequest  *mp_TimerReq;

    struct IOMPEGReq    *mp_MPEGReq;
    struct IOMPEGReq    *mp_MPEGReq2;
    struct IOStdReq     *mp_AudioReq;
    struct IOStdReq     *mp_AudioReq2;
    struct MsgPort      *mp_IOPort;
    APTR                 mp_Outstanding;
    ULONG                mp_LowSector;
    ULONG                mp_HighSector;
    enum MPEGChannels    mp_MPEGChannel;

    struct Task         *mp_ThumbnailTask;
    APTR                 mp_ThumbnailTrack;
    LONG		 mp_VideoCDType;

    // for play screen, when playing mpeg or audio
    struct Screen       *mp_PlayScreen;
    struct Window       *mp_PlayWindow;
    struct BitMap       *mp_DPFBitMap;
    struct RasInfo       mp_DPFRasInfo;
    struct RastPort      mp_DPFRastPort;
    ULONG                mp_LastTrack;
    ULONG                mp_LastOptions;
    ULONG                mp_LastTrackElapsed;
    ULONG                mp_LastTrackRemaining;
    ULONG                mp_LastDiskElapsed;
    ULONG                mp_LastDiskRemaining;
    ULONG                mp_IconCountdown;
    ULONG                mp_TimeCountdown;
    BOOL                 mp_TimeDisplay;
    UWORD                mp_Pad1;

    // for scrolling list of tracks
    struct BitMap       *mp_TrackBM;
    ULONG                mp_OldLeft;
    ULONG                mp_OldTop;
    ULONG                mp_TrackCount;
    BOOL                 mp_AudioLayout;
    WORD                 mp_TrackBMXOffset;
    WORD                 mp_TrackBMYOffset;
    BOOL                 mp_DrawArrows;

    // for scrolling list of credits
    struct BitMap       *mp_CreditsBM;
    ULONG                mp_CreditsBMHeight;
    LONG                 mp_OldCreditTop;

    // for text display
    struct MinList       mp_TextList;
    APTR                 mp_TextPool;
    APTR                 mp_TopNode;

    // for eventloop.c
    enum PageTypes       mp_CurrentPage;
    BOOL                 mp_RandomModeChanged;
    WORD                 mp_Pad3;

    struct CDGPrefs     *mp_CDGPrefs;
    APTR                 mp_CDGSprites;
    enum CDGStates       mp_CDGState;
    ULONG                mp_CDGChannel;

    // spliner stuff
    struct Task         *mp_SplinerTask;
    struct Context       mp_Spliner_Context;
    ULONG                mp_Spliner_seed;
    struct Box          *mp_Spliner_ptr;
    struct Box          *mp_Spliner_eptr;
    struct Box           mp_Spliner_store[MAXLINES];
    WORD                 mp_Spliner_screenWidth;
    WORD                 mp_Spliner_screenHeight;
    WORD                 mp_Spliner_maxPoints;
    WORD                 mp_Spliner_numlines;
    WORD                 mp_Spliner_mdelta;
    WORD                 mp_Spliner_maxlines;
    WORD                 mp_Spliner_dx[MAXPOINTS];
    WORD                 mp_Spliner_dy[MAXPOINTS];
    WORD                 mp_Spliner_ox[MAXPOINTS];
    WORD                 mp_Spliner_oy[MAXPOINTS];
    WORD                 mp_Spliner_nx[MAXPOINTS];
    WORD                 mp_Spliner_ny[MAXPOINTS];
    WORD                 mp_Spliner_dr;
    WORD                 mp_Spliner_dg;
    WORD                 mp_Spliner_db;
    WORD                 mp_Spliner_nr;
    WORD                 mp_Spliner_ng;
    WORD                 mp_Spliner_nb;
    WORD                 mp_Spliner_closed;
    char                 mp_Spliner_realfunc[14];
};

#define SysBase       MPEGPlayerBase->mp_SysLib
#define GfxBase       MPEGPlayerBase->mp_GfxLib
#define IntuitionBase MPEGPlayerBase->mp_IntuitionLib
#define DeBoxBase     MPEGPlayerBase->mp_DeBoxLib
#define LowLevelBase  MPEGPlayerBase->mp_LowLevelLib
#define VideoCDBase   MPEGPlayerBase->mp_VideoCDLib
#define CDGBase       MPEGPlayerBase->mp_CDGLib
#define UtilityBase   MPEGPlayerBase->mp_UtilityLib

#define ASM           __asm
#define REG(x)	      register __ ## x
#define BOOLEAN       UBYTE            /* byte boolean */

/* scan an exec list */
#define SCANLIST(l,n) for (n = (APTR)((struct MinList *)l)->mlh_Head;\
                      ((struct MinNode *)n)->mln_Succ;\
                      n = (APTR)((struct MinNode *)n)->mln_Succ)

#define SCANLISTREVERSE(l,n) for (n = (APTR)((struct MinList *)l)->mlh_TailPred;\
                             ((struct MinNode *)n)->mln_Pred;\
                             n = (APTR)((struct MinNode *)n)->mln_Pred)


/*****************************************************************************/


/* debox.library crap */
BYTE CheckHeader(struct CompHeader *);
ULONG HeaderSize(struct CompHeader *);
void NextComp(struct CompHeader *, void *);
LONG DecompData(struct CompHeader *, void *, void *);
ULONG BMInfoSize(struct BMInfo *,struct CompHeader *, void *);
struct BMInfo *DecompBMInfo	(struct BMInfo *, struct CompHeader *, void *);
VOID FreeBMInfo	(struct BMInfo *);
LONG DecompBitMap	(struct CompHeader *, void *, struct BitMap *, UBYTE *);
LONG MemSet(void *, BYTE, ULONG);
struct SuperView *CreateView(struct SuperView *, struct BitMap *,UWORD, UWORD, UWORD);
VOID DeleteView	(struct SuperView *);
VOID CenterViewPort(struct View *,struct ViewPort *);
int CycleColors(struct BMInfo *, ULONG);
ULONG GetSpecialData	(ULONG, void * );
ULONG *GetBMInfoRGB32	(struct BMInfo *, UWORD, UWORD);
VOID FreeBMInfoRGB32	(ULONG *);
BOOL CycleRGB32	(struct BMInfo *,ULONG,ULONG *,ULONG *);

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

/* cd32mpeg.device crap */
ULONG GetSector(APTR unit);
#pragma libcall CD32Base GetSector 36 801


/*****************************************************************************/


#endif /* MPEGPLAYERBASE_H */
