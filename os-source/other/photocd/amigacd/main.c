/*** main.c ******************************************************************
 *
 *  $Id: main.c,v 1.5 94/03/16 18:20:53 jjszucs Exp Locker: jjszucs $
 *
 *  Photo CD Player for Amiga CD
 *  Main Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:   main.c,v $
 * Revision 1.5  94/03/16  18:20:53  jjszucs
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
 * Revision 1.4  94/03/08  13:56:11  jjszucs
 * Reset on CD removal is now disabled by application. NoReboot
 * no longer necessary in startup script.
 *
 * freeanim.library is now used to terminate startup animation.
 * FreeAnim is no longer necessary in startup script.
 *
 * Revision 1.3  94/02/18  15:56:42  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 *
 * Revision 1.2  94/01/13  17:05:07  jjszucs
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
 * Revision 1.1  94/01/06  11:57:43  jjszucs
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

#include <dos/dos.h>

#include <libraries/lowlevel.h>

#include <libraries/photocd.h>
#include <libraries/photocdbase.h>

#include <devices/cd.h>
#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/photocd_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include <pragmas/photocd_pragmas.h>

/* ANSI includes */
#include <stdlib.h>
#include <string.h>

/* Local includes */
#include "global.h"
#include "image.h"
#include "interface.h"

/****************************************************************************
 *                                                                          *
 *  main() - entry point                                                    *
 *                                                                          *
 ****************************************************************************/

int main(int argc,char *argv[])
{

    struct appContext       thisAppContext; /* Application context */
    struct appContext       *appContext;    /* Application context pointer */

    struct cliArguments     cliArguments;   /* Parsed CLI arguments */

    static struct TagItem sysReqKill[] ={
        SCON_KillReq, TRUE,
        TAG_DONE
    };

    static struct TagItem cdConfig[] ={
        TAGCD_EJECTRESET, FALSE,
        TAG_DONE
    };

    /*
     *  Early initialization
     */

    /* Clear application context */
    memset(&thisAppContext,0,sizeof(thisAppContext));

    /* Initialize application context pointer */
    appContext=&thisAppContext;

    /* Find exec.library; this cannot fail if the system is running */
    SysBase=*pHardExecBase;

    /* Add rendevous port to system message port list */
    appContext->ac_RendevousPort.mp_Node.ln_Name=RENDEVOUS_NAME;
    AddPort((struct MsgPort *) appContext);

    /*
     *  Begin shutdown of startup animation
     */

    /* N.B.: This is not error-checked because freeanim.library
             is not present on some target systems (i.e., A1200/CD1200,
             A4000 w/SCSI CD-ROM, etc.) */
    appContext->ac_FreeAnimBase=
        OpenLibrary("freeanim.library", KICKSTART_VERSION);

    /*
     *  Open libraries
     */

    UtilityBase=OpenLibrary("utility.library",KICKSTART_VERSION);
    if (!UtilityBase) {
        fatalError(appContext,"Unable to open utility.library");
    }

    DOSBase=OpenLibrary("dos.library",KICKSTART_VERSION);
    if (!DOSBase) {
        fatalError(appContext,"Unable to open dos.library");
    }

    GfxBase=OpenLibrary("graphics.library",KICKSTART_VERSION);
    if (!GfxBase) {
        fatalError(appContext,"Unable to open graphics.library");
    }

    LayersBase=OpenLibrary("layers.library",KICKSTART_VERSION);
    if (!LayersBase) {
        fatalError(appContext,"Unable to open layers.library");
    }

    SpecialFXBase=OpenLibrary("specialfx.library",KICKSTART_VERSION);
    if (!SpecialFXBase) {
        fatalError(appContext,"Unable to open specialfx.library");
    }

    IntuitionBase=OpenLibrary("intuition.library",KICKSTART_VERSION);
    if (!IntuitionBase) {
        fatalError(appContext,"Unable to open intuition.library");
    }

    LowLevelBase=OpenLibrary("lowlevel.library",KICKSTART_VERSION);
    if (!LowLevelBase) {
        fatalError(appContext,"Unable to open lowlevel.library");
    }

    NVBase=OpenLibrary("nonvolatile.library",KICKSTART_VERSION);
    if (!NVBase) {
        fatalError(appContext,"Unable to open nonvolatile.library");
    }

    DeBoxBase=OpenLibrary("debox.library",DEBOX_VERSION);
    if (!DeBoxBase) {
        fatalError(appContext,"Unable to open debox.library");
    }

    PhotoCDBase=
        (struct PhotoCDLibrary *) OpenLibrary("photocd.library",PHOTOCD_VERSION);
    if (!PhotoCDBase) {
        fatalError(appContext,"Unable to open photocd.library");
    }

    /*
     *  Open devices
     */

    /* Open cd.device */
    appContext->ac_CDPort=CreateMsgPort();
    if (!appContext->ac_CDPort) {
        fatalError(appContext,"Unable to create cd.device reply port");
    }

    appContext->ac_CDRequest=
        CreateIORequest(appContext->ac_CDPort,
            sizeof(*(appContext->ac_CDRequest)));
    if (!appContext->ac_CDRequest) {
        fatalError(appContext,"Unable to create cd.device I/O request");
    }

    if (OpenDevice("cd.device",0,
        (struct IORequest *) appContext->ac_CDRequest,NULL)!=0) {
        fatalError(appContext,"Unable to open cd.device unit 0");
    }
    appContext->ac_CDOpen=TRUE;

    /* Open timer.device */
    appContext->ac_TimerPort=CreateMsgPort();
    if (!appContext->ac_TimerPort) {
        fatalError(appContext, "Unable to create timer.device reply port");
    }

    appContext->ac_TimerRequest=
        CreateIORequest(appContext->ac_TimerPort,
            sizeof(*(appContext->ac_TimerRequest)));
    if (!appContext->ac_TimerRequest) {
        fatalError(appContext, "Unable to create timer.device I/O request");
    }

    if (OpenDevice("timer.device", UNIT_MICROHZ,
        (struct IORequest *) appContext->ac_TimerRequest,NULL)!=0) {
        fatalError(appContext, "Unable to open timer.device microhertz unit");
    }
    appContext->ac_TimerOpen=TRUE;

    /* Open input.device */
    appContext->ac_InputPort=CreateMsgPort();
    if (!appContext->ac_InputPort) {
        fatalError(appContext,"Unable to create input.device reply port");
    }

    appContext->ac_InputRequest=
        CreateIORequest(appContext->ac_CDPort,
            sizeof(*(appContext->ac_CDRequest)));
    if (!appContext->ac_InputRequest) {
        fatalError(appContext,"Unable to create input.device I/O request");
    }

    if (OpenDevice("input.device",0,
        (struct IORequest *) appContext->ac_InputRequest,NULL)!=0) {
        fatalError(appContext,"Unable to open input.device unit 0");
    }
    appContext->ac_InputOpen=TRUE;


    /*
     *  Parse command-line arguments, if any
     */

    memset(&cliArguments,0,sizeof(cliArguments));
    appContext->ac_RDArgs=ReadArgs(CLI_TEMPLATE, (LONG *) &cliArguments, NULL);
    if (!appContext->ac_RDArgs) {

        PrintFault(IoErr(), PROGRAM_NAME);
        fatalError(appContext, "Command line error");
    }
    appContext->ac_EnableExit=!cliArguments.noExit;

    /*
     *  Run application
     */

    /* Complete shutdown of startup animation */
    if (appContext->ac_FreeAnimBase) {

        CloseLibrary(appContext->ac_FreeAnimBase);

    }

    /* Install low-memory handler */
    appContext->ac_MemHandlerInt.is_Node.ln_Name=MEMHANDLER_NAME;
    appContext->ac_MemHandlerInt.is_Node.ln_Pri=MEMHANDLER_PRI;
    appContext->ac_MemHandlerInt.is_Data=(APTR) appContext;
    appContext->ac_MemHandlerInt.is_Code=(void (*)()) memHandlerCode;
    AddMemHandler(&appContext->ac_MemHandlerInt);
    appContext->ac_fMemHandlerInstall=TRUE;

    /* Allocate image buffer */
    appContext->ac_ImageBuffer=(ULONG *)
        AllocPCDImageBuffer(
            IMAGE_ALLOC_RES,
            PCD_Lines, IMAGE_ALLOC_LINES,
            TAG_DONE
        );
    if (!appContext->ac_ImageBuffer) {
        fatalError(appContext, "Error allocating image buffer\n");
    }

    /* Initialize semaphores */
    InitSemaphore(&appContext->ac_DisplaySemaphore);
    InitSemaphore(&appContext->ac_InterfaceSemaphore);

    /* Launch shimmer task */
    if (!launchShimmer(appContext)) {
        fatalError(appContext,"Error launching shimmer task");
    }

    /* Kill system requesters */
    SystemControlA(sysReqKill);
    appContext->ac_SysReqKill=TRUE;

    /* Disable reset on CD removal */
    appContext->ac_CDRequest->io_Command=CD_CONFIG;
    appContext->ac_CDRequest->io_Data=cdConfig;
    appContext->ac_CDRequest->io_Length=0;
    if (DoIO((struct IORequest *) appContext->ac_CDRequest)) {
        fatalError(appContext, "Error disabling reset on CD removal");
    }

    /* Initialize HAM8 module */
    initHAM8(appContext);

    /* Open display environment */
    if (openDisplay(appContext)) {

        /* Open user interface */
        if (openInterface(appContext)) {

            appContext->ac_State=as_Bad;
            /* If disc is Photo CD disc ... */
            if (IsPhotoCD()) {

                /* Begin in Thumbnail state */
                newState(appContext,as_Thumbnail);

            } else {

                /* Begin in No Disc state
                   N.B.: No Disc state is entered although a disc
                         (the Photo CD distribution disc) is in the
                         CD-ROM drive because this disc may not
                         necessarily be a Photo CD disc and
                         entering Invalid Disc state on application
                         startup is disruptive to the user. */

                newState(appContext,as_NoDisc);

            }

            /* Start event loop */
            eventLoop(appContext);

        } else {

            fatalError(appContext,"Unable to open interface");

        }

    } else {

        fatalError(appContext,"Unable to open display");

    }

    /*
     *  Fall-through
     */

    /* Success */
    goodbye(appContext,RETURN_OK);
    return RETURN_OK;

}

/****************************************************************************
 *                                                                          *
 *  goodbye() - terminate application                                       *
 *                                                                          *
 ****************************************************************************/

void goodbye(struct appContext *appContext, int returnCode)
{

    /* Tags to restore system requesters */
    static struct TagItem sysReqRestore[] ={
        SCON_KillReq, FALSE,
        TAG_DONE
    };

#ifdef DEBUG
    int i;
#endif /* DEBUG */
    /*
     *  Close application
     */

    /* Terminate shimmer task */
    killShimmer(appContext);

    /* Remove memory handler */
    if (appContext->ac_fMemHandlerInstall) {
        RemMemHandler(&appContext->ac_MemHandlerInt);
    }

    /* Terminate any open session */
    endSession(appContext);

    /* Close user interface */
    closeInterface(appContext);

    /* Close display environment */
    closeDisplay(appContext);

    /* Release image buffer */
    if (appContext->ac_ImageBuffer) {
        FreePCDImageBuffer((UBYTE *) appContext->ac_ImageBuffer);
    }

    /* Restore system requesters */
    if (appContext->ac_SysReqKill) {
        SystemControlA(sysReqRestore);
        appContext->ac_SysReqKill=FALSE;
    }

    /* Release dos.library/ReadArgs() control structure */
    if (appContext->ac_RDArgs) {
        FreeArgs(appContext->ac_RDArgs);
        appContext->ac_RDArgs=NULL;
    }

    /*
     *  Close devices
     */

    /* Close timer.device */
    if (appContext->ac_TimerOpen) {

        /* Abort any active timer request */
        if (appContext->ac_TimerActive) {
            AbortIO((struct IORequest *) appContext->ac_TimerRequest);
            WaitIO((struct IORequest *) appContext->ac_TimerRequest);
            appContext->ac_TimerActive=FALSE;
        }

        CloseDevice((struct IORequest *) appContext->ac_TimerRequest);
        appContext->ac_TimerOpen=FALSE;
    }

    if (appContext->ac_TimerRequest) {
        DeleteIORequest((struct IORequest *) appContext->ac_TimerRequest);
        appContext->ac_TimerRequest=NULL;
    }

    if (appContext->ac_TimerPort) {
        DeleteMsgPort(appContext->ac_TimerPort);
        appContext->ac_TimerPort=NULL;
    }

    /* Close input.device */
    if (appContext->ac_InputOpen) {

        CloseDevice((struct IORequest *) appContext->ac_InputRequest);
        appContext->ac_InputOpen=FALSE;

    }
    if (appContext->ac_InputRequest) {

        DeleteIORequest((struct IORequest *) appContext->ac_InputRequest);
        appContext->ac_InputRequest=NULL;

    }
    if (appContext->ac_InputPort) {

        DeleteMsgPort(appContext->ac_InputPort);
        appContext->ac_InputPort=NULL;

    }

    /* Close cd.device */
    if (appContext->ac_CDOpen) {
        CloseDevice((struct IORequest *) appContext->ac_CDRequest);
        appContext->ac_CDOpen=FALSE;
    }

    if (appContext->ac_CDRequest) {
        DeleteIORequest((struct IORequest *) appContext->ac_CDRequest);
        appContext->ac_CDRequest=NULL;
    }

    if (appContext->ac_CDPort) {
        DeleteMsgPort(appContext->ac_CDPort);
        appContext->ac_CDPort=NULL;
    }

    /*
     *  Close libraries
     */

    if (PhotoCDBase) {
        CloseLibrary(PhotoCDBase);
        PhotoCDBase=NULL;
    }

    if (DeBoxBase) {
        CloseLibrary(DeBoxBase);
        DeBoxBase=NULL;
    }

    if (NVBase) {
        CloseLibrary(NVBase);
        NVBase=NULL;
    }

    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
        LowLevelBase=NULL;
    }

    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
        IntuitionBase=NULL;
    }

    if (SpecialFXBase) {
        CloseLibrary(SpecialFXBase);
        SpecialFXBase=NULL;
    }

    if (LayersBase) {
        CloseLibrary(LayersBase);
        LayersBase=NULL;
    }

    if (GfxBase) {
        CloseLibrary((struct Library *) GfxBase);
        GfxBase=NULL;
    }

    if (DOSBase) {
        CloseLibrary(DOSBase);
        DOSBase=NULL;
    }

    if (UtilityBase) {
        CloseLibrary(UtilityBase);
        UtilityBase=NULL;
    }

    /*
     * De-initialize
     */

    /* Remove rendevous port */
    RemPort((struct MsgPort *) appContext);

#ifdef DEBUG

    /*
     *  Dump resources
     */
    kprintf("Resource dump:\n");
    kprintf("   ac_SysBase=$%08lx\n", appContext->ac_SysBase);
    kprintf("   ac_FreeAnimBase=$%08lx\n", appContext->ac_FreeAnimBase);
    kprintf("   ac_UtilityBase=$%08lx\n", appContext->ac_UtilityBase);
    kprintf("   ac_DOSBase=$%08lx\n", appContext->ac_DOSBase);
    kprintf("   ac_LayersBase=$%08lx\n", appContext->ac_LayersBase);
    kprintf("   ac_SpecialFXBase=$%08lx\n", appContext->ac_SpecialFXBase);
    kprintf("   ac_IntuitionBase=$%08lx\n", appContext->ac_LowLevelBase);
    kprintf("   ac_LowLevelBase=$%08lx\n", appContext->ac_LowLevelBase);
    kprintf("   ac_NVBase=$%08lx\n", appContext->ac_NVBase);
    kprintf("   ac_DeBoxBase=$%08lx\n", appContext->ac_DeBoxBase);
    kprintf("   ac_GfxBase=$%08lx\n", appContext->ac_GfxBase);
    kprintf("   ac_PhotoCDBase=$%08lx\n", appContext->ac_PhotoCDBase);
    kprintf("   ac_CDPort=$%08lx\n", appContext->ac_CDPort);
    kprintf("   ac_CDRequest=$%08lx\n", appContext->ac_CDRequest);
    kprintf("   ac_TimerPort=$%08lx\n", appContext->ac_TimerPort);
    kprintf("   ac_TimerRequest=$%08lx\n", appContext->ac_TimerRequest);
    kprintf("   ac_InputPort=$%08lx\n", appContext->ac_InputPort);
    kprintf("   ac_InputRequest=$%08lx\n", appContext->ac_InputRequest);
    kprintf("   ac_PhotoCDHandle=$%08lx\n", appContext->ac_PhotoCDHandle);
    kprintf("   ac_CDOpen=%s\n", appContext->ac_CDOpen?"TRUE":"FALSE");
    kprintf("   ac_TimerOpen=%s\n", appContext->ac_CDOpen?"TRUE":"FALSE");
    kprintf("   ac_TimerActive=%s\n", appContext->ac_TimerActive?"TRUE":"FALSE");
    kprintf("   ac_InputOpen=%s\n", appContext->ac_InputOpen?"TRUE":"FALSE");
    kprintf("   ac_SysReqKill=%s\n", appContext->ac_SysReqKill?"TRUE":"FALSE");
    kprintf("   ...\n");

    for (i=0; i<GLYPH_COUNT; i++) {

        if (appContext->ac_Glyphs[i]) {
            kprintf("   Glyph %ld not unloaded\n", i);
        }

    }

#endif /* DEBUG */
    /*
     *  Exit
     */

    /* Exit with return code */
    exit(returnCode);

}
