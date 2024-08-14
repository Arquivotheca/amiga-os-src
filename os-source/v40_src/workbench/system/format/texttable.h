#ifndef FORMAT_TEXTTABLE_H
#define FORMAT_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define FORMAT


/*****************************************************************************/


#include <exec/types.h>
#include <localestr/system.h>


/*****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* FORMAT_TEXTTABLE_H */
