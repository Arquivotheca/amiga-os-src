
#include <exec/types.h>
#include <libraries/locale.h>

#include <clib/exec_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "ce_strings.h"


extern struct Library *SysBase;

#define LocaleBase li.li_LocaleBase
#define catalog    li.li_Catalog

struct LocaleInfo li;


VOID OpenCat(VOID)
{
    if (LocaleBase = OpenLibrary("locale.library",38))
        catalog = OpenCatalogA(NULL,"sys/commodities.catalog",NULL);
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


STRPTR GetStr(ULONG id)
{
    return GetString(&li,id);
}

