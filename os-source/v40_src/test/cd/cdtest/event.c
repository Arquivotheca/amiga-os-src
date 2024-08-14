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

/****** cd/eventLoop ******************************************
*
*   NAME
*       eventLoop   -   event loop
*
*   SYNOPSIS
*       eventLoop();
*
*       void eventLoop(void);
*
*   FUNCTION
*       Event loop.
*
*   INPUTS
*       None
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
*
******************************************************************************
*
*/

void eventLoop(void)
{

    ULONG signalFlags;
    struct RexxMsg *rxMsg;

    static struct commandEntry commandTable[] ={
        /* cd.device commands */
        "ADDCHANGEINT", cdAddChangeInt,
        "ADDFRAMEINT", cdAddFrameInt,
        "ATTENUATE", cdAttenuate,
        "CHANGENUM", cdChangeNum,
        "CHANGESTATE", cdChangeState,
        "CONFIG", cdConfig,
        "EJECT", cdEject,
        "GETGEOMETRY", cdGetGeometry,
        "INFO", cdInfo,
        "MOTOR", cdMotor,
        "PAUSE", cdPause,
        "PLAYLSN", cdPlayLSN,
        "PLAYMSF", cdPlayMSF,
        "PLAYTRACK", cdPlayTrack,
        "PROTSTATUS", cdProtStatus,
        "QCODELSN", cdQCodeLSN,
        "QCODEMSF", cdQCodeMSF,
        "READ", cdRead,
        "READXL", cdReadXL,
        "REMCHANGEINT", cdRemChangeInt,
        "REMFRAMEINT", cdRemFrameInt,
        "SEARCH", cdSearch,
        "SEEK", cdSeek,
        "TOCLSN", cdTOCLSN,
        "TOCMSF", cdTOCMSF,

        /* Support commands */
        "GETCHANGEINT", getChangeInt,
        "GETFRAMEINT", getFrameInt,

        /* Debugging commands */
        "SETSTEMVARINT", testStemVarInt,

        NULL, NULL
    };

    /* Infinite loop */
    FOREVER {

        /* Wait for ... */
        signalFlags=Wait(
            SIGBREAKF_CTRL_C            /* Break ... */
            |ARexxSignal(rexxContext)   /* ... or ARexx message */
        );

        /* If ARexx message ... */
        if (signalFlags&ARexxSignal(rexxContext)) {

            /* Process ARexx messages */
            while (rxMsg=GetARexxMsg(rexxContext)) {

                struct commandEntry *commandEntry;

                char commandBuffer[REXXCOMMAND_LENGTH];
                char *nextChar;
                STRPTR result;
                LONG error;

                /* Fetch command */
                nextChar=stptok(ARG0(rxMsg),commandBuffer,
                    REXXCOMMAND_LENGTH," ,");
                if (*nextChar) {
                    nextChar++;
                }

                /* If "QUIT" ... */
                if (stricmp(commandBuffer,"QUIT")==0) {
                    /* Reply to ARexx message */
                    ReplyARexxMsg(rexxContext,rxMsg,NULL,RC_OK);
                    /* Exit event loop */
                    return;
                }

                /* Look-up command */
                commandEntry=commandTable;
                while (commandEntry->command) {

                    if (stricmp(commandBuffer,commandEntry->command)==0) {

                        if (debugMode) {
                            printf("Command %s\n",commandBuffer);
                        }

                        /* Clear error value */
                        error=RC_OK;

                        /* Execute command code */
                        result=(commandEntry->function)(rxMsg,nextChar,&error);

                        /* Terminate look-up loop */
                        break;

                    }
                    commandEntry++;

                }

                /* If command not found ... */
                if (!commandEntry->command) {

                    /* Error */
                    if (debugMode) {
                        printf("Command %s not recognized\n",
                            commandBuffer);
                    }
                    error=RC_ERROR;
                    result=NULL;

                }

                /* Reply to ARexx message */
                ReplyARexxMsg(rexxContext,rxMsg,result,error);

            }

        }

        /* If break ... */
        if (signalFlags&SIGBREAKF_CTRL_C) {
            /* Return */
            return;
        }

    }

}
