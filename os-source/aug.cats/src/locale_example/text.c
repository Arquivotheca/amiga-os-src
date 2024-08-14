#define STRINGARRAY


#include <exec/types.h>
#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>
#include "texttable.h"


#define LocaleBase li->li_LocaleBase
#define catalog    (struct Catalog *)li->li_Catalog

extern struct AppString AppStrings[];

UBYTE *NULLSTRING = "";


STRPTR GetString(struct LocaleInfo *li,LONG id)
{
struct AppString *as;
STRPTR s = NULLSTRING;
int k,l;

    l = sizeof(AppStrings);
    as = AppStrings;
    for(k=0; k<l; k++, as++)
        {
        if(as->as_ID == id)
            {
            s = as->as_Str;
            break;
            }
        }

    if((LocaleBase)&&(catalog)&&(*s))
        {
        s = GetCatalogStr(catalog,id,s);
        }

    return(s);
}
