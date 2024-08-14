
#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "commoditiesbase.h"
#include "pool.h"
#include "broker.h"
#include "objects.h"


/*****************************************************************************/


#define GetCxObj(size) AllocPoolMem(CxBase,size)
#define FreeCxObj(obj) FreePoolMem(CxBase,obj)


/*****************************************************************************/


/* size of extra object data */
static const LONG DataSizeTable[] =
{
    0,                 /* CX_INVALID    */
    sizeof(FILTERD),   /* CX_FILTER     */
    0,                 /* CX_TYPEFILTER */
    sizeof(SENDD),     /* CX_SEND       */
    sizeof(SIGNALD),   /* CX_SIGNAL     */
    0,                 /* CX_TRANSLATE  */
    sizeof(BROKERD),   /* CX_BROKER     */
    0,                 /* CX_DEBUG      */
    sizeof(CUSTOMD),   /* CX_CUSTOM     */
    0,                 /* CX_ZERO       */
};


/*****************************************************************************/


struct CxObj * ASM CreateCxObjLVO(REG(d0) ULONG type,
                                  REG(a0) LONG  arg1,
                                  REG(a1) LONG  arg2,
                                  REG(a6) struct CxLib *CxBase)
{
struct CxObj *co;
LONG          extra_data;

    extra_data = DataSizeTable[type];

    if (co = GetCxObj(sizeof(struct CxObj) + extra_data))
    {
        CO_DATA(co)  = (union CxObjData *) (extra_data?  &co[1]: 0L);
        CO_TYPE(co)  = type;
        CO_FLAGS(co) = COF_ENABLE;
        CO_ERROR(co) = 0;

        if (type == CX_BROKER)
            CO_FLAGS(co) |= ((struct NewBroker *)arg1)->nb_Flags;

        NewList((struct List *)CO_LIST(co));

        switch (type)
        {
            case CX_FILTER    : SetFilter(co,(STRPTR)arg1);
                                break;

            case CX_TYPEFILTER: CO_DATA(co) = (union CxObjData *)arg1;
                                break;

            case CX_SEND      : SENDER_PORT(co) = (struct MsgPort *)arg1;
                                SENDER_ID(co)   = arg2;
                                break;

            case CX_SIGNAL    : SIGNAL_TASK(co)   = (struct Task *)arg1;
                                SIGNAL_SIGNAL(co) = arg2;
                                break;

            case CX_TRANSLATE : ASSIGN_XLATE_IE(co,(struct InputEvent *)arg1);
                                break;

            case CX_BROKER    : InitBroker(CxBase,co,(struct NewBroker *)arg1);
                                break;

            case CX_DEBUG     : CO_DATA(co) = (union CxObjData *)arg1;
                                break;

            case CX_CUSTOM    : CUSTOM_ACTION(co) = (LONG (*)()) arg1;
                                CUSTOM_ID(co)     = arg2;
                                break;

            case CX_ZERO      : break;
        }
    }

    return(co);
}


/*****************************************************************************/


VOID ASM DeleteCxObjLVO(REG(a0) struct CxObj *co,
                        REG(a6) struct CxLib *CxBase)
{
    if (co)
    {
        RemoveCxObj(co);
        FreeCxObj(co);
    }
}


/*****************************************************************************/


static VOID CxWalk(struct CxLib *CxBase, struct List *list)
{
struct CxObj *co;

    /* WARNING! We're using RemHead() for speed, this should be
     * RemoveCxObj()
     */
    while (co = (struct CxObj *)RemHead(list))
    {
         CxWalk(CxBase,(struct List *)CO_LIST(co));
         FreeCxObj(co);
    }
}


/*****************************************************************************/


VOID ASM DeleteCxObjAllLVO(REG(a0) struct CxObj *co,
                           REG(a6) struct CxLib *CxBase)
{
    if (co)
    {
        RemoveCxObj(co);
        CxWalk(CxBase,(struct List *)CO_LIST(co));
        FreeCxObj(co);
    }
}


/*****************************************************************************/


LONG ASM ActivateCxObjLVO(REG(a0) struct CxObj *co,
                          REG(d0) LONG true,
                          REG(a6) struct CxLib *CxBase)
{
LONG prev = 0;

    if (co)
    {
        prev = CO_FLAGS(co) & COF_ENABLE;

        if (true)
            CO_FLAGS(co) |= COF_ENABLE;
        else
            CO_FLAGS(co) &= ~COF_ENABLE;

        if (CO_TYPE(co) == CX_BROKER)
            BrokerCommand(NULL,CXCMD_LIST_CHG);
    }

    return(prev);
}


/*****************************************************************************/


ULONG ASM CxObjTypeLVO(REG(a0) struct CxObj *co,
                       REG(a6) struct CxLib *CxBase)
{
    if (co)
        return((ULONG)CO_TYPE(co));

    return(CX_INVALID);
}


/*****************************************************************************/


LONG ASM CxObjErrorLVO(REG(a0) struct CxObj *co,
                       REG(a6) struct CxLib *CxBase)
{
    if (co)
        return((LONG)CO_ERROR(co));

    return(COERR_ISNULL);
}


/*****************************************************************************/


VOID ASM ClearCxObjErrorLVO(REG(a0) struct CxObj *co,
                            REG(a6) struct CxLib *CxBase)
{
    if (co)
        CO_ERROR(co) = 0;
}


/*****************************************************************************/


LONG ASM SetCxObjPriLVO(REG(a0) struct CxObj *co,
                        REG(d0) LONG pri,
                        REG(a6) struct CxLib *CxBase)
{
LONG oldPri = 0;

    if (co)
    {
        oldPri = CO_PRI(co);
        CO_PRI(co) = pri;
    }

    return(oldPri);
}


/*****************************************************************************/


VOID ASM SetTranslateLVO(REG(a0) struct CxObj *co,
                         REG(a1) struct InputXpression *ie,
                         REG(a6) struct CxLib *CxBase)
{
    if (co)
    {
        LockCX();

        if (CO_TYPE(co) == CX_TRANSLATE)
            CO_DATA(co) = (union CxObjData *) ie;
        else
            CO_ERROR(co) |= COERR_BADTYPE;

        UnlockCX();
    }
}


/*****************************************************************************/


VOID ASM SetFilterIXLVO(REG(a0) struct CxObj *co,
                        REG(a1) struct InputXpression *ix,
                        REG(a6) struct CxLib *CxBase)
{
    if (co)
    {
        LockCX();

        if (CO_TYPE(co) == CX_FILTER)
        {
            if (ix->ix_Version == IX_VERSION)
            {
                *FILTER_IX(co) = *ix;
                CO_ERROR(co) &= ~COERR_BADFILTER;
            }
            else
            {
                CO_ERROR(co) |= COERR_BADFILTER;
            }
        }
        else
        {
            CO_ERROR(co) |= COERR_BADTYPE;
        }

        UnlockCX();
    }
}


/*****************************************************************************/


VOID ASM SetFilterLVO(REG(a0) struct CxObj *co,
                      REG(a1) STRPTR descr,
                      REG(a6) struct CxLib *CxBase)
{
    if (co)
    {
        LockCX();

        if (CO_TYPE(co) == CX_FILTER)
        {
            if (!descr || ParseIX(descr,FILTER_IX(co)))
            {
                CO_ERROR(co) |= COERR_BADFILTER;
            }
            else
            {
                CO_ERROR(co) &= ~COERR_BADFILTER;
            }
        }
        else
        {
            CO_ERROR(co) |= COERR_BADTYPE;
        }

        UnlockCX();
    }
}


/*****************************************************************************/


/* Commented out until new attributes are needed which would make the
 * function actually useful
 *
 *

/* Tags for SetCxObjAttrs() */
#define SCOA_TagBase    0x100000
#define SCOA_ObjEnabled SCOA_TagBase+1
#define SCOA_Priority   SCOA_TagBase+2
#define SCOA_Filter     SCOA_TagBase+3
#define SCOA_FilterIX   SCOA_TagBase+4
#define SCOA_Translate  SCOA_TagBase+5


VOID ASM SetCxObjAttrsALVO(REG(a0) struct CxObj *co,
                           REG(a2) struct TagItem *attrs,
                           REG(a6) struct CxLib *CxBase)
{
struct TagItem *tag;
struct TagItem *tags = attrs;
ULONG           data;

    if (co)
    {
        LockCX();

        while (tag = NextTagItem(&tags))
        {
            data = tag->ti_Data;
            switch (tag->ti_Tag)
            {
                case SCOA_ObjEnabled : if (data)
                                           CO_FLAGS(co) |= COF_ENABLE;
                                       else
                                           CO_FLAGS(co) &= ~COF_ENABLE;

                                       if (CO_TYPE(co) == CX_BROKER)
                                           BrokerCommand(NULL,CXCMD_LIST_CHG);

                                       break;

                case SCOA_Priority   : CO_PRI(co) = data;
                                       break;

                case SCOA_Filter     : if (CO_TYPE(co) == CX_FILTER)
                                       {
                                           if (!data || ParseIX((STRPTR)data,FILTER_IX(co)))
                                           {
                                               CO_ERROR(co) |= COERR_BADFILTER;
                                           }
                                           else
                                           {
                                               CO_ERROR(co) &= ~COERR_BADFILTER;
                                           }
                                       }
                                       else
                                       {
                                           CO_ERROR(co) |= COERR_BADTYPE;
                                       }
                                       break;

                case SCOA_FilterIX   : if (CO_TYPE(co) == CX_FILTER)
                                       {
                                           if (((struct InputXpression *)data)->ix_Version == IX_VERSION)
                                           {
                                               *FILTER_IX(co) = *(struct InputXpression *)data;
                                               CO_ERROR(co) &= ~COERR_BADFILTER;
                                           }
                                           else
                                           {
                                               CO_ERROR(co) |= COERR_BADFILTER;
                                           }
                                       }
                                       else
                                       {
                                           CO_ERROR(co) |= COERR_BADTYPE;
                                       }
                                       break;

                case SCOA_Translate  : if (CO_TYPE(co) == CX_TRANSLATE)
                                           CO_DATA(co) = (union CxObjData *) data;
                                       else
                                           CO_ERROR(co) |= COERR_BADTYPE;
                                       break;
            }
        }

        UnlockCX();
    }
}
*/
