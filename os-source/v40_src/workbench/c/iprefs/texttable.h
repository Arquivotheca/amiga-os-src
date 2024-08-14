#ifndef IP_TEXTTABLE_H
#define IP_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define IPREFS


/*****************************************************************************/


#include <exec/types.h>
#include <localestr/c.h>


/*****************************************************************************/


VOID OpenCat(VOID);
VOID CloseCat(VOID);
STRPTR GetString(AppStringsID id);


/*****************************************************************************/


#endif /* IP_TEXTTABLE_H */
