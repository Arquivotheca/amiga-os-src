
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/sghooks.h>
#include <libraries/gadtools.h>
#include <devices/inputevent.h>
#include <utility/hooks.h>
#include <stdio.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "pe_custom.h"
#include "edithook.h"
#include "conversion.h"


/*****************************************************************************/


#define	RETURN	(0x44)
#define	TAB	(0x42)


/*****************************************************************************/


ULONG ASM FloatHook(REG(a0) struct Hook *hook,
                    REG(a2) struct SGWork *sgw,
                    REG(a1) ULONG *msg)
{
LONG  numdec  = (LONG)sgw->Gadget->UserData;
BOOL  exiting = FALSE;
LONG  tmplong;
LONG  tmpdec;
ULONG result;
UBYTE buffer[32];

    result = 0;
    if (*msg == SGH_KEY)
    {
        result = 1;
        switch (sgw->IEvent->ie_Code)
        {
            case TAB   :
            case RETURN: exiting = TRUE;
        }

        if (CheckNumber(sgw->WorkBuffer, &tmplong, &tmpdec, numdec, exiting))
        {
            if (exiting)
            {
                sprintf(buffer,"%%ld.%%0%ldld",numdec);
                sprintf(sgw->WorkBuffer, buffer,tmplong,tmpdec);
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
