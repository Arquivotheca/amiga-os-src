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

#include <utility/tagitem.h>

#include <dos/dos.h>

#include <cd/cd.h>

#include <devices/trackdisk.h>

#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <rexx/errors.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/alib_protos.h>
#include <clib/debug_protos.h>

/*
**  Local includes
*/

#include "simplerexx.h"
#include "cdtest.h"

/****** interrupt/changeInterruptCode ******************************************
*
*   NAME
*       changeInterruptCode - disk change interrupt code
*
*   SYNOPSIS
*       changeInterruptCode(intData);
*
*       void __asm __interrupt __saveds
*           changeInterruptCode(register __a1 APTR intData);
*
*   FUNCTION
*       Disk change interrupt code.
*
*   INPUTS
*       intData     -   interrupt data
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*       intData is compared against a magic cookie (COOKIE_CHANGEINT).
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

void __asm __interrupt __saveds changeInterruptCode(register __a1 APTR intData)
{

    /* Increment total call count */
    changeIntCall++;

    /* Verify interrupt data */
    if (intData!=(APTR) COOKIE_CHANGEINT) {
        changeIntBadData++;
    }

}

/****** interrupt/frameInterruptCode ******************************************
*
*   NAME
*       frameInterruptCode - frame interrupt code
*
*   SYNOPSIS
*       frameInterruptCode(intData);
*
*       void __asm __interrupt __saveds
*           frameInterruptCode(register __a1 APTR intData);
*
*   FUNCTION
*       Frame interrupt code.
*
*   INPUTS
*       intData     -   interrupt data
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*       intData is compared against a magic cookie (COOKIE_FRAMEINT).
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

void __asm __interrupt __saveds frameInterruptCode(register __a1 APTR intData)
{

    if (debugMode) {
        kprintf("frameInterruptCode: Entry\n");
    }

    /* Increment total call count */
    frameIntCall++;

    /* Verify interrupt data */
    if (intData!=(APTR) COOKIE_FRAMEINT) {
        frameIntBadData++;
    }

}

STRPTR getChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    struct options {
        STRPTR stem;
    } options;
    struct RDArgs *rdArgs;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Return change interrupt data */
    setStemVarInt(rxMsg,options.stem,"Call",changeIntCall);
    setStemVarInt(rxMsg,options.stem,"BadData",changeIntBadData);

    /* Return with no result */
    return(NULL);

}

STRPTR getFrameInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    struct options {
        STRPTR stem;
    } options;
    struct RDArgs *rdArgs;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

#ifdef DUMMY

    printf("getFrameInt\n");

#else

    /* Return frame interrupt data */
    setStemVarInt(rxMsg,options.stem,"Call",frameIntCall);
    setStemVarInt(rxMsg,options.stem,"BadData",frameIntBadData);

#endif /* DUMMY */

    /* Return with no result */
    return(NULL);

}
