#ifndef EVENTLOOP_H
#define EVENTLOOP_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


VOID EventLoop(struct MsgPort *notifyPort, struct SignalSemaphore *iprefsLock);
VOID ExtractPrefsName(UWORD index, STRPTR name);


/*****************************************************************************/


#endif /* EVENTLOOP_H */
