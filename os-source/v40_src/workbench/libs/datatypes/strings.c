/* strings.c
 *
 */

#define STRINGARRAY	TRUE

#include "datatypesbase.h"

#define	DB(x)	;

/****** datatypes.library/GetDTString ****************************************
*
*    NAME
*	GetDTString - Obtain a DataTypes string.                (V39)
*
*    SYNOPSIS
*	str = GetDTString (id);
*	d0		   d0
*
*	STRPTR GetDTString (ULONG id);
*
*    FUNCTION
*	This function is used to obtain a pointer to a localized
*	DataTypes string.
*
*    INPUTS
*	id - ID of the string to obtain.
*
*    RETURNS
*	A pointer to a NULL terminated string.
*
*    SEE ALSO
*
*******************************************************************************
*
* Created:  31-May-92, David N. Junod
*
*/

/*****************************************************************************/

STRPTR ASM GetDTStringLVO (REG (a6) struct DataTypesLib * dtl, REG (d0) ULONG id)
{
    STRPTR local = NULL;
    UWORD i;

    DB (kprintf ("GetDTStringLVO %ld\n", id));
    i = 0;
    while (!local)
    {
	if (AppStrings[i].as_ID == id)
	    local = AppStrings[i].as_Str;
	i++;
    }

    DB (kprintf (" default=%s\n", local));
    if (dtl->dtl_LocaleBase)
    {
	local = GetCatalogStr (dtl->dtl_Catalog, id, local);
    }
    DB (kprintf (" localized=%s\n", local));
    return (local);
}
