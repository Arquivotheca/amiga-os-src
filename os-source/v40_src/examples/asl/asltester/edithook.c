
/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/


/****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/sghooks.h>
#include <devices/inputevent.h>
#include <utility/hooks.h>
#include <string.h>
#include <stdio.h>

#include <clib/utility_protos.h>

#include <pragmas/utility_pragmas.h>

#include "edithook.h"


/*****************************************************************************/


#define	RETURN	 (0x44)
#define	TAB	 (0x42)


/*****************************************************************************/


extern struct Library *UtilityBase;


/*****************************************************************************/


/* Check whether the buffer only contains a valid hex number... */
BOOL CheckHex(STRPTR buffer, ULONG *value)
{
char  ch;
ULONG num;
UWORD i;

    if ((buffer[0] == '0') && (buffer[1] == 'x'))
    {
        num = 0;
        i   = 2;
        while (buffer[i])
        {
            ch  = ToUpper(buffer[i]);
            num = num * 16;
            if ((ch >= 'A') && (ch <= 'F'))
                num += ch - 'A' + 10;
            else if ((ch >= '0') && (ch <= '9'))
                num += ch - '0';
            else
                return(FALSE);

            i++;
        }
        *value = num;
        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


/* Function used as a hexadecimal editing hook for a string gadget. Only
 * allows hex numbers to be entered in a string gadget. The current value of
 * the hex number is stored in the gadget's UserData field.
 */
ULONG __asm HexHook(register __a0 struct Hook *hook,
                    register __a2 struct SGWork *sgw,
                    register __a1 ULONG *msg)
{
BOOL  exiting = FALSE;
ULONG result;

    geta4();

    result = 0;
    if (*msg == SGH_KEY)
    {
        result = 1;
        switch (sgw->IEvent->ie_Code)
        {
            case TAB   :
            case RETURN: exiting = TRUE;
        }

        if (CheckHex(sgw->WorkBuffer, (ULONG *)&sgw->Gadget->UserData))
        {
            if (exiting)
            {
                sprintf(sgw->WorkBuffer,"0x%08lX",sgw->Gadget->UserData);
                sgw->NumChars  = strlen(sgw->WorkBuffer);
                sgw->BufferPos = 0;
            }
        }
        else
        {
            sgw->EditOp  = EO_BADFORMAT;
            sgw->Actions = SGA_BEEP;
        }
    }

    return (result);
}
