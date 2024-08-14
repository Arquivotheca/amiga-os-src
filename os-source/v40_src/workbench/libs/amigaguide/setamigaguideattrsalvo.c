/* setamigaguideattrsalvo.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/SetAmigaGuideAttrsA ************************
*
*    NAME
*	SetAmigaGuideAttrsA - Set an AmigaGuide attribute.      (V34)
*
*    SYNOPSIS
*	retval = SetAmigaGuideAttrsA (handle, attrs);
*	d0			      a0      a1
*
*	LONG SetAmigaGuideAttrsA (AMIGAGUIDECONTEXT, struct TagItem *);
*
*	retval = SetAmigaGuideAttrs (handle, tag1, ...);
*
*	LONG SetAmigaGuideAttrs (AMIGAGUIDECONTEXT, Tag, ...);
*
*    FUNCTION
*	This function is used to set AmigaGuide attributes.
*
*    INPUTS
*	handle	- Pointer to an AmigaGuide handle.
*
*	attrs	- Attribute pairs to set.
*
*    TAGS
*	AGA_Activate (BOOL) - AmigaGuide activates the window when
*	    it receives a LINK command.  This tag allows the
*	    application developer to turn that feature off and on.
*
*    SEE ALSO
*	GetAmigaGuideAttr()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

LONG ASM SetAmigaGuideAttrsALVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct Client *cl, REG (a1) struct TagItem * attrs)
{
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG tidata;
    LONG ret = 0;
    LONG key;

    /* lock the base for exclusive access */
    key = LockAmigaGuideBaseLVO (ag, NULL);

    /* Process the tag list */
    tstate = attrs;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case AGA_Activate:
		cl->cl_Flags &= ~AGF_NOACTIVATE;
		cl->cl_Flags |= (tidata) ? NULL : AGF_NOACTIVATE;
		ret = 1;
		break;

#if 0
	    case AGA_Path:
		if (ag->ag_PathList)
		    FreePathListLVO (ag, ag->ag_PathList);
		ag->ag_PathList = CopyPathListLVO (ag, (BPTR) tidata);
		ret = 1;
		break;
#endif
	}
    }

    /* unlock the base */
    UnlockAmigaGuideBaseLVO (ag, key);

    return (ret);
}
