#ifndef COMMODITIES_BASE_H
#define COMMODITIES_BASE_H


/*****************************************************************************/


#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <exec/io.h>
#include <exec/interrupts.h>
#include <devices/inputevent.h>
#include <dos.h>

#include "commodities.h"


/*****************************************************************************/


typedef IX FILTERD;

typedef struct
{
    struct MsgPort *sender_port;
    LONG            sender_id;
} SENDD;

typedef struct
{
    struct Task *signal_task;
    UBYTE        signal_signal;
} SIGNALD;

/**** ZZZ: note a copy of this is in the controller program ***/
typedef struct CxBrokerData
{
    char            cbd_Name[CBD_NAMELEN];     /* (internal) node name      */
    char            cbd_Title[CBD_TITLELEN];   /* user-visible title        */
    char            cbd_Descr[CBD_DESCRLEN];   /* user-visible description  */
    struct Task    *cbd_Task;                  /* who created this broker?  */
    struct MsgPort *cbd_Port;		       /* where to send CXM_COMMAND */
    WORD            cbd_Channel;
} BROKERD;

typedef struct
{
   LONG (*custom_action)();
   LONG custom_id;
} CUSTOMD;

union CxObjData
{
    FILTERD d_fd;
    SENDD   d_sendd;
    SIGNALD d_sigd;
    BROKERD d_brd;
    CUSTOMD d_cusd;
};


/*****************************************************************************/


struct CxObj
{
    struct Node      co_Node;
    UBYTE            co_Flags;
    UBYTE            co_Error;
    struct MinList   co_List;
    union CxObjData *co_Data;
};

/* CxObj Flags */
#define COF_INLIST    1
#define COF_ENABLE    2
#define COF_SHOW_HIDE 4

/* access to object common data   */
#define co_Succ      co_Node.ln_Succ
#define co_Type      co_Node.ln_Type
#define co_Name      co_Node.ln_Name
#define co_Pri       co_Node.ln_Pri
#define CO_TYPE(co)  (((co)->co_Type))
#define CO_LIST(co)  (&((co)->co_List))
#define CO_ERROR(co) (((co)->co_Error))
#define CO_DATA(co)  (((co)->co_Data))
#define CO_FLAGS(co) (((co)->co_Flags))
#define CO_PRI(co)   (((co)->co_Pri))

/* access to type specific data   */
#define FILTER_IX(filter)       ((IX *) CO_DATA(filter))
#define TFD(tf)                 ((LONG) CO_DATA(tf))
#define SENDER_PORT(s)          (CO_DATA(s)->d_sendd.sender_port)
#define SENDER_ID(s)            (CO_DATA(s)->d_sendd.sender_id)
#define SIGNAL_TASK(s)          (CO_DATA(s)->d_sigd.signal_task)
#define SIGNAL_SIGNAL(s)        (CO_DATA(s)->d_sigd.signal_signal)
#define XLATE_IE(xl)            ((struct InputEvent *) CO_DATA(xl))
#define ASSIGN_XLATE_IE(xl,new) (CO_DATA(xl) = (union CxObjData *)(new))
#define BROKER_DATA(b)          ((struct CxBrokerData *)CO_DATA(b))
#define DEBUGD(db)              ((LONG) CO_DATA(db))
#define CUSTOM_ACTION(c)        (CO_DATA(c)->d_cusd.custom_action)
#define CUSTOM_ID(c)            (CO_DATA(c)->d_cusd.custom_id)


/*****************************************************************************/


/* If CxMsg is made bigger, also make the value of NUMPOOLS (defined below)
 * bigger
 */

#define STACKSLOTS 50


struct CxMsg
{
    struct Message  cxm_Msg;           /* system linkage, or sent to port */
    struct CxObj   *cxm_Destination;
    LONG            cxm_ID;            /* copied from sending object   */
    UWORD           cxm_Echo;          /* how many times has this message been sent?   */
    UBYTE           cxm_Type;
    UBYTE	    cxm_StackIndex;
    struct CxObj   *cxm_RoutingStack[STACKSLOTS];
};

#define cxm_Node              cxm_Msg.mn_Node
#define cxm_Succ              cxm_Node.ln_Succ
#define cxm_Length            cxm_Msg.mn_Length
#define CXMSGTYPE(cxm)        ((cxm)->cxm_Type)
#define CXMSGDATA(cxm)        ((cxm)->cxm_Node.ln_Name)
#define CXMSGDESTINATION(cxm) ((cxm)->cxm_Destination)
#define CXMSGID(cxm)          ((cxm)->cxm_ID)

#define ROUTECXMSG(cxm, co)   ((cxm)->cxm_Destination = (co))

#define CXM_TOOMANYECHOS      1024


/*****************************************************************************/


struct CxObj *CreateCxObj( unsigned long type, long arg1, long arg2 );
struct CxObj *CxBroker( struct NewBroker *nb, LONG *error );
LONG ActivateCxObj(struct CxObj *co, long true );
void DeleteCxObj(struct CxObj *co );
void DeleteCxObjAll(struct CxObj *co );
ULONG CxObjType(struct CxObj *co );
LONG CxObjError(struct CxObj *co );
void ClearCxObjError(struct CxObj *co );
void SetCxObjPri(struct CxObj *co, long pri );
void AttachCxObj(struct CxObj *headobj,struct CxObj *co );
void EnqueueCxObj(struct CxObj *headobj,struct CxObj *co );
void InsertCxObj(struct CxObj *headobj,struct CxObj *co,struct CxObj *pred );
void RemoveCxObj(struct CxObj *co );
void SetTranslate(struct CxObj *translator, struct InputEvent *events );
void SetFilter(struct CxObj *filter, STRPTR text );
void SetFilterIX(struct CxObj *filter, IX *ix );
LONG ParseIX(STRPTR description, IX *ix );
ULONG CxMsgTyp(struct CxMsg *cxm );
UBYTE *CxMsgData(struct CxMsg *cxm );
LONG CxMsgID(struct CxMsg *cxm );
void DivertCxMsg(struct CxMsg *cxm,struct CxObj *headobj,struct CxObj *ret );
void RouteCxMsg(struct CxMsg *cxm,struct CxObj *co );
void DisposeCxMsg(struct CxMsg *cxm );
BOOL InvertKeyMap( unsigned long ansicode, struct InputEvent *event, struct KeyMap *km );
void AddIEvents( struct InputEvent *events );
LONG BrokerCommand( STRPTR name, LONG longcommand );
BOOL MatchIX(struct InputEvent *ie, IX *ix );
VOID FreeBrokerList(struct List *list);


/*****************************************************************************/


#pragma libcall CxBase CreateCxObj 1e 98003
#pragma libcall CxBase CxBroker 24 0802
#pragma libcall CxBase ActivateCxObj 2a 0802
#pragma libcall CxBase DeleteCxObj 30 801
#pragma libcall CxBase DeleteCxObjAll 36 801
#pragma libcall CxBase CxObjType 3c 801
#pragma libcall CxBase CxObjError 42 801
#pragma libcall CxBase ClearCxObjError 48 801
#pragma libcall CxBase SetCxObjPri 4e 0802
#pragma libcall CxBase AttachCxObj 54 9802
#pragma libcall CxBase EnqueueCxObj 5a 9802
#pragma libcall CxBase InsertCxObj 60 A9803
#pragma libcall CxBase RemoveCxObj 66 801
#pragma libcall CxBase SetTranslate 2 9802
#pragma libcall CxBase SetFilter 78 9802
#pragma libcall CxBase SetFilterIX 7e 9802
#pragma libcall CxBase ParseIX 84 9802
#pragma libcall CxBase CxMsgType 8a 801
#pragma libcall CxBase CxMsgData 90 801
#pragma libcall CxBase CxMsgID 96 801
#pragma libcall CxBase DivertCxMsg 9c A9803
#pragma libcall CxBase RouteCxMsg a2 9802
#pragma libcall CxBase DisposeCxMsg a8 801
#pragma libcall CxBase InvertKeyMap ae 98003
#pragma libcall CxBase AddIEvents b4 801
#pragma libcall CxBase CopyBrokerList ba 801
#pragma libcall CxBase FreeBrokerList c0 801
#pragma libcall CxBase BrokerCommand c6 0802
#pragma libcall CxBase MatchIX cc 9802


/*****************************************************************************/


#define NUMPOOLS 33


/*****************************************************************************/


struct CxLib
{
    struct Library          cx_Lib;
    UWORD		    cx_UsageCnt;
    APTR                    cx_UtilityBase;
    APTR                    cx_KeymapBase;
    APTR                    cx_SysBase;
    APTR		    cx_GfxBase;
    ULONG                   cx_SegList;

    struct IOStdReq         cx_IHandlerReq;
    struct Interrupt        cx_IHandlerNode;
    struct MsgPort          cx_IHandlerPort;

    struct MinList	    cx_Objects;
    struct MinList   	    cx_Messages;
    struct CxObj           *cx_Zero;       /* to send messages if end of chain */

    struct SignalSemaphore  cx_LibLock;
    WORD		    cx_Pad0;       /* long align */

    struct InputEvent      *cx_EndChain;
    struct InputEvent      *cx_ReturnChain;

    APTR                    cx_Pools[NUMPOOLS];

    struct MsgPort          cx_EventReply;
};


#define ASM         __asm
#define REG(x)	    register __ ## x

#define SysBase     CxBase->cx_SysBase
#define GfxBase     CxBase->cx_GfxBase
#define UtilityBase CxBase->cx_UtilityBase
#define KeymapBase  CxBase->cx_KeymapBase

#define CXOBJLIST   (&CxBase->cx_Objects)
#define CXMSGLIST   (&CxBase->cx_Messages)

sprintf(STRPTR,...);


/*****************************************************************************/


#define LockCX()   ObtainSemaphore(&CxBase->cx_LibLock)
#define UnlockCX() ReleaseSemaphore(&CxBase->cx_LibLock)


/*****************************************************************************/


BOOL ASM InitComodities(REG(a6) struct CxLib *CxBase);
VOID ASM ShutdownCommodities(REG(a6) struct CxLib *CxBase);

VOID AttachIHandler(struct CxLib *CxBase);
VOID RemoveIHandler(struct CxLib *CxBase);

VOID NewList(struct List *list);


/*****************************************************************************/


#endif /* COMMODITIES_BASE_H */
