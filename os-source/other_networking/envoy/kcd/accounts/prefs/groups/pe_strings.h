#ifndef PE_STRINGS_H
#define PE_STRINGS_H


/*****************************************************************************/


#define AppStringsID LONG
#include "texttable.h"


/*****************************************************************************/


#include <exec/types.h>
#include <localestr/envoyprefs.h>


/*****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* PE_STRINGS_H */
