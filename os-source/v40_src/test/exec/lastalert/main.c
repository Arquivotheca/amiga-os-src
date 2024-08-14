/****** exec/LastAlert **********************************************
*
*   NAME
*       LastAlert -- test program to display/set last alert
*
*   SYNOPSIS
*       LastAlert TIMEOUT/N/K
*
*   FUNCTION
*       Test program for lowlevel.library/SetJoyPortAttrs().
*
*   INPUTS
*       TIMEOUT             -   new timeout value
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
#include <exec/execbase.h>

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
#include "lastalert.h"

/****** LastAlert/main ******************************************
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
        LONG *timeout;
    } options;

    /*
     * Parse arguments
     */

    /* Parse arguments from Workbench, CLI, or GUI */
    memset(&options,0,sizeof(options));
    rdArgs=ReadArgs("TIMEOUT/N/K",(LONG *) &options,NULL);
    if (!rdArgs) {
        PrintFault(IoErr(),PROGRAM_NAME);
    }

    /*
     * Set timeout
     */
    if (options.timeout) {
        SysBase->LastAlert[3]=*options.timeout;
        Printf("%s: Alert timeout set to $%08lx\n",
            PROGRAM_NAME,*options.timeout);
    }

    /*
     * Output last alert values
     */

    Printf("LastAlert=\n");
    Printf("    $%08lx\n",SysBase->LastAlert[0]);
    Printf("    $%08lx\n",SysBase->LastAlert[1]);
    Printf("    $%08lx\n",SysBase->LastAlert[2]);
    Printf("    $%08lx (Timeout)\n",SysBase->LastAlert[3]);

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
