/* GetAmigaGuideAttrLVO.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/GetAmigaGuideAttr ********************************
*
*    NAME
*	GetAmigaGuideAttr - Get an AmigaGuide attribute.        (V34)
*
*    SYNOPSIS
*	retval = GetAmigaGuideAttr (tag, handle, storage);
*	d0			    d0	 a0	 a1
*
*	LONG GetAmigaGuideAttr (Tag, AMIGAGUIDECONTEXT, ULONG *);
*
*    FUNCTION
*	This function is used to obtain attributes from AmigaGuide.
*
*    INPUTS
*	tag - Attribute to obtain.
*	handle - Handle to an AmigaGuide system.
*	storage	- Pointer to appropriate storage for the answer.
*
*    TAGS
*	AGA_Path (BPTR) - Pointer to the current path used by
*	    AmigaGuide.
*
*	AGA_XRefList (struct List *) - Pointer to the current
*	    cross reference list.
*
*    RETURNS
*
*    SEE ALSO
*	SetAmigaGuideAttrsA()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

LONG ASM GetAmigaGuideAttrLVO (REG (a6) struct AmigaGuideLib *ag, REG (d0) ULONG tag, REG (a0) VOID *handle, REG (a1) ULONG *storage)
{
#if 0
    return (0);
#else
    struct AmigaGuideToken *agt = ag->ag_Token;
    LONG retval = 1L;

    switch (tag)
    {
#if 0
	case AGA_Path:
	    *storage = (ULONG)ag->ag_PathList;
	    break;
#endif
	case AGA_XRefList:
	    *storage = (ULONG)&agt->agt_XRefList;
	    break;

	default:
	    retval = 0L;
	    break;
    }

    return (retval);
#endif
}
