#ifndef IP_TEXTTABLE_H
#define IP_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define IPREFS


/*****************************************************************************/


#include <exec/types.h>

#define AppStringsID LONG
#define IPREFS
#define CATCOMP_NUMBERS
#include "iprefs_strings.h"


/*****************************************************************************/


VOID OpenCat(VOID);
VOID CloseCat(VOID);
STRPTR GetStr(AppStringsID id);


/*****************************************************************************/


#endif /* IP_TEXTTABLE_H */
