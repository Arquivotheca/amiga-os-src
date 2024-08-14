/* stubs.c
 *
 */

#include "amigaguidebase.h"

LONG ASM LoadXRefLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) BPTR lock, REG (a1) STRPTR name)
{
    return LoadXRef (lock, name);
}

VOID ASM ExpungeXRefLVO (REG (a6) struct AmigaGuideLib *ag)
{
    ExpungeXRef ();
}

BPTR ASM AddPathEntriesLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) BPTR path, REG (d0) STRPTR * argptr)
{
    return AddPathEntries (path, argptr);
}

BPTR ASM CopyPathListLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) BPTR p)
{
    return CopyPathList (p);
}

VOID ASM FreePathListLVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) BPTR p)
{
    FreePathList (p);
}

ULONG ASM ParsePathStringLVO (REG (a6) struct AmigaGuideLib *ag, REG (d0) STRPTR line, REG (a0) STRPTR * argv, REG (d1) ULONG max)
{
    return ParsePathString (line, argv, max);
}

BPTR ASM LockELVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) BPTR p, REG (d1) STRPTR name, REG (d2) LONG mode)
{
    return LockE (p, name, mode);
}

BPTR ASM OpenELVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) BPTR p, REG (d1) STRPTR name, REG (d2) LONG mode)
{
    return OpenE (p, name, mode);
}

BPTR ASM SetCurrentDirELVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) BPTR p, REG (d1) STRPTR name)
{
    return SetCurrentDirE (p, name);
}


/****** amigaguide.library/GetAmigaGuideString ****************************
*
*   NAME
*	GetAmigaGuideString - Get an AmigaGuide string.
*                                                               (V34)
*   SYNOPSIS
*	txt = GetAmigaGuideString (id);
*	d0			   d0
*
*	STRPTR GetAmigaGuideString (ULONG);
*
*   FUNCTION
*	This function is used to obtain a localized string given the
*	ID.
*
*   INPUTS
*	ID -- Valid AmigaGuide string id.
*
*   RETURNS
*	A pointer to the string.   NULL for an invalid string.
*
*   SEE ALSO
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

STRPTR ASM GetAmigaGuideStringLVO (REG (a6) struct AmigaGuideLib *ag, REG(d0) LONG id)
{
    return GetAGString (id);
}

ULONG ASM AddAmigaGuideHostALVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) struct Hook *h, REG (d0) STRPTR name, REG (a1) struct TagItem * attrs)
{
    return AddAGHostA (h, name, attrs);
}

LONG ASM RemoveAmigaGuideHostALVO (REG (a6) struct AmigaGuideLib *ag, REG (a0) VOID * handle, REG (a1) struct TagItem *attrs)
{
    return RemoveAGHostA (handle, attrs);
}
