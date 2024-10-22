/*** interface.c *************************************************************
 *
 *  $Id: interface.c,v 1.5 94/03/16 18:20:39 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  User Interface Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright � 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   interface.c,v $
 * Revision 1.5  94/03/16  18:20:39  jjszucs
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
 * Revision 1.4  94/03/09  17:07:51  jjszucs
 * Slideshow control panel implemented.
 *
 * Revision 1.3  94/02/18  15:56:11  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:04:57  jjszucs
 * Large portions of debugging output removed
 * Display and interface screen now opened with interleaved bitmaps
 *     to reduce artifacts during erasing
 * Potential Enforcer hit resulting from gaps in the quantization
 *     octree has been worked around
 * Now uses low-resolution non-interlaced HAM8 display for images.
 *     This produces good image quality, while minimizing the pixel
 *     processing and video RAM requirements.
 * Now scales images to adjust for aspect ratio and fit entire image
 *     on-screen. This corrects the aspect ratio distortions observed
 *     in Photo CD (Amiga CD) 40.1 and eliminates the need to scroll
 *     HAM8 displays.
 *
 * Revision 1.1  94/01/06  11:57:31  jjszucs
 * Initial revision
 *
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* ANSI includes */
#include <stdio.h>

/* Amiga includes */
#include <exec/types.h>
#include <exec/memory.h>

#include <libraries/lowlevel.h>

#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/modeid.h>
#include <graphics/layers.h>

#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* Local includes */
#include "global.h"
#include "interface.h"

/****************************************************************************
 *                                                                          *
 *  Local prototypes                                                        *
 *                                                                          *
 ****************************************************************************/

static ULONG __saveds __asm interfaceBackfill(register __a0 struct Hook *pHook,
    register __a2 APTR pObject, register __a1 APTR pMessage);

/****************************************************************************
 *                                                                          *
 *  Local constants                                                         *
 *                                                                          *
 ****************************************************************************/

#define INTERFACE_AREA_SIZE     64          /* Number of WORDs
                                               for area buffer */

/****************************************************************************
 *                                                                          *
 *  CloseWindowSafely() - close window with shared IDCMP                    *
 *                                                                          *
 ****************************************************************************/

void CloseWindowSafely(struct appContext *appContext, struct Window *pWindow)
{

    /* Detach IDCMP port for this window */
    DetachIDCMP(appContext, pWindow);

    /* Close window */
    CloseWindow(pWindow);

}

/****************************************************************************
 *                                                                          *
 *  BlankWindowPointer() - blank mouse pointer for a window                 *
 *                                                                          *
 ****************************************************************************/

void BlankWindowPointer(struct appContext *appContext, struct Window *pWindow)
{

    static chip UWORD blankPointer[] ={
        /* Posctl */
        0x0000, 0x0000,
        /* Plane 0 */
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        /* Plane 1 */
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        /* Reserved */
        0x0000, 0x0000
    };

#define BLANK_POINTER_WIDTH 16
#define BLANK_POINTER_HEIGHT 16
#define BLANK_POINTER_XOFFSET 0
#define BLANK_POINTER_YOFFSET 0

    /* Set pointer to transparent image */
    SetPointer(pWindow,
               blankPointer,
               BLANK_POINTER_WIDTH, BLANK_POINTER_HEIGHT,
               BLANK_POINTER_XOFFSET, BLANK_POINTER_YOFFSET);

}

/****************************************************************************
 *                                                                          *
 *  DetachIDCMP() - detach IDCMP port from window                           *
 *                                                                          *
 ****************************************************************************/

void DetachIDCMP(struct appContext *appContext, struct Window *pWindow)
{

    struct IntuiMessage *intuiMessage;
    struct Node *successor;

    /* If window has no IDCMP ... */
    if (!pWindow->UserPort) {

        /* No action */
        return;

    }

    /* Disable multitasking to prevent transmission of any further
       IDCMP events */
    Forbid();

    /* Remove pending messages for this window from IDCMP */
    intuiMessage=
        (struct IntuiMessage *) pWindow->UserPort->mp_MsgList.lh_Head;
    while (successor=intuiMessage->ExecMessage.mn_Node.ln_Succ) {

        if (intuiMessage->IDCMPWindow==pWindow) {

            /* Remove and reply to message */
            Remove(intuiMessage);
            ReplyMsg(intuiMessage);

        }

        intuiMessage=(struct IntuiMessage *) successor;

    }

    /* Clear IDCMP port to prevent port from being
       released by intuition.library */
    pWindow->UserPort=NULL;

    /* Clear IDCMP flags */
    ModifyIDCMP(pWindow,NULL);

    /* Re-enable multitasking */
    Permit();

}

/****************************************************************************
 *                                                                          *
 *  openInterface() - open user interface                                   *
 *                                                                          *
 ****************************************************************************/

BOOL openInterface(struct appContext *appContext)
{

    static struct Hook backfillHook;

    BOOL status;            /* Return status */

    /* Default pen array */
    UWORD pens[] ={
        Largest(UWORD), /* effectively ~0 */
    };

    /* SystemControlA() tag arrays to add IDCMP_RAWKEY handler */
    struct TagItem sysConRawkey0Tags[] ={
        SCON_AddCreateKeys, 0,
        TAG_DONE
    };
    struct TagItem sysConRawkey1Tags[] ={
        SCON_AddCreateKeys, 1,
        TAG_DONE
    };
    /* Palette loading table for graphics.library/LoadRGB32() */
    static ULONG paletteLoad[] ={
        RGBLoad(0,16),                          /* Load 16 entries beginning
                                                    at pen 0 */
                                                /* R, G, B values */
        Gun32(  0), Gun32(  0), Gun32(  0),     /* Background (pen 0) */
        Gun32(255), Gun32(255), Gun32(255),     /* Cursor (pen 1) */
        Gun32(130), Gun32(142), Gun32(145),     /* Scratch */
        Gun32(153), Gun32(136), Gun32(  0),     /* Scratch */
        Gun32(187), Gun32(170), Gun32(  0),     /* Scratch */
        Gun32(  0), Gun32(255), Gun32(  0),     /* LED green (pen 5) */
        Gun32(119), Gun32(119), Gun32(136),     /* Button background (pen 6) */
        Gun32(235), Gun32( 12), Gun32( 12),     /* Warning (pen 7) */
        Gun32(  0), Gun32(  0), Gun32( 17),     /* Grayscale */
        Gun32( 34), Gun32( 34), Gun32( 51),     /* Grayscale - LED segment (pen 9) */
        Gun32( 68), Gun32( 68), Gun32( 85),     /* Grayscale */
        Gun32(102), Gun32(104), Gun32(125),     /* Grayscale */
        Gun32(130), Gun32(142), Gun32(145),     /* Grayscale */
        Gun32(170), Gun32(170), Gun32(187),     /* Grayscale */
        Gun32(204), Gun32(204), Gun32(221),     /* Grayscale */
        Gun32(238), Gun32(238), Gun32(255),     /* Grayscale */
        0                                       /* Terminator */
    };

    DisplayInfoHandle       hDisplayInfo;
    struct DimensionInfo    dimensionInfo;

    /* Assume success */
    status=TRUE;

    /* Get dimensions of interface display mode */
    hDisplayInfo=FindDisplayInfo(INTERFACE_PANE_MODEID);
    if (hDisplayInfo) {

        if (GetDisplayInfoData(hDisplayInfo, (UBYTE *) &dimensionInfo,
            sizeof(dimensionInfo), DTAG_DIMS, NULL)) {

            /* Open interface screen as a child of display screen */
            ObtainSemaphore(&appContext->ac_InterfaceSemaphore);
            appContext->ac_InterfaceScreen=OpenScreenTags(NULL,
                SA_Title, NULL,
                SA_Colors32, paletteLoad,
                SA_Pens, &pens,
                SA_Top,
                    appContext->ac_DisplayScreen->Height-INTERFACE_PANE_OFFSETY,
                SA_Height, INTERFACE_PANE_OFFSETY,
                SA_Depth, INTERFACE_PANE_DEPTH,
                SA_DisplayID, INTERFACE_PANE_MODEID,
                SA_Overscan, OSCAN_STANDARD,
                SA_Interleaved, TRUE,
                SA_MinimizeISG, TRUE,
                SA_ShowTitle, FALSE,
                SA_Quiet, TRUE,
                TAG_DONE);
            ReleaseSemaphore(&appContext->ac_InterfaceSemaphore);
            if (appContext->ac_InterfaceScreen) {

                /* Open interface window */
                if (appContext->ac_InterfaceWindow=OpenWindowTags(NULL,
                    WA_CustomScreen, appContext->ac_InterfaceScreen,
                    WA_Title, NULL,
                    WA_NoCareRefresh, TRUE,
                    WA_Borderless, TRUE,
                    WA_Backdrop, TRUE,
                    WA_RMBTrap, TRUE,
                    WA_IDCMP,
                        IDCMP_RAWKEY|IDCMP_DISKINSERTED|IDCMP_DISKREMOVED|IDCMP_ACTIVEWINDOW,
                    WA_Activate, TRUE,
                    TAG_DONE)) {

                    /* Create area buffer */
                    if (appContext->ac_InterfaceAreaBuf=AllocVec(
                        sizeof(WORD)*INTERFACE_AREA_SIZE, MEMF_CLEAR)) {
                        /* N.B.: MEMF_CLEAR is required to clear the
                                 area buffer */

                        /* Create AreaInfo for interface */
                        if (appContext->ac_InterfaceAreaInfo=
                            AllocVec(sizeof(struct AreaInfo),NULL)) {

                            InitArea(appContext->ac_InterfaceAreaInfo,
                                appContext->ac_InterfaceAreaBuf,
                                (sizeof(WORD)*INTERFACE_AREA_SIZE)/AREA_BYTES_PER_VERTEX);

                            appContext->ac_InterfaceWindow->RPort->AreaInfo=
                                appContext->ac_InterfaceAreaInfo;

                            /* Create temporary raster plane */
                            appContext->ac_InterfaceTmpPlaneWidth=
                                appContext->ac_InterfaceWindow->Width;
                            appContext->ac_InterfaceTmpPlaneHeight=
                                appContext->ac_InterfaceWindow->Height;
                            if (appContext->ac_InterfaceTmpPlane=
                                AllocRaster(
                                    appContext->ac_InterfaceTmpPlaneWidth,
                                    appContext->ac_InterfaceTmpPlaneHeight)) {

                                /* Create TmpRas for interface */
                                if (appContext->ac_InterfaceTmpRas=
                                    AllocVec(sizeof(struct TmpRas),NULL)) {

                                    InitTmpRas(appContext->ac_InterfaceTmpRas,
                                        appContext->ac_InterfaceTmpPlane,
                                        RASSIZE(appContext->ac_InterfaceWindow->Width,
                                            appContext->ac_InterfaceWindow->Height));
                                    appContext->ac_InterfaceWindow->RPort->TmpRas=
                                        appContext->ac_InterfaceTmpRas;

                                    /* Set-up IDCMP_RAWKEY handler for gameport 0 */
                                    if (SystemControlA(sysConRawkey0Tags)==0) {

                                        /* Set-up IDCMP_RAWKEY handler for gameport 1 */
                                        if (SystemControlA(sysConRawkey1Tags)==0) {

                                            /* Kill mouse pointer */
                                            BlankWindowPointer(appContext,
                                            appContext->ac_InterfaceWindow);

                                            /* Display background imagery */
                                            putGlyph(
                                                appContext, GLYPH_PANEL_BKGD,
                                                appContext->ac_InterfaceWindow->RPort,
                                                0, 0);

                                            /* Install backfill hook */
                                            backfillHook.h_Entry=(HOOKFUNC) interfaceBackfill;
                                            backfillHook.h_SubEntry=NULL;
                                            backfillHook.h_Data=appContext;
                                            InstallLayerHook(
                                                appContext->ac_InterfaceWindow->WLayer,
                                                &backfillHook);

                                            /* If display window is already open ... */
                                            if (appContext->ac_DisplayWindow) {

                                                /* Interface window IDCMP port is shared
                                                    with display window */
                                                ModifyIDCMP(appContext->ac_DisplayWindow,
                                                    NULL);
                                                appContext->ac_DisplayWindow->UserPort=
                                                appContext->ac_InterfaceWindow->UserPort;
                                                ModifyIDCMP(appContext->ac_DisplayWindow,
                                                    IDCMP_RAWKEY|IDCMP_DISKINSERTED|IDCMP_DISKREMOVED|IDCMP_ACTIVEWINDOW);

                                            }

                                            /* User interface is now open */

                                        } else {

                                            status=FALSE;

                                        }

                                    } else {

                                        status=FALSE;

                                    }

                                } else {

#ifdef DEBUG
                                    kprintf("openInterface(): Error allocating TmpRas\n");
#endif /* DEBUG */
                                    status=FALSE;

                                }

                            } else {

#ifdef DEBUG
                                kprintf("openInterface(): Error allocating temporary plane\n");
#endif /* DEBUG */
                                status=FALSE;

                            }

                        } else {

#ifdef DEBUG
                            kprintf("openInterface(): Error allocating AreaInfo\n");
#endif /* DEBUG */
                            status=FALSE;

                        }

                    } else {

#ifdef DEBUG
                        kprintf("openInterface(): Error allocating area buffer\n");
#endif /* DEBUG */
                        status=FALSE;

                    }

                } else {

                    status=FALSE;

                }

            } else {

                status=FALSE;

            }

        }

    }

    /* Return status */
    return status;

}

/****************************************************************************
 *                                                                          *
 *  closeInterface() - close user interface                                 *
 *                                                                          *
 ****************************************************************************/

void closeInterface(struct appContext *appContext)
{

    UWORD glyphID;

    struct TagItem sysConRemRawkeyTags[] ={
        SCON_RemCreateKeys, 0,
        SCON_RemCreateKeys, 1,
        TAG_DONE
    };

    /* If display window is open and sharing this IDCMP port ... */
    if (appContext->ac_DisplayWindow &&
        appContext->ac_DisplayWindow->UserPort) {

        /* Detach display window from this IDCMP port */
        DetachIDCMP(appContext, appContext->ac_DisplayWindow);

    }

    /* Close interface window */
    if (appContext->ac_InterfaceWindow) {

        CloseWindow(appContext->ac_InterfaceWindow);
        appContext->ac_InterfaceWindow=NULL;

        SystemControlA(sysConRemRawkeyTags);

    }

    /* Dispose of pointer object */
    if (appContext->ac_ZoomImage) {
        DisposeObject(appContext->ac_ZoomImage);
        appContext->ac_ZoomImage=NULL;
    }

    /* Release area buffer */
    if (appContext->ac_InterfaceAreaBuf) {

        FreeVec(appContext->ac_InterfaceAreaBuf);
        appContext->ac_InterfaceAreaBuf=NULL;

    }

    /* Release AreaInfo */
    if (appContext->ac_InterfaceAreaInfo) {

        FreeVec(appContext->ac_InterfaceAreaInfo);
        appContext->ac_InterfaceAreaInfo=NULL;

    }

    /* Release temporary raster plane */
    if (appContext->ac_InterfaceTmpPlane) {

        FreeRaster(appContext->ac_InterfaceTmpPlane,
            appContext->ac_InterfaceTmpPlaneWidth,
            appContext->ac_InterfaceTmpPlaneHeight);
        appContext->ac_InterfaceTmpPlane=NULL;

    }

    /* Release TmpRas */
    if (appContext->ac_InterfaceTmpRas) {

        FreeVec(appContext->ac_InterfaceTmpRas);
        appContext->ac_InterfaceTmpRas=NULL;

    }

    /* Close interface screen with protection from race condition with
       shimmer task */
    if (appContext->ac_InterfaceScreen) {

        ObtainSemaphore(&appContext->ac_InterfaceSemaphore);
        CloseScreen(appContext->ac_InterfaceScreen);
        appContext->ac_InterfaceScreen=NULL;
        ReleaseSemaphore(&appContext->ac_InterfaceSemaphore);

    }

    /* Unload glyphs */
    for (glyphID=0;glyphID<GLYPH_COUNT;glyphID++) {
        unloadGlyph(appContext,glyphID);
    }

}

/****************************************************************************
 *                                                                          *
 *  eventLoop() - user interface event loop                                 *
 *                                                                          *
 ****************************************************************************/

void eventLoop(struct appContext *appContext)
{

    BOOL loopFlag=TRUE; /* Loop continuation flag */
    ULONG signalMask, signalFlags;

    struct IntuiMessage *intuiMessage;
    ULONG class;
    USHORT code;
    UWORD qualifier;

    /* Compute signal mask */
    signalMask=
        (1<<appContext->ac_InterfaceWindow->UserPort->mp_SigBit)|
        (1<<appContext->ac_TimerPort->mp_SigBit);

    /* While in event loop ... */
    while (loopFlag) {

        /* Wait for signal */
        signalFlags=Wait(signalMask);

        /* If interface window signal ... */
        if (signalFlags &
            (1<<appContext->ac_InterfaceWindow->UserPort->mp_SigBit)) {

            /* Get interface window IDCMP messages */
            while (intuiMessage=(struct IntuiMessage *)
                GetMsg(appContext->ac_InterfaceWindow->UserPort)) {

                /* Fetch relevant IDCMP message fields */
                class=intuiMessage->Class;
                code=intuiMessage->Code;
                qualifier=intuiMessage->Qualifier;

                /* Reply to IDCMP message */
                ReplyMsg(intuiMessage);

                /* Switch on class */
                switch (class) {

                    /* Raw keyboard event */
                    case IDCMP_RAWKEY:
                        rawkeyEvent(appContext, code, qualifier);
                        break;

                    /* Disk insertion */
                    case IDCMP_DISKINSERTED:
                        diskInsertEvent(appContext);
                        break;

                    /* Disk removal */
                    case IDCMP_DISKREMOVED:
                        diskRemoveEvent(appContext);
                        break;

                    /* Window activation */
                    case IDCMP_ACTIVEWINDOW:
                        activeWindowEvent(appContext);
                        break;

                    /* Other events */
                    default:

#ifdef DEBUG
                        printf("eventLoop(): Unrecognized IDCMP class $%08lx\n",class);
#endif /* DEBUG */
                        break;

                }

            }

        }

        /* If timer signal ... */
        if (signalFlags &
            (1<<appContext->ac_TimerPort->mp_SigBit)) {

            /* Handle timer event */
            timerEvent(appContext);

        }

    }

}

/****************************************************************************
 *                                                                          *
 *  showInterface() - show interface pane                                   *
 *                                                                          *
 ****************************************************************************/

void showInterface(struct appContext *appContext)
{

    /* Interface screen to front */
    ScreenToFront(appContext->ac_InterfaceScreen);

    /* Interface is visible */
    appContext->ac_InterfaceVisible=TRUE;

}

/****************************************************************************
 *                                                                          *
 *  hideInterface() - hide interface pane                                   *
 *                                                                          *
 ****************************************************************************/

void hideInterface(struct appContext *appContext)
{

    /* Interface screen to back */
    ScreenToBack(appContext->ac_InterfaceScreen);

    /* Interface is hidden */
    appContext->ac_InterfaceVisible=FALSE;

}

/****************************************************************************
 *                                                                          *
 *  interfaceToggle() - toggle interface pane                               *
 *                                                                          *
 ****************************************************************************/

void interfaceToggle(struct appContext *appContext)
{

    /* Toggle interface pane */
    if (appContext->ac_InterfaceVisible) {

        hideInterface(appContext);

    } else {

        showInterface(appContext);

    }

}

/****************************************************************************
 *                                                                          *
 *  interfaceBusyStart() - start interface busy period                      *
 *                                                                          *
 ****************************************************************************/

BOOL interfaceBusyStart(struct appContext *appContext, UWORD glyphID)
{

    /* Erase interface panel */
    EraseRect(
        appContext->ac_InterfaceWindow->RPort,
            0, 0,
            appContext->ac_InterfaceWindow->Width-1,
            appContext->ac_InterfaceWindow->Height-1);

    /* Move interface screen to front */
    ScreenToFront(appContext->ac_InterfaceScreen);

    /* Display busy status panel */
    if (putGlyph(appContext, GLYPH_BUSY,
            appContext->ac_InterfaceWindow->RPort,
            STATUS_PANEL_LEFT, STATUS_PANEL_TOP)) {

        /* Display busy glyph */
        return putGlyph(appContext, glyphID,
            appContext->ac_InterfaceWindow->RPort,
            BUSY_GLYPH_X, BUSY_GLYPH_Y);

    }

    return FALSE;

}

/****************************************************************************
 *                                                                          *
 *  interfaceBusyEnd() - end interface busy period                          *
 *                                                                          *
 ****************************************************************************/

BOOL interfaceBusyEnd(struct appContext *appContext)
{

    struct IntuiMessage *intuiMessage;

    /* Move interface screen to back if hidden */
    if (!appContext->ac_InterfaceVisible) {
        ScreenToBack(appContext->ac_InterfaceScreen);
    }

    /* Refresh controls */
    if (appContext->ac_ControlArray) {

        refreshControls(appContext);

    }

    /* Display status panel for state */
    newStatus(appContext, appContext->ac_State);

    /* Update status panel */
    updateStatus(appContext);

    /* Empty interface message queue */
    while (intuiMessage=
        (struct IntuiMessage *) GetMsg(appContext->ac_InterfaceWindow->UserPort)) {

        ReplyMsg(intuiMessage);

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  interfaceBusyUpdate() - update interface busy                           *
 *                                                                          *
 ****************************************************************************/

void interfaceBusyUpdate(struct appContext *appContext,
    UWORD total, UWORD current)
{

    struct RastPort *rastport;
    ULONG barWidth;

    /* Compute bar width */
    barWidth=(current*BUSY_BAR_WIDTH)/total;

    /* Render busy bar */
    rastport=appContext->ac_InterfaceWindow->RPort;
    SetAPen(rastport,INTERFACE_PEN_LED_LIGHT);
    SetOPen(rastport,INTERFACE_PEN_LED_LIGHT);
    RectFill(rastport,
        BUSY_BAR_X, BUSY_BAR_Y,
        BUSY_BAR_X+barWidth-1, BUSY_BAR_Y+BUSY_BAR_HEIGHT-1);

}

/****************************************************************************
 *                                                                          *
 *  interfaceBackfill() - interface backfill hook                           *
 *                                                                          *
 ****************************************************************************/

static ULONG __saveds __asm interfaceBackfill(register __a0 struct Hook *pHook,
    register __a2 APTR pObject, register __a1 APTR pMessage)
{

    struct appContext *appContext;

    static struct {
        struct Layer        *pLayer;
        struct Rectangle    bounds;
        LONG                offsetX, offsetY;
    } *pBackfillMsg;

    struct BitMap *pBitMap;

    /* Get application context */
    appContext=pHook->h_Data;

    /* Get message */
    pBackfillMsg=pMessage;

    /* Load panel background glyph */
    pBitMap=loadGlyph(appContext, GLYPH_PANEL_BKGD);
    if (pBitMap) {

        /* Backfill from background glyph */
        BltBitMap(
            pBitMap,
                pBackfillMsg->bounds.MinX, pBackfillMsg->bounds.MinY,
            appContext->ac_InterfaceWindow->RPort->BitMap,
                pBackfillMsg->bounds.MinX, pBackfillMsg->bounds.MinY,
            pBackfillMsg->bounds.MaxX-pBackfillMsg->bounds.MinX+1,
            pBackfillMsg->bounds.MaxY-pBackfillMsg->bounds.MinY+1,
            MINTERM_COPY,
            0xFF,
            NULL);

    }

    /* Done */
    return 0;

}
