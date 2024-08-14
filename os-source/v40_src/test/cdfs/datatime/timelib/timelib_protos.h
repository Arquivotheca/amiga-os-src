#ifndef TIME_LIB_PROTOS_H
#define TIME_LIB_PROTOS_H

#include <exec/types.h>
#include <devices/timer.h>

VOID   subETime(struct EClockVal *, struct EClockVal *);
STRPTR timeCmp(struct EClockVal *, struct EClockVal *, ULONG);
BOOL   UDiv6432(ULONG, ULONG, ULONG, ULONG *, ULONG *);
ULONG  giveFract(ULONG, ULONG, ULONG);

#endif /* TIME_LIB_PROTOS_H */
