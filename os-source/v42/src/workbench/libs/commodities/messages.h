#ifndef MESSAGES_H
#define MESSAGES_H


/****************************************************************************/


#include <exec/types.h>
#include <devices/inputevent.h>
#include "commoditiesbase.h"


/*****************************************************************************/


struct CxMsg *CreateCxMsg(struct CxLib *CxBase, ULONG type, struct InputEvent *ie);
VOID ASM DisposeCxMsgLVO(REG(a0) struct CxMsg *cxm, REG(a6) struct CxLib *CxBase);
VOID ASM DivertCxMsgVO(REG(a0) struct CxMsg *cxm, REG(a1) struct CxObj *co, REG(a2) struct CxObj *returnObj, REG(a6) struct CxLib *CxBase);
VOID ASM RouteCxMsgLVO(REG(a0) struct CxMsg *cxm, REG(a1) struct CxObj *co, REG(a6) struct CxLib *CxBase);
ULONG ASM CxMsgTypeLVO(REG(a0) struct CxMsg *cxm, REG(a6) struct CxLib *CxBase);
APTR ASM CxMsgDataLVO(REG(a0) struct CxMsg *cxm, REG(a6) struct CxLib *CxBase);
LONG ASM CxMsgIDLVO(REG(a0) struct CxMsg *cxm, REG(a6) struct CxLib *CxBase);
VOID ASM AddIEventsLVO(REG(a0) struct InputEvent *ie, REG(a6) struct CxLib *CxBase);

ULONG IESize(struct CxLib *CxBase, struct InputEvent *ie);
VOID CopyIE(struct CxLib *CxBase, struct InputEvent *original, struct InputEvent *copy);
  /* "copy" must point to memory of size IESize(ie) */

#define FreeCxMsg(msg) FreePoolMem(CxBase,msg)


/*****************************************************************************/


#endif /* MESSAGES_H */
