
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "commoditiesbase.h"
#include "exchange.h"


/*****************************************************************************/


struct cxBrokerCopy
{
    struct Node cxbc_Node;
    BROKERD     cxbc_BrokerD;
    LONG        cxbc_Flags;
};


/*****************************************************************************/


LONG ASM CopyBrokerListLVO(REG(a0) struct List *blist,
                           REG(a6) struct CxLib *CxBase)
{
struct Node         *node;
struct cxBrokerCopy *bcopy;
BROKERD             *bdata;
LONG                 count = 0;

    FreeBrokerList(blist);

    LockCX();

    /* walk broker list */
    node = (struct Node *)CXOBJLIST->mlh_Head;
    while (node->ln_Succ)
    {
        if (!(bcopy = AllocMem(sizeof(struct cxBrokerCopy),MEMF_ANY)))
           break;

        bdata                    = (BROKERD *) ((struct CxObj *) node)->co_Data;
        bcopy->cxbc_BrokerD      = *bdata;
        bcopy->cxbc_Node.ln_Name = bcopy->cxbc_BrokerD.cbd_Name;
        bcopy->cxbc_Flags        = ((struct CxObj *)node)->co_Flags;

        AddTail(blist,(struct Node *)bcopy);
        count++;
        node = node->ln_Succ;
    }

    UnlockCX();

    return (count);
}


/*****************************************************************************/


VOID ASM FreeBrokerListLVO(REG(a0) struct List *list,
                           REG(a6) struct CxLib *CxBase)
{
struct Node *node;

    while (node = RemHead(list))
        FreeMem(node,sizeof(struct cxBrokerCopy));
}
