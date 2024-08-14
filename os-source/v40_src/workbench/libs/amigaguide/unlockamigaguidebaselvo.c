/* unlockamigaguidebaselvo.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/UnlockAmigaGuideBase *************************
*
*    NAME
*	UnlockAmigaGuideBase - Unlock an AmigaGuide client.     (V34)
*
*    SYNOPSIS
*	UnlockAmigaGuideBase (key);
*			      d0
*
*	VOID UnlockAmigaGuideBase (LONG);
*
*    FUNCTION
*	This function is used to release a lock obtained with
*	LockAmigaGuideBase().
*
*    INPUTS
*	key - Value returned by LockAmigaGuideBase().
*
*    SEE ALSO
*	LockAmigaGuideBase().
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

VOID ASM UnlockAmigaGuideBaseLVO (REG (a6) struct AmigaGuideLib *ag, REG (d0) LONG key)
{
    if (key)
    {
	ReleaseSemaphore ((struct SignalSemaphore *)key);
    }
}
