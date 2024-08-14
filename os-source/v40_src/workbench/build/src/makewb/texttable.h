#ifndef MAKEWB_TEXTTABLE_H
#define MAKEWB_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG


/*****************************************************************************/


#include <exec/types.h>
#include "makewb_strings.h"


/*****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* MAKEWB_TEXTTABLE_H */
