
#include <exec/types.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "commoditiesbase.h"
#include "pool.h"
#include "messages.h"


/*****************************************************************************/


struct CxMsg *CreateCxMsg(struct CxLib *CxBase, ULONG type, struct InputEvent *ie)
{
struct CxMsg *cxm;
ULONG         msgSize;

    msgSize = sizeof(struct CxMsg);
    if (type == CXM_IEVENT)
        msgSize += IESize(CxBase,ie);

    if (cxm = AllocPoolMem(CxBase,msgSize))
    {
        cxm->cxm_Msg.mn_Length    = msgSize;
        cxm->cxm_Msg.mn_ReplyPort = &CxBase->cx_EventReply;
        CXMSGTYPE(cxm)            = type;

        /* done by GetCxMsg() above
         * CXMSGID(cxm)        = 0;
         * cxm->cxm_Echo       = 0;
         * cxm->cxm_StackIndex = 0;
         */

        if (type == CXM_IEVENT)
        {
            CXMSGDATA(cxm) = (char *) &cxm[1];
            CopyIE(CxBase,ie,(struct InputEvent *)CXMSGDATA(cxm));
        }
    }

    return (cxm);
}


/*****************************************************************************/


VOID ASM DisposeCxMsgLVO(REG(a0) struct CxMsg *cxm,
                         REG(a6) struct CxLib *CxBase)
{
    LockCX();
    Remove(cxm);
    UnlockCX();

    FreeCxMsg(cxm);
}


/*****************************************************************************/


VOID ASM DivertCxMsgLVO(REG(a0) struct CxMsg *cxm,
                        REG(a1) struct CxObj *co,
                        REG(a2) struct CxObj *returnObj,
                        REG(a6) struct CxLib *CxBase)
{
    ROUTECXMSG(cxm,(struct CxObj *)CO_LIST(co)->mlh_Head);

    if (cxm->cxm_StackIndex < STACKSLOTS)
        cxm->cxm_RoutingStack[cxm->cxm_StackIndex++] = returnObj;
}


/*****************************************************************************/


VOID ASM RouteCxMsgLVO(REG(a0) struct CxMsg *cxm,
                       REG(a1) struct CxObj *co,
                       REG(a6) struct CxLib *CxBase)
{
    ROUTECXMSG(cxm,co);
}


/*****************************************************************************/


ULONG ASM CxMsgTypeLVO(REG(a0) struct CxMsg *cxm,
                       REG(a6) struct CxLib *CxBase)
{
    return ((ULONG)CXMSGTYPE(cxm));
}


/*****************************************************************************/


APTR ASM CxMsgDataLVO(REG(a0) struct CxMsg *cxm,
                      REG(a6) struct CxLib *CxBase)
{
    if (cxm)
        return(CXMSGDATA(cxm));

    return(NULL);
}


/*****************************************************************************/


LONG ASM CxMsgIDLVO(REG(a0) struct CxMsg *cxm,
                    REG(a6) struct CxLib *CxBase)
{
    return (CXMSGID(cxm));
}


/*****************************************************************************/


ULONG IESize(struct CxLib *CxBase, struct InputEvent *ie)
{
struct IENewTablet *nt;
struct TagItem	   *tstate;
ULONG               size;

    size = sizeof(struct InputEvent);

    if (ie->ie_Class == IECLASS_NEWPOINTERPOS)
    {
        switch (ie->ie_SubClass)
        {
            case IESUBCLASS_PIXEL    : size += sizeof(struct IEPointerPixel);
                                       break;

            case IESUBCLASS_TABLET   : size += sizeof(struct IEPointerTablet);
                                       break;

            case IESUBCLASS_NEWTABLET: size += sizeof(struct IENewTablet);

                                       /* include size of tag list */
                                       nt = ie->ie_EventAddress;
                                       tstate = nt->ient_TagList;
                                       while (NextTagItem(&tstate))
                                           size += sizeof(struct TagItem);

                                       /* don't forget to leave room for TAG_DONE */
                                       size += sizeof(struct TagItem);
                                       break;
        }
    }

    return(size);
}


/*****************************************************************************/


/* "copy" must point to a memory block of IESize(copy) bytes. */
VOID CopyIE(struct CxLib *CxBase, struct InputEvent *original, struct InputEvent *copy)
{
struct IENewTablet *nt0;
struct IENewTablet *nt1;
struct TagItem	   *tstate;
struct TagItem	   *ti;
struct TagItem     *t0;
struct TagItem     *t1;

    CopyMem(original,copy,sizeof(struct InputEvent));

    copy->ie_NextEvent = NULL;    /* be safe */

    if (copy->ie_Class == IECLASS_NEWPOINTERPOS)
    {
        copy->ie_EventAddress = &copy[1];
        switch (copy->ie_SubClass)
        {
            case IESUBCLASS_PIXEL    : CopyMem(original->ie_EventAddress,copy->ie_EventAddress,sizeof(struct IEPointerPixel));
                                       break;

            case IESUBCLASS_TABLET   : CopyMem(original->ie_EventAddress,copy->ie_EventAddress,sizeof(struct IEPointerTablet));
                                       break;

            case IESUBCLASS_NEWTABLET: CopyMem(original->ie_EventAddress,copy->ie_EventAddress,sizeof(struct IENewTablet));
                                       nt0 = original->ie_EventAddress;
                                       nt1 = copy->ie_EventAddress;
                                       nt1->ient_TagList = (struct TagItem *)&nt1[1];
                                       t0 = nt0->ient_TagList;
                                       t1 = nt1->ient_TagList;

                                       tstate = t0;
                                       while (ti = NextTagItem(&tstate))
                                       {
                                           *t1 = *ti;
                                           t1++;
                                       }
                                       t1->ti_Tag = TAG_DONE;

                                        break;

            case IESUBCLASS_COMPATIBLE: break;  /* ignore */

            default                   : copy->ie_Class = IECLASS_NULL;
                                        break;
        }
    }
}


/*****************************************************************************/


VOID ASM AddIEventsLVO(REG(a0) struct InputEvent *ie,
                       REG(a6) struct CxLib *CxBase)
{
struct CxMsg *cxm;
struct CxLib *cx = CxBase;

    LockCX();

    while (ie)
    {
        if (cxm = CreateCxMsg(CxBase,CXM_IEVENT,ie))
        {
            ROUTECXMSG(cxm,(struct CxObj *)CXOBJLIST->mlh_Head);
            AddTail((struct List *)&cx->cx_Messages,cxm);
        }
        else
        {
            break;
        }
        ie = ie->ie_NextEvent;
    }

    UnlockCX();
}
