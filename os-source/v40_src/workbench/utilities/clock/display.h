#ifndef DISPLAY_H
#define DISPLAY_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


VOID DrawAnalog(ULONG seconds, BOOL initial);
VOID UpdateDigital(ULONG seconds);


/*****************************************************************************/


#endif /* DISPLAY_H */
