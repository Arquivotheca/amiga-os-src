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


VOID ASM FlushCatalogs(REG(a6) struct LocaleLib *LocaleBase);


/*****************************************************************************/


#endif /* CATALOG_H */
