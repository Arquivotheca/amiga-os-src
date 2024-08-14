
#include <exec/types.h>
#include <exec/nodes.h> 
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/tasks.h>
#include <exec/io.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>

typedef void (*__fptr)();

void NewList( struct List *lh ) {
    lh->lh_Tail = 0;
    lh->lh_TailPred = (struct Node *)lh;
    lh->lh_Head = (struct Node *)&lh->lh_Tail;
    }

struct IORequest *CreateExtIO(struct MsgPort *ioReplyPort, LONG size) {

register struct IORequest *ioReq;
    
    if (!ioReplyPort) return(NULL);

    if (!(ioReq = AllocMem(size, MEMF_CLEAR | MEMF_PUBLIC))) return(NULL);

    ioReq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;                        /* mark as finished */
    ioReq->io_Message.mn_Length = size;
    ioReq->io_Message.mn_ReplyPort = ioReplyPort;

    return(ioReq);
    }



void DeleteExtIO(struct IORequest *ioExt) {

    if (!ioExt) return;
    
    ioExt->io_Message.mn_Node.ln_Type = 0xff;
    ioExt->io_Device = (struct Device *)-1;
    ioExt->io_Unit = (struct Unit *)-1;
    
    FreeMem( ioExt, ioExt->io_Message.mn_Length );
    }



struct IOStdReq *CreateStdIO(struct MsgPort *ioReplyPort) {

    return((struct IOStdReq *)CreateExtIO(ioReplyPort, sizeof(struct IOStdReq)));
    }



void DeleteStdIO(struct IOStdReq *ioStdReq) {

    DeleteExtIO((struct IORequest *)ioStdReq);
    }



/*************************************************************************
 * CreateTask() / DeleteTask()                                           *
 *************************************************************************/


#define ME_TASK     0
#define ME_STACK    1
#define NUMENTRIES  2


struct FakeMemEntry {

    ULONG   fme_Reqs;
    ULONG   fme_Length;
    };


struct FakeMemList {

    struct Node         fml_Node;
    UWORD               fml_NumEntries;
    struct FakeMemEntry fml_ME[NUMENTRIES];
    };


struct FakeMemList  __far TaskMemTemplate = {
    {0},
    NUMENTRIES,
    {
        { MEMF_PUBLIC | MEMF_CLEAR, sizeof(struct Task) },
        { MEMF_CLEAR, 0 }
        }
    };



struct Task *CreateTask(char *name, LONG pri, APTR initPC, ULONG stacksize) {

register struct MemList *ml;
register struct Task    *new;
struct FakeMemList      fakememlist;

    stacksize = (stacksize + 3) & ~3;

    CopyMem ((char *) &TaskMemTemplate, (char *) &fakememlist, sizeof (fakememlist));

    fakememlist.fml_ME[ME_STACK].fme_Length = stacksize;

    ml = (struct MemList *)AllocEntry((struct MemList *)&fakememlist);

    if (((((ULONG)ml)& 0x80000000) == 0x80000000))return(NULL);

    new = (struct Task *)ml->ml_ME[ME_TASK].me_Addr;

    new->tc_SPLower      = ml->ml_ME[ME_STACK].me_Addr;
    new->tc_SPReg        =
    new->tc_SPUpper      = (APTR)((ULONG)new->tc_SPLower + stacksize);

    new->tc_Node.ln_Type = NT_TASK;
    new->tc_Node.ln_Pri  = pri;
    new->tc_Node.ln_Name = name;

    NewList(&new->tc_MemEntry);
    AddHead(&new->tc_MemEntry, ml);

    AddTask(new, (__fptr)initPC, NULL);
    return(new);
    }


void DeleteTask(struct Task *task) {

    RemTask(task);
    }


