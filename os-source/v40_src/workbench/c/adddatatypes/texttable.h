#ifndef MOUNT_TEXTTABLE_H
#define MOUNT_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define MOUNT


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


#endif /* MOUNT_TEXTTABLE_H */
