
#include <exec/memory.h>
#include <exec/semaphores.h>

#include <devices/input.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "commoditiesbase.h"
#include "ihandler.h"
#include "pool.h"
#include "messages.h"


/*****************************************************************************/


BOOL ASM InitCommodities(REG(a6) struct CxLib *CxBase)
{
    InitSemaphore(&CxBase->cx_LibLock);

    NewList((struct List *)&CxBase->cx_Messages);
    NewList((struct List *)&CxBase->cx_Objects);

    CxBase->cx_EventReply.mp_Node.ln_Type = NT_MSGPORT;
    CxBase->cx_EventReply.mp_Flags        = PA_IGNORE;
    NewList(&CxBase->cx_EventReply.mp_MsgList);

    if (CxBase->cx_Zero = CreateCxObj(CX_ZERO,0,0))
        return(TRUE);

    return(FALSE);
}


/*****************************************************************************/


VOID ASM ShutdownCommodities(REG(a6) struct CxLib *CxBase)
{
struct CxLib      *cx = CxBase;
struct Message    *msg;
struct InputEvent *ptr;
struct InputEvent *next;

    while (msg = GetMsg(&cx->cx_EventReply))
        FreeCxMsg(msg);

    ptr = cx->cx_ReturnChain;
    while (ptr)
    {
        next = ptr->ie_NextEvent;
        FreePoolMem(CxBase,ptr);
        ptr = next;
    }
    cx->cx_ReturnChain = NULL;

    DeleteCxObj(cx->cx_Zero);

    FlushPools(CxBase);
}


/*****************************************************************************/


typedef VOID (* VOIDFUNC)();


VOID AttachIHandler(struct CxLib *CxBase)
{
    CxBase->cx_IHandlerPort.mp_Node.ln_Type = NT_MSGPORT;
    CxBase->cx_IHandlerPort.mp_Flags        = PA_SIGNAL;
    CxBase->cx_IHandlerPort.mp_SigBit       = SIGB_SINGLE;
    CxBase->cx_IHandlerPort.mp_SigTask      = FindTask(NULL);
    NewList(&CxBase->cx_IHandlerPort.mp_MsgList);

    CxBase->cx_IHandlerReq.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
    CxBase->cx_IHandlerReq.io_Message.mn_Length       = sizeof(struct IOStdReq);
    CxBase->cx_IHandlerReq.io_Message.mn_ReplyPort    = &CxBase->cx_IHandlerPort;

    if (OpenDevice("input.device",0,&CxBase->cx_IHandlerReq,0) == 0)
    {
        CxBase->cx_IHandlerNode.is_Code         = (VOIDFUNC)CxHandler;
        CxBase->cx_IHandlerNode.is_Data         = CxBase;
        CxBase->cx_IHandlerNode.is_Node.ln_Pri  = 53;
        CxBase->cx_IHandlerNode.is_Node.ln_Name = CxBase->cx_Lib.lib_Node.ln_Name;
        CxBase->cx_IHandlerReq.io_Command       = IND_ADDHANDLER;
        CxBase->cx_IHandlerReq.io_Data          = (APTR)&CxBase->cx_IHandlerNode;

        DoIO(&CxBase->cx_IHandlerReq);
    }
}


/*****************************************************************************/


VOID RemoveIHandler(struct CxLib *CxBase)
{
    if (CxBase->cx_IHandlerReq.io_Device)
    {
        CxBase->cx_IHandlerPort.mp_SigTask = FindTask(NULL);
        CxBase->cx_IHandlerReq.io_Command  = IND_REMHANDLER;
        DoIO(&CxBase->cx_IHandlerReq);
        CloseDevice(&CxBase->cx_IHandlerReq);
        CxBase->cx_IHandlerReq.io_Device = NULL;
    }
}


/*****************************************************************************/


VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)list;
}
