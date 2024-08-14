/****** CDGS/SetJoyPortAttrs **********************************************
*
*   NAME
*       SetJoyPortAttrs -- test program for lowlevel.library/SetJoyPortAttrs()
*
*   SYNOPSIS
*       SetJoyPortAttrs PORT/N,REINITIALIZE/S
*
*   FUNCTION
*       Test program for lowlevel.library/SetJoyPortAttrs().
*
*   INPUTS
*       PORT                -   Port number (default is 0)
*       REINITIALIZE        -   Re-initialize port to defaults
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

#include <libraries/lowlevel.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
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
#include "setjoyportattrs.h"

/****** SetJoyPortAttrs/main ******************************************
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
        LONG reinitialize;
    } options;

    ULONG portNumber;

    /*
     * Parse arguments
     */

    /* Parse arguments from Workbench, CLI, or GUI */
    memset(&options,0,sizeof(options));
    rdArgs=ReadArgs("PORT/N/A,REINITIALIZE/S",(LONG *) &options,NULL);
    if (!rdArgs) {
        PrintFault(IoErr(),PROGRAM_NAME);
    }
    portNumber=*options.port;

    /*
     * Open libraries
     */

    /* Open lowlevel.library */
    LowLevelBase=OpenLibrary("lowlevel.library",KICKSTART_VERSION);
    if (!LowLevelBase) {
        Printf("%s: Error opening lowlevel.library V%ld\n",
            PROGRAM_NAME,KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    /*
     * Re-initialize
     */

    /* If REINITIALIZE ... */
    if (options.reinitialize) {
        /* Re-initialize port */
        SetJoyPortAttrs(portNumber,
            SJA_Reinitialize, TRUE,
            TAG_DONE);
        Printf("%s: Port %ld re-initialized\n",PROGRAM_NAME,portNumber);
    }

    /*
     * Fall-through
     */

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
     * Close libraries
     */

    /* Close lowlevel.library */
    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
    }

    /*
     * Close command-line parsing
     */

    /* Close dos.library/ReadArgs() control structure */
    if (rdArgs) {
        FreeArgs(rdArgs);
    }

    /*
     * Exit
     */

    /* Exit */
    exit(returnCode);

}
