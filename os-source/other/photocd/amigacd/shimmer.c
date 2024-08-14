/*** shimmer.c ******************************************************************
 *
 *  $Id: shimmer.c,v 1.2 94/03/08 14:00:19 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Cursor Shimmer Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	shimmer.c,v $
 * Revision 1.2  94/03/08  14:00:19  jjszucs
 * Cursor glow parameters adjusted to eliminate pure black at
 * low end and momentary flash of green at high end. Glow rate
 * also slightly increased to improve visiblity of cursor.
 * 
 * Revision 1.1  94/02/18  15:57:31  jjszucs
 * Initial revision
 *
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* Amiga includes */
#include <exec/types.h>
#include <exec/io.h>

#include <devices/timer.h>

#include <graphics/gfx.h>
#include <graphics/view.h>

#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

/* Local includes */
#include "global.h"
#include "interface.h"

/****************************************************************************
 *                                                                          *
 *  Cursor shimmering definitions                                           *
 *                                                                          *
 ****************************************************************************/

#define SHIMMER_INTERVAL        1500            /* Cursor shimmer interval
                                                   (microseconds) */
#define SHIMMER_TASK            "PhotoCD Shimmer" /* Task name */
#define SHIMMER_PRIORITY        0               /* Priority of shimmer task */
#define SHIMMER_STACK           4096            /* Stack size */
#define SHIMMER_PEN             INTERFACE_PEN_CURSOR /* Pen to shimmer */
#define SHIMMER_R_MIN           0x07070707      /* Minimum shimmer value */
#define SHIMMER_G_MIN           0x07070707
#define SHIMMER_B_MIN           0x00000000
#define SHIMMER_R_MAX           0xFFFFFFFF      /* Maximum shimmer value */
#define SHIMMER_G_MAX           0xE5E5E5E5
#define SHIMMER_B_MAX           0x00000000
#define SHIMMER_R_STEP          0x04040404      /* Shimmer step */
#define SHIMMER_G_STEP          0x03030303
#define SHIMMER_B_STEP          0x00000000

/* SysBase and GfxBase macros cannot be used in this code */
#undef SysBase
#undef GfxBase

/****************************************************************************
 *                                                                          *
 *  shimmerCode() - shimmer task code                                       *
 *                                                                          *
 ****************************************************************************/

static void __saveds shimmerCode(void)
{

    struct Library *SysBase, *GfxBase;
    struct appContext *appContext=NULL;

    struct MsgPort *timerPort;
    struct timerequest *timerRequest;

    UBYTE ioError;

    BOOL run;
    ULONG signalFlags;

    ULONG colorLoad[5];

    /* Find exec.library */
    SysBase=*pHardExecBase;

    /* Open graphics.library */
    GfxBase=OpenLibrary("graphics.library",KICKSTART_VERSION);
    if (GfxBase) {

        /* Find rendevous port */
        appContext=(struct appContext *) FindPort(RENDEVOUS_NAME);
        if (appContext) {

            /* Allocate kill signal */
            appContext->ac_ShimmerKillSig=AllocSignal(-1);
            if (appContext->ac_ShimmerKillSig!=-1) {

                /* Create timer.device reply port */
                timerPort=CreateMsgPort();
                if (timerPort) {

                    /* Create timer.device I/O request */
                    timerRequest=CreateIORequest(timerPort, sizeof(*timerRequest));
                    if (timerRequest) {

                        /* Open timer.device microhertz timer */
                        ioError=OpenDevice("timer.device",UNIT_MICROHZ,timerRequest,NULL);
                        if (!ioError) {

                            /* Initialize shimmer color and step */
                            appContext->ac_ShimmerR=SHIMMER_R_MIN;
                            appContext->ac_ShimmerG=SHIMMER_G_MIN;
                            appContext->ac_ShimmerB=SHIMMER_B_MIN;
                            appContext->ac_ShimmerRStep=SHIMMER_R_STEP;
                            appContext->ac_ShimmerGStep=SHIMMER_G_STEP;
                            appContext->ac_ShimmerBStep=SHIMMER_B_STEP;

                            /* Initiate first delay */
                            timerRequest->tr_node.io_Command=TR_ADDREQUEST;
                            timerRequest->tr_node.io_Flags=NULL;
                            timerRequest->tr_time.tv_secs=0;
                            timerRequest->tr_time.tv_micro=SHIMMER_INTERVAL;
                            SendIO(timerRequest);

                            /* Loop until terminated */
                            run=TRUE;
                            while (run) {

                                /* Wait for timer event or kill signal */
                                signalFlags=Wait(
                                    (1<<appContext->ac_ShimmerKillSig) |
                                    (1<<timerPort->mp_SigBit)
                                );

                                /* If kill signal ... */
                                if (signalFlags&(1<<appContext->ac_ShimmerKillSig)) {

                                    /* Stop running */
                                    run=FALSE;

                                }

                                /* If timer event ... */
                                if (signalFlags&(1<<timerPort->mp_SigBit)) {

                                    /* Complete timer.device I/O request */
                                    WaitIO(timerRequest);

                                    /* Compute new shimmer value */
                                    appContext->ac_ShimmerR+=
                                        appContext->ac_ShimmerRStep;
                                    appContext->ac_ShimmerG+=
                                        appContext->ac_ShimmerGStep;
                                    appContext->ac_ShimmerB+=
                                        appContext->ac_ShimmerBStep;
                                    if (
                                        ( (appContext->ac_ShimmerR>=SHIMMER_R_MAX ||
                                           appContext->ac_ShimmerR<=SHIMMER_R_MIN) &&
                                            SHIMMER_R_MIN!=SHIMMER_R_MAX) ||
                                        ( (appContext->ac_ShimmerG>=SHIMMER_G_MAX ||
                                           appContext->ac_ShimmerG<=SHIMMER_G_MIN) &&
                                            SHIMMER_G_MIN!=SHIMMER_G_MAX) ||
                                        ( (appContext->ac_ShimmerB>=SHIMMER_B_MAX ||
                                           appContext->ac_ShimmerB<=SHIMMER_B_MIN) &&
                                            SHIMMER_B_MIN!=SHIMMER_B_MAX)
                                        ) {

                                        appContext->ac_ShimmerRStep=
                                            -appContext->ac_ShimmerRStep;
                                        appContext->ac_ShimmerGStep=
                                            -appContext->ac_ShimmerGStep;
                                        appContext->ac_ShimmerBStep=
                                            -appContext->ac_ShimmerBStep;

                                    }

                                    /* Create LoadRGB32() table */
                                    colorLoad[0]=RGBLoad(INTERFACE_PEN_CURSOR,1);
                                    colorLoad[1]=appContext->ac_ShimmerR;
                                    colorLoad[2]=appContext->ac_ShimmerG;
                                    colorLoad[3]=appContext->ac_ShimmerB;
                                    colorLoad[4]=0L;

                                    /* If in thumbnail state and display
                                       screen is open ... */
                                    ObtainSemaphoreShared(&appContext->ac_DisplaySemaphore);
                                    if (appContext->ac_State==as_Thumbnail &&
                                        appContext->ac_DisplayScreen) {

                                        /* Glow display screen */
                                        LoadRGB32(
                                            &appContext->ac_DisplayScreen->ViewPort,
                                            colorLoad);

                                    } else {

                                        /* Wait for top of frame */
                                        /* N.B.: This is a easy method to
                                                 roughly approximate the
                                                 delay caused by the above
                                                 LoadRGB32() call. This is
                                                 necessary because without
                                                 this delay, the glow is
                                                 significantly faster when
                                                 the display screen is not
                                                 included. */
                                        WaitTOF();

                                    }
                                    ReleaseSemaphore(&appContext->ac_DisplaySemaphore);

                                    /* If interface screen is open ... */
                                    ObtainSemaphoreShared(&appContext->ac_InterfaceSemaphore);
                                    if (appContext->ac_InterfaceScreen) {

                                        /* Glow interface screen */
                                        LoadRGB32(
                                            &appContext->ac_InterfaceScreen->ViewPort,
                                            colorLoad);

                                    } else {

                                        /* Wait for top of frame */
                                        /* N.B.: This is a easy method to
                                                 roughly approximate the
                                                 delay caused by the above
                                                 LoadRGB32() call. This is
                                                 necessary because without
                                                 this delay, the glow is
                                                 significantly faster when
                                                 the display screen is not
                                                 included. */
                                        WaitTOF();

                                    }

                                    ReleaseSemaphore(&appContext->ac_InterfaceSemaphore);

                                    /* Send new timer request */
                                    timerRequest->tr_node.io_Command=TR_ADDREQUEST;
                                    timerRequest->tr_node.io_Flags=NULL;
                                    timerRequest->tr_time.tv_secs=0;
                                    timerRequest->tr_time.tv_micro=SHIMMER_INTERVAL;
                                    SendIO(timerRequest);

                                }

                            }

                            /* Abort timer request */
                            AbortIO(timerRequest);
                            WaitIO(timerRequest);

                            /* Close timer.device */
                            CloseDevice(timerRequest);

                        } else {

#ifdef DEBUG
                            kprintf("shimmerCode(): Error %ld opening timer.device UNIT_MICROHZ\n",ioError);
#endif /* DEBUG */

                        }

                        /* Delete timer.device I/O request */
                        DeleteIORequest(timerRequest);

                    } else {

#ifdef DEBUG
                        kprintf("shimmerCode(): Cannot create timer.device I/O request\n");
#endif /* DEBUG */

                    }

                    /* Delete timer.device reply port */
                    DeleteMsgPort(timerPort);

                } else {

#ifdef DEBUG
                    kprintf("shimmerCode(): Cannot create timer.device reply port\n");
#endif /* DEBUG */

                }

                /* Release kill signal */
                FreeSignal(appContext->ac_ShimmerKillSig);

            } else {

#ifdef DEBUG
                kprintf("shimmerCode(): Cannot allocate kill signal\n");
#endif /* DEBUG */

            }

        } else {
#ifdef DEBUG
            kprintf("shimmerCode(): Cannot find application rendevous port\n");
#endif /* DEBUG */
        }

    } else {

#ifdef DEBUG
        kprintf("shimmerCode(): Cannot open graphics.library V%ld\n",KICKSTART_VERSION);
#endif /* DEBUG */

    }

    /* Delete this task */
    Forbid();
    if (appContext) {
        appContext->ac_ShimmerTask=NULL;
    }
    DeleteTask(FindTask(NULL));
    Permit();

}

/* Restore definition of SysBase and GfxBase macros for other functions */
#define SysBase appContext->ac_SysBase
#define GfxBase appContext->ac_GfxBase

/****************************************************************************
 *                                                                          *
 *  launchShimmer() - launch shimmer task                                   *
 *                                                                          *
 ****************************************************************************/

BOOL launchShimmer(struct appContext *appContext)
{

    /* Create shimmer task */
    appContext->ac_ShimmerTask=customCreateTask(
        SHIMMER_TASK,
        SHIMMER_PRIORITY,
        shimmerCode,
        SHIMMER_STACK);

    /* Return */
    return (BOOL) (appContext->ac_ShimmerTask?TRUE:FALSE);

}

/****************************************************************************
 *                                                                          *
 *  killShimmer() - kill shimmer task                                       *
 *                                                                          *
 ****************************************************************************/

void killShimmer(struct appContext *appContext)
{


    /* If shimmer task is running ... */
    if (appContext->ac_ShimmerTask &&
        appContext->ac_ShimmerKillSig &&
        appContext->ac_ShimmerKillSig!=-1) {

        /* Signal shimmer task to terminate */
        Signal(appContext->ac_ShimmerTask, 1<<appContext->ac_ShimmerKillSig);

    }

}
