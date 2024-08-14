
#define STRINGARRAY


#include <exec/types.h>
#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>
#include "pe_strings.h"


#define LocaleBase li->li_LocaleBase
#define catalog    (struct Catalog *)li->li_Catalog


STRPTR GetString(struct LocaleInfo *li,AppStringsID id)
{
UWORD  i;
STRPTR local = NULL;

    i = 0;
    while (!local)
    {
        if (AppStrings[i].as_ID == id)
            local = AppStrings[i].as_Str;
        i++;
    }

    if (LocaleBase)
        return(GetCatalogStr(catalog,id,local));

    return(local);
}
