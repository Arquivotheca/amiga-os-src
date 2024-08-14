
#include <string.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "commoditiesbase.h"
#include "broker.h"
#include "messages.h"


/*****************************************************************************/


/* Return values for BrokerCommand() (for use by Exchange program) */
#define CMDE_OK        (0)
#define CMDE_NOBROKER  (-1)
#define CMDE_NOPORT    (-2)
#define CMDE_NOMEM     (-3)


/*****************************************************************************/


static LONG DoBrokerCommand(struct CxObj *broker, WORD commandid)
{
struct CxMsg   *msg;
struct MsgPort *port;
struct Task    *task;

    if (!broker)
        return (CMDE_NOBROKER);

    if (!(port = BROKER_DATA(broker)->cbd_Port))
    {
        /* alternate means for kill command   */
        if ((commandid == CXCMD_KILL) && (task = BROKER_DATA(broker)->cbd_Task))
        {
            Signal(task,SIGBREAKF_CTRL_E);
        }
        return (CMDE_NOPORT);
    }

    if (msg = CreateCxMsg(CXM_COMMAND,NULL))
    {
        CXMSGID(msg) = commandid;
        PutMsg(port,(struct Message *)msg);

        return(CMDE_OK);
    }

    return (CMDE_NOMEM);
}


/*****************************************************************************/


LONG ASM BrokerCommandLVO(REG(a0) STRPTR name,
                          REG(d0) LONG longcommand)
{
LONG         result;
struct List *list;

    if (!name)
        name = "Exchange";

    LockCX();
    list = (struct List *)CXOBJLIST;
    result = DoBrokerCommand((struct CxObj *)FindName(list,name),(WORD)longcommand);
    UnlockCX();

    return(result);
}


/*****************************************************************************/


struct CxObj * ASM CxBrokerLVO(REG(a0) struct NewBroker *nb, REG(d0) LONG *errorptr)
{
struct CxObj *broker;
LONG          err = CBERR_OK;
struct List  *list;

    LockCX();

    /* duplicate protection */
    list = (struct List *)CXOBJLIST;
    broker = (struct CxObj *)FindName(list,nb->nb_Name);

    if (broker && (nb->nb_Unique & NBU_NOTIFY))
        DoBrokerCommand(broker,CXCMD_UNIQUE);

    if (broker && (nb->nb_Unique & NBU_UNIQUE))
    {
        err    = CBERR_DUP;
        broker = NULL;
    }
    else
    {
        if (broker = CreateCxObj(CX_BROKER,(LONG)nb,0))
        {
            if (IsListEmpty(list))
                AttachIHandler();
            else
                BrokerCommand(NULL,CXCMD_LIST_CHG);

            /* note: don't use EnqueueCxObj, which takes CxObj param */
            Enqueue(list,(struct Node *)broker);
            CO_FLAGS(broker) |= COF_INLIST;
        }
        else
        {
            err = CBERR_SYSERR;
        }
    }

    UnlockCX();

    /* pass back error codes if caller wants them */
    if (errorptr != NULL)
        *errorptr = err;

    return (broker);
}


/*****************************************************************************/


static VOID CopyStr(STRPTR to, STRPTR from, WORD maxLen)
{
   to[--maxLen] = 0;

   while (maxLen-- > 0)
       to[maxLen] = from[maxLen];
}


/*****************************************************************************/


VOID InitBroker(struct CxObj *broker, struct NewBroker *nb)
{
struct CxBrokerData *cbd = BROKER_DATA(broker);

    CopyStr(cbd->cbd_Name,  nb->nb_Name,  CBD_NAMELEN);
    CopyStr(cbd->cbd_Title, nb->nb_Title, CBD_TITLELEN);
    CopyStr(cbd->cbd_Descr, nb->nb_Descr, CBD_DESCRLEN);
    cbd->cbd_Task = FindTask(NULL);

    broker->co_Flags &= ~COF_ENABLE;   /* start disabled   */
    broker->co_Name   = &cbd->cbd_Name[0];
    broker->co_Pri    = nb->nb_Pri;

    if (nb->nb_Version >= 5)
        cbd->cbd_Port = nb->nb_Port;
    else
        cbd->cbd_Port = NULL;
}
