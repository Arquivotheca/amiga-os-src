#ifndef MAKEBUILDLIST_TEXTTABLE_H
#define MAKEBUILDLIST_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define MOUNT


/*****************************************************************************/


#include <exec/types.h>
#include "makebuildlist_strings.h"


/*****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* MAKEBUILDLIST_TEXTTABLE_H */
