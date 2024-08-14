#ifndef REQUESTFILE_TEXTTABLE_H
#define REQUESTFILE_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define REQUESTFILE


/*****************************************************************************/


#include <exec/types.h>
#include <localestr/c.h>


/*****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* REQUESTFILE_TEXTTABLE_H */
