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
#include <exec/io.h>

#include <dos/dos.h>
#include <dos/dostags.h>

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

#include "simplerexx.h"
#include "cdtest.h"

/****** cd/byteVerify ******************************************
*
*   NAME
*       byteVerify  -   verify byte pattern
*
*   SYNOPSIS
*       pass=byteVerify(buffer,length,byte);
*
*       BOOL byteVerify(UBYTE *buffer,ULONG length,UBYTE byte);
*
*   FUNCTION
*       Verify byte pattern.
*
*   INPUTS
*       buffer      -   buffer to verify
*       length      -   buffer length (bytes)
*       byte        -   byte value to test against
*
*   RESULT
*       pass        -   TRUE if pass; FALSE if error
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

BOOL byteVerify(UBYTE *buffer,ULONG length,UBYTE byte)
{

    UBYTE *thisByte;
    ULONG loop;

    /* Verify byte pattern */
    for (thisByte=buffer,loop=0;loop<length;thisByte++,loop++) {
        /* If no match ... */
        if (*thisByte!=byte) {
            /* Fail */
            return(FALSE);
        }
    }

    /* Pass */
    return(TRUE);

}

/****** cd/wordVerify ******************************************
*
*   NAME
*       wordVerify  -   verify word pattern
*
*   SYNOPSIS
*       pass=wordVerify(buffer,length,word);
*
*       BOOL wordVerify(UBYTE *buffer,ULONG length,UWORD word);
*
*   FUNCTION
*       Verify word pattern.
*
*   INPUTS
*       buffer      -   buffer to verify
*       length      -   buffer length (bytes)
*       byte        -   word value to test against
*
*   RESULT
*       pass        -   TRUE if pass; FALSE if error
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

BOOL wordVerify(UBYTE *buffer,ULONG length,UWORD word)
{

    UWORD *thisWord;
    ULONG loop;

    /* Verify byte pattern */
    for (thisWord=(UWORD *) buffer,loop=0;
         loop<length/sizeof(word);
         thisWord++,loop++) {
        /* If no match ... */
        if (*thisWord!=word) {
            /* Fail */
            return(FALSE);
        }
    }

    /* Pass */
    return(TRUE);

}

/****** cd/offsetVerify ******************************************
*
*   NAME
*       offsetVerify  -   verify offset pattern
*
*   SYNOPSIS
*       pass=offsetVerify(buffer,length,start);
*
*       BOOL byteVerify(UBYTE *buffer,ULONG length,ULONG start);
*
*   FUNCTION
*       Verify word pattern.
*
*   INPUTS
*       buffer      -   buffer to verify
*       length      -   buffer length (bytes)
*       start       -   starting value of pattern
*
*   RESULT
*       pass        -   TRUE if pass; FALSE if error
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

BOOL offsetVerify(UBYTE *buffer,ULONG length,ULONG start)
{

    ULONG *thisOffset;
    ULONG loop, value;

    /* Verify offset pattern */
    for (thisOffset=(ULONG *) buffer,loop=0,value=start;
         loop<length/sizeof(*thisOffset);
         thisOffset++,loop++,value+=sizeof(*thisOffset)) {
        /* If no match ... */
        if (*thisOffset!=value) {
            /* Fail */
            return(FALSE);
        }
    }

    /* Pass */
    return(TRUE);

}
