/****** CDGS/KBInt **********************************************************
*
*   NAME
*       KBInt -- test program for lowlevel.library/AddKBInt(),
*                lowlevel.library/RemKBInt(), and lowlevel.library/GetKey()
*
*   SYNOPSIS
*       KBInt
*
*   FUNCTION
*       Test program for lowlevel.library/AddKBInt(), lowlevel.library/RemKBInt(),
*       and lowlevel.library/GetKey().
*
*       Sets up keyboard interrupt handler and displays all keyboard
*       activity.
*
*       To terminate this program, select the close gadget.
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
*       If keyboard interrupts occur faster than keyboard update
*       routine can display changes, some keyboard events will not
*       be displayed.
*
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

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>

#include <devices/inputevent.h>

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/keymap_protos.h>
#include <clib/lowlevel_protos.h>

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

#include "kbint.h"

/****** KBInt/main ******************************************
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

    /*
     * Open libraries
     */

    /* Open intuition.library */
    IntuitionBase=OpenLibrary("intuition.library",KICKSTART_VERSION);
    if (!IntuitionBase) {
        Printf("%s: Error opening intuition.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /* Open gadtool.library */
    GadtoolsBase=OpenLibrary("gadtools.library",KICKSTART_VERSION);
    if (!GadtoolsBase) {
        Printf("%s: Error opening gadtools.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /* Open keymap.library */
    KeymapBase=OpenLibrary("keymap.library",KEYMAP_VERSION);
    if (!KeymapBase) {
        Printf("%s: Error opening keymap.library V%ld\n",
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
     * Set-up keyboard interrupt handler
     */

    /* Allocate keyboard signal */
    kbSignal=AllocSignal(-1);
    if (!kbSignal) {
        Printf("%s: Error allocating keyboard signal\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Find this task */
    kbIntTask=FindTask(NULL);
    if (!kbIntTask) {
        Printf("%s: Error finding KBInt task\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Add keyboard interrupt handler */
    kbIntHandle=AddKBInt(kbInterrupt,(APTR) KBINT_COOKIE);
    if (!kbIntHandle) {
        Printf("%s: Error installing keyboard interrupt handler\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }
#ifdef DEBUG
    Printf("%s: Added keyboard interrupt handler $%08lx\n",
        PROGRAM_NAME,kbIntHandle);
#endif /* DEBUG */

    /*
     * GUI
     */

    /* Open GUI */
    if (!guiOpen()) {
        Printf("%s: Error opening user interface\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Process GUI */
    guiLoop();

    /*
     * Termination
     */

    /* If any bad data to keyboard interrupt handler ... */
    if (kbIntBadData) {
        /* Warning */
        Printf("%s: %lu calls to keyboard interrupt handler with bad data\n",
            PROGRAM_NAME,kbIntBadData);
    }

    /* Exit */
    goodbye(kbIntBadData?RETURN_WARN:RETURN_OK);

}

/****** KBInt/goodbye ******************************************
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
     * Remove keyboard interrupt handler
     */

    /* Free keyboard signal */
    if (kbSignal!=-1) {
        FreeSignal(kbSignal);
    }

    /* Remove keyboard interrupt handler */
    if (kbIntHandle) {
        RemKBInt(kbIntHandle);
    }

    /*
     * Close GUI
     */

    /* Close GUI */
    guiClose();

    /*
     * Close libraries
     */

    /* Close lowlevel.library */
    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
    }

    /* Close keymap.library */
    if (KeymapBase) {
        CloseLibrary(KeymapBase);
    }

    /* Close gadtools.library */
    if (GadtoolsBase) {
        CloseLibrary(GadtoolsBase);
    }

    /* Close intuition.library */
    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }

    /*
     * Exit
     */

    /* Exit */
    exit(returnCode);

}

