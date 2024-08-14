/****** que/queue-handler.h ***************************************************

    Mitchell                Version 1.02                30 Oct 1990

    Structures for queue-handler
******************************************************************************/

typedef struct QueBuf   {
    struct QueBuf *next, *prev; // NULL-pointer-terminated list of que buffs
    struct Queue *q;            // Parent Queue
    long in, out;               // Index pointers for the ins and outs of data
    UBYTE data[1];              // First byte of data in buffer
    } QueBuf;

typedef struct Queue  {
    struct Queue *next, *prev;  // List of instance Queue, one for each device: name given
    UBYTE name[64];             // DeviceName given to this instance
    ULONG bufsize;              // Buffer size for buffers
    long qblimit;               // If not zero, max number of que buffs allowed
    long qbcount;               // Actual number of buffers allocated
    QueBuf *qbhead, *qbtail;    // Buffer FIFO
    struct Process *task_in,
                   *task_out;   // Open tasks. Can't be deleted if either are non-NULL.
    BOOL ever_in, ever_out;     // Ever-opened flags. Set, but never cleared.
    BOOL  in_wait, out_wait;    // If TRUE, task_x is waiting for data.
    short in_sig,  out_sig;     // x_port signal number for re-awakening
    long open_cnt;              // additional open count.
    } Queue;
