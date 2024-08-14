
#define STRINGARRAY


#include <exec/types.h>
#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include "texttable.h"


extern struct Library *SysBase;

struct Library *LocaleBase;
struct Catalog *catalog;


VOID OpenCat(VOID)
{
    if (LocaleBase = OpenLibrary("locale.library",38))
        catalog = OpenCatalogA(NULL,"sys/utilities.catalog",NULL);
}


VOID CloseCat(VOID)
{
    if (LocaleBase)
    {
        CloseCatalog(catalog);
        CloseLibrary(LocaleBase);
        LocaleBase = NULL;
    }
}


STRPTR GetString(AppStringsID id)
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
