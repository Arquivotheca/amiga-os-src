#ifndef CLOCK_TEXTTABLE_H
#define CLOCK_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define CLOCK


/*****************************************************************************/


#include <exec/types.h>
#include <libraries/locale.h>
#include <localestr/utilities.h>


/*****************************************************************************/


struct LocaleInfo
{
    APTR           li_LocaleBase;
    struct Locale *li_Locale;
    APTR           li_Catalog;
};


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* CLOCK_TEXTTABLE_H */
