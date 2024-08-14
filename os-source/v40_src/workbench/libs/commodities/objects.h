#ifndef OBJECTS_H
#define OBJECTS_H


/*****************************************************************************/


#include <exec/types.h>
#include "commoditiesbase.h"


/*****************************************************************************/


struct CxObj * ASM CreateCxObjLVO(REG(d0) ULONG type, REG(a0) LONG arg1, REG(a1) LONG arg2);
VOID ASM DeleteCxObjLVO(REG(a0) struct CxObj *co);
VOID ASM DeleteCxObjAllLVO(REG(a0) struct CxObj *co);
LONG ASM ActivateCxObjLVO(REG(a0) struct CxObj *co, REG(d0) LONG true);
ULONG ASM CxObjTypeLVO(REG(a0) struct CxObj *co);
LONG ASM CxObjErrorLVO(REG(a0) struct CxObj *co);
VOID ASM ClearCxObjErrorLVO(REG(a0) struct CxObj *co);
LONG ASM SetCxObjPriLVO(REG(a0) struct CxObj *co, REG(d0) LONG pri);
VOID ASM SetTranslateLVO(REG(a0) struct CxObj *co, REG(a1) struct InputXpression *ie);
VOID ASM SetFilterIXLVO(REG(a0) struct CxObj *co, REG(a1) struct InputXpression *ix);
VOID ASM SetFilterLVO(REG(a0) struct CxObj *co, REG(a1) STRPTR descr);
VOID ASM SetCxObjAttrsALVO(REG(a0) struct CxObj *co, REG(a2) struct TagItem *attrs);


/*****************************************************************************/


#endif /* OBJECTS_H */
