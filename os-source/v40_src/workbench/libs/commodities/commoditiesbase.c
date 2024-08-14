
#include <exec/memory.h>
#include <exec/semaphores.h>

#include <devices/input.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "commoditiesbase.h"
#include "ihandlerstub.h"
#include "pool.h"
#include "messages.h"


/*****************************************************************************/


BOOL InitCommodities(VOID)
{
struct CxLib *cx = CxBase;

    InitSemaphore(&cx->cx_LibLock);

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


VOID ShutdownCommodities(VOID)
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
        FreePoolMem(ptr);
        ptr = next;
    }
    cx->cx_ReturnChain = NULL;

    DeleteCxObj(cx->cx_Zero);

    FlushPools();
}


/*****************************************************************************/


typedef VOID (* VOIDFUNC)();


VOID AttachIHandler(VOID)
{
struct CxLib *cx = CxBase;

    cx->cx_IHandlerPort.mp_Node.ln_Type = NT_MSGPORT;
    cx->cx_IHandlerPort.mp_Flags        = PA_SIGNAL;
    cx->cx_IHandlerPort.mp_SigBit       = SIGB_SINGLE;
    cx->cx_IHandlerPort.mp_SigTask      = FindTask(NULL);
    NewList(&cx->cx_IHandlerPort.mp_MsgList);

    cx->cx_IHandlerReq.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
    cx->cx_IHandlerReq.io_Message.mn_Length       = sizeof(struct IOStdReq);
    cx->cx_IHandlerReq.io_Message.mn_ReplyPort    = &cx->cx_IHandlerPort;

    if (OpenDevice("input.device",0,&cx->cx_IHandlerReq,0) == 0)
    {
        cx->cx_IHandlerNode.is_Code         = (VOIDFUNC)IHandlerStub;
        cx->cx_IHandlerNode.is_Data         = cx;
        cx->cx_IHandlerNode.is_Node.ln_Pri  = 53;
        cx->cx_IHandlerNode.is_Node.ln_Name = cx->cx_Lib.lib_Node.ln_Name;
        cx->cx_IHandlerReq.io_Command       = IND_ADDHANDLER;
        cx->cx_IHandlerReq.io_Data          = (APTR)&cx->cx_IHandlerNode;

        DoIO(&cx->cx_IHandlerReq);
    }
}


/*****************************************************************************/


VOID RemoveIHandler(VOID)
{
struct CxLib *cx = CxBase;

    if (cx->cx_IHandlerReq.io_Device)
    {
        cx->cx_IHandlerPort.mp_SigTask = FindTask(NULL);
        cx->cx_IHandlerReq.io_Command  = IND_REMHANDLER;
        DoIO(&cx->cx_IHandlerReq);
        CloseDevice(&cx->cx_IHandlerReq);
        cx->cx_IHandlerReq.io_Device = NULL;
    }
}


/*****************************************************************************/


VOID LockCX(VOID)
{
struct CxLib *cx = CxBase;

    ObtainSemaphore(&cx->cx_LibLock);
}


/*****************************************************************************/


VOID UnlockCX(VOID)
{
struct CxLib *cx = CxBase;

    ReleaseSemaphore(&cx->cx_LibLock);
}


/*****************************************************************************/


VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)list;
}
