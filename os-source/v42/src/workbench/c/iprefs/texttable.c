
#include <exec/types.h>
#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>
#include <dos.h>

#include "texttable.h"


extern struct Library *SysBase;
extern struct Library *LocaleBase;
struct LocaleInfo      li;


VOID OpenCat(VOID)
{
    li.li_LocaleBase = LocaleBase;
    if (LocaleBase)
        li.li_Catalog = OpenCatalogA(NULL,"sys/c.catalog",NULL);
}


VOID CloseCat(VOID)
{
    if (LocaleBase)
        CloseCatalog(li.li_Catalog);
}


STRPTR __asm GetString(register __a0 struct LocaleInfo *li, register __d0 AppStringsID id);


STRPTR GetStr(AppStringsID id)
{
    return(GetString(&li,id));
}
