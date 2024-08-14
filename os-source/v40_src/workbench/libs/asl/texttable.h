#ifndef ASL_TEXTTABLE_H
#define ASL_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define ASL


/*****************************************************************************/


#include <exec/types.h>
#include <libraries/locale.h>
#include <localestr/libs.h>


/*****************************************************************************/


struct LocaleInfo
{
    struct LocaleBase *li_LocaleBase;
    struct Locale     *li_Locale;
    APTR               li_Catalog;
};


/*****************************************************************************/


#define MSG_BARLABEL -1


/*****************************************************************************/


STRPTR GetString(struct LocaleInfo *li, AppStringsID id);


/*****************************************************************************/


#endif /* ASL_TEXTTABLE_H */
