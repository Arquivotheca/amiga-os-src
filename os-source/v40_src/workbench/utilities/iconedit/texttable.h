#ifndef IE_TEXTTABLE_H
#define IE_TEXTTABLE_H


/*****************************************************************************/


#define AppStringsID LONG
#define ICONEDIT


/*****************************************************************************/


#include <exec/types.h>
#include <localestr/utilities.h>


/*****************************************************************************/


VOID OpenCat(VOID);
VOID CloseCat(VOID);
STRPTR GetString(AppStringsID id);


/*****************************************************************************/


#endif /* IE_TEXTTABLE_H */
