#ifndef OBJECTS_H
#define OBJECTS_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/*****************************************************************************/


struct CxObj * ASM CreateCxObjLVO(REG(d0) ULONG type, REG(a0) LONG arg1, REG(a1) LONG arg2, REG(a6) struct CxLib *CxBase);
VOID ASM DeleteCxObjLVO(REG(a0) struct CxObj *co, REG(a6) struct CxLib *CxBase);
VOID ASM DeleteCxObjAllLVO(REG(a0) struct CxObj *co, REG(a6) struct CxLib *CxBase);
LONG ASM ActivateCxObjLVO(REG(a0) struct CxObj *co, REG(d0) LONG true, REG(a6) struct CxLib *CxBase);
ULONG ASM CxObjTypeLVO(REG(a0) struct CxObj *co, REG(a6) struct CxLib *CxBase);
LONG ASM CxObjErrorLVO(REG(a0) struct CxObj *co, REG(a6) struct CxLib *CxBase);
VOID ASM ClearCxObjErrorLVO(REG(a0) struct CxObj *co, REG(a6) struct CxLib *CxBase);
LONG ASM SetCxObjPriLVO(REG(a0) struct CxObj *co, REG(d0) LONG pri, REG(a6) struct CxLib *CxBase);
VOID ASM SetTranslateLVO(REG(a0) struct CxObj *co, REG(a1) struct InputXpression *ie, REG(a6) struct CxLib *CxBase);
VOID ASM SetFilterIXLVO(REG(a0) struct CxObj *co, REG(a1) struct InputXpression *ix, REG(a6) struct CxLib *CxBase);
VOID ASM SetFilterLVO(REG(a0) struct CxObj *co, REG(a1) STRPTR descr, REG(a6) struct CxLib *CxBase);
VOID ASM SetCxObjAttrsALVO(REG(a0) struct CxObj *co, REG(a2) struct TagItem *attrs, REG(a6) struct CxLib *CxBase);


/*****************************************************************************/


#endif /* OBJECTS_H */
