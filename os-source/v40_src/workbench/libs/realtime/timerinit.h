#ifndef TIMERINIT_H
#define TIMERINIT_H


/*****************************************************************************/


#include <exec/types.h>
#include "realtimebase.h"


/*****************************************************************************/


LONG OpenTimer(struct RealTimeLib *RealTimeBase);
VOID CloseTimer(struct RealTimeLib *RealTimeBase);


/*****************************************************************************/


#endif /* TIMERINIT_H */
