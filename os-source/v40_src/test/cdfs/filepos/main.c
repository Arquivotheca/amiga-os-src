/****** CDFS/FilePos ******************************************
*
*   NAME
*       FilePos - find physical location of file
*
*   SYNOPSIS
*       FilePos FILENAME/A
*
*   FUNCTION
*       Output physical location (block number) of file on
*       ISO-9660 CD-ROM disc.
*
*   INPUTS
*       FILENAME    -   name of file
*
*   RESULT
*       RETURN_OK (0)     -   Success
*       RETURN_ERROR (10) -   Error
*       RETURN_FAIL (20)  -   Failure
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
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

#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

/*
 * ANSI includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Constant definitions
 */

#define PROGRAM_NAME "FilePos"

#define CLI_TEMPLATE "FILENAME/A"

/****** FilePos/main ******************************************
*
*   NAME
*       main - main entry point
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
*       argc - argument count
*       argv - argument value array
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       If there are any, you can get to 'em from here.
*
*   SEE ALSO
*       goodbye()
*
******************************************************************************
*
*/

void main(int argc,char *argv[])
{

    struct RDArgs *rdArgs;
    struct options {
        STRPTR filename;
    } options;

    BPTR bptrFileLock;
    struct FileLock *fileLock;

    /* Parse arguments */
    memset(&options,0,sizeof(options));
    rdArgs=ReadArgs("FILENAME/A",(LONG *) &options,NULL);
    if (!rdArgs) {
        PrintFault(IoErr(),PROGRAM_NAME);
        exit(RETURN_ERROR);
    }

    /* Lock file */
    bptrFileLock=Lock(options.filename,ACCESS_READ);
    if (!bptrFileLock) {
        printf("Error locking %s\n",options.filename);
        FreeArgs(rdArgs);
        exit(RETURN_FAIL);
    }

    /* Convert BCPL-style lock to C-style lock */
    fileLock=(struct FileLock *) BADDR(bptrFileLock);

    /* Output position (in fl_Key field of FileLock) */
    Printf("%lu",fileLock->fl_Key);

    /* Unlock file */
    UnLock(bptrFileLock);

    /* Free argument parsing control structure */
    FreeArgs(rdArgs);

    /* Success */
    exit(RETURN_OK);

}
