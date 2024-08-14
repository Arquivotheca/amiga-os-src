#ifndef CE_STRINGS_H
#define CE_STRINGS_H


/*****************************************************************************/


#include <exec/types.h>
#define AppStringsID LONG
#include "ce_custom.h"


/*****************************************************************************/


VOID OpenCat(VOID);
VOID CloseCat(VOID);
STRPTR GetString(AppStringsID id);


/*****************************************************************************/


#endif /* CE_STRINGS_H */
