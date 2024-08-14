/*
**  cd.device Test
**  Written by John J. Szucs
**  Copyright © 1993 Commodore-Amiga, Inc.
**  All Rights Reserved
*/

/*
**  ANSI includes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
**  System includes
*/

#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/io.h>

#include <dos/dos.h>

#include <cd/cd.h>

#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <rexx/errors.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/alib_protos.h>

/*
**  Local includes
*/

#define MAIN

#include "simplerexx.h"
#include "cdtest.h"

/****** cd/main() ******************************************
*
*   NAME
*       main    -   entry point
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
*       argc    -   argument count
*       argv    -   argument value array
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
*       goodbye()
*
******************************************************************************
*
*/

void main(int argc,char *argv[])
{

    BYTE error;

    struct options {
        LONG debug;
        LONG noCD;
    } options;

    /*
     * Parse arguments
     */

    /* Parse command-line arguments */
    memset(&options,0,sizeof(options));
    rdArgs=ReadArgs("DEBUG/S,NOCD/S",(LONG *) &options,NULL);
    if (!rdArgs) {
        PrintFault(IoErr(),PROGRAM_NAME);
        goodbye(RETURN_ERROR);
    }

    /* Parse DEBUG */
    debugMode=options.debug;

    /* Parse NOCD */
    noCD=options.noCD;

    /*
     *  Open devices
     */

    /* Create cd.device reply port */
    cdPort=CreateMsgPort();
    if (!cdPort) {
        Printf("%s: Error creating cd.device reply port\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    /* Create cd.device I/O request */
    cdRequest=CreateIORequest(cdPort,sizeof(*cdRequest));
    if (!cdRequest) {
        Printf("%s: Error creating cd.device I/O request\n",PROGRAM_NAME);
        goodbye(RETURN_FAIL);
    }

    if (!noCD) {

        /* Open cd.device */
        error=OpenDevice("cd.device",0,cdRequest,NULL);
        if (error) {
            Printf("%s: Error %ld opening cd.device\n",PROGRAM_NAME,error);
            goodbye(RETURN_FAIL);
        }

    }

    /*
     *  Set-up ARexx interface
     */

    rexxContext=InitARexx(PROGRAM_NAME,REXXSCRIPT_EXTENSION);
    if (!rexxContext) {
        Printf("%s: Error creating ARexx interface\n",PROGRAM_NAME);
    }
    if (debugMode) {
        printf("Ready\n");
    }

    /*
     *  Event loop
     */

    /* Event loop */
    eventLoop();

    /*
     *  Fall-through
     */

    /* Success */
    goodbye(RETURN_OK);

}

/****** cd/goodbye ******************************************
*
*   NAME
*       goodbye     -   terminate program
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
*       returnCode  -   return code
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
     *  Close ARexx interface
     */

    /*  Free ARexx context */
    if (rexxContext) {
        FreeARexx(rexxContext);
    }

    /*
     *  Close devices
     */

    /* Close cd.device */
    if (cdOpen) {
        CloseDevice(cdRequest);
    }

    /* Delete cd.device I/O request */
    if (cdRequest) {
        DeleteIORequest(cdRequest);
    }

    /* Delete cd.device reply port */
    if (cdPort) {
        DeleteMsgPort(cdPort);
    }

    /*
     *  Close interface
     */

    /* Free dos.library/ReadArgs() control structure */
    if (rdArgs) {
        FreeArgs(rdArgs);
    }

    /*
     *  Exit
     */

    /* Exit with return code */
    exit(returnCode);

}
