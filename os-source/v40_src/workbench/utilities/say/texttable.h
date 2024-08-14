#ifndef SAY_TEXTTABLE_H
#define SAY_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define SAY


/*****************************************************************************/


#include <exec/types.h>
#include <localestr/utilities.h>


/*****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* SAY_TEXTTABLE_H */
