/*** global.h ****************************************************************
 *
 *  $Id: global.h,v 1.10 94/03/31 16:10:01 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Global Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   global.h,v $
 * Revision 1.10  94/03/31  16:10:01  jjszucs
 * o   Applying mirror manipulation to portrait-orientation images no longer
 *     causes inappropriate cropping.
 *
 * o   The Normalize glyph is now a 65x65 bitmap, like all other button glyphs.
 *     The glyph was 64x64 and strange lines when the glyph was displayed
 *     as a 65x65 mask was applied.
 *
 * o   The Zoom In and Zoom Out glyphs had black vertical line in the far
 *     right column. This has been eliminated.
 *
 * o   The interface panel is now made visible when the thumbnail state
 *     is initially entered at the start of a session.
 *
 * o   Changed fixed palette range from 16...240 to 0...255 for each
 *     color component (R/G/B). Although the clipped color component range
 *     was theoretically better for image quality, this change significantly
 *     reduces the complexity of the fixed palette code (which is called for
 *     all displayed pixels), increasing image display speed. The observed
 *     impact of the change on image quality is neglible.
 *
 * o   For NTSC systems, quick scaling was inadvertently being performed
 *     on the X-axis for portrait-orientation images. This increased
 *     the aspect ratio distortion, instead of decreasing the aspect
 *     ratio (as was the intent). This has been corrected, with significant
 *     code savings as a bonus.
 *
 * o   Center of zoom box now accurately corresponds to zoom cursor for
 *     all cases.
 *
 * o   Zoom center point is now at the center of the lenses of the magnifying
 *     glass image, not at the center of the entire image (which includes
 *     the handle).
 *
 * o   Zoom cursor bounds-checking is now correct for all cases.
 *
 * o   Zooming of portrait-orientation images now works correctly.
 *
 * o   Yet another attempt was made to implement a smooth scroll in the
 *     thumbnail display. However, due to the depth of the thumbnail screen
 *     (8 bitplanes), this results in a noticable "inchworm" effect. Since this
 *     is undesirable, jump scrolling continues to be used. This is being noted
 *     in this release note (and the associated RCS logs) primarily as a note
 *     to myself (and possibly others) once and for all that this is a
 *     Bad Idea(TM).
 *
 * o   Yet more changes to the "Easter Egg" text.
 *
 *
 * Revision 1.9  94/03/17  16:28:45  jjszucs
 * Changes per 40.13.
 *
 * Revision 1.8  94/03/16  18:21:49  jjszucs
 * Zoom manipulation implemented.
 *
 * Fixed palette image display implemented with
 * DISPLAY_FIXED_PALETTE conditional compilation
 * option.
 *
 * Pan X/Y offsets eliminated from image node
 * structure, no longer needed as pan is not
 * necessary.
 *
 * All image manipulation features correctly mark
 * images as manipulated. Normalize button is
 * now correctly enabled/disabled depending on
 * state of image.
 *
 * Control list cleared during all state transitions.
 * Prevents potential crashes due to inappropriate
 * controls remaining available.
 *
 * Further changes to "Easter Egg" screen per davidj
 * and eric.
 *
 * Revision 1.7  94/03/09  17:11:58  jjszucs
 * Changes per version 40.11.
 *
 * Revision 1.6  94/03/08  14:07:24  jjszucs
 * Changes per 40.10.
 *
 * Revision 1.5  94/03/01  16:17:21  jjszucs
 * Changes to support special-casing of display modes,
 * quick scaling, busy bar updating during load, and
 * other 40.9 changes.
 *
 * Revision 1.4  94/02/18  15:59:02  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.3  94/01/13  17:08:52  jjszucs
 * Changes per version 40.3
 *
 * Revision 1.2  94/01/06  13:36:01  jjszucs
 * Changed Gun32() macro to replicate value in all
 * bytes of long-word; this is consistent with the
 * original intent of graphics.library/SetRGB32(), etc.
 * and works around a bug that caused poor color
 * resolution (4 bits per gun vs. 8) in the blue
 * component.
 *
 * Revision 1.1  94/01/06  12:00:05  jjszucs
 * Initial revision
 *
*/

#ifndef APP_GLOBAL_H
#define APP_GLOBAL_H

/****************************************************************************
 *                                                                          *
 *  Conditional Compilation Options                                         *
 *                                                                          *
 ****************************************************************************/

/* If DISPLAY_FIXED_PALETTE is defined, a fixed base palette is used for
   display screens. This significantly increases display performance
   (approximately 50% better), but has the disadvantage of significantly
   degrading color quality. */

#define DISPLAY_FIXED_PALETTE

/* If DISPLAY_QUICK_SCALE is defined, a quick scale is used for image
   display. This significantly increases display performance, but produces
   significantly lower display quality than the fully-interpolating scaling
   routine. */

#define DISPLAY_QUICK_SCALE

/* If THUMBNAIL_INCREMENTAL_SCROLL is defined, incremental scrolling is
   used in the thumbnail state. If THUMBNAIL_INCREMENTAL_SCROLL is not
   defined, a jump scroll is used. */
/*
#define THUMBNAIL_QUICK_SCALE
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif /* EXEC_LISTS_H */

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif /* EXEC_PORTS_H */

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif /* EXEC_SEMAPHORES_H */

#ifndef EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif /* EXEC_INTERRUPTS_H */

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif /* GRAPHICS_GFX_H */

#ifndef APP_GLYPHS_H
#include "glyphs.h"
#endif /* APP_GLYPHS_H */

#ifndef APP_QUANTIZATION_H
#include "quantization.h"
#endif /* APP_QUANTIZATION_H */

#ifndef APP_DISPLAY_H
#include "display.h"
#endif /* APP_DISPLAY_H */

#include <clib/debug_protos.h>

#undef printf
#define printf kprintf

/****************************************************************************
 *                                                                          *
 *  General Definitions                                                     *
 *                                                                          *
 ****************************************************************************/

#define PROGRAM_NAME        "PhotoCD"   /* Name of application */

#define KICKSTART_VERSION   40          /* Minimum version of
                                           Kickstart modules */
#define WORKBENCH_VERSION   40          /* Minimum version of
                                           Workbench modules */
#define PHOTOCD_VERSION     40          /* Minimum version of photocd.library */
#define DEBOX_VERSION       39          /* Minimum version of debox.library */

/****************************************************************************
 *                                                                          *
 *  Amiga OS-related macros and definitions                                 *
 *                                                                          *
 ****************************************************************************/

/* Generate packed 32-bit RGB value from R, G, and B components */
#define RGB(r,g,b)          ( ( (r) <<16) | ( (g) <<8) | (b) )

/* Generate 32-bit gun value from 8-bit gun value */
#define Gun32(gun)          ( ((gun)<<24) | ((gun)<<16) | ((gun)<<8) | gun )

/* LoadRGB32() table entry to specify number of entries and starting entry */
#define RGBLoad(start,nEntries) \
    ( (nEntries<<16) | start )

/* Compute number of colors from number of planes */
#define NumColors(nPlanes)  ((1<<(nPlanes))-1)

/* MinTerm for COPY operation */
#define MINTERM_COPY        0xC0

/* Dimensions of struct Rectangle */
#define RectWidth(rect) (rect.MaxX-rect.MinX+1)
#define RectHeight(rect) (rect.MaxY-rect.MinY+1)

/* Number of bytes per vertex for graphics.library area buffer */
#define AREA_BYTES_PER_VERTEX 5

/* Granularity of nonvolatile.library storage */
#define NONVOLATILE_GRANULARITY 10

/* Memory handler */
#define MEMHANDLER_NAME     "Photo CD"
#define MEMHANDLER_PRI      0

/****************************************************************************
 *                                                                          *
 *  Photo CD-related macros and definitions                                 *
 *                                                                          *
 ****************************************************************************/

/* Extract components from 32-bit packed value */
#define YfromYCC(ycc)       (((ycc)>>16)&0xFF)
#define C1fromYCC(ycc)      (((ycc>>>8)&0xFF)
#define C2fromYCC(ycc)      ((ycc)&0xFF)

#define RfromRGB(rgb)       (((rgb)>>16)&0xFF)
#define GfromRGB(rgb)       (((rgb)>>8)&0xFF)
#define BfromRGB(rgb)       ((rgb)&0xFF)

/* Indexed component manipulation */
#define RIndex 1
#define GIndex 2
#define BIndex 3

#define RinRGB(rgb)    (((UBYTE *) &rgb)[RIndex])
#define GinRGB(rgb)    (((UBYTE *) &rgb)[GIndex])
#define BinRGB(rgb)    (((UBYTE *) &rgb)[BIndex])

/****************************************************************************
 *                                                                          *
 *  Stupid compiler tricks                                                  *
 *                                                                          *
 ****************************************************************************/

#define Largest(type)   ((1<<sizeof(type)*8)-1) /* Largest value for type */

#define BitTest(value,bit)  (((value)&(1<<(bit)))>>(bit)) /* Bit test */

#define difference(x,y)     ((x<y)?(y-x):(x-y)) /* Magnitude difference
                                                   between two values */

/****************************************************************************
 *                                                                          *
 *  State definitions                                                       *
 *                                                                          *
 ****************************************************************************/

enum appState {

    as_Bad=0,           /* Bad */
    as_NoDisc,          /* No disc */
    as_InvalidDisc,     /* Invalid disc */
    as_Thumbnail,       /* Thumbnail */
    as_Image,           /* Image */

};

/****************************************************************************
 *                                                                          *
 *  Application context                                                     *
 *                                                                          *
 ****************************************************************************/

#define RENDEVOUS_NAME          "PhotoCD Rendevous" /* Name of rendevous
                                                       message port */

struct appContext {

    struct MsgPort              ac_RendevousPort;   /* Message port; used for
                                                       locating main task */

    /* Library bases */
    struct Library *            ac_SysBase,         /* exec.library base */
                   *            ac_FreeAnimBase,    /* freeanim.library base */
                   *            ac_UtilityBase,     /* utility.library base */
                   *            ac_DOSBase,         /* dos.library base */
                   *            ac_LayersBase,      /* layers.library base */
                   *            ac_SpecialFXBase,   /* specialfx.library base */
                   *            ac_IntuitionBase,   /* intuition.library base */
                   *            ac_LowLevelBase,    /* lowlevel.library base */
                   *            ac_NVBase,          /* nonvolatile.library base */
                   *            ac_DeBoxBase;       /* debox.library base */
    struct GfxBase *            ac_GfxBase;         /* graphics.library base */
    struct PhotoCDLibrary *     ac_PhotoCDBase;     /* photocd.library base */

    /* Device resources */
    struct MsgPort *            ac_CDPort;          /* cd.device reply port */
    struct IOStdReq *           ac_CDRequest;       /* cd.device I/O request */

    struct MsgPort *            ac_TimerPort;       /* timer.device reply port */
    struct timerequest *        ac_TimerRequest;    /* timer.device I/O request */

    struct MsgPort *            ac_InputPort;       /* input.device reply port */
    struct IOStdReq *           ac_InputRequest;    /* input.device I/O request */

    /* Resource handles */
    void *                      ac_PhotoCDHandle;   /* Photo CD session handle */

    /* Resource tracking */
    BOOL                        ac_CDOpen;          /* cd.device open? */
    BOOL                        ac_TimerOpen;       /* timer.device open? */
    BOOL                        ac_TimerActive;     /* timer.device active? */
    BOOL                        ac_InputOpen;       /* input.device open? */
    BOOL                        ac_SysReqKill;      /* System requesters killed? */

    /* Display environment */
    struct Screen *             ac_DisplayScreen;   /* Display screen */
    struct Window *             ac_DisplayWindow;   /* Display window */
    enum appState               ac_DisplayState;    /* Display state */
    BOOL                        ac_DisplayPortrait; /* Display portrait? */
    ULONG                       ac_DisplayModeID;   /* Display mode ID (used
                                                       with graphics.library/
                                                       FindDisplayInfo()) */
    struct backgroundData *     ac_DisplayBkgd;     /* Display background */
    struct SignalSemaphore      ac_DisplaySemaphore;/* Display screen semaphore */
    ULONG *                     ac_ColorTable;      /* Color table generated
                                                       by quantizer */
    /* User interface */
    struct Screen *             ac_InterfaceScreen; /* Interface screen */
    struct Window *             ac_InterfaceWindow; /* Interface window */
    BOOL                        ac_InterfaceVisible;/* Interface visible? */
    struct SignalSemaphore      ac_InterfaceSemaphore;/* Interface screen semaphore */
    WORD *                      ac_InterfaceAreaBuf; /* Area buffer for interface window */
    struct AreaInfo *           ac_InterfaceAreaInfo;/* AreaInfo for interface window */
    PLANEPTR                    ac_InterfaceTmpPlane;/* Temporary raster plane for interface window */
    UWORD                       ac_InterfaceTmpPlaneWidth, /* Dimensions of ac_InterfaceTmpPlane for freeing */
                                ac_InterfaceTmpPlaneHeight;
    struct TmpRas *             ac_InterfaceTmpRas; /* TmpRas for interface window */

    struct controlItem *        ac_ControlArray;    /* Current control array */
    struct controlItem *        ac_ActiveControl;   /* Active control */

    /* Command line interface */
    struct RDArgs *             ac_RDArgs;          /* dos.library/ReadArgs()
                                                       control structure */
    BOOL                        ac_EnableExit;      /* TRUE to enable exit;
                                                       FALSE to disable exit */

    /* Glyphs */
    struct BitMap *             ac_Glyphs[GLYPH_COUNT]; /* Glyph bitmaps */
    UWORD                       ac_GlyphWidth[GLYPH_COUNT]; /* Glyph dimensions */
    UWORD                       ac_GlyphHeight[GLYPH_COUNT];

    /* State */
    enum appState               ac_State;           /* Current state */

    /* Slideshow */
    BOOL                        ac_SlideshowPlay;   /* Slideshow playing? */
    BOOL                        ac_SlideshowRepeat; /* Repeating slideshow? */
    BOOL                        ac_SlideshowPause;  /* Slideshow paused? */
    struct imageNode *          ac_SlideshowSelect; /* Selection before entering
                                                       slideshow */
    enum appState               ac_SlideshowState;  /* State before entering
                                                       slideshow */

    /* Images */
    STRPTR                      ac_DiscSerNo;       /* Disc serial number */
    UWORD                       ac_NImages;         /* Number of images */
    struct MinList              ac_ImageList;       /* List of images */
    UWORD                       ac_NSlides;         /* Number of slides */
    struct imageNode *          ac_Selection;       /* Image selection; NULL
                                                       is global */
    struct imageNode *          ac_Top;             /* First visible image;
                                                       NULL is global */
    ULONG *                     ac_ImageBuffer;     /* Image buffer */

    /* Quantization */
    UBYTE                       ac_GunDivTable[256];/* Gun division table */
    UBYTE                       ac_GunMulTable[     /* Gun multiplication table */
                                    FIXED_PALETTE_GUN_RANGE
                                ];
#ifndef DISPLAY_FIXED_PALETTE
    OCTREE                      ac_QuantTree;       /* Quantization octree */
#endif /* DISPLAY_FIXED_PALETTE */

    /* Shimmer */
    struct Task *               ac_ShimmerTask;     /* Shimmer task */
    BYTE                        ac_ShimmerKillSig;  /* Kill shimmer signal number */
    ULONG                       ac_ShimmerR,        /* Shimmer color value (32 bits per gun) */
                                ac_ShimmerG,
                                ac_ShimmerB;
    LONG                        ac_ShimmerRStep,    /* Shimmer step values (32-bit) */
                                ac_ShimmerGStep,
                                ac_ShimmerBStep;

    /* Zoom */
    WORD                        ac_ZoomX, ac_ZoomY; /* Zoom center X/Y */
    WORD                        ac_ZoomW, ac_ZoomH; /* Zoom dimensions */
    APTR                        ac_ZoomImage;       /* Zoom image object */
    WORD                        ac_ZoomPicX, ac_ZoomPicY; /* Zoom picture
                                                             offset X/Y */
    WORD                        ac_ZoomPicW, ac_ZoomPicH; /* Zoom picture
                                                             width/height */
    WORD                        ac_ZoomCropX, ac_ZoomCropY; /* Zoom crop
                                                               X/Y */
    WORD                        ac_ZoomImgW, ac_ZoomImgH;   /* Image width/height */

    /* Storage */
    struct storageNode *        ac_StorageData;     /* Nonvolatile storage data */

    /* Memory management */
    struct Interrupt            ac_MemHandlerInt;   /* Memory handler interrupt */
    BOOL                        ac_fMemHandlerInstall;/* Memory handler installed? */

    /* Miscellaneous */
    UWORD                       ac_HookTotal;       /* Total line count for
                                                       image load hook */
    UWORD                       ac_HookStart;       /* Starting line for image
                                                       load hook */
    BOOL                        ac_Credits;         /* In credits state? */

};

#define pHardExecBase   ((struct Library **) 0x00000004)
                        /* Absolute address of pointer to exec.library base */

/* Library base macros */
#define SysBase         (appContext->ac_SysBase)
#define UtilityBase     (appContext->ac_UtilityBase)
#define DOSBase         (appContext->ac_DOSBase)
#define GfxBase         (appContext->ac_GfxBase)
#define LayersBase      (appContext->ac_LayersBase)
#define SpecialFXBase   (appContext->ac_SpecialFXBase)
#define IntuitionBase   (appContext->ac_IntuitionBase)
#define LowLevelBase    (appContext->ac_LowLevelBase)
#define NVBase          (appContext->ac_NVBase)
#define DeBoxBase       (appContext->ac_DeBoxBase)
#define PhotoCDBase     (appContext->ac_PhotoCDBase)

/****************************************************************************
 *                                                                          *
 *  Structures                                                              *
 *                                                                          *
 ****************************************************************************/

struct backgroundData {

    struct Screen *bgd_Screen;      /* Screen */
    APTR        bgd_FXHandle;       /* FX handle */
    APTR        bgd_DisplayHandle;  /* Display handle */

};

/****************************************************************************
 *                                                                          *
 *  Prototypes                                                              *
 *                                                                          *
 ****************************************************************************/

/* from main.c */
int main(int argc,char *argv[]);
void goodbye(struct appContext *appContext,int returnCode);

/* from error.c */
void fatalError(struct appContext *appContext,STRPTR message);

/* from interface.c */
void CloseWindowSafely(struct appContext *appContext, struct Window *pWindow);
void DetachIDCMP(struct appContext *appContext, struct Window *pWindow);
void BlankWindowPointer(struct appContext *appContext, struct Window *pWindow);

BOOL openInterface(struct appContext *appContext);
void closeInterface(struct appContext *appContext);
void eventLoop(struct appContext *appContext);
void showInterface(struct appContext *appContext);
void hideInterface(struct appContext *appContext);

void interfaceToggle(struct appContext *appContext);

BOOL interfaceBusyStart(struct appContext *appContext, UWORD glyphID);
BOOL interfaceBusyEnd(struct appContext *appContext);
void interfaceBusyUpdate(struct appContext *appContext, UWORD total, UWORD current);

/* from display.c */
BOOL openDisplay(struct appContext *appContext);
void closeDisplay(struct appContext *appContext);
BOOL newDisplay(struct appContext *appContext, enum appState newState, BOOL newPortrait);
BOOL displayStateProperties(struct appContext *appContext, enum appState state,
    BOOL portrait, ULONG *pModeID, UWORD *pDepth, UWORD *pOverscan, BOOL *pFX);
BOOL newDisplayMode(struct appContext *appContext, ULONG modeID,
    UWORD depth, UWORD overscan, BOOL bkgdFX);
BOOL changeDisplay(struct appContext *appContext, enum appState newState, BOOL newPortrait);
struct backgroundData *installBackground(struct appContext *appContext,
    struct Screen *screen, UWORD gradientLines);
void removeBackground(struct appContext *appContext,
    struct backgroundData *bgData);

/* from state.c */
BOOL newState(struct appContext *context,enum appState newState);

/* from event.c */
BOOL rawkeyEvent(struct appContext *appContext, USHORT code, UWORD qualifier);
BOOL diskInsertEvent(struct appContext *appContext);
BOOL diskRemoveEvent(struct appContext *appContext);
BOOL timerEvent(struct appContext *appContext);
BOOL activeWindowEvent(struct appContext *appContext);

/* from status.c */
BOOL newStatus(struct appContext *appContext,enum appState newState);
BOOL updateStatus(struct appContext *appContext);

/* from control.c */
BOOL newControl(struct appContext *appContext,enum appState newState);

BOOL setControls(struct appContext *appContext,
    struct controlItem *pControlArray,struct controlItem *pActivate);
void clearControls(struct appContext *appContext);
BOOL refreshControls(struct appContext *appContext);

BOOL controlPrevious(struct appContext *appContext);
BOOL controlNext(struct appContext *appContext);
BOOL controlActivate(struct appContext *appContext);

BOOL controlChangeGlyph(struct appContext *appContext, UWORD controlID,
    UWORD glyphID);
BOOL controlSelect(struct appContext *appContext, UWORD controlID);
BOOL controlEnable(struct appContext *appContext, UWORD controlID, BOOL enable);

BOOL moveMouse(struct appContext *appContext, struct Screen *screen, WORD x, WORD y);

/* from glyph.c */
struct BitMap *loadGlyph(struct appContext *appContext,UWORD glyphID);
void unloadGlyph(struct appContext *appContext,UWORD glyphID);
BOOL putGlyph(struct appContext *appContext,UWORD glyphID,
    struct RastPort *rastport,UWORD x,UWORD y);
BOOL putGlyphMask(struct appContext *appContext, UWORD glyphID, UWORD maskID,
    struct RastPort *rastport,UWORD x,UWORD y);

/* from led.c */
int ipow(int base, int exponent);
BOOL clearLED(struct appContext *appContext, struct RastPort *rastport,
    UWORD x, UWORD y, USHORT nDigits, BOOL firstNarrow);
BOOL displayLEDNumber(struct appContext *appContext, struct RastPort *rastport,
    UWORD value, UWORD x, UWORD y, USHORT nDigits, BOOL firstNarrow);
BOOL displayLEDSymbol(
    struct appContext *appContext, struct RastPort *rastport,
    UBYTE symbol, UWORD x, UWORD y,
    BOOL visible);

/* from session.c */
BOOL beginSession(struct appContext *appContext);
void endSession(struct appContext *appContext);
struct imageNode *imageBySequence(struct appContext *appContext,UWORD seqNumber);

/* from thumbnail.c */
UBYTE *loadThumbnail(struct appContext *appContext,struct imageNode *imageNode);
void unloadThumbnail(struct appContext *appContext,struct imageNode *imageNode);
void lockThumbnail(struct appContext *appContext, struct imageNode *imageNode);
void unlockThumbnail(struct appContext *appContext, struct imageNode *imageNode);

void thumbnailGridSize(struct appContext *appContext, UWORD *pnRows, UWORD *pnColumns);
void thumbnailGridPosition(struct appContext *appContext, struct imageNode *imageNode,
    UWORD *pRow, UWORD *pColumn);
void thumbnailGridPlace(struct appContext *appContext,struct imageNode *imageNode,
    UWORD *pX, UWORD *pY);

BOOL putThumbnail(struct appContext *appContext, struct imageNode *imageNode,
    UWORD row, UWORD column);
BOOL putThumbnailFrame(struct appContext *appContext, struct imageNode *imageNode,
    UWORD row, UWORD column);
BOOL thumbnailCursor(struct appContext *appContext,struct imageNode *imageNode,
    BOOL show);
BOOL thumbnailScroll(struct appContext *appContext,struct imageNode *imageNode);

BOOL thumbnailSelect(struct appContext *appContext,struct imageNode *newSelection);
void thumbnailNextImage(struct appContext *appContext);
void thumbnailPreviousImage(struct appContext *appContext);
void thumbnailNextRow(struct appContext *appContext);
void thumbnailPreviousRow(struct appContext *appContext);
void thumbnailImage(struct appContext *appContext);
void thumbnailDo(struct appContext *appContext);
void thumbnailExit(struct appContext *appContext);
void thumbnailCancel(struct appContext *appContext);

void thumbnailControls(struct appContext *appContext);

/* from image.c */
BOOL displayImage(struct appContext *appContext,struct imageNode *imageNode);

BOOL imageSelect(struct appContext *appContext,struct imageNode *newSelection);

void imageNextImage(struct appContext *appContext);
void imagePreviousImage(struct appContext *appContext);
void imageThumbnail(struct appContext *appContext);

void imageControls(struct appContext *appContext);

/* from slide.c */
void slideDo(struct appContext *appContext);

void slideInclusion(struct appContext *appContext);
void slideSwapPrevious(struct appContext *appContext);
void slideSwapNext(struct appContext *appContext);
void slideCancel(struct appContext *appContext);
void slideNextImage(struct appContext *appContext);
void slidePreviousImage(struct appContext *appContext);

void slideIncludeAll(struct appContext *appContext);
void slideExcludeAll(struct appContext *appContext);

void slideSave(struct appContext *appContext);

/* from slideshow.c */
void slideshowPlay(struct appContext *appContext);
void slideshowRepeat(struct appContext *appContext);
void slideshowNext(struct appContext *appContext);
void slideshowPrevious(struct appContext *appContext);
void slideshowStop(struct appContext *appContext);

void slideshowShow(struct appContext *appContext, struct imageNode *imageNode);
void slideshowTimer(struct appContext *appContext);

/* from manipulate.c */
void manipulateDo(struct appContext *appContext);

void manipulateMirror(struct appContext *appContext);
void manipulateRotate(struct appContext *appContext);
void manipulateZoom(struct appContext *appContext);
void manipulateNormalize(struct appContext *appContext);
void manipulateCancel(struct appContext *appContext);

/* from ham8.c */
#ifdef DISPLAY_FIXED_PALETTE
UBYTE quantizeFixedPal(struct appContext *appContext, ULONG rgb);
ULONG fixedPalRGB(struct appContext *appContext, UBYTE pen);
#endif /* DISPLAY_FIXED_PALETTE */
UBYTE ham8Pick(struct appContext *appContext, ULONG thisRGB, ULONG *pLastRGB);
void initHAM8(struct appContext *appContext);

/* from shimmer.c */
BOOL launchShimmer(struct appContext *appContext);
void killShimmer(struct appContext *appContext);

/* from zoom.c */
void doZoomIn(struct appContext *appContext);

/* from nvstorage.c */
BOOL saveProgram(struct appContext *appContext);
BOOL loadProgram(struct appContext *appContext);
void unloadProgram(struct appContext *appContext);
BOOL programImage(struct appContext *appContext, struct imageNode *imageNode);

/* from task.c */
struct Task *customCreateTask(STRPTR name, LONG pri, APTR initPC, ULONG stackSize);

/* from memory.c */
UBYTE __asm __interrupt __saveds memHandlerCode(
    register __a0 struct MemHandlerData *pMemHandlerData,
    register __a1 struct appContext *appContext
);

/* from credits.c */
void showCredits(struct appContext *appContext);

#endif /* APP_GLOBAL_H */
