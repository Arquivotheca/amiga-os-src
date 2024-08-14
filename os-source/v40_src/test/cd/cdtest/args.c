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
#include <ctype.h>
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

/****** cd/obtainArgs() ******************************************
*
*   NAME
*       obtainArgs  -   parse ARexx command arguments
*
*   SYNOPSIS
*       rdArgs=obtainArgs(string,template,options,optionsSize);
*
*       struct RDArgs *obtainArgs(STRPTR string,STRPTR template,
*           void *options,ULONG optionsSize);
*
*   FUNCTION
*       Parse ARexx command arguments.
*
*   INPUTS
*       string      -   argument string
*       template    -   command template (in dos.library/ReadArgs() format)
*       options     -   options array/structure (as for dos.library/ReadArgs())
*       optionsSize -   size of options array/structure
*
*   RESULT
*       rdArgs      -   dos.library/ReadArgs() control structure;
*                       NULL if error
*
*   EXAMPLE
*
*   NOTES
*       A new-line is appended to the input string by this function
*       if one is not already present. There must be sufficent space in the
*       string for this.
*
*   BUGS
*
*   SEE ALSO
*       releaseArgs()
*
******************************************************************************
*
*/

struct RDArgs *obtainArgs(STRPTR string,STRPTR template,
    void *options,ULONG optionsSize)
{

    struct RDArgs *rdArgs;

    /* Clear options array */
    memset(options,0,optionsSize);

    /* Append new-line to argument string */
    if (string[strlen(string)-1]!='\n') {
        strcat(string,"\n");
    }

    /* Allocate RDArgs control structure */
    rdArgs=AllocDosObjectTags(DOS_RDARGS,
        TAG_DONE, NULL);
    if (!rdArgs) {
        Printf("%s: Error allocating RDArgs control structure\n",
            PROGRAM_NAME);
        return(NULL);
    }

    /* Initialize RDArgs control structure */
    rdArgs->RDA_Source.CS_Buffer=string;            /* Source is string */
    rdArgs->RDA_Source.CS_Length=strlen(string);    /* Length is string length */
    rdArgs->RDA_Source.CS_CurChr=0L;                /* Current character is start */
    rdArgs->RDA_Buffer=NULL;                        /* Use dos.library-allocated buffer */
    rdArgs->RDA_BufSiz=0L;                          /* Same */
    rdArgs->RDA_ExtHelp=NULL;                       /* No extended help */
    rdArgs->RDA_Flags=RDAF_NOPROMPT;                /* No prompted input */

    /* Parse string */
    if (!ReadArgs(template,options,rdArgs)) {
        PrintFault(IoErr(),PROGRAM_NAME);
        FreeArgs(rdArgs);
        return(NULL);
    }

    /* Return RDArgs control structure */
    return(rdArgs);

}

/****** cd/releaseArgs() ******************************************
*
*   NAME
*       releaseArgs     -   release argument parsing control structure
*
*   SYNOPSIS
*       releaseArgs(rdArgs);
*
*       void releaseArgs(struct RDArgs *rdArgs);
*
*   FUNCTION
*       Release dos.library/ReadArgs() control structure returned by
*       obtainArgs().
*
*   INPUTS
*       rdArgs      -   dos.library/ReadArgs() control structure
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       This should be implemented as a macro.
*
*   SEE ALSO
*
******************************************************************************
*
*/

void releaseArgs(struct RDArgs *rdArgs)
{
    FreeArgs(rdArgs);
}

/****** cd/strToMSF ******************************************
*
*   NAME
*       strToMSF    -   convert string to MSF
*
*   SYNOPSIS
*       msf=strToMSF(string);
*
*       ULONG strToMSF(STRPTR string);
*
*   FUNCTION
*       Convert string (in mm:ss:ff format) to MSF value.
*
*   INPUTS
*       string -- string to convert
*
*   RESULT
*       msf -- MSF value
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

ULONG strToMSF(STRPTR string)
{

    ULONG minutes, seconds, frames;
    ULONG msf;

    /* Parse minutes, seconds, and frames */
    sscanf(string,"%ld:%ld:%ld",&minutes,&seconds,&frames);

    /* Pack MSF value */
    msf=(minutes<<16)|(seconds<<8)|frames;

    /* Return MSF value */
    return(msf);

}

/****** cd/setStemVarInt ******************************************
*
*   NAME
*       setStemVarInt -- set integer stem variable
*
*   SYNOPSIS
*       status=setStemVarInt(rxMsg,base,stem,value);
*
*       LONG setStemVarInt(struct RexxMsg *rxMsg,STRPTR base,
*           STRPTR stem,LONG value);
*
*   FUNCTION
*       Set integer stem variable.
*
*   INPUTS
*       rxMsg   -   valid ARexx message
*       base    -   base name of stem; do not include leading .
*       stem    -   stem
*       value   -   value to set
*
*   RESULT
*       status  -   0 if success; !=0 if failure
*
*   EXAMPLE
*       To set foo.bar.gak to 42, ignoring errors:
*
*           setStemVarInt(rxMsg,"foo","bar.gak",42);
*
*   NOTES
*       The base and stem are converted to all upper-case,
*       since ARexx requires all variable names to be in
*       upper-case.
*
*   BUGS
*
*   SEE ALSO
*
****************************************************************
*
*/

LONG setStemVarInt(struct RexxMsg *rxMsg,STRPTR base,STRPTR stem,LONG value)
{
    static UBYTE name[REXX_STEM_BUFFER];
    static UBYTE buffer[REXX_RESULT_BUFFER];

    LONG result;
    char *cp;

    /* Construct value string */
    bsprintf(buffer,"%ld",value);

    /* Construct stem variable name */
    bsprintf(name,"%s.%s",base,stem);

    /* Convert variable name to upper-case */
    for (cp=name;*cp;cp++) {
        if (islower(*cp)) {
            *cp=toupper(*cp);
        }
    }

    /* Set variable */
    result=SetRexxVar(rxMsg,name,buffer,strlen(buffer));

    /* Return result */
    return(result);

}

/****** cd/setStemVarIntArray ******************************************
*
*   NAME
*       setStemVarIntArray -- set integer stem array variable
*
*   SYNOPSIS
*       status=setStemVarIntArray(rxMsg,base,stem,index,value);
*
*       LONG setStemVarIntArray(struct RexxMsg *rxMsg,STRPTR base,
*           STRPTR stem,LONG index,LONG value);
*
*   FUNCTION
*       Set integer stem variable in array.
*
*   INPUTS
*       rxMsg   -   valid ARexx message
*       base    -   base name of stem; do not include leading .
*       stem    -   stem
*       index   -   index
*       value   -   value to set
*
*   RESULT
*       status  -   0 if success; !=0 if failure
*
*   EXAMPLE
*       To set foo.17.gak to 42, ignoring errors:
*
*           setStemVarIntArray(rxMsg,"foo",17,"gak",42);
*
*   NOTES
*       The base and stem are converted to all upper-case,
*       since ARexx requires all variable names to be in
*       upper-case.
*
*   BUGS
*
*   SEE ALSO
*
****************************************************************
*
*/

LONG setStemVarIntArray(struct RexxMsg *rxMsg,STRPTR base,LONG index,STRPTR stem,LONG value)
{
    static UBYTE name[REXX_STEM_BUFFER];
    static UBYTE buffer[REXX_RESULT_BUFFER];
    char *cp;

    LONG result;

    /* Construct value string */
    bsprintf(buffer,"%ld",value);

    /* Construct stem variable name */
    bsprintf(name,"%s.%ld.%s",base,index,stem);

    /* Convert variable name to upper-case */
    for (cp=name;*cp;cp++) {
        if (islower(*cp)) {
            *cp=toupper(*cp);
        }
    }

    /* Set variable */
    result=SetRexxVar(rxMsg,name,buffer,strlen(buffer));

    /* Return result */
    return(result);

}
