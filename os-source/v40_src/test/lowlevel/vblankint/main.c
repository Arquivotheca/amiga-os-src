/****** CDGS/VBlankInt **********************************************************
*
*   NAME
*       VBlankInt -- test program for lowlevel.library/AddVBlankInt(),
*                and lowlevel.library/RemVBlankInt().
*
*   SYNOPSIS
*       VBlankInt
*
*   FUNCTION
*
*       Test program for lowlevel.library/AddVBlankInt() and
*       lowlevel.library/RemVBlankInt().
*
*       The program automatically terminates after one second.
*
*   INPUTS
*       None
*
*   RESULT
*       RETURN_OK (0)       -   success
*       RETURN_WARN (5)     -   warning
*       RETURN_ERROR (10)   -   error
*       RETURN_FAIL (20)    -   failure
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       Hopefully very few.
*
*   SEE ALSO
*
******************************************************************************
*
*/

/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>

#include <intuition/intuition.h>
#include <intuition/screens.h>

#include <devices/timer.h>

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/alib_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/lowlevel_pragmas.h>

/*
 * ANSI includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes
 */

#define MAIN

#include "vblankint.h"

/****** VBlankInt/main ******************************************
*
*   NAME
*       main -- main entry point
*
*   SYNOPSIS
*       main(argc,argv);
*
*       void main(int argc,char *argv[]);
*
*   FUNCTION
*       Main entry point.
*
*   INPUTS
*       argc -- argument count
*       argv -- argument value arrraay
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       If there are any, you can get to 'em from here!
*
*   SEE ALSO
*       shutdown()
*
******************************************************************************
*
*/
void main(int argc,char *argv[])
{

    BYTE error;

    /*
     * Open libraries
     */

    /* Open graphics.library */
    GfxBase=(struct GfxBase *) OpenLibrary("graphics.library",
        KICKSTART_VERSION);
    if (!GfxBase) {
        Printf("%s: Error opening graphics.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /* Open intuition.library */
    IntuitionBase=OpenLibrary("intuition.library",KICKSTART_VERSION);
    if (!IntuitionBase) {
        Printf("%s: Error opening intuition.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /* Open lowlevel.library */
    LowLevelBase=OpenLibrary("lowlevel.library",KICKSTART_VERSION);
    if (!LowLevelBase) {
        Printf("%s: Error opening lowlevel.library V%d\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /*
     * Open devices
     */

    /* Create timer.device reply port */
    timerPort=CreateMsgPort();
    if (!timerPort) {
        Printf("%s: Error creating timer.device reply port\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Create timer.device I/O request */
    timerRequest=CreateIORequest(timerPort,sizeof(struct timerequest));
    if (!timerRequest) {
        Printf("%s: Error creating timer.device I/O request\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Open timer.device microhertz timer */
    error=OpenDevice("timer.device",UNIT_MICROHZ,timerRequest,NULL);
    if (error) {
        Printf("%s: Error %ld opening timer.device\n",PROGRAM_NAME,
            (LONG) error);
        goodbye(RETURN_FAIL);
    }
    timerOpen=TRUE;

    /*
     * Open test environment
     */

    /* Open high-resolution non-interlaced screen in default mode */
    screen=OpenScreenTags(NULL,
        SA_Width, SCREEN_WIDTH,
        SA_Height, SCREEN_HEIGHT,
        SA_Depth, SCREEN_DEPTH,
        SA_DisplayID, SCREEN_DISPLAYID,
        SA_Title, PROGRAM_NAME,
        TAG_DONE);
    if (!screen) {
        Printf("%s: Error opening screen\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Compute vertical blank beam position threshold */
    vbeamThreshold=(GfxBase->DisplayFlags&NTSC)?
        VBEAMPOS_THRESHOLD_NTSC:VBEAMPOS_THRESHOLD_PAL;

    /*

    /*
     * Test vertical blank interrupt handler
     */

    /* Set-up timer.device I/O request to wait one second */
    timerRequest->tr_node.io_Command=TR_ADDREQUEST;
    timerRequest->tr_time.tv_secs=1;
    timerRequest->tr_time.tv_micro=0;

    /* Add vertical blank interrupt handler */
    vblankIntHandle=AddVBlankInt(vblankInterrupt,(APTR) VBLANKINT_COOKIE);
    if (!vblankIntHandle) {
        Printf("%s: Error adding vertical blank interrupt handler\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Wait one second */
    error=DoIO(timerRequest);

    /* Remove vertical blank interrupt handler */
    RemVBlankInt(vblankIntHandle);

    /* Check for error in timer.device I/O ... */
    if (error) {
        Printf("%s: Error %ld waiting with timer.device\n",PROGRAM_NAME,
            (LONG) error);
        goodbye(RETURN_FAIL);
    }

    /*
     * Diagnostics
     */

    /* Output total calls to vertical blank interrupt handler */
    printf("%s: %lu total calls to vertical blank interrupt handler\n",
        PROGRAM_NAME,vblankIntCall);

    /* If any bad data to vertical blank interrupt handler ... */
    if (vblankIntBadData) {
        /* Warning */
        Printf("%s: %lu calls to vertical blank interrupt handler with bad data\n",
            PROGRAM_NAME,vblankIntBadData);
    }

    /* If any out of thresholdcalls to vertical blank interrupt handler ... */
    if (vblankIntThreshold) {
        /* Warning */
        Printf("%s: %lu calls to vertical blank interrupt handler with vertical beam position >%ld\n",
            PROGRAM_NAME,vblankIntThreshold,vbeamThreshold);
    }

    /*
     * Termination
     */

    /* Exit */
    goodbye( (vblankIntBadData || vblankIntThreshold) ?RETURN_WARN:RETURN_OK);

}

/****** VBlankInt/goodbye ******************************************
*
*   NAME
*       goodbye -- terminate program
*
*   SYNOPSIS
*       goodbye(returnCode);
*
*       void goodbye(int returnCode);
*
*   FUNCTION
*       Terminate program.
*
*   INPUTS
*       returnCode -- return code (from dos/dos.h RETURN_*)
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       main()
*
******************************************************************************
*
*/
void goodbye(int returnCode)
{

    /*
     * Close test environment
     */

    /* Close screen */
    if (screen) {
        CloseScreen(screen);
    }

    /*
     * Close devices
     */

    /* Close timer.device */
    if (timerOpen) {
        CloseDevice(timerRequest);
    }

    /* Destroy timer.device I/O request */
    if (timerRequest) {
        DeleteIORequest(timerRequest);
    }

    /* Destroy timer.device reply port */
    if (timerPort) {
        DeleteMsgPort(timerPort);
    }

    /*
     * Close libraries
     */

    /* Close lowlevel.library */
    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
    }

    /* Close intuition.library */
    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }

    /* Close graphics.library */
    if (GfxBase) {
        CloseLibrary(GfxBase);
    }

    /*
     * Exit
     */

    /* Exit */
    exit(returnCode);

}
