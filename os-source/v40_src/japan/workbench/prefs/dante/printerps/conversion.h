

#include <exec/types.h>


/*****************************************************************************/


BOOL CheckNumber(STRPTR buffer, LONG *longvar, LONG *decvar, LONG maxdec, BOOL exiting);
STRPTR FromMille(EdDataPtr ed, ULONG mp, LONG numdec);
STRPTR FromMilleNC(EdDataPtr ed, ULONG mp, LONG numdec);
ULONG LongFromMille(EdDataPtr ed, ULONG mp);
ULONG ToMille(EdDataPtr ed, STRPTR value);
ULONG ToMilleNC(EdDataPtr ed, STRPTR value);
