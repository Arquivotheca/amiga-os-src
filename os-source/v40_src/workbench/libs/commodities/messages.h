#ifndef MESSAGES_H
#define MESSAGES_H


/****************************************************************************/


#include <exec/types.h>
#include <devices/inputevent.h>
#include "commoditiesbase.h"


/*****************************************************************************/


struct CxMsg *CreateCxMsg(ULONG type, struct InputEvent *ie);
VOID ASM DisposeCxMsgLVO(REG(a0) struct CxMsg *cxm);
VOID ASM DivertCxMsgVO(REG(a0) struct CxMsg *cxm, REG(a1) struct CxObj *co, REG(a2) struct CxObj *returnObj);
VOID ASM RouteCxMsgLVO(REG(a0) struct CxMsg *cxm, REG(a1) struct CxObj *co);
ULONG ASM CxMsgTypeLVO(REG(a0) struct CxMsg *cxm);
APTR ASM CxMsgDataLVO(REG(a0) struct CxMsg *cxm);
LONG ASM CxMsgIDLVO(REG(a0) struct CxMsg *cxm);
VOID ASM AddIEventsLVO(REG(a0) struct InputEvent *ie);

ULONG IESize(struct InputEvent *ie);
VOID CopyIE(struct InputEvent *original, struct InputEvent *copy);
  /* "copy" must point to memory of size IESize(ie) */

#define FreeCxMsg(msg) FreePoolMem(msg)


/*****************************************************************************/


#endif /* MESSAGES_H */
