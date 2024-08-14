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

#include "simplerexx.h"
#include "cdtest.h"

STRPTR testStemVarInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    struct options {
        STRPTR stem;
        STRPTR member;
        LONG *value;
    } options;
    struct RDArgs *rdArgs;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A,MEMBER/A,VALUE/N/A",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Entry */
    if (debugMode) {
        printf("testStemVarInt: stem=%s, member=%s, value=%ld\n",
            options.stem,options.member,*options.value);
    }

    /* Set integer stem variable */
    setStemVarInt(rxMsg,options.stem,options.member,*options.value);

}
