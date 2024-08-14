#ifndef CONVFUNC_H
#define CONVFUNC_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


LONG __saveds __stdargs DriverConvert(char *buf, LONG c, LONG flag);
VOID ProcChar(char c);
VOID CheckNewPage(BOOL forced, BOOL directWrite);
VOID MarginClip(BOOL directWrite);


/*****************************************************************************/


#endif /* CONVFUNC_H */
