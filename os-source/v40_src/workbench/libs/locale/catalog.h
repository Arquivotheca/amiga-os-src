#ifndef CATALOG_H
#define CATALOG_H


/*****************************************************************************/


#include <exec/types.h>
#include "locale.h"
#include "localebase.h"


/*****************************************************************************/


struct ExtCatalog
{
    struct Catalog ec_Catalog;
    APTR	   ec_Strings;
    char	   ec_Language[32];
    UWORD  	   ec_UsageCnt;
};


/*****************************************************************************/


struct ExtCatalog * ASM OpenCatalogALVO(REG(a0) struct ExtLocale *locale,
				        REG(a1) STRPTR name,
				        REG(a2) struct TagItem *tags);

VOID ASM CloseCatalogLVO(REG(a0) struct ExtCatalog *catalog);

STRPTR ASM GetCatalogStrLVO(REG(a0) struct ExtCatalog *catalog,
			    REG(d0) LONG stringNum,
			    REG(a1) STRPTR defaultString);

VOID ASM FlushCatalogs(VOID);


/*****************************************************************************/


#endif /* CATALOG_H */
