#ifndef DATE_H
#define DATE_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


VOID GetDate(ULONG seconds, STRPTR format, STRPTR result);
VOID GetTime(ULONG seconds, STRPTR format, STRPTR result);


/*****************************************************************************/


#endif /* DATE_H */
