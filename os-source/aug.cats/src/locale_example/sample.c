/* sample.c - simple locale.library example */

#include <exec/types.h>
#include <exec/libraries.h>
#include <libraries/locale.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "texttable.h"


/*****************************************************************************/


extern struct Library   *DOSBase;
extern struct Library   *SysBase;
       
struct LocaleInfo li;


/*****************************************************************************/


#define LocaleBase li.li_LocaleBase
#define catalog    li.li_Catalog


/*****************************************************************************/


VOID main(VOID)
{
    if (LocaleBase = OpenLibrary("locale.library",38))
        catalog = OpenCatalogA(NULL,"sample.catalog",NULL);

    PutStr(GetString(&li,MSG_HELLO));
    PutStr(GetString(&li,MSG_BYE));

    if (LocaleBase)
    {
        CloseCatalog(catalog);
        CloseLibrary(LocaleBase);
    }
}
