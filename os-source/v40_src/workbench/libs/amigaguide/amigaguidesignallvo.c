/* amigaguidesignallvo.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/AmigaGuideSignal ********************************
*
*   NAME
*	AmigaGuideSignal - Obtain aysnc AmigaGuide signal.      (V34)
*
*   SYNOPSIS
*	signal = AmigaGuideSignal ( handle );
*	d0			    a0
*
*	ULONG AmigaGuideSignal (AMIGAGUIDECONTEXT);
*
*   FUNCTION
*	This function returns the signal bit to Wait on for AmigaGuideMsg's
*	for a particular AmigaGuide database.
*
*   INPUTS
*	handle	- Handle to a AmigaGuide system.
*
*   EXAMPLE
*	ULONG sigw, sigh;
*	AMIGAGUIDECONTEXT handle;
*
*	\* get the signal bit to wait on for a AmigaGuide message *\
*	sigh = AmigaGuideSignal(handle);
*
*	\* add the signal bit into the total signals to wait on *\
*	sigw |= sigh;
*
*   RETURNS
*	signal	- Signal bit to Wait on.
*
*   SEE ALSO
*	OpenAmigaGuideAsyncA(), GetAmigaGuideMsg(), ReplyAmigaGuideMsg()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

ULONG ASM AmigaGuideSignalLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct Client * cl)
{
    if (cl && cl->cl_AsyncPort)
	return ((ULONG)(1L << cl->cl_AsyncPort->mp_SigBit));
    return (NULL);
}
