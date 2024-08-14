

#include <exec/types.h>
#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>
#include "texttable.h"


STRPTR far AppStrings[] =
{
    "",
    "ERROR: can't open keymap.resource\n",
    "ERROR: can't open console.device\n",
    "ERROR: can't set console keymap\n",
    "ERROR: can't load requested keymap\n",
    "ERROR: invalid keymap file\n"
};


#define LocaleBase li->li_LocaleBase
#define catalog    (struct Catalog *)li->li_Catalog


STRPTR GetString(struct LocaleInfo *li,enum AppStringsID id)
{
    if (LocaleBase)
        return(GetCatalogStr(catalog,id,AppStrings[id]));

    return(AppStrings[id]);
}
