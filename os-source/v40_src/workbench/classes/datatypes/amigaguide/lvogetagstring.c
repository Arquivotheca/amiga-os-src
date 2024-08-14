/* LVOGetAGString.c
 *
 */

#define STRINGARRAY	TRUE

#include "amigaguidebase.h"

#define	DB(x)	;

/****** amigaguide.clas/GetAGString ****************************
*
*    NAME
*	GetAGString - Obtain localized AmigaGuide text  (V34)
*
*    SYNOPSIS
*	string = GetAGString (id);
*	d0		      d0
*
*	STRPTR GetAGString (LONG);
*
*    FUNCTION
*	This function is used to obtain a pointer to the string
*	associated with the given ID.
*
*    INPUTS
*	id	- Catalog entry to obtain.
*
*    RETURNS
*	Returns a pointer to the NULL terminated string. NULL on
*	failure.
*
*    SEE ALSO
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

STRPTR null = "";

STRPTR ASM LVOGetAGString (REG (a6) struct AGLib *ag, REG(d0) LONG id)
{
    STRPTR local;
    UWORD  i;

    for (i = 0, local = null; (local == null); i++)
    {
        if (AppStrings[i].as_ID == id)
            local = AppStrings[i].as_Str;
    }

    if (ag->ag_LocaleBase)
    {
        local = GetCatalogStr (ag->ag_Catalog, id, local);
    }

    return(local);
}
