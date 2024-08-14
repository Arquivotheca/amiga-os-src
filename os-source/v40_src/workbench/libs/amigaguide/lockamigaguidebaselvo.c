/* lockamigaguidebaselvo.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/LockAmigaGuideBase **************************
*
*    NAME
*	LockAmigaGuideBase - Lock an AmigaGuide client.         (V34)
*
*    SYNOPSIS
*	key = LockAmigaGuideBase (AMIGAGUIDECONTEXT handle);
*				  a0
*
*	LONG LockAmigaGuideBase (AMIGAGUIDECONTEXT);
*
*    FUNCTION
*	This function is used to lock the AmigaGuide context handle
*	while working with data obtained with the the
*	GetAmigaGuideAttr() function.
*
*    INPUTS
*	handle - AMIGAGUIDECONTEXT handle obtained with
*	    OpenAmigaGuideAsync().
*
*    RETURNS
*	Returns a key to pass to UnlockAmigaGuideBase().
*
*    SEE ALSO
*	UnlockAmigaGuideBase()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

LONG ASM LockAmigaGuideBaseLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) VOID *handle)
{
    ObtainSemaphore (&ag->ag_Lock);
    return ((LONG) &ag->ag_Lock);
}
