#ifndef EVENTLOOP_H
#define EVENTLOOP_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/ports.h>
#include <utility/hooks.h>
#include <dos.h>


/*****************************************************************************/


VOID EventLoop(struct MsgPort *port);

ULONG __asm SnipHook(register __a0 struct Hook *hook,
                     register __a1 struct SnipHookMsg *snipMsg);


/*****************************************************************************/


#endif /* EVENTLOOP_H */
