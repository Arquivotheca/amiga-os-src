
#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "commoditiesbase.h"
#include "messages.h"
#include "dispatch.h"
#include "pool.h"
#include "serialout.h"
#include "ihandler.h"


/*****************************************************************************/


/*
#define ICOLOR(c) (*((SHORT *)0xdff180)=(c))
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>
*/


/*****************************************************************************/


#if 0
VOID DumpIE(struct InputEvent *ie)
{
    if (ie->ie_Class != IECLASS_TIMER)
    {
        kprintf("Class       : ");
        switch (ie->ie_Class)
        {
            case IECLASS_NULL           : kprintf("IECLASS_NULL\n");
                                          break;

            case IECLASS_RAWKEY         : kprintf("IECLASS_RAWKEY\n");
                                          break;

            case IECLASS_RAWMOUSE       : kprintf("IECLASS_RAWMOUSE\n");        break;
            case IECLASS_EVENT          : kprintf("IECLASS_EVENT\n");           break;
            case IECLASS_POINTERPOS     : kprintf("IECLASS_POINTERPOS\n");      break;
            case IECLASS_TIMER          : kprintf("IECLASS_TIMER\n");           break;
            case IECLASS_GADGETDOWN     : kprintf("IECLASS_GADGETDOWN\n");      break;
            case IECLASS_GADGETUP       : kprintf("IECLASS_GADGETUP\n");        break;
            case IECLASS_REQUESTER      : kprintf("IECLASS_REQUESTER\n");       break;
            case IECLASS_MENULIST       : kprintf("IECLASS_MENULIST\n");        break;
            case IECLASS_CLOSEWINDOW    : kprintf("IECLASS_CLOSEWINDOW\n");     break;
            case IECLASS_SIZEWINDOW     : kprintf("IECLASS_SIZEWINDOW\n");      break;
            case IECLASS_REFRESHWINDOW  : kprintf("IECLASS_REFRESHWINDOW\n");   break;
            case IECLASS_NEWPREFS       : kprintf("IECLASS_NEWPREFS\n");        break;
            case IECLASS_DISKREMOVED    : kprintf("IECLASS_DISKREMOVED\n");     break;
            case IECLASS_DISKINSERTED   : kprintf("IECLASS_DISKINSERTED\n");    break;
            case IECLASS_ACTIVEWINDOW   : kprintf("IECLASS_ACTIVEWINDOW\n");    break;
            case IECLASS_INACTIVEWINDOW : kprintf("IECLASS_INACTIVEWINDOW\n");  break;
            case IECLASS_NEWPOINTERPOS  : kprintf("IECLASS_NEWPOINTERPOS\n");   break;
            case IECLASS_MENUHELP       : kprintf("IECLASS_MENUHELP\n");        break;
            case IECLASS_CHANGEWINDOW   : kprintf("IECLASS_CHANGEWINDOW\n");    break;
            default                     : kprintf("Unknown class\n");
        }
        kprintf("SubClass    : %lx\nCode        : %lx\nQualifier   : %lx\nEventAddress: %lx\n\n------\n\n",ie->ie_SubClass,ie->ie_Code,ie->ie_Qualifier,ie->ie_EventAddress);
    }
}
#endif


#if 0
VOID DumpObjList(struct MinList *list, UWORD indent)
{
struct CxObj *co;
UWORD         i;

    co = (struct CxObj *)list->mlh_Head;
    while (co->co_Succ)
    {
        for (i = 0; i < indent; i++) kprintf(" ");
        kprintf("Name : %s\n",co->co_Name);

        for (i = 0; i < indent; i++) kprintf(" ");
        switch (CO_TYPE(co))
        {
            case CX_INVALID     : kprintf("Type : CX_INVALID     \n"); break;
            case CX_FILTER      : kprintf("Type : CX_FILTER      \n"); break;
            case CX_TYPEFILTER  : kprintf("Type : CX_TYPEFILTER  \n"); break;
            case CX_SEND        : kprintf("Type : CX_SEND        \n"); break;
            case CX_SIGNAL      : kprintf("Type : CX_SIGNAL      \n"); break;
            case CX_TRANSLATE   : kprintf("Type : CX_TRANSLATE   \n"); break;
            case CX_BROKER      : kprintf("Type : CX_BROKER      \n"); break;
            case CX_DEBUG       : kprintf("Type : CX_DEBUG       \n"); break;
            case CX_CUSTOM      : kprintf("Type : CX_CUSTOM      \n"); break;
            case CX_ZERO        : kprintf("Type : CX_ZERO        \n"); break;
            default             : kprintf("Type : Unknown (%ld)\n", CO_TYPE(co)); break;
        }

        for (i = 0; i < indent; i++) kprintf(" ");
        kprintf("Flags: ");
        if (COF_INLIST & CO_FLAGS(co))
            kprintf("COF_INLIST ");
        if (COF_ENABLE & CO_FLAGS(co))
            kprintf("COF_ENABLE ");
        if (COF_SHOW_HIDE & CO_FLAGS(co))
            kprintf("COF_SHOW_HIDE ");

        if (CO_FLAGS(co) == 0)
            kprintf("None");

        kprintf("\n");

        for (i = 0; i < indent; i++) kprintf(" ");
        kprintf("----------\n");

        DumpObjList(CO_LIST(co),indent+4);

        co = (struct CxObj *)co->co_Succ;
    }
}


VOID DumpNetwork(VOID)
{
    LockCX();
    DumpObjList(&CxBase->cx_Objects,0);
    kprintf("\n**************************************************************\n\n");
    UnlockCX();
}
#endif


/*****************************************************************************/


static VOID Cx_Send(struct CxLib *CxBase, struct CxMsg *cxm, struct CxObj *sender)
{
   /* copies cxm into an exec message from pool and sends it.
    * original CxMsg is not swallowed, nor re-routed
    */

struct CxMsg      *msg;
struct MsgPort    *port;
struct InputEvent *ie;

    if (port = SENDER_PORT(sender))
    {
        ie = (struct InputEvent *)CXMSGDATA(cxm);
        if (ie->ie_SubClass)
        {
            if ((ie->ie_Class != IECLASS_NEWPOINTERPOS) || (ie->ie_SubClass > IESUBCLASS_NEWTABLET))
                ie->ie_Class = IECLASS_NULL;
        }

        if (msg = (struct CxMsg *)CreateCxMsg(CxBase,CXM_IEVENT,ie))
        {
            CopyMem(cxm,msg,sizeof(struct CxMsg));

            /* fix up Data pointer */
            CXMSGDATA((struct CxMsg *) msg) = (char *) &((struct CxMsg *) msg)[1];

            /* copy user id into message */
            CXMSGID(msg) = SENDER_ID(sender);

            PutMsg(port,&msg->cxm_Msg);
        }
    }
}


/*****************************************************************************/


static VOID Cx_Debug(struct CxLib *CxBase, struct CxMsg *cxm, struct CxObj *db)
{
struct InputEvent *ie;

    kprintf("\n----\nDEBUG NODE: %lx, ID: %lx\n", db, (LONG) DEBUGD(db));
    kprintf("\tCxMsg: %lx, type: %x, data %lx destination %lx\n",cxm, CXMSGTYPE(cxm), CXMSGDATA(cxm), CXMSGDESTINATION(cxm));

    if (CXMSGTYPE(cxm) == CXM_IEVENT)
    {
	ie = (struct InputEvent *)CXMSGDATA(cxm);
        kprintf("dump IE: %lx\n\tClass %lx\n\tCode %lx\n\tQualifier %lx\n\tEventAddress %lx\n",ie,ie->ie_Class,ie->ie_Code,ie->ie_Qualifier,ie->ie_EventAddress);
    }
}


/*****************************************************************************/


static VOID Cx_Zero(struct CxLib *CxBase, struct CxMsg *cxm, struct CxObj *co)
{
struct InputEvent *ie;

    if (ie = AllocPoolMem(CxBase,IESize(CxBase,(struct InputEvent *)CXMSGDATA(cxm))))
    {
        CopyIE(CxBase,(struct InputEvent *)CXMSGDATA(cxm),ie);
        CxBase->cx_EndChain->ie_NextEvent = ie;
        CxBase->cx_EndChain               = ie;
    }

    DisposeCxMsg(cxm);
}


/*****************************************************************************/


static VOID Cx_Translate(struct CxLib *CxBase, struct CxMsg *original, struct CxObj *xl)
{
   /* regardless of CxMsgType, replaces message with a chain of
    * clones with type CXM_IEVENT, one for each input event in chain
    * pointed to by xl.  Special case of NULL xl events
    * has effect of swallowing all CXM_IEVENT messages
    */

struct InputEvent *ie;
struct CxMsg      *cxm;
UWORD              i;
struct List       *list;

    ie   = XLATE_IE(xl);
    list = (struct List *)CXMSGLIST;

    /* convert all input events to a series of CxMsg */
    while (ie)
    {
        if (cxm = CreateCxMsg(CxBase,CXM_IEVENT,ie))
        {
            /* stamp all translated events with the time of the original event */
            ((struct InputEvent *)CXMSGDATA(cxm))->ie_TimeStamp = ((struct InputEvent *)CXMSGDATA(original))->ie_TimeStamp;

            i = cxm->cxm_StackIndex;
            while (i > 0)
            {
                i--;
                cxm->cxm_RoutingStack[i] = original->cxm_RoutingStack[i];
            }
            cxm->cxm_StackIndex = original->cxm_StackIndex;

            ROUTECXMSG(cxm,CXMSGDESTINATION(original));
            CXMSGID(cxm) = CXMSGID(original);
            AddHead(list,cxm);
        }

        ie = ie->ie_NextEvent;
    }
    DisposeCxMsg(original);
}


/*****************************************************************************/


struct InputEvent * ASM CxHandler(REG(a0) struct InputEvent *ie,
				  REG(a1) struct CxLib *CxBase)
{
struct CxMsg      *cxm;
struct CxMsg      *succxm;
struct CxObj      *firstBroker;
struct CxLib      *cx = CxBase;
struct Message    *msg;
struct CxObj      *target;
struct InputEvent *ptr;
struct InputEvent *next;

    if (ie && !IsListEmpty((struct List *)CXOBJLIST))
    {
/*
        while (VBeamPos() < 150);
        ICOLOR(0xfff);
*/

        LockCX();

        ptr = CxBase->cx_ReturnChain;
        while (ptr)
        {
            next = ptr->ie_NextEvent;
            FreePoolMem(CxBase,ptr);
            ptr = next;
        }
        CxBase->cx_ReturnChain = NULL;
        CxBase->cx_EndChain    = (struct InputEvent *)&CxBase->cx_ReturnChain;

        while (msg = GetMsg(&cx->cx_EventReply))
            FreeCxMsg(msg);

        firstBroker = (struct CxObj *) CXOBJLIST->mlh_Head;
        for (cxm = (struct CxMsg *) CXMSGLIST->mlh_Head; succxm = (struct CxMsg *) cxm->cxm_Succ; cxm = succxm)
            ROUTECXMSG(cxm,firstBroker);

        AddIEvents(ie);

	/* send all messages through the object network */
        while (TRUE)
        {
            cxm = (struct CxMsg *) CXMSGLIST->mlh_Head;
            if (!cxm->cxm_Succ)
                break;

            target = CXMSGDESTINATION(cxm);
            if (cxm->cxm_Echo > CXM_TOOMANYECHOS)
            {
                target = NULL;
            }

            /* If target is a tail node, pop stack to find
             * next target (successor of popped object)
             */
            while (target && !target->co_Succ)
            {
                if (cxm->cxm_StackIndex)
                {
                    if (target = cxm->cxm_RoutingStack[--cxm->cxm_StackIndex])
                        target = (struct CxObj *) target->co_Succ;
                }
                else
                {
                    target = NULL;
                }
            }

            if (!target)
            {
                Cx_Zero(CxBase,cxm,CxBase->cx_Zero);
            }
            else
            {
                ROUTECXMSG(cxm, (struct CxObj *) target->co_Succ);

                if (CO_FLAGS(target) & COF_ENABLE)
                {
                    cxm->cxm_Echo++;
                    switch(CO_TYPE(target))
                    {
                        case CX_FILTER    : if ((CXMSGTYPE(cxm) == CXM_IEVENT) && MatchIX((struct InputEvent *)CXMSGDATA(cxm),(IX *)FILTER_IX(target)))
                                                DivertCxMsg(cxm,target,target);
                                            break;

                        case CX_TYPEFILTER: if (TFD(target) & CXMSGTYPE(cxm))
                                                DivertCxMsg(cxm,target,target);
                                            break;

                        case CX_SEND      : Cx_Send(CxBase,cxm,target);
                                            break;

                        case CX_SIGNAL    : Signal(SIGNAL_TASK(target), 1L << SIGNAL_SIGNAL(target));
                                            break;

                        case CX_TRANSLATE : Cx_Translate(CxBase,cxm,target);
                                            break;

                        case CX_BROKER    : DivertCxMsg(cxm,target,target);
                                            break;

                        case CX_DEBUG     : Cx_Debug(CxBase,cxm,target);
                                            break;

                        case CX_CUSTOM    : CXMSGID(cxm) = CUSTOM_ID(target);
                                            CxDispatch((LONG)CUSTOM_ACTION(target),cxm,target,CxBase);
                                            break;

                        case CX_ZERO      : Cx_Zero(CxBase,cxm,target);
                                            break;

                        case CX_INVALID   :
                        default           : break;
                    }
                }
            }
        }

        UnlockCX();

/*
        ICOLOR(0x001);
*/

        return(CxBase->cx_ReturnChain);
    }

    return(ie);
}
