/****** que/queue-handler *****************************************************

    NAME
        queue_handler -- an 'infinite' pipe (FIFO) handler

    VERSION
        1.06
        1.07    10 Jan 1991
        1.08    01 Mar 1991 - changed behavior to dump orphaned
                              data overboard

    AUTHOR
        Fred Mitchell, Products Assurance, Commodore-Amiga, INC.

    SYNOPSIS
        open:
            QUE:[channel_name][[/[buf_size]][/que_limit]]]

            (Device name is specified in the MountList)

        packets reconized:
            ACTION_STARTUP
            ACTION_FINDINPUT
            ACTION_FINDOUTPUT
            ACTION_FINDUPDATE
            ACTION_READ
            ACTION_WRITE
            ACTION_END
            ACTION_IS_FILESYSTEM

    FUNCTION
        queue-handler takes input from the output channel, buffers it up,
        allocating more memory as needed, and sends to to the input channel,
        as requested by the applications. Que never locks the output channel
        unless a que_limit was specified. Quantum buffer size can be
        specified only on the first reference to a particular channel.

        Queue-handler works by invoking a seperate process every time an
        open packet is received, and uses a fifo buffer pool approach to
        track the actual data.

    INPUTS
        channel_name    --  Unique channel name to specify. Must begin with a
                            non-numeric character. The channel_name is
                            optional.

        buf_size        --  quantum size in bytes of the buffers to allocate
                            (default is 1024). The buf_size This is optional.

        que_limit       --  Max. number of Que Buffers allowed. Will suspend
                            the output channel if exceeded. If que_limit is
                            specified as zero (0), then the limit becomes the
                            number that will fit within your total available
                            RAM.

    MOUNTLIST ENTRIES
        Simple Handler with internal defaults:
            QUE:    Handler = l:queue-handler
                    Priority = 5
                    StackSize = 2000
                    globvec = -1
            #

        Specification of default buffer size and number of buffers. All
        entries need to be present for the MOUNT command to function;
        however, only the first 6 entries affect the queue-handler. The
        remaining are ignored. "BlockSize" becomes the default for buf_size,
        and "Buffers" become the default for que_limit:

            QUE:    FileSystem = l:queue-handler
                    Priority = 5
                    StackSize = 2000
                    GlobVec = -1
                    BlockSize = 1024
                    Buffers = 2

                    Surfaces = 1
                    BlocksPerTrack = 1
                    LowCyl = 0
                    HighCyl = 1
                    Device = ""
                    Unit = 0
            #


    EXAMPLE
        From process 1:
            list >que: work: all

        From process 2:
            type que:

    EXAMPLE
        To gather the results of several c compilations:
            lc >que:ll foo
            lc >que:ll bar
            lc >que:ll road
            lc >que:ll kill
            ed que:ll

    EXAMPLE
        To use channel names:
            list >que:crazy
            copy #?.c to >que:all_c/32000   ;Specifies a channel 'all_c' and
                                            ;a quatum buffer size of 32000
                                            ;bytes
    EXAMPLE
        To set a limit on the number of que buffers to 5:
            dir >que://5        ; create a channel without a channel-name, and
                                ; only allow 5 que buffer entries.

    BUGS
        Full checking is not done to ensure that the application can't read
        from a write channel or vice-versa. The results of this operation
        shall be undefined. Don't Do It.

*****i* que/internal docs/queue-handler ***************************************

    * The old-style method of specifying quantum-buffers has been slightly
      altered. Slashes are now required for delimiting. See docs above.

*****i* que/edit history/queue-handler ****************************************

    HOLD FOR OUTPUT - 07 Nov 1990 Fred Mitchell (1.04)
        The input-channel behavior of Que as of 07 Nov 1990 has been modified
        according to the following paradigm: When both input and output sides of
        the channel are opened and the channel is exhausted, the former behavior
        was that the input channel got returned an ACTION_READ packet of zero (0)
        bytes. Many readers will interpret this as an EOF indication and proceede
        to shut down. In lieu of this, the new methodology disallows zero-length
        ACTION_READ packets when the output channel remains open. Instead, the
        input channel will suspend itself, pending a wakeup call from the output
        channel, at which point it will restart the ACTION_READ process. In this
        way, the input channel will be held open by application programs until
        the output channel closes.

        Note that, upon closing, the output channel MUST signal the input channel
        in the same manner as if data were intercepted. This way, the input
        channel will attempt check the channel and not hang around waiting for
        that wakeup call that will never come.

    OPTIONAL MAX BUFFS SPECIFICATION - 09 Nov 1990 Fred Mitchell (1.05)
        **  Plan of attack
            We have several issues here. We would like to place an optional
            'cap' on the amount of memory Que: can use. There are two ways to
            accomplish this:
                (1) Specification by BYTE size
                (2) Specification by max. quantum buffers

            (1) is the most intuitive way to approach the problem. But it adds to
                the overhead requirement  and will not give 'exact' results anyway,
                since Que allocates everything on the quantum-buffer basis, anyway.
                This will only lead to an internal case of (2).

            (2) involves some technical knowledge on the part of the user, but
                if the user is already dealing with Que at this level, he is
                likely to understand the issue anyway. And this will be the simplist
                to implement.

            In lieu of these two view points, we have decided to go with (2).

        **  Behavior of an overflow case
            When the output channel overflows, we have two options: we can
            either abort the write operation, or we can hold the task until
            the input channel frees up more space. We will, of course, go with
            the latter. Also, the manner of how and when the output channel
            gets re-awakened will be a touchy issue. Since all of this is
            based on buffering, we have decided to re-awaken the output task
            when a que-buffer is freed.

            ISSUE: We must avoid the possibility of dead-locking the two. Is
            there a case when both the output channel and input channel
            can become locked?

            The only time the input channel becomes suspended is when
            a) there are no queue buffers present
            b) the output channel was never opened
            It would seem safe to assume that there would be no problem here,
            given these two cases.

    MOUNTLIST SUPPORT FOR MAX BUFFER SPECIFICATION - 29 Nov 1990 Fred Mitchell (1.06)

    08 Jan 1991 FM  Fixed problem with TurboText interaction.

    01 Mar 1991 FM  (1.08) CAREFUL modification of behavior to dump orphaned
                    bufs overboard. This change was made just in case Eng.
                    aggrees to it. Since we are 'out of time', we did
                    nothing else fancy; No Feature Creepers! :-(
                    Reverting to 37.9 behavior is accomplished by the #define
                    OLD_37_9 .


******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <devices/serial.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include "queue-handler.h"
#include <string.h>
#include <pragmas/exec_pragmas.h>
#include "protos.h"
#include "queue-handler.i"
#include "queue-handler_rev.h"

#define DEFAULT_BUFSIZE     1024    // Default Bufsize
#define DEFAULT_NUMBUFFERS  8       // Default max. number of buffers

struct Library  *SysBase = NULL; /* HARD-COODED */
struct Library	*DOSBase	= NULL;

char *version = VERSTAG;

// This is naughty, but is the only way for now... static single-threaded pointer to queue list
static Queue *qlist = NULL;

#define OPLIB(base, name) if (!base) if (!(base = (void *) OpenLibrary(name, 36))) { libfail = TRUE; }

#define ISFULL(qb)  (((qb)->in == (qb)->out-1) || (!(qb)->out && (qb)->in == (qb)->q->bufsize-1))
#define ISEMPTY(qb) ((qb)->in == (qb)->out)
#define ADDBYTE(qb, c)  { (qb)->data[(qb)->in++] = (c); if ((qb)->in >= (qb)->q->bufsize) (qb)->in = 0; }
#define GETBYTE(qb, c)  { (c) = (qb)->data[(qb)->out++]; if ((qb)->out >= (qb)->q->bufsize) (qb)->out = 0; }

__saveds void queue_handler()  /* ENTRY POINT!!! Register A4 loaded here */
{
    // Tasks, Ports, and Messages
    struct Process *task;
    struct MsgPort *port;
    struct Message *msg;
    struct DosPacket *pkt;

    // Device Information (Initialized at startup. Is not expected to change during the existence of this process.)
    struct DeviceNode *devnode = NULL;
    struct FileSysStartupMsg *fssm = NULL;
    struct DosEnvec *de = NULL;

    // General
    struct FileHandle *fh;
    Queue *q = NULL;    // Instance of q
    QueBuf *qb;
    UBYTE name[64], dev[20];
    long bufsize;   // Not to be used execept for holder of the parsing result. Use q->bufsize instead.
    long limit;     // Ditto for limit.
    BOOL libfail = FALSE, loop;
    ULONG sigmask, sigbits, portbit = NULL;
    int size;
    UBYTE *b; // general pointer
    BOOL out = FALSE, in = FALSE; // IO status of this process. out is ACTION_FINDOUTPUT, in is ACTION_FINDINPUT.

    //Set up SysBase
    SysBase = *((struct Library **) 4);

#ifdef DEBUG
    kprintf("q: Entered\n");
#endif
    /* Get this process's port
    */
    task = (struct Process *) FindTask(NULL);
    port = &task->pr_MsgPort;

    /* Event Loop
    */
    for (loop = TRUE, sigbits = portbit = 1 << port->mp_SigBit; loop; )
    {
#ifdef DEBUG
        kprintf("\nq: Waiting...\n");
#endif
        sigmask = Wait(sigbits);
#ifdef DEBUG
        kprintf("q: Woke Up...");
#endif
        if (sigmask & portbit)  /* Message from the process's port */
            while (msg = (void *) GetMsg(port))
            {
                pkt = (struct DosPacket *) msg->mn_Node.ln_Name;
#ifdef DEBUG
                kprintf("q:pkt: Type %ld, dp_Port=%lx, dp_Link=%lx, myport=%lx, pkt=%lx\n", pkt->dp_Type, pkt->dp_Port, pkt->dp_Link, port, pkt);
#endif
                switch (pkt->dp_Type)
                {
                case ACTION_STARTUP:
                    /* open the libraries, if they are not opened yet
                    */
#ifdef DEBUG
                    kprintf("q: invoked ACTION-STARTUP. Task=%lx, qlist=%lx\n", task, qlist);
#endif
                    OPLIB(DOSBase, "dos.library");

                    if (libfail) /* some library failed to open? */
                    {
#ifdef DEBUG
                        kprintf("q:???Library screwup\n");
#endif
                        pkt->dp_Res1 = DOSFALSE;
                        pkt->dp_Res2 = ERROR_INVALID_RESIDENT_LIBRARY;
                        loop = FALSE;
                        break;
                    }
#ifdef  DEBUG
                    kprintf("q: Libraries are open\n");
#endif
                    /* We want a new process on each subsequent open.
                    ** So we clear the DosNode task pointer. <Linda T>
                    */
                    devnode = (struct DeviceNode *) BADDR(pkt->dp_Arg3);
                    devnode->dn_Task = NULL;

                    // And let us retreive more device information.
                    if (fssm = (struct FileSysStartupMsg *) BADDR(devnode->dn_Startup))
                        de = (struct DosEnvec *) BADDR(fssm->fssm_Environ);
#ifdef DEBUG
                    kprintf("q:ACTION_STARTUP DEVNODE: de=%lx, fssm=%lx, devnode=%lx %lx \n", de, fssm, devnode, devnode->dn_Startup);
#endif

                    // Everything's OK, so tell our caller. We won't hang around, either!
                    pkt->dp_Res1 = DOSTRUE;
                    pkt->dp_Res2 = 0;
                    break;

                case ACTION_FINDOUTPUT:
                // case ACTION_FINDUPDATE:
                case ACTION_FINDINPUT:
                    //Get FileHandle and name field
                    fh = (struct FileHandle *) BADDR(pkt->dp_Arg1);
                    BSTRcptoSTR(pkt->dp_Arg3, name);
#ifdef DEBUG
                    kprintf("q:ACTION_FINDx Name to Parse = %s type(%ld)\n", name, pkt->dp_Type);
#endif
                    // Set a couple of default limits   &&& is the divide by sizeof(ULONG) needed?
                    limit = (de && de->de_TableSize > DE_NUMBUFFERS)
                                    ? de->de_NumBuffers
                                    : DEFAULT_NUMBUFFERS;  // The default limit.
                    bufsize = (de && de->de_TableSize > DE_SIZEBLOCK && de->de_SizeBlock)
                                    ? de->de_SizeBlock * sizeof(ULONG)
                                    : DEFAULT_BUFSIZE;      // The default bufsize

#ifdef DEBUG
                    kprintf("q:ACTION_FINDx Defaults: de=%lx, ts=%ld, limit=%ld, bufsize=%ld\n", de, (de) ? de->de_TableSize : 0,  limit, bufsize);
#endif
                    if (ParseName(name, dev, &bufsize, &limit))
                    {
                        // Search for an existing queue for input, or create a new one.
                        Forbid();
                        if (!(q = FindQueue(dev)))
                            q = AddQueue(dev, bufsize, limit);

                        if (pkt->dp_Type == ACTION_FINDOUTPUT)
                        {
                            if (q->task_out) // already opened for output?
                            {
                                loop = FALSE;
                                pkt->dp_Res1 = DOSFALSE;
                                pkt->dp_Res2 = ERROR_OBJECT_IN_USE;
                                break;
                            }
                            q->task_out = task, out = q->ever_out = TRUE;
                        }
                        else if (pkt->dp_Type == ACTION_FINDINPUT)
                        {
                            if (q->task_in) // already opened for input?
                            {
                                loop = FALSE;
                                pkt->dp_Res1 = DOSFALSE;
                                pkt->dp_Res2 = ERROR_OBJECT_IN_USE;
                                break;
                            }
                            q->task_in = task, in = q->ever_in = TRUE;
                        }
                        ++q->open_cnt;
                        Permit();

                        // We're OK!
                        if (q)
                        {
                            pkt->dp_Res1 = DOSTRUE;
                            pkt->dp_Res2 = 0;
                        }
                        else // Out of memory, perhaps?
                        {
                            loop = FALSE;
                            pkt->dp_Res1 = DOSFALSE;
                            pkt->dp_Res2 = ERROR_NO_FREE_STORE;
                        }
                    }
                    else /* parse error */
                    {
                        loop = FALSE;
                        pkt->dp_Res1 = DOSFALSE;
                        pkt->dp_Res2 = ERROR_BAD_STREAM_NAME;
                    }
                    break;

                case ACTION_WRITE:  // (fh, buf, len) -> act_len  // We write at the Head  (we're OUT)
#ifdef  DEBUG
                    kprintf("q: WRITE len=%ld o=%ld i=%ld\n", pkt->dp_Arg3, out, in);
#endif

                    // The following FOR loop is ineffecient. Can be replaced later by a more efficient copy scheme...
                    for (size = pkt->dp_Arg3, b = (void *) pkt->dp_Arg2, qb = q->qbhead; size > 0; ++b, --size) // i is index for packet buf
                    {
                        if (!qb) // No buffers? Just in case.
                            qb = AddNewQueBuf(q);

                        if (qb) // We have a quebuf!
                        {
                            if (ISFULL(qb))
                                if (!(qb = AddNewQueBuf(q)))  // Did we not get a new qb?
                                    break;

                            // We have room for at least one more byte!
                            ADDBYTE(qb, *b);
                        }
                        else // Out of memory!
                            break;
                    }
                    // Check to see if the input channel is sleepy. If so, wake it up.
                    if (q->in_wait)
                        Signal(q->task_in, 1 << q->in_sig);

                    pkt->dp_Res1 = pkt->dp_Arg3 - size;
                    pkt->dp_Res2 = 0;
                    break;

                case ACTION_READ:   // (fh, buf, len) -> act_len  // We read from the tail (we're IN)

#ifdef  DEBUG
                    kprintf("q READ: len=%ld q=%lx o=%ld i=%ld\n", pkt->dp_Arg3, q, out, in);
#endif
#ifdef OLD
                    // The following FOR loop is ineffecient. Can be replaced later by a more efficient copy scheme...
                    for (size = pkt->dp_Arg3, b = pkt->dp_Arg2, qb = q->qbtail; size > 0; ++b, --size) // i is index for packet buf
                    {
                        if (qb) // Buffers?
                        {
                            if (ISEMPTY(qb))
                                if (!(qb = DeleteOldQueBuf(q)))  // Did we not get a new qb?
                                    break;

                            // We have room for at least one more byte!
                            GETBYTE(qb, *b);
                        }
                        else
                            break;
                    }

                    pkt->dp_Res1 = pkt->dp_Arg3 - size;
                    pkt->dp_Res2 = 0;
#else   // NEW - go to sleep if exhausted
                    // The following FOR loop is ineffecient. Can be replaced later by a more efficient copy scheme...
                    for (size = pkt->dp_Arg3, b = (void *) pkt->dp_Arg2, qb = q->qbtail; size > 0;) // i is index for packet buf
                    {
                        if (qb) // Buffers?
                        {
                            if (ISEMPTY(qb))
                                if (!(qb = DeleteOldQueBuf(q)))  // Did we not get a new qb?
                                {
                                    /*  Wait to be awakened by the Output channel (input to us). We
                                    **  will wait under these conditions:
                                    **      a) The output channel was never opened (ever_out is FALSE)
                                    **      b) The output channel is opened, but the channel is exhausted
                                    **  We will go into a forbid and check again for a new qb.
                                    */
                                    Forbid();
                                    while (!q->ever_out || (q->task_out && !(qb = q->qbtail)))
                                    {
#ifdef DEBUG
                                        kprintf("q:READ Waiting for data\n");
#endif
                                        WaitForOutputChannel(q);
                                    }
                                    Permit();

                                    if (!qb)   // Still empty channel!
                                        break;
                                }

                            // We have room for at least one more byte!
                            GETBYTE(qb, *b);
                            ++b, --size;
                        }
                        else if (Forbid(), !q->ever_out || (q->task_out && !(qb = q->qbtail)))
                        {
                            WaitForOutputChannel(q);
                            qb = q->qbtail;
                            Permit();
                        }
                        else
                        {
                            Permit();
                            break;
                        }
                    }

                    pkt->dp_Res1 = pkt->dp_Arg3 - size;
                    pkt->dp_Res2 = 0;
#endif
                    break;

                case ACTION_END:
                    Forbid();
                    --q->open_cnt;
#ifdef  DEBUG
                    kprintf("q: END: open_count = %ld\n", q->open_cnt);
#endif
                    if (out)
                    {
                        out = FALSE, q->task_out = NULL;
                        // Check to see if the input channel is sleepy. If so, wake it up.
                        if (q->in_wait)
                            Signal(q->task_in, 1 << q->in_sig); // If the input channel regains control here, we're safe.
                    }
                    if (in) in = FALSE, q->task_in = NULL;
                    DeleteQueue(q);
                    Permit();

                    q = NULL; // Just in case we don't exit as expected!
                    loop = FALSE;

                    pkt->dp_Res1 = DOSTRUE;
                    pkt->dp_Res2 = 0;
                    break;

                case ACTION_IS_FILESYSTEM:
                    loop = FALSE;
                    pkt->dp_Res1 = DOSFALSE;
                    pkt->dp_Res2 = 0;
                    break;
#ifdef OLD
                case ACTION_WAIT_CHAR:
                case ACTION_DIE:
                case ACTION_EVENT:
#ifdef DEBUG
                    kprintf("q: umip action #%ld\n", pkt->dp_Type);
#endif
                    pkt->dp_Res1 = DOSFALSE;
                    pkt->dp_Res2 = ERROR_NOT_IMPLEMENTED;
                    loop = !!q; // Exit if file not opened
                    break;
#endif
                default:
#ifdef DEBUG
                    kprintf("q: invoked Unknown ACTION #%ld.\n", pkt->dp_Type);
#endif
                    pkt->dp_Res1 = DOSFALSE;
                    pkt->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
                    loop = !!q; // Exit if file not opened
                    break;
                }
#ifdef DEBUG
                kprintf("q:\07 *** return pkt=[%ld,%ld] ****qlist=%lx\n", pkt->dp_Res1, pkt->dp_Res2, qlist);
#endif
                ReturnPacket(pkt, port);
            }
    }
#ifdef DEBUG
    kprintf("q: At end of main.\n");
#endif

}

#ifdef OLD
void ShutDown() /* Hey, we have a problem! We just can't exit(), now can we? */
{
    /* We have another problem! We can't just close libraries- we are multithreaded! &&& */
    return;;     /* I'm warning you this might not work! &&& */
}
#endif

void ReturnPacket (struct DosPacket *pkt, struct MsgPort *myport)
{
    struct Message *mess;
    struct MsgPort *replyport;

    replyport		  = pkt->dp_Port;
    mess		   	  = pkt->dp_Link;
    pkt->dp_Port	  = myport;
#ifdef DEBUG_1
    kprintf("q:ReturnPacket: dp_Port=%lx, dp_Link=%lx, myport=%lx, pkt=%lx\n",
                pkt->dp_Port,
                pkt->dp_Link,
                myport,
                pkt);
#endif

    mess->mn_Node.ln_Name   = (char *) pkt;
    mess->mn_Node.ln_Succ   = NULL;
    mess->mn_Node.ln_Pred   = NULL;

#ifdef DEBUG_1
    kprintf("q:ReturnPacket: About to call PutMsg()");
#endif
    PutMsg (replyport, mess);
#ifdef DEBUG_1
    kprintf("q:ReturnPacket: Return from PutMsg()");
#endif
}

BOOL ParseName(char *name, char *dev, long *pbsize, long *plimit) // Returns TRUE if successful
{
    char buf[64], *s, *p;
    BOOL ret = TRUE;
    int slcnt;  /* slash count */

    // we break on ':', '/', or '\0'
    for (slcnt = 1, s = name, *(p = buf) = '\0'; ret && ((s == name) || *(s-1)); ++s)
    {
        switch(*s)  {
        case ':':   //device name
            strcpy(dev, buf);   // Copy device name out
            *(p = buf) = '\0';  //reset to beginning
            break;

        case '/':
        case '\0':
            if (strlen(buf))   // if there is a string
            {
                switch(slcnt)
                {
                case 1: // Channel Name
                    strcat(strcat(dev, ":"), buf);  // Append Channel name
                    break;

                case 2: // Quantum Buffer Size
                    if (strlen(buf))
                        stcd_l(buf, pbsize);
                    ret = *pbsize >= 2; // FALSE if buffer size is 1 or 0 or a negative value
                    break;

                case 3: // Que Buffer Limit
                    if (strlen(buf))
                        stcd_l(buf, plimit);
                    ret = *plimit >= 0; // FALSE if limit is negative
                    break;

                default:
                    ret = FALSE;
                    break;
                }
            }

            ++slcnt;
            *(p = buf) = '\0'; /*reset to beginning*/
            break;

        default:
            *p++ = *s;
            *p = '\0';
            break;
        }
    }
    return ret;
}

QueBuf *AddNewQueBuf(Queue *q) // Adds new QueBuf to head of list. Only the output task will call this!
{
    QueBuf *qb = NULL;

#ifdef DEBUG
    kprintf("q:AddNewQueBuf q=%lx ", q);
#endif
    Forbid();

    // Check for buffer overrun
    while (q->qblimit && q->qbcount >= q->qblimit)
        WaitForInputChannel(q);

    // Add the buffer
    if (qb = (void *) AllocMem(sizeof(*qb) + q->bufsize, MEMF_PUBLIC | MEMF_CLEAR))
    {
        qb->next = q->qbhead;
        q->qbhead = qb;
        if (!q->qbtail)
            q->qbtail = qb;
        else
            qb->next->prev = qb;
        qb->q = q;
        ++q->qbcount;
    }
#ifdef DEBUG
    kprintf(" qb=%lx next=%lx, prev=%lx\n",qb, qb->next, qb->prev);
#endif
    Permit();
    return qb;
}

QueBuf *DeleteOldQueBuf(Queue *q)  // Deletes old buf from tail of list. Returns next tail. Only input channel will call this!
{
    QueBuf *qb;

#ifdef DEBUG
    kprintf("q:DeleteOldQueBuf q=%lx head=%lx tail=%lx ******* ", q, q->qbhead, q->qbtail);
#endif
    if (qb = q->qbtail)
    {
        Forbid();
        if (q->qbhead == q->qbtail)     // Only one entry?
            q->qbhead = NULL;
        q->qbtail = qb->prev;
        FreeMem(qb, sizeof(*qb) + q->bufsize);

        --q->qbcount;
        if (q->out_wait)
            Signal(q->task_out, 1 << q->out_sig);

        Permit();
    }
#ifdef DEBUG
    kprintf(" return qbtail=%lx\n", q->qbtail);
#endif
    return q->qbtail;
}

#ifndef OLD_37_9
void FlushQueBufs(Queue *q) // Flush all remaining Queue bufs.
{
    Forbid();
    while (DeleteOldQueBuf(q))
        ;
    Permit();   // Question: What happens if new data is slapped in at this point?
}
#endif

Queue *AddQueue(UBYTE *devname, long bufsize, long limit)   //  Add a new queue structure
{
    Queue *q = NULL;

#ifdef DEBUG
    kprintf("q:AddQueue %s bsize=%ld, limit=%ld\n", devname, bufsize, limit);
#endif
    if (q = (void *) AllocMem(sizeof(*q), MEMF_PUBLIC | MEMF_CLEAR))
    {
        strcpy(q->name, devname);
        q->bufsize = bufsize;
        q->qblimit = limit;

        Forbid();
        if (qlist)
        {
            qlist->prev = q;
        }
        q->next = qlist;
        qlist = q;
        Permit();
    }
    return q;
}

Queue *FindQueue(UBYTE *devname)     // Find a queue by name...
{
    Queue *q;

#ifdef DEBUG
    kprintf("q:FindQueue %s\n", devname);
#endif
    Forbid();
    for (q = qlist; q; q = q->next)
        if (!stricmp(devname, q->name))
            break;
    Permit();
    return q;
}

BOOL DeleteQueue (Queue *q)     // Delete Queue from list, ONLY IF there are no QueBufs left... (later we may modify this)
{
    BOOL did_we;  // Did we delete it?

#ifdef DEBUG
    kprintf("q:DeleteQueue %lx head=%lx tail=%lx next=%lx prev=%lx ", q, q->qbhead, q->qbtail, q->next, q->prev);
#endif
    Forbid();
#ifdef OLD_37_9
    if (did_we = (q->ever_in && q->ever_out)
                 && !q->open_cnt
                 && (!q->qbtail || ((q->qbtail == q->qbhead) && ISEMPTY(q->qbtail)))) // did_we = Is queue empty?
#else // 37.12 or beyond
    if (did_we = (q->ever_in && q->ever_out) && !q->open_cnt)
#endif
    {
#ifndef OLD_37_9
        FlushQueBufs(q);
#endif
        if (q->next)
            q->next->prev = q->prev;

        if (q->prev)
            q->prev->next = q->next;
        else
            qlist = q->next;

        FreeMem(q, sizeof(*q));
    }
    Permit();
#ifdef DEBUG
    kprintf("did_we = %ld\n", did_we);
#endif
    return did_we;
}

void WaitForOutputChannel(Queue *q) // Called by Input Channel
{
    q->in_wait = TRUE;
    q->in_sig = AllocSignal(~0);    // WARN: Note that we aren't checking if this failed- we don't allocate that many signals!
    if (q->out_wait) // Do we risk losing control here?
        Signal(q->task_out, 1 << q->out_sig);
    Wait(1 << q->in_sig);           // Breaks the Forbid() for awhile (as expected)
    FreeSignal(q->in_sig);          // We don't need this signal anymore; get rid of it!
    q->in_wait = FALSE;
}

void WaitForInputChannel(Queue *q)  // Called by Output Channel
{
    q->out_wait = TRUE;
    q->out_sig = AllocSignal(~0);    // WARN: Note that we aren't checking if this failed- we don't allocate that many signals!
    if (q->in_wait) // Do we risk losing control here?
        Signal(q->task_in, 1 << q->in_sig);
    Wait(1 << q->out_sig);           // Breaks the Forbid() for awhile (as expected)
    FreeSignal(q->out_sig);          // We don't need this signal anymore; get rid of it!
    q->out_wait = FALSE;
}
