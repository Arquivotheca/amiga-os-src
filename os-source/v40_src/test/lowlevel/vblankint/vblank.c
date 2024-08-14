/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>

#include <intuition/intuition.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/alib_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

/*
 * ANSI includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Local includes
 */

#include "vblankint.h"

/****** vblankInt/vblankInterrupt ******************************************
*
*   NAME
*       vblankInterrupt -- vertical blank interrupt handler
*
*   SYNOPSIS
*       vblankInterrupt(intData);
*
*       void __asm __interrupt __saveds vblankInterrupt(register __a1 APTR intData);
*
*   FUNCTION
*       Vertical blank interrupt handler.
*
*   INPUTS
*       intData -- interrupt data
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

void __asm __interrupt __saveds vblankInterrupt(register __a1 APTR intData)
{

    /* Check vertical beam position */
    if (VBeamPos()>vbeamThreshold) {
        vblankIntThreshold++;
    }

    /* Check interrupt data */
    if (intData!=(APTR) VBLANKINT_COOKIE) {
        vblankIntBadData++;
    }

    /* Increment call count */
    vblankIntCall++;

}
