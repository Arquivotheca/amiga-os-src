
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "commoditiesbase.h"
#include "objlists.h"
#include "broker.h"


/*****************************************************************************/


VOID ASM AttachCxObjLVO(REG(a0) struct CxObj *headObj,
                        REG(a1) struct CxObj *co,
                        REG(a6) struct CxLib *CxBase)
{
    if (headObj)
    {
        if (co)
        {
            LockCX();

            AddTail((struct List *)CO_LIST(headObj),(struct Node *)co);
            CO_FLAGS(co) |= COF_INLIST;

            UnlockCX();
        }
        else
        {
            CO_ERROR(headObj) |= COERR_NULLATTACH;
        }
    }
    else
    {
        DeleteCxObjAll(co);
    }
}


/*****************************************************************************/


VOID ASM EnqueueCxObjLVO(REG(a0) struct CxObj *headObj,
                         REG(a1) struct CxObj *co,
                         REG(a6) struct CxLib *CxBase)
{
    if (headObj)
    {
        if (co)
        {
            LockCX();

            Enqueue((struct List *)CO_LIST(headObj),(struct Node *)co);
            CO_FLAGS(co) |= COF_INLIST;

            UnlockCX();
        }
        else
        {
            CO_ERROR(headObj) |= COERR_NULLATTACH;
        }
    }
    else
    {
        DeleteCxObjAll(co);
    }
}


/*****************************************************************************/


VOID ASM InsertCxObjLVO(REG(a0) struct CxObj *headObj,
                        REG(a1) struct CxObj *co,
                        REG(a2) struct CxObj *pred,
                        REG(a6) struct CxLib *CxBase)
{
    if (headObj)
    {
        if (co)
        {
            LockCX();

            Insert((struct List *)CO_LIST(headObj),(struct Node *)co,(struct Node *)pred);
            CO_FLAGS(co) |= COF_INLIST;

            UnlockCX();
        }
        else
        {
            CO_ERROR(headObj) |= COERR_NULLATTACH;
        }
    }
    else
    {
        DeleteCxObjAll(co);
    }
}


/*****************************************************************************/


/* WARNING! CxWalk in objects.c simply calls RemHead() on the objects
 * in an object list instead of calling this routine, For speed
 * reasons.
 */

VOID ASM RemoveCxObjLVO(REG(a0) struct CxObj *co,
                        REG(a6) struct CxLib *CxBase)
{
    if (co)
    {
        if (CO_FLAGS(co) & COF_INLIST)
        {
            LockCX();

            Remove((struct Node *)co);
            CO_FLAGS(co) &= ~COF_INLIST;

            UnlockCX();

            if (CO_TYPE(co) == CX_BROKER)
            {
                if (IsListEmpty((struct List *)CXOBJLIST))
                    RemoveIHandler(CxBase);
                else
                    BrokerCommand(NULL,CXCMD_LIST_CHG);
            }
        }
    }
}
