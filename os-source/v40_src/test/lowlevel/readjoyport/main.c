/****** CDGS/ReadJoyPort **********************************************
*
*   NAME
*       ReadJoyPort -- test program for lowlevel.library/ReadJoyPort()
*
*   SYNOPSIS
*       ReadJoyPort PORT/N,STICKY/S,DEBUG/S
*
*   FUNCTION
*       Test program for lowlevel.library/ReadJoyPort().
*
*       Continually displays controller type and status for
*       specified port.
*
*       To terminate this program, select the close gadget.
*
*   INPUTS
*       PORT                -   Port number (default is 0)
*       STICKY              -   Sticky (default is off)
*       DEBUG               -   debug mode
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

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
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
#include "readjoyport.h"

#include <local/WBCliArgs.h>
#include <local/calib_protos.h>
#include <local/mwlib_protos.h>

/****** ReadJoyPort/main ******************************************
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

    struct options {
        LONG *port;
        LONG sticky;
        LONG debug;
    } options;

    /*
     * Parse arguments
     */

    /* Parse arguments from Workbench, CLI, or GUI */
    memset(&options,0,sizeof(options));
    parseData=WBCliArgsTags(
        WC_Template, "PORT/N,STICKY/S,DEBUG/S",
        WC_Args, &options,
        WC_Startup, argv,
        WC_TrustStartup, !argc,
        WC_Project, TRUE);
    if (options.port) {
        portNumber=*options.port;
    }
    sticky=options.sticky;
    debugMode=options.debug;

    /*
     * Open libraries
     */

    /* Open graphics.library */
    GfxBase=OpenLibrary("graphics.library",KICKSTART_VERSION);
    if (!GfxBase) {
        Printf("%s: Error opening graphics.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }
    if (debugMode) {
        Printf("ReadJoyPort/main: graphics.library open\n");
    }

    /* Open intuition.library */
    IntuitionBase=OpenLibrary("intuition.library",KICKSTART_VERSION);
    if (!IntuitionBase) {
        Printf("%s: Error opening intuition.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }
    if (debugMode) {
        Printf("ReadJoyPort/main: intuition.library open\n");
    }

    /* Open gadtools.library */
    GadtoolsBase=OpenLibrary("gadtools.library",KICKSTART_VERSION);
    if (!GadtoolsBase) {
        Printf("%s: Error opening gadtools.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }
    if (debugMode) {
        Printf("ReadJoyPort/main: gadtools.library open\n");
    }

    /* Open lowlevel.library */
    LowLevelBase=OpenLibrary("lowlevel.library",KICKSTART_VERSION);
    if (!LowLevelBase) {
        Printf("%s: Error opening lowlevel.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }
    if (debugMode) {
        Printf("ReadJoyPort/main: lowlevel.library open\n");
    }

    /*
     * GUI
     */

    /* Open GUI */
    if (!guiOpen()) {
        Printf("%s: Error opening user interface\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }
    if (debugMode) {
        Printf("ReadJoyPort/main: GUI good\n");
    }

    /* Process GUI */
    guiLoop();
    if (debugMode) {
        Printf("ReadJoyPort/main: Event loop complete\n");
    }

    /*
     * Fall-through
     */

    if (debugMode) {
        Printf("ReadJoyPort/main: Debug mode\n");
    }
    goodbye(RETURN_OK);

}

/****** ReadJoyPort/goodbye ******************************************
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
     * Close GUI
     */

    /* Close GUI */
    guiClose();

    /*
     * Close libraries
     */

    /* Close lowlevel.library */
    if (LowLevelBase) {
        /* Reset ports 0 and 1 to autosense before closing */
        SetJoyPortAttrs(0,
            SJA_Type, SJA_TYPE_AUTOSENSE,
            TAG_DONE);
        SetJoyPortAttrs(1,
            SJA_Type, SJA_TYPE_AUTOSENSE,
            TAG_DONE);
        CloseLibrary(LowLevelBase);
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
     * Close command-line parsing
     */

    /* Close WBCliArgs() control structure */
    if (parseData) {
        FreeWBCli(parseData);
    }

    /*
     * Exit
     */

    /* Exit */
    exit(returnCode);

}
