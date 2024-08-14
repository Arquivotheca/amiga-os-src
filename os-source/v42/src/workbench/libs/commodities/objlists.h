#ifndef OBJLISTS_H
#define OBJLISTS_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/****************************************************************************/


VOID ASM AttachCxObjLVO(REG(a0) struct CxObj *headObj, REG(a1) struct CxObj *co, REG(a6) struct CxLib *CxBase);
VOID ASM EnqueueCxObjLVO(REG(a0) struct CxObj *headObj, REG(a1) struct CxObj *co, REG(a6) struct CxLib *CxBase);
VOID ASM InsertCxObjLVO(REG(a0) struct CxObj *headObj, REG(a1) struct CxObj *co, REG(a2) struct CxObj *pred, REG(a6) struct CxLib *CxBase);
VOID ASM RemoveCxObjLVO(REG(a0) struct CxObj *co, REG(a6) struct CxLib *CxBase);


/****************************************************************************/


#endif /* OBJLISTS_H */
