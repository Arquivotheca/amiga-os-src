#ifndef TASKS_H
#define TASKS_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


struct Task *MakeTask(struct MPEGPlayerLib *MPEGPlayerBase, STRPTR name, LONG pri, APTR initPC, ULONG stackSize);
VOID SyncSignal(struct MPEGPlayerLib *MPEGPlayerBase, struct Task *task, ULONG sigMask);


/*****************************************************************************/


#endif  /* TASKS_H */
