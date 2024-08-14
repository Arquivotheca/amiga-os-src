#include <exec/types.h>
#include <exec/memory.h>
#include <proto/all.h>

/*****i exec_support/CreateStdIO **************************************
*
*   NAME   
*   CreateStdIO() -- create a standard IO request
*
*   !!!OBSOLETE - USETE CreateExtIO()!!!!
*
*   SYNOPSIS
*   ioStdReq = CreateStdIO( ioReplyPort );   
*
*   FUNCTION
*   Allocates memory for and initializes a new IO request block.
*
*   INPUTS
*   ioReplyPort - a pointer to an already initialized
*      message port to be used for this IO request's
*      reply port.
*
*   RESULT
*   Returns a pointer to the new io request block.  A NULL
*   indicates that there was not enough memory for the IO Request,
*   or that the reply port was not a valid port.
*
*   EXAMPLE
*   struct IOStdReq *myBlock;
*   struct MsgPort *port;
*
*   myBlock = CreateStdIO( port );
*   if( myBlock == NULL) {
*      printf( "Insufficient memory" );
*   }
*
*   SEE ALSO
*   DeleteStdIO, CreateExtIO
*
***********************************************************************/

struct IOStdReq *CreateStdIO( ioReplyPort )
struct MsgPort *ioReplyPort;
{
    return( (struct IOStdReq *)
   CreateExtIO( ioReplyPort, sizeof( struct IOStdReq ) ) );
}

/*****i exec_support/DeleteStdIO ***************************************
*
*   NAME
*   DeleteStdIO( ioStdReq ) - return memory allocated for IO request
*
*   SYNOPSIS
*   DeleteStdIO(ioStdReq);
*
*   !!!OBSOLETE - USETE DeleteExtIO()!!!!
*
*   FUNCTION
*   Free memory allocate for the io request.
*
*   INPUTS
*   A pointer to the IOStdReq block whose resources are to be freed.
*
*   RESULT
*   no defined return value
*
*   EXAMPLE
*   struct IOStdReq *ioRequest;
*   DeleteStdIO( ioRequest );
*
*   SEE ALSO
*   CreateStdIO, DeleteExtIO
*
**************************************************************************/

void DeleteStdIO( ioStdReq )
struct IOStdReq *ioStdReq;
{
    DeleteExtIO((struct IORequest *) ioStdReq );
}

/****** amiga.lib/CreateExtIO **************************************
*
*   NAME
*   CreateExtIO() -- create an IORequest structure
*
*   SYNOPSIS
*   ioReq = CreateExtIO( ioReplyPort, size );
*
*   struct IORequest *CreateExtIO(struct MsgPort *, ULONG);
*
*   FUNCTION
*   Allocates memory for and initializes a new IO request block
*   of a user-specified number of bytes.  The number of bytes
*   MUST be the size of a legal IORequest (or extended IORequest)
*   or very nasty things will happen.
*
*   INPUTS
*   ioReplyPort - a pointer to an already initialized message port
*       to be used for this IO request's reply port. (usually created
*       by CreatePort()).  If NULL this function fails.
*   size - the size of the IO request to be created.
*
*   RESULT
*   Returns a pointer to the new IO Request block, or NULL if
*      the request failed.
*
*   EXAMPLE
*   if( myReq = CreateExtIO( CreatePort(0L,0L),sizeof(struct IOExtTD) ) )
*
*   SEE ALSO
*   DeleteExtIO, CreatePort()
*
**********************************************************************
*/

struct IORequest *CreateExtIO( ioReplyPort, size )
struct MsgPort *ioReplyPort;
LONG size;
{
struct IORequest *ioReq;

    if(! ioReplyPort )
   return ( NULL );
    if (!( ioReq = AllocMem( size, MEMF_CLEAR|MEMF_PUBLIC ) ))
   return( NULL );

    /* Mark request as inactive (NT_REPLYMSG) */
    ioReq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
    ioReq->io_Message.mn_Length       = size;
    ioReq->io_Message.mn_ReplyPort    = ioReplyPort;

    return( ioReq );
}

/****** amiga.lib/DeleteExtIO **************************************
*
*   NAME
*   DeleteExtIO() - return memory allocated for extended IO request
*
*   SYNOPSIS
*   DeleteExtIO( ioReq );
*
*   void DeleteExtIO(struct IORequest *);
*
*   FUNCTION
*   Frees up an IO request as allocated by CreateExtIO().  By
*   looking at the mn_Length field, it knows how much memory
*   to deallocate.
*
*   INPUTS
*   ioReq - A pointer to the IORequest block to be freed, or NULL.
*
*   SEE ALSO
*   CreateExtIO
*
*************************************************************************
*/

void DeleteExtIO( ioExt )
struct IORequest *ioExt;
{
    if (!ioExt)
   return;

    /* try to make it hard to reuse the request by accident */
    ioExt->io_Message.mn_Node.ln_Succ = (void *) -1;
    ioExt->io_Device = (struct Device *) -1;

    FreeMem( ioExt, ioExt->io_Message.mn_Length );
}
/****** amiga.lib/CreatePort ************************************************
*
*   NAME
*   CreatePort - Allocate and initialize a new message port
*
*   SYNOPSIS
*   CreatePort(name,pri)
*
*   struct MsgPort * CreatePort(char *,LONG);
*
*   FUNCTION
*   Allocates and initializes a new message port.  The message list
*   of the new port will be prepared for use (via NewList).  A signal
*   bit will be allocated, and the port will be set to signal your
*   task when a message arrives (PA_SIGNAL).
*
*   You *must* use DeletePort() to delete ports created with
*   CreatePort()!
*
*   INPUTS
*   name - NULL if other tasks will not search for this port with the
*          FindPort() call.  If this port will be searched, provide
*          a name string.  The name is not copied.
*   pri  - Priority used for insertion into the public port list.
*
*   RESULT
*   A new MsgPort structure ready for use.  NULL if the system
*   is out of memory, or if all task signals are in use.
*
*   SEE ALSO
*   DeletePort(), exec/FindPort(), exec/ports.h
*
*****************************************************************************
*/
struct MsgPort *CreatePort (name, pri)
char *name;
long pri;
{
    ULONG sigBit;
    struct MsgPort *port;

    if ((sigBit = AllocSignal (-1)) == -1) {
   return ((struct MsgPort *) 0);
    }

    port = AllocMem ((ULONG) sizeof (struct MsgPort), MEMF_CLEAR | MEMF_PUBLIC);

    if (port == 0) {
   FreeSignal (sigBit);
   return ((struct MsgPort *) (0));
    }

    port -> mp_Node.ln_Name = name;
    port -> mp_Node.ln_Pri = pri;
    port -> mp_Node.ln_Type = NT_MSGPORT;

    port -> mp_Flags = PA_SIGNAL;
    port -> mp_SigBit = sigBit;
    port -> mp_SigTask = FindTask (0);  /* pointer to self (calling task) */

    if (name != 0)
   AddPort (port);
    else
   NewList (&(port -> mp_MsgList));

    return (port);
}
/****** amiga.lib/DeletePort ************************************
*
*   NAME
*   DeletePort - Free a message port created by CreatePort
*
*   SYNOPSIS
*   DeletePort(msgPort)
*
*   void DeletePort(struct MsgPort *);
*
*   FUNCTION
*   Frees a message port created by CreatePort.  All messages that
*   may have been attached to this port must have already been
*   replied to.
*
*   INPUTS
*   msgPort - A message port
*
*   SEE ALSO
*   CreatePort()
*
***********************************************************************
*/

void DeletePort(port)
struct MsgPort *port;
{
    if ((port -> mp_Node.ln_Name) != 0)
   RemPort (port);

    port -> mp_Node.ln_Type = 0xff;
    port -> mp_MsgList.lh_Head = (struct Node  *) -1;

    FreeSignal (port -> mp_SigBit);

    FreeMem (port, (ULONG) sizeof (*port));
}
