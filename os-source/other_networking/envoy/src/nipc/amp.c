/**************************************************************************
**
** amp.c        - NIPC Library API
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: amp.c,v 1.57 93/10/20 23:23:52 kcd Exp $
**
***************************************************************************/

/* Don't delete link entities on servers while any transactions are outstanding */

#define OURHOSTNAME ((char *)&(gb->LocalHostName))

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"

#include <envoy/errors.h>
#include <string.h>

#include <exec/devices.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/timer_pragmas.h>
#include <clib/timer_protos.h>

#define MIN(x, y) ((x) < (y) ? (x):(y))
#define MAX(x, y) ((x) > (y) ? (x):(y))

/* This next line is here only temporarily, until I can get access to nipcinternal_protos.h */
//extern ULONG CopyBroken(ULONG *st, ULONG *dt, ULONG soff, ULONG doff);
extern void RespondToPing(struct RDPConnection *c, struct PingRequest *pr);
extern void GotResponse(struct PingResponse *pr);
extern void __asm Mul64by16(register __a0 ULONG *ptr,register __d0 ULONG multiplier);
void ReturnTransaction(struct Transaction *t);

/*------------------------------------------------------------------------*/

BOOL InitANMP()
{
    register struct NBase *nb = gb;
    BOOL init;
    NewList((struct List *) & gb->ANMPEntityList);
    NewList((struct List *) & gb->HostList);
    NewList((struct List *) & gb->PingList);
    NewList((struct List *) & gb->WakeFind);
    InitSemaphore(&nb->WakeLock);
    InitSemaphore(&nb->PingListSemaphore);
    InitSemaphore(&nb->ANMPELSemaphore);

    gb->TRANSSEQUENCE = 0;
    init = FALSE;

    /* Create the well-known Entity-Resolver RDP port */
    Forbid();
    if(gb->EntityResolver = OpenPassive(RESOLVERPORT, (RDP_DATA) & ResolverIn, (RDP_STATUS) & ResolverStatus))
    {
        gb->EntityResolver->conn_Flags |= CONN_MULTIPLE;
        init = TRUE;
    }
    Permit();
    return(init);
}

/*------------------------------------------------------------------------*/

void DeinitANMP()
{
    register struct NBase *nb = gb;
    struct Entity *eptr;

    /* And a helluva lot of other stuff, too! */
    /* FIXME -- Should this free up transactions hanging around on Entities? */
    /* Should I even be deleting Entities??  I think so - for this code to be run,
     * we must've been asked to flush.  For that to happen, nobody has the library
     * open.  Thus, they've abandoned their Entities.
     */
    ObtainSemaphore(&nb->ANMPELSemaphore);        /* I shouldn't HAVE to do this, you know.  Nobody SHOULD have the thing locked ... */

    while(eptr = (struct Entity *) RemHead((struct List *)&nb->ANMPEntityList))
    {
        struct Entity *le;
        le = (struct Entity *) eptr->entity_linklist.mlh_Head;
        while (le->entity_Port.mp_Node.ln_Succ)
        {
            struct Entity *tmpe;
            struct Buffer *abuff;
            if (le->entity_connection)
                CloseConnection(le->entity_connection);
            tmpe = le;
            while (abuff = (struct Buffer *) RemHead((struct List *) &le->entity_Partials))
                FreeBuffer(abuff);
            le = (struct Entity *) le->entity_Port.mp_Node.ln_Pred;
            Remove((struct Node *)tmpe);
            FreeMem(tmpe,sizeof(struct Entity));
            le = (struct Entity *) le->entity_Port.mp_Node.ln_Succ;
        }
        FreeMem(eptr,sizeof(struct Entity));
    }
}


/****** nipc.library/BeginTransaction *****************************************
*
*   NAME
*     BeginTransaction -- Start an NIPC Transaction
*
*   SYNOPSIS
*     BeginTransaction(dest_entity,src_entity, transaction)
*                         A0          A1          A2
*
*     VOID BeginTransaction(struct Entity *,struct Entity *,
*                           struct Transaction *);
*
*   FUNCTION
*     Attempts to begin a transaction (described appropriately by the
*     transaction structure) to a given entity.
*
*   INPUTS
*     dest_entity - An abstract data type - a "magic cookie" - that identifies
*                   the destination entity.  (From FindEntity().)
*     src_entity  - An abstract data type - a "magic cookie" - that identifies
*                   the destination entity.  (From CreateEntity().)
*     transaction - a pointer to the Transaction to start.
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*     After this function begins, the Transaction structure passed becomes
*     the property of the nipc.library, and CANNOT be freed until this
*     Transaction returns to the source Entity.  Once the Transaction
*     returns onto the local Entity, the programmer should check
*     trans_Error, to verify that it completed properly.
*
*     Because data transmission isn't an instantaneous operation,
*     a limit must be imposed on exactly how many transactions
*     can be queued up at any given time.  Otherwise, it would be
*     for an application program to overflow the throughput of any given
*     networking protocol, occupying all available memory.  If the
*     underlying networking protocol becomes overloaded, subsequent
*     calls to BeginTransaction() may block until the networking
*     protocol is no longer overwhelmed.
*
*   BUGS
*
*   SEE ALSO
*       DoTransaction(), CheckTransaction(), WaitTransaction(),
*       AbortTransaction(), <envoy/nipc.h>
*
******************************************************************************
*
*/

BOOL __asm BeginTransaction(register __a0 struct Entity * dest_entity,
                            register __a1 struct Entity * src_entity,
                            register __a2 struct Transaction * trans)
{

    register struct NBase *nb = gb;
    trans->trans_SourceEntity = src_entity;
    trans->trans_DestinationEntity = dest_entity;
    trans->trans_Type = TYPE_REQUEST;
    trans->trans_Error = 0L;

    if (dest_entity->entity_Flags & ENTF_LINK)
    {
        /* networked */
        struct RequestPacket *rq;
        ULONG packsize;
        UBYTE *dataarea;
        ULONG packlimit, datalimit, offset;
        BOOL onepacket=FALSE;

        /* Add in netlag + 1 extra second */
        if(trans->trans_Timeout)
        {
            trans->trans_Timeout += ((dest_entity->entity_connection->conn_TimeStamp[0]/1024)+1) +
                                    ( dest_entity->entity_connection->conn_TimeoutDelay );
            // Because I'm using a 32 bit/16 bit divide route, I have to deal with
            // limitations --
            // The formula for the amount of ideal time X bytes would take to transfer
            // would be:  TimeInSeconds = 10*X/BPS, or = X/(BPS/10)
            // Therefore, since the 32 bit quantity is X, and the 16 bit is
            // (BPS/10), then the denominator can be no larger than 65535.
            // Meaning -- that if the BPS value is greater than 655359,
            // the division is impossible.  Considering that .65MBps is pretty
            // darned fast, I'll punt and skip the timeouts for above that
            // range, where they're really not that important anyways.
            // The RDPConnection field for initial BPS -- the BPS of the
            // network interface that this connection started over -- is
            // actually BPS/10 -- precalculated.  So, the check -appears-
            // to be off by a factor of ten.  It isn't.
            if (dest_entity->entity_connection->conn_InitialBPS < 65536)
            {
                trans->trans_Timeout += DivSASSucks(trans->trans_ReqDataActual+trans->trans_RespDataLength,
                                                    dest_entity->entity_connection->conn_InitialBPS)+1;
            }
        }

        Forbid();
        trans->trans_Sequence = gb->TRANSSEQUENCE++;
        Permit();
        trans->trans_SourceEntity = src_entity;

        ObtainSemaphore(&nb->RDPCLSemaphore);       /* To prevent deadlocks */
        ObtainSemaphore(&dest_entity->entity_transsema);
        AddTail((struct List *) & dest_entity->entity_translist, (struct Node *) & trans->trans_Msg.mn_Node);

        packsize = trans->trans_ReqDataActual;  /* Total data to send */
        if (!((struct Entity *) dest_entity)->entity_connection)
        {
            Remove((struct Node *) trans);
            ReleaseSemaphore(&dest_entity->entity_transsema);
            ReleaseSemaphore(&nb->RDPCLSemaphore);
            trans->trans_Error = ENVOYERR_CANTDELIVER;
            ReturnTransaction(trans);
            return(FALSE);
        }
        else
            packlimit = ((struct Entity *) dest_entity)->entity_connection->conn_SendMaxSize;       /* Max packet size */
        datalimit = packlimit - sizeof(struct RequestPacket);   /* Max data area per
                                                                 * packet */
        offset = 0L;
        while ((packsize) || (!onepacket))
        {
            ULONG thispack;
            struct Buffer *b;
            struct BuffEntry *be;

            onepacket = TRUE; /* have sent at least one packet */
            thispack = MIN(packsize, datalimit) + sizeof(struct RequestPacket);

            if (b = AllocBuffer(thispack))
            {
                be = (struct BuffEntry *)b->buff_list.mlh_Head;
                rq = (struct RequestPacket *)be->be_data;
                be->be_length = thispack;

		rq->reqpack_Flags = 0;
                rq->reqpack_PacketType = PACKET_DATA;
                if (trans->trans_RequestData == trans->trans_ResponseData)
                    rq->reqpack_Flags = REQFLAG_INPLACE;
                rq->reqpack_Filler = 0;
                rq->reqpack_Command = trans->trans_Command;
                rq->reqpack_Sequence = trans->trans_Sequence;
                rq->reqpack_Segment = 0;
                rq->reqpack_ResponseDataSize = trans->trans_RespDataLength;
                rq->reqpack_DataLength = thispack - sizeof(struct RequestPacket);
                dataarea = (UBYTE *) (((ULONG) rq) + (ULONG) sizeof(struct RequestPacket));

//                if (trans->trans_Flags & TRANSF_REQUESTTABLE)
//                {
//                    ULONG tx[4];
//                    tx[0] = (ULONG) dataarea;
//                    tx[1] = (ULONG) rq->reqpack_DataLength;
//                    tx[2] = 0L;
//                    tx[3] = 0L;
//                    CopyBroken((ULONG *)trans->trans_RequestData,(ULONG *)&tx,offset,0);
//                }
//                else
		if(trans->trans_Flags & TRANSF_REQNIPCBUFF)
		    CopyFromNIPCBuff((struct NIPCBuff *)trans->trans_RequestData, dataarea, offset, rq->reqpack_DataLength);
		else
                    CopyMem(&(((UBYTE *)trans->trans_RequestData)[offset]), dataarea, rq->reqpack_DataLength);

                if (!(packsize - thispack + sizeof(struct RequestPacket)))      /* If this is the last
                                                                                 * one .. */
                    rq->reqpack_Flags |= REQFLAG_LASTSEGMENT;
                if (dest_entity->entity_connection->conn_State == STATE_OPEN)
                {

                    if (dest_entity->entity_connection->conn_SendNxt -
                        dest_entity->entity_connection->conn_SendOldest >=
                        dest_entity->entity_connection->conn_SendMax)
                    {
                        if (!AddSigBit(dest_entity->entity_connection))
                        {
                            FreeBuffer(b);
                            Remove((struct Node *) trans);      /* Get it off the local list */
                            trans->trans_Error = ENVOYERR_CANTDELIVER;
                            ReturnTransaction(trans);
                            ReleaseSemaphore(&dest_entity->entity_transsema);
                            ReleaseSemaphore(&nb->RDPCLSemaphore);
                            return (FALSE);

                        }
                        dest_entity->entity_connection->conn_Flags |= CONN_WINDOWSIGNAL;
                        while ((dest_entity->entity_connection->conn_SendNxt -
                                dest_entity->entity_connection->conn_SendOldest >=
                                dest_entity->entity_connection->conn_SendMax) &&
                               (dest_entity->entity_connection->conn_State == STATE_OPEN))
                        {
                            ReleaseSemaphore(&dest_entity->entity_transsema);
                            ReleaseSemaphore(&nb->RDPCLSemaphore);
                            Sleep(dest_entity->entity_connection);
                            ObtainSemaphore(&nb->RDPCLSemaphore);       /* To prevent deadlocks */
                            ObtainSemaphore(&dest_entity->entity_transsema);
                        }
                        dest_entity->entity_connection->conn_Flags &= !CONN_WINDOWSIGNAL;
                        DelSigBit(dest_entity->entity_connection);
                    }
                    rdp_output(dest_entity->entity_connection, b);
                }
                else
                {
                    FreeBuffer(b);
                    Remove((struct Node *) trans);      /* Get it off the local list */
                    trans->trans_Error = ENVOYERR_CANTDELIVER;
                    ReturnTransaction(trans);
                    ReleaseSemaphore(&dest_entity->entity_transsema);
                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                    return (FALSE);
                }
                FreeBuffer(b);
            }
            else
            {
                Remove((struct Node *) trans);
                trans->trans_Error = ENVOYERR_CANTDELIVER;
                ReturnTransaction(trans);
                ReleaseSemaphore(&dest_entity->entity_transsema);
                ReleaseSemaphore(&nb->RDPCLSemaphore);
                return (FALSE);
            }

            offset += (thispack - sizeof(struct RequestPacket));
            packsize -= (thispack - sizeof(struct RequestPacket));
        }

        ReleaseSemaphore(&dest_entity->entity_transsema);
        ReleaseSemaphore(&nb->RDPCLSemaphore);
    }
    else
    {
        /* local transaction */
        if (!(dest_entity->entity_Flags & ENTF_LINK))
            if (!(dest_entity->entity_Flags & ENTF_DELAYEDDELETE))
                PutMsg(&dest_entity->entity_Port, &trans->trans_Msg);
            else
            {
                trans->trans_Error = ENVOYERR_CANTDELIVER;
                ReturnTransaction(trans);
            }
    }

    return (TRUE);
}


/****** nipc.library/DoTransaction ******************************************
*
*   NAME
*     DoTransaction -- Begin a Transaction, and wait for it to complete.
*
*   SYNOPSIS
*     DoTransaction(dest_entity,src_entity,transaction)
*                     A0            A1          A2
*
*     VOID DoTransaction(struct Entity *, struct Entity *,
*                        struct Transaction *);
*
*   FUNCTION
*     Processes an entire Transaction; it sends the request to the host
*     Entity and awaits the response, or returns at any point because
*     of an error.
*
*   INPUTS
*     dest_entity - An abstract data type - a "magic cookie" - that identifies
*                   the destination entity.  (From FindEntity().)
*     src_entity  - An abstract data type - a "magic cookie" - that identifies
*                   the destination entity.  (From CreateEntity().)
*     transaction - a pointer to the Transaction to complete.
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*     DoTransaction() causes the current context to go into
*     a Wait() state until the Transaction is complete (or cannot be
*     completed.  After a DoTransaction() call, the trans_Error
*     field should be checked to verify that the Transaction was
*     properly completed.
*
*     DoTransaction(), like WaitTransaction() is potentially dangerous, and
*     should be used carefully.  If no trans_Timeout value is included,
*     and a server never replies the Transaction (for an unknown reason),
*     DoTransaction() can conceivably wait forever.
*
*   BUGS
*
*   SEE ALSO
*     BeginTransaction(), <envoy/nipc.h>
*
******************************************************************************
*
*/

BOOL __asm DoTransaction(register __a0 struct Entity * dest_entity,
                         register __a1 struct Entity * src_entity,
                         register __a2 struct Transaction * trans)
{

    BeginTransaction(dest_entity, src_entity, trans);
    WaitTransaction(trans);
    return (TRUE);

}


/****** nipc.library/AbortTransaction ******************************************
*
*   NAME
*     AbortTransaction -- Abort an attempted transaction.
*
*   SYNOPSIS
*     AbortTransaction(transaction)
*                          A1
*
*     VOID AbortTransaction(struct Transaction *);
*
*   FUNCTION
*     Aborts a Transaction that was previously started by a call to
*     BeginTransaction().
*
*   INPUTS
*     transaction - Pointer to the Transaction to abort.
*
*   RESULT
*     If the transaction hadn't already been completed (or failed), it will
*     be immediately aborted, and placed on the local Entity.
*
*   EXAMPLE
*
*   NOTES
*     Because local Transactions are passed by reference, a transaction
*     sent to a local Entity can be in such a state (being processed)
*     where another process may be referencing them.  To ensure that
*     a Transaction is either aborted or completed, an AbortTransaction()
*     call should be followed by a WaitTransaction().  If the transaction
*     given has already completed (or failed), no action will be taken.

*   BUGS
*
*   SEE ALSO
*     WaitTransaction(), DoTransaction(), BeginTransaction(),
*     CheckTransaction()
*
******************************************************************************
*
*/

void __asm AbortTransaction(register __a1 struct Transaction * trans)
{

    struct Entity *dest;

    /* If the transaction type isn't marked as a request, don't do anything. */

    if (trans->trans_Type == -1L)
        return;

    dest = trans->trans_DestinationEntity;

    if (trans->trans_DestinationEntity->entity_Flags & ENTF_LINK)
    {
        /*
         * For network transactions, the transaction structure should be
         * sitting on the trans_translist of the link entity to that
         * connection.
         */

        ObtainSemaphore(&dest->entity_transsema);
        if (trans->trans_Type == TYPE_RESPONSE)
        {
            ReleaseSemaphore(&dest->entity_transsema);
            return;
        }
        Remove((struct Node *) trans);
        ReleaseSemaphore(&dest->entity_transsema);

    }
    else
    {

        /*
         * For locals, I need only get it off of the MsgPort.
         * GetTransaction() will not do a Permit until the Type field is set
         * to RESPONSE - implying that inside the following F/P pair, if Type
         * isn't REQUEST, I can't abort it, because GetTransaction() already
         * has it.
         */

        Forbid();
        if ((trans->trans_Type == TYPE_RESPONSE) || (trans->trans_Type == TYPE_SERVICING))
        {
            Permit();
            return;
        }
        Remove((struct Node *) trans);
        Permit();
    }

    trans->trans_Error = ENVOYERR_ABORTED;
    ReturnTransaction(trans);

}


/****** nipc.library/CreateEntityA ******************************************
*
*   NAME
*     CreateEntityA -- Creates an Entity for communication.
*
*   SYNOPSIS
*     myentity = CreateEntityA(taglist)
*        D0                      A0
*
*     struct Entity *CreateEntityA(struct TagItem *);
*     struct Entity *CreateEntity(tag1, tag2, tag3, ...);
*
*   FUNCTION
*     This function allows you to create an Entity for one endpoint
*     for communication.  Given the appropriate tags, you may declare that
*     this entity is public - and therefore can be located by FindEntity().
*
*   INPUTS
*     taglist -- a list of tags that declare items such as the Entity name
*                (which is only necessary for public Entities), signal
*                bits, etc.
*
*                The following tags are defined:
*                ENT_Name:
*                 Allows the user to declare the name for this entity.  This
*                 name must be unique -- and if an Entity of similar name
*                 already exists, CreateEntity() will return NULL.
*                 (default: no provided name)
*                ENT_Public:
*                 If included, this entity is _public_, and can be found
*                 by FindEntity() call.  To declare an entity public,
*                 you must also declare the entity name.
*                 (default: private)
*                ENT_Signal:
*                 Defines that when something arrives at this entity,
*                 the given signal bit (bit number, not mask) should
*                 be set.  (Useful when you intend to maintain a LOT
*                 of entities - more than possible to allocate a single
*                 signal bit for each.  In this method, a large number
*                 of entities share the signal bit.  The downside is that
*                 every entity with that bit must be checked when
*                 the bit is found set.)
*                 (default: FALSE)
*                ENT_AllocSignal:
*                 Asks that CreateEntity() allocate a signal bit from
*                 the current context, and set it whenever the
*                 entity receives any data, to set that bit.  The
*                 ti_Data field of this TagItem must be a pointer
*                 to a ULONG which CreateEntity will fill in with the
*                 bit number, such that you know what bit number to
*                 Wait() on.  If you provide a NULL as the ti_Data
*                 entry, you will not be informed of the signal bit
*                 number.
*                 (default: signal not allocated)
*
*                 A NULL taglist will provide the defaults.
*
*   RESULT
*     entity  --  a 'magic cookie' that defines your entity, or NULL for
*                 failure
*
*   EXAMPLE
*
*   NOTES
*     ENT_Signal and ENT_AllocSignal are mutually exclusive.
*     ENT_Public requires that you also provide ENT_Name; the
*       converse, however, is not true.
*
*   BUGS
*
*   SEE ALSO
*     DeleteEntity()
*
******************************************************************************
*
*/

void *__asm CreateEntityA(register __a0 struct TagItem * taglist)
{
        register struct NBase *nb = gb;
    BOOL TName, TPublic, TSignal, TAllocSignal; /* Tags that were passed */
    UBYTE *Name;
    ULONG UseSigBit;
    ULONG *StoreSigBit;
    struct TagItem *tstate;
    struct TagItem *tag;
    struct Entity *eptr;

    TName = TPublic = TSignal = FALSE;
    TAllocSignal = FALSE;
    StoreSigBit = 0L;

    if (taglist)
    {
        tstate = taglist;
        while (tag = (struct TagItem *) NextTagItem(&tstate))
        {
            switch (tag->ti_Tag)
            {
                case ENT_Name:
                    Name = (UBYTE *) tag->ti_Data;
                    TName = TRUE;
                    break;
                case ENT_Public:
                    TPublic = TRUE;
                    break;
                case ENT_Signal:
                    UseSigBit = tag->ti_Data;
                    TSignal = TRUE;
                    break;
                case ENT_AllocSignal:
                    StoreSigBit = (ULONG *) tag->ti_Data;
                    TAllocSignal = TRUE;
                    break;
            }
        }
    }

    if (TAllocSignal && TSignal)/* They're mutually exclusive!  Can't have
                                 * both! */
        return (0L);

    if (TPublic && !TName)      /* Can't be public without a name, for
                                 * goshsakes */
        return (0L);

    if (TName)
    {
        ObtainSemaphore(&nb->ANMPELSemaphore);
        if (FindNameI((struct List *)&nb->ANMPEntityList,(STRPTR) Name))
        {
            ReleaseSemaphore(&nb->ANMPELSemaphore);
            return(0L);
        }
        ReleaseSemaphore(&nb->ANMPELSemaphore);
    }

    eptr = (struct Entity *) AllocMem(sizeof(struct Entity), MEMF_PUBLIC | MEMF_CLEAR);
    if (!eptr)                  /* If no mem, forget it. */
        return (0L);

    /* Init the msg port */
    NewList((struct List *) & (eptr->entity_Port.mp_MsgList));
    NewList((struct List *) & (eptr->entity_Partials));

    /* If we were asked to, allocate a signal bit */
    if (TAllocSignal)
    {
        eptr->entity_Flags |= ENTF_ALLOCSIGNAL;
        UseSigBit = AllocSignal(-1L);
        if (StoreSigBit)
            *StoreSigBit = UseSigBit;
        if (UseSigBit == -1L)
        {
            FreeMem(eptr, sizeof(struct Entity));
            return (0L);        /* Can't allocate the signal bit */
        }
    }

    /* If this entity is USING signal bits at all, set the port up correctly */
    if (TAllocSignal || TSignal)
    {
        eptr->entity_Port.mp_SigBit = UseSigBit;
        eptr->entity_Port.mp_SigTask = (struct Task *) FindTask(0L);
        eptr->entity_Port.mp_Flags = PA_SIGNAL;
    }

    if (!(TAllocSignal || TSignal))
        eptr->entity_Port.mp_Flags = PA_IGNORE;

    /* Was a name provided? */
    if (TName)
    {
        eptr->entity_Port.mp_Node.ln_Name = (UBYTE *) AllocMem(strlen(Name) + 1, MEMF_PUBLIC | MEMF_CLEAR);
        if (!eptr->entity_Port.mp_Node.ln_Name)
        {
            if (TAllocSignal)
                FreeSignal(eptr->entity_Port.mp_SigBit);
            FreeMem(eptr, sizeof(struct Entity));
            return (0L);        /* Not enough mem for name */
        }
        strcpy(eptr->entity_Port.mp_Node.ln_Name, Name);
    }
    else
        eptr->entity_Port.mp_Node.ln_Name = 0L;

        ObtainSemaphore(&nb->ANMPELSemaphore);
        AddTail((struct List *) & nb->ANMPEntityList, (struct Node *) eptr);
        ReleaseSemaphore(&nb->ANMPELSemaphore);

        if (TPublic)
            eptr->entity_Flags |= ENTF_PUBLIC;

    InitSemaphore(&(eptr->entity_linksema));
    NewList((struct List *) & (eptr->entity_linklist));

    return ((void *) eptr);
}


/****** nipc.library/DeleteEntity ******************************************
*
*   NAME
*     DeleteEntity -- Delete an Entity
*
*   SYNOPSIS
*     DeleteEntity(entity);
*                    A0
*
*     VOID DeleteEntity(struct Entity *);
*
*   FUNCTION
*     DeleteEntity() is the converse of CreateEntity().  It removes the
*     entity from NIPC lists that it may have been attached to,
*     frees up any resources attached to this Entity that were
*     allocated by CreateEntity(), and frees up the actual Entity
*     structure.  If NULL is passed as the argument, no action will
*     be taken.
*
*   INPUTS
*     entity  --  a 'magic cookie' that defines your Entity
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*     If an attempt is made to DeleteEntity() an Entity that has been
*     found (via FindEntity()), but not lost (via LoseEntity()),
*     the Entity being deleted will be maintained by NIPC until
*     all references to it no longer exist.
*     (The same behavior should arguably occur for an Entity on which
*     previously sent Transactions are yet to return.)
*
*     IMPORTANT WARNING:
*     Use DeleteEntity() ONLY on Entities created with CreateEntity().
*     Do NOT use DeleteEntity() on Entities created with FindEntity()!!
*
*   BUGS
*
*   SEE ALSO
*     CreateEntityA(), LoseEntity()
*
******************************************************************************
*
*/
void __asm DeleteEntity(register __a0 void *entin)
{
    register struct NBase *nb = gb;
    struct Entity *eptr;
    struct Transaction *t;

    eptr = (struct Entity *) entin;

    if (!entin)
        return;                 /* Some moron has passed a NULL in as the entity */

    if (eptr->entity_LinkCount)
    {
        if (eptr->entity_Port.mp_SigTask == FindTask(0L))
            if (eptr->entity_Flags & ENTF_ALLOCSIGNAL)          /* If we can, get rid of the signal bit on the context of the true owner of this entity */
            {
                Forbid();
                if (eptr->entity_Port.mp_Node.ln_Name)
                {
                    FreeMem(eptr->entity_Port.mp_Node.ln_Name,
                            strlen(eptr->entity_Port.mp_Node.ln_Name) + 1);
                    eptr->entity_Port.mp_Node.ln_Name = 0L;
                }
                eptr->entity_Flags &= !ENTF_ALLOCSIGNAL;
                FreeSignal(eptr->entity_Port.mp_SigBit);
                eptr->entity_Port.mp_Flags = PA_IGNORE;
                Permit();
            }
        eptr->entity_Flags |= ENTF_DELAYEDDELETE;
        return;
    }

    /* Get it off any system lists */
    ObtainSemaphore(&nb->ANMPELSemaphore);
    Remove((struct Node *) eptr);
    ReleaseSemaphore(&nb->ANMPELSemaphore);

    /* Search through the list of Transactions on this Entity.  If any are found that meet
     * the criteria of:
     *
     *  - A request, not a response from something we sent long ago
     *  - Sent from a LOCAL Entity, not from another machine
     *
     * Return it as 'cantdeliver'.
     *
     * All other Transactions will be IGNORED.  (Should I free them?!?)
     */

    while (t = (struct Transaction *) RemHead( (struct List *) &eptr->entity_Port.mp_MsgList))
    {
        if ((!(t->trans_SourceEntity->entity_Flags & ENTF_LINK)) && (t->trans_Type == TYPE_REQUEST))
        {
            t->trans_Error = ENVOYERR_CANTDELIVER;
            ReturnTransaction(t);
        }

    }



    /* Free the name string */
    if (eptr->entity_linkname)
        FreeMem(eptr->entity_linkname,strlen(eptr->entity_linkname)+1);

    if (eptr->entity_Port.mp_Node.ln_Name)
        FreeMem(eptr->entity_Port.mp_Node.ln_Name, strlen(eptr->entity_Port.mp_Node.ln_Name) + 1);

    /* If CreateEntity allocated a signal bit, free it up */
    if (eptr->entity_Flags & ENTF_ALLOCSIGNAL)
        FreeSignal(eptr->entity_Port.mp_SigBit);

    /* Free any links associated with it */
    ObtainSemaphore(&nb->RDPCLSemaphore);   /* To prevent deadlock */
    ObtainSemaphore(&nb->ANMPELSemaphore);
    if (eptr->entity_Flags & ENTF_LINK)
    {
        ObtainSemaphore(&(eptr->entity_linksema));
        while (!(IsListEmpty((struct List *) & eptr->entity_linklist)))
        {
            struct Entity *le;
            le = (struct Entity *) RemHead((struct List *) & eptr->entity_linklist);
            if (le->entity_connection)
            {
                struct RDPConnection *oldc;
                struct Buffer *abuff;
                while (abuff = (struct Buffer *) RemHead((struct List *) &le->entity_Partials))
                    FreeBuffer(abuff);
                oldc = le->entity_connection;
                if (!(oldc->conn_WakeBit+1))        /* Don't delete the connection if somebody's waiting on it */
                {
                    le->entity_connection = 0L;
                    CloseConnection(oldc);
                }
            }
            FreeMem(le, sizeof(struct Entity));
        }
        ReleaseSemaphore(&(eptr->entity_linksema));
    }
    ReleaseSemaphore(&nb->RDPCLSemaphore);
    ReleaseSemaphore(&nb->ANMPELSemaphore);

    /* Free the structure itself */

    FreeMem(eptr, sizeof(struct Entity));
}

/****** nipc.library/WaitEntity ******************************************
*
*   NAME
*     WaitEntity -- Waits for a Transaction to arrive at an Entity.
*
*   SYNOPSIS
*     WaitEntity(localentity)
*                    A0
*
*     VOID WaitEntity(struct Entity *);
*
*   FUNCTION
*     WaitEntity() simply causes the current process to Wait() until
*     something arrives at the entity.
*
*   INPUTS
*     entity      - An abstract data type - a "magic cookie" - that
*                   identifies an Entity.
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*     This is potentially dangerous - if nothing ever arrives at this
*     Entity, this function will never return.  Unless warrented,
*     the caller should be using Wait() with a signal mask.
*
*   BUGS
*
*   SEE ALSO
*     GetTransaction()
*
******************************************************************************
*
*/
void __asm WaitEntity(register __a0 void *entin)
{
    struct Entity *ent;

    ent = (struct Entity *) entin;
    WaitPort(&(ent->entity_Port));
}


/****** nipc.library/FindEntity ******************************************
*
*   NAME
*     FindEntity -- Locate a specific entity on a certain host
*
*   SYNOPSIS
*     remote_entity = FindEntity(hostname,entityname,src_entity,errorptr)
*           D0                      A0         A1        A2        A3
*
*     struct Entity *FindEntity(STRPTR, STRPTR, struct Entity *,ULONG *);
*
*   FUNCTION
*     Attempts to locate a certain entity on a given host.
*     If you pass a NULL for the entityname, FindEntity will
*     interpret that as "the local machine".
*
*   INPUTS
*     hostname --   pointer to a null-terminated string declaring the
*                   host name of the machine on which you expect to
*                   find an Entity named the same as 'entityname'.
*
*     entityname -- pointer to a null-terminated string declaring the
*                   name of a public entity.
*
*     src_entity -- an Entity returned by CreateEntity() that you wish to
*                   use as the 'near' side of a communications path.
*
*     errorptr   -- a pointer to a ULONG in which FindEntity() will provide
*                   detailed error information in the event of a FindEntity()
*                   failure.  If a NULL pointer is passed, no detailed error
*                   information will be available.
*
*   RESULT
*     remote_entity --
*        NULL if the given Entity cannot be found.
*        Otherwise, the magic cookie describing the Entity that you found.
*
*   EXAMPLE
*
*   NOTES
*     If you do a FindEntity to a different machine and it succeeds,
*     you have no guarantee that the remote machine's Entity will
*     continue to exist.  Because of this, you should check all returned
*     Transactions for error status - if a Transaction to a remote
*     Entity fails because the nipc.library can no longer find the remote
*     Entity, the transaction will be returned as errored.
*     Regardless of what happens, every SUCCESSFUL FindEntity() REQUIRES an
*     associated LoseEntity().
*
*     Since the FindEntity() establishes a communications link between
*     the source and destination Entities, it's -dependant- on the
*     source.  Therefore, do not DeleteEntity() the source entity before
*     LoseEntity()'ing.
*
*   BUGS
*
*   SEE ALSO
*     LoseEntity()
*
******************************************************************************
*
*/
void *__asm FindEntity(register __a0 UBYTE * hostname,
                                 register __a1 UBYTE * portname,
                                 register __a2 void *src_entity,
                                 register __a3 ULONG * detailerror)
{
    register struct NBase *nb = gb;
    void *ent;
    struct Entity *src;
    ULONG dest;
    struct WakeNode wn;

    src = (struct Entity *) src_entity;


    /* If they ask for specific host, rather than NULL for the local one */
    if (hostname)
    {
        dest = ResolveName(hostname);
        if (!dest)
        {
            if (detailerror)
                *detailerror = ENVOYERR_UNKNOWNHOST;
            return ((void *) 0L);
        }
        if(dest == 0xffffffff)          /* Local Request */
            hostname = (UBYTE *) 0L;
    }

    if (!hostname)              /* Local request! */
    {
        ObtainSemaphore(&nb->ANMPELSemaphore);
        ent = (void *) FindNameI((struct List *) &nb->ANMPEntityList, portname);    /* Does this entity */
        Forbid();
        if (ent)
            ((struct Entity *)ent)->entity_LinkCount++;
        else
            if (detailerror)
                *detailerror = ENVOYERR_UNKNOWNENTITY;
        Permit();
        ReleaseSemaphore(&nb->ANMPELSemaphore);
        return (ent);
    }
    else
        /* Remote request */
    {
        UBYTE oldstate;
        struct RDPConnection *askc;

        Forbid();
        askc = OpenActive(dest, RESOLVERPORT, (RDP_DATA) & ask_input, (RDP_STATUS) & ask_status);
        if (!askc)
        {
            Permit();
            if (detailerror)
            {
                *detailerror = ENVOYERR_NORESOLVER;
            }
            return(0L);
        }
        if (!AddSigBit(askc))
        {
            CloseConnection(askc);
            if (detailerror)
                *detailerror = ENVOYERR_NORESOURCES;
            return(0L);
        }
        Permit();

        wn.wn_Connection = askc;
        wn.wn_Time = 30;
        ObtainSemaphore(&nb->WakeLock);
        AddTail((struct List *)&nb->WakeFind,(struct Node *)&wn);
        ReleaseSemaphore(&nb->WakeLock);

        oldstate = 0;
        while (askc->conn_State <= STATE_OPEN)
        {
            ULONG ret_addr;
            int offset;
            struct NameRequest *lnr;
            struct Buffer *b;
            struct BuffEntry *be;

            if ((askc->conn_State == STATE_OPEN) && (oldstate != STATE_OPEN))
            {
                if(b = AllocBuffer(sizeof(struct NameRequest)))
                {
                    char local[256];

                    be = (struct BuffEntry *)b->buff_list.mlh_Head;
                    be->be_length = sizeof(struct NameRequest);
                    lnr = (struct NameRequest *)be->be_data;

                    stccpy((UBYTE *) & lnr->nr_Name, portname, 80);
                    if (src->entity_Port.mp_Node.ln_Name)
                        stccpy((UBYTE *) & lnr->nr_FromName, src->entity_Port.mp_Node.ln_Name, 80);
                    else
                        stccpy((UBYTE *) & lnr->nr_FromName,"UNNAMED ENTITY",80);
                    local[0]=0;
                    offset = 0;
                    if (gb->RealmServer)
                    {
                        stccpy((UBYTE *)&local[0],(UBYTE *)&gb->RealmName,64);
                        offset = strlen(local);
                        local[offset++]=':';
                    }
                    stccpy(&local[offset],OURHOSTNAME,64);
                    stccpy((UBYTE *) &lnr->nr_FromHost, (UBYTE *) &local[0], 128);
                    rdp_output(askc, b);
                }
            }

            oldstate = askc->conn_State;

            ObtainSemaphore(&askc->conn_InSema);
            if (!IsListEmpty((struct List *) & askc->conn_InList))
            {
                struct Buffer *bf;
                struct BuffEntry *be;
                struct NameReply *mnr;
                char temphost[128];

                ObtainSemaphore(&nb->WakeLock);
                Remove((struct Node *)&wn);
                ReleaseSemaphore(&nb->WakeLock);

                bf = (struct Buffer *) RemHead((struct List *) & askc->conn_InList);
                mnr = (struct NameReply *) BuffPointer(bf, 0, &be);
                ret_addr = mnr->nr_PortNumber;
                stccpy((UBYTE *)&temphost[0],(UBYTE *)&mnr->nr_ThisHost[0],128);
                FreeBuffer(bf);
                ReleaseSemaphore(&askc->conn_InSema);
                DelSigBit(askc);
                CloseConnection(askc);

                if (ret_addr)   /* If that name exists, and they returned a
                                 * port # */
                {
                    struct RDPConnection *linkcon;
                    struct Entity *linkent;
                    UBYTE *linkname;

                    linkent = (struct Entity *) AllocMem(sizeof(struct Entity), MEMF_CLEAR | MEMF_PUBLIC);

                    if (linkent)
                    {
                        linkent->entity_linkname = (UBYTE *) AllocMem(strlen(temphost) + 1,MEMF_PUBLIC);
                        if (linkent->entity_linkname)                  /* Label this link entity with the foreign host name */
                            strcpy(linkent->entity_linkname,temphost);
                        linkname = (UBYTE *) AllocMem(strlen(portname) + 1, MEMF_PUBLIC);
                        strcpy(linkname, portname);
                        linkent->entity_Port.mp_Node.ln_Name = linkname;
                        NewList((struct List *) & (linkent->entity_Partials));
                        linkent->entity_Flags |= ENTF_LINK;
                        NewList((struct List *) & linkent->entity_translist);
                        InitSemaphore(&linkent->entity_transsema);
                        ObtainSemaphore(&(src->entity_linksema));
                        linkent->entity_owner = src;
                        AddTail((struct List *) & src->entity_linklist, (struct Node *) linkent);
                        ReleaseSemaphore(&(src->entity_linksema));

                        Forbid();
                        linkcon = OpenActive(dest, ret_addr, &EntityInput, &EntityStatus);
                        if (linkcon)
                        {
                            AddSigBit(linkcon);
                            linkent->entity_connection = linkcon;
                            linkcon->conn_linkentity = linkent;
                        }
                        else
                        {
                            struct Buffer *abuff;
                            Permit();
                            ObtainSemaphore(&src->entity_linksema);
                            Remove((struct Node *) linkent);
                            ReleaseSemaphore(&src->entity_linksema);
                            if (linkent->entity_linkname)
                                FreeMem(linkent->entity_linkname,strlen(linkent->entity_linkname)+1);
                            if (linkent->entity_Port.mp_Node.ln_Name)
                                FreeMem(linkent->entity_Port.mp_Node.ln_Name, strlen(linkent->entity_Port.mp_Node.ln_Name) + 1);
                            while (abuff = (struct Buffer *) RemHead((struct List *) &linkent->entity_Partials))
                                FreeBuffer(abuff);
                            FreeMem(linkent, sizeof(struct Entity));
                            return(0L);
                        }
                        Permit();
                        WaitConnect(linkcon);
                        DelSigBit(linkcon);

                        if (linkcon->conn_State != STATE_OPEN)
                        {
                            struct Buffer *abuff;

                            CloseConnection(linkcon);
                            ObtainSemaphore(&src->entity_linksema);
                            Remove((struct Node *) linkent);
                            ReleaseSemaphore(&src->entity_linksema);
                            if (linkent->entity_linkname)
                                FreeMem(linkent->entity_linkname,strlen(linkent->entity_linkname)+1);
                            if (linkent->entity_Port.mp_Node.ln_Name)
                                FreeMem(linkent->entity_Port.mp_Node.ln_Name, strlen(linkent->entity_Port.mp_Node.ln_Name) + 1);
                            while (abuff = (struct Buffer *) RemHead((struct List *)&linkent->entity_Partials))
                                FreeBuffer(abuff);
                            FreeMem(linkent, sizeof(struct Entity));
                            ret_addr = 0L;
                        }
                        else
                            ret_addr = (ULONG) linkent;
                    }
                    else
                    {
                        if (detailerror)
                            *detailerror = ENVOYERR_NORESOURCES;
                        return(0L);
                    }
                }
                if ((!ret_addr) && (detailerror))
                {
                    *detailerror = ENVOYERR_UNKNOWNENTITY;
                }
                return ((void *) ret_addr);
            }
            ReleaseSemaphore(&askc->conn_InSema);
            Sleep(askc);
            if (!wn.wn_Time)
                break;
        }
        ObtainSemaphore(&nb->WakeLock);
        Remove((struct Node *)&wn);
        ReleaseSemaphore(&nb->WakeLock);
        DelSigBit(askc);

        CloseConnection(askc);  /* The connection went to the CLOSE state */
        if (detailerror)
            *detailerror = ENVOYERR_NORESOLVER;
        return ((void *) 0L);
    }
}

/*------------------------------------------------------------------------*/

BOOL ask_input(c, b)
struct RDPConnection *c;
struct Buffer *b;
{

    ObtainSemaphore(&c->conn_InSema);
    AddTail((struct List *) & c->conn_InList, (struct Node *) b);
    ReleaseSemaphore(&c->conn_InSema);
    Wake(c);

    return (TRUE);
}

/*------------------------------------------------------------------------*/

BOOL ask_status(c)
struct RDPConnection *c;
{

    Wake(c);

    return (FALSE);
}

/****** nipc.library/LoseEntity ******************************************
*
*   NAME
*     LoseEntity -- Free up any resources attached from a FindEntity.
*
*   SYNOPSIS
*     LoseEntity(entity)
*                  A0
*
*     VOID LoseEntity(struct Entity *);
*
*   FUNCTION
*     This will merely free up any resources allocated with a
*     successful FindEntity() call.
*
*   INPUTS
*     entity      - An abstract data type - a "magic cookie" - that
*                   identifies an Entity.  NULL for no action.
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*     LoseEntity() should only be used on Entities returned by FindEntity().
*
*
*   BUGS
*
*   SEE ALSO
*     CreateEntity(), DeleteEntity()
*
******************************************************************************
*
*/
void __asm LoseEntity(register __a0 void *entin)
{
    register struct NBase *nb = gb;
    struct Entity *ent;
    struct Entity *eptr;
    struct Buffer *abuff;

    if (!entin)
        return;

    ent = (struct Entity *) entin;
    if (!(ent->entity_Flags & ENTF_LINK))
    {
        ent->entity_LinkCount--;
        DelayedDelete(ent);
        return;                 /* If not a link, don't touch */
    }

    ObtainSemaphore(&nb->RDPCLSemaphore);   /* To prevent deadlock */
    ObtainSemaphore(&nb->ANMPELSemaphore);
    eptr = ent->entity_owner;
    ObtainSemaphore(&(eptr->entity_linksema));

    if (ent->entity_connection)
    {
        struct RDPConnection *oldc;
        oldc = ent->entity_connection;
        if (!(ent->entity_connection->conn_WakeBit+1))  /* Don't delete them if somebody's waiting on it */
        {
            ent->entity_connection = 0L;
            CloseConnection(oldc);
        }
    }

    Remove((struct Node *) ent);

    ReleaseSemaphore(&(eptr->entity_linksema));
    ReleaseSemaphore(&nb->ANMPELSemaphore);
    ReleaseSemaphore(&nb->RDPCLSemaphore);

    if (ent->entity_linkname)
        FreeMem(ent->entity_linkname,strlen(ent->entity_linkname)+1);

    /* Free up the target name */
    if (ent->entity_Port.mp_Node.ln_Name)
        FreeMem(ent->entity_Port.mp_Node.ln_Name, strlen(ent->entity_Port.mp_Node.ln_Name) + 1);

    while (abuff = (struct Buffer *) RemHead((struct List *)&ent->entity_Partials))
        FreeBuffer(abuff);

    if (ent->entity_Flags & ENTF_LINK)
    {
        struct Transaction *at;
        ObtainSemaphore(&ent->entity_transsema);
        while (at = (struct Transaction *) RemHead((struct List *)&ent->entity_translist))
        {
            at->trans_Error = ENVOYERR_ABORTED;
            ReturnTransaction(at);
        }
        ReleaseSemaphore(&ent->entity_transsema);
    }

    FreeMem(ent, sizeof(struct Entity));
}

/****** nipc.library/AllocTransactionA *****************************************
*
*   NAME
*     AllocTransactionA -- Allocate a transaction structure.
*
*   SYNOPSIS
*     transaction = AllocTransactionA(taglist)
*        D0                             A0
*
*     struct Transaction *AllocTransactionA(struct TagItem *);
*     struct Transaction *AllocTransaction(tag1, tag2, tag3, ...);
*
*   FUNCTION
*     This function will attempt to create a Transaction structure
*     for the user.
*
*   INPUTS
*     taglist -- A pointer to a list of tags defining options and requirements
*                for the structure.
*
*                TRN_AllocReqBuffer:
*                 Attempts to allocate a request buffer for you of the
*                 size in the ti_Data field of this TagItem.
*                 (default: no request buffer)
*                TRN_AllocRespBuffer:
*                 Attempts to allocate a response buffer for you of the
*                 size in the ti_Data field of this TagItem.
*                 (default: no response buffer)
*                TRN_ReqDataNIPCBuff:
*                TRN_RespDataNIPCBuff:
*                 These Tags are used to tell nipc.library that either
*                 trans_ResponseData or trans_RequestData are pointers
*                 to NIPCBuff structures.  "Normal" programmers won't
*                 need to use this feature.
*
*                A NULL taglist will yield the defaults.
*
*   RESULT
*     transaction -- If conditions permit, AllocTransaction() will return
*                    a pointer to a Transaction structure.  If the library is
*                    unable to either allocate memory or meet the requirements
*                    set by the tags passed, the function will return a NULL.
*
*   EXAMPLE
*
*   NOTES
*                You are completely free to allocate your own buffer space
*                for either a request or a response buffer.  If done, you
*                are also responsible for freeing them.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

struct Transaction *__asm AllocTransactionA(register __a0 struct TagItem * taglist)
{

    BOOL AllocReq, AllocResp, BufferReq, BufferResp;
    ULONG ReqBuffSize, RespBuffSize;
    struct TagItem *tag;
    struct TagItem *tstate;
    struct Transaction *newtrans;

    AllocReq = AllocResp = BufferReq = BufferResp = FALSE;

    if (taglist)
    {
        tstate = taglist;
        while (tag = (struct TagItem *) NextTagItem(&tstate))
        {
            switch (tag->ti_Tag)
            {
                case TRN_AllocReqBuffer:
                    ReqBuffSize = tag->ti_Data;
                    AllocReq = TRUE;
                    break;

                case TRN_AllocRespBuffer:
                    RespBuffSize = tag->ti_Data;
                    AllocResp = TRUE;
                    break;

                case TRN_ReqDataNIPCBuff:
                    BufferReq = TRUE;
                    break;

                case TRN_RespDataNIPCBuff:
                    BufferResp = TRUE;
                    break;
            }
        }
    }

    if(newtrans = (struct Transaction *) AllocMem(sizeof(struct Transaction), MEMF_CLEAR | MEMF_PUBLIC))
    {
    	if(BufferReq)
    	    newtrans->trans_Flags |= TRANSF_REQNIPCBUFF;

    	if(BufferResp)
    	    newtrans->trans_Flags |= TRANSF_RESPNIPCBUFF;

        if (AllocReq)
        {
            newtrans->trans_ReqDataLength = ReqBuffSize;
            newtrans->trans_ReqDataActual = ReqBuffSize;

            if(BufferReq)
            {
            	if(!(newtrans->trans_RequestData = (UBYTE *) AllocBuffer(ReqBuffSize)))
            	{
                    FreeMem(newtrans, sizeof(struct Transaction));
            	    return((struct Transaction *) 0L);
            	}
            }
            else if (!(newtrans->trans_RequestData = (UBYTE *) AllocMem(ReqBuffSize, MEMF_PUBLIC)))
            {
                FreeMem(newtrans, sizeof(struct Transaction));
                return ((struct Transaction *) 0L);
            }
            newtrans->trans_Flags |= TRANSF_REQBUFFERALLOC;
        }

        if (AllocResp)
        {
            newtrans->trans_RespDataLength = RespBuffSize;
            newtrans->trans_RespDataActual = RespBuffSize;

            if(BufferResp)
            {
            	if(!(newtrans->trans_ResponseData = (UBYTE *)AllocBuffer(RespBuffSize)))
            	{
            	    if(AllocReq)
            	    {
            	    	if(BufferReq)
            	    	    FreeBuffer((struct Buffer *)newtrans->trans_RequestData);
            	    	else
            	    	    FreeMem(newtrans->trans_RequestData, ReqBuffSize);
            	    }
            	    FreeMem(newtrans, sizeof(struct Transaction));
            	    return((struct Transaction *)0L);
            	}
            }
            else if (!(newtrans->trans_ResponseData = (UBYTE *) AllocMem(RespBuffSize, MEMF_PUBLIC)))
            {
                if (AllocReq)
                {
                    if(BufferReq)
                        FreeBuffer((struct Buffer *)newtrans->trans_RequestData);
                    else
                        FreeMem(newtrans->trans_RequestData, ReqBuffSize);
		}
                FreeMem(newtrans, sizeof(struct Transaction));
                return ((struct Transaction *) 0L);
            }
            newtrans->trans_Flags |= TRANSF_RESPBUFFERALLOC;
        }
        newtrans->trans_Type = 255;
    }
    return (newtrans);
}

/****** nipc.library/FreeTransaction ******************************************
*
*   NAME
*     FreeTransaction -- Free a previously allocated Transaction structure.
*
*   SYNOPSIS
*     FreeTransaction(transaction)
*                          A1
*
*     VOID FreeTransaction(struct Transaction *);
*
*   FUNCTION
*     Transaction structures created by AllocTransaction() (which ought be
*     ALL of them), must be deallocated with FreeTransaction().
*     FreeTransaction() frees only the Transaction structure and any
*     portions of the structure that were allocated by the AllocTransaction()
*     function.
*
*   INPUTS
*     transaction - a pointer to the Transaction structure to free
*                   if NULL, no action is taken.
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*     Don't try to free anything that wasn't allocated by AllocTransaction().
*
*   BUGS
*
*   SEE ALSO
*     AllocTransactionA()
*
******************************************************************************
*
*/
void __asm FreeTransaction(register __a1 struct Transaction * oldtrans)
{

    if (!oldtrans)      /* Some moron has called FreeTransaction() will a null transaction */
        return;

    if (oldtrans->trans_Flags & TRANSF_REQBUFFERALLOC)
    {
    	if(oldtrans->trans_Flags & TRANSF_REQNIPCBUFF)
    	    FreeBuffer((struct Buffer *)oldtrans->trans_RequestData);
    	else
            FreeMem(oldtrans->trans_RequestData, oldtrans->trans_ReqDataLength);
    }
    if (oldtrans->trans_Flags & TRANSF_RESPBUFFERALLOC)
    {
    	if(oldtrans->trans_Flags & TRANSF_RESPNIPCBUFF)
    	    FreeBuffer((struct Buffer *)oldtrans->trans_ResponseData);
    	else
            FreeMem(oldtrans->trans_ResponseData, oldtrans->trans_RespDataLength);
    }
    FreeMem(oldtrans, sizeof(struct Transaction));
}

/*------------------------------------------------------------------------*/


/****** nipc.library/GetTransaction ******************************************
*
*   NAME
*     GetTransaction -- receive the next Transaction waiting on an Entity
*
*   SYNOPSIS
*     transaction = GetTransaction(entity)
*       D0                           A0
*
*     struct Transaction *GetTransaction(struct Entity *);
*
*   FUNCTION
*     GetTransaction() attempts to remove the next available Transaction
*     from the given Entity.
*
*   INPUTS
*     entity  -- An entity created by CreateEntity().  (Not from
*                FindEntity() as you're NOT allowed to read from
*                someone else's Entity.)
*
*   RESULT
*     transaction   --
*                 NULL if no Transactions are waiting,
*                 a pointer to a Transaction structure otherwise.
*
*   EXAMPLE
*
*   NOTES
*     After a successful GetTransaction(), you should check the trans_Type
*     field of a transaction to determine whether it's a request, or a
*     returned failed request/response that was previously
*     BeginTransaction()'d from this Entity.
*
*   BUGS
*
*   SEE ALSO
*     BeginTransaction(), DoTransaction(), WaitTransaction(),
*     ReplyTransaction(), <envoy/nipc.h>
*
******************************************************************************
*
*/


struct Transaction *__asm GetTransaction(register __a0 void *from_entity)
{
    struct Entity *f_entity;
    struct Transaction *t;

    f_entity = (struct Entity *) from_entity;

    Forbid();
    t = (struct Transaction *) GetMsg(&(f_entity->entity_Port));
    if (t)
        if (t->trans_Type == TYPE_REQUEST)
            t->trans_Type = TYPE_SERVICING;
    Permit();

    return (t);

}

/*------------------------------------------------------------------------*/

/****** nipc.library/ReplyTransaction ******************************************
*
*   NAME
*     ReplyTransaction -- Reply a Transaction.
*
*   SYNOPSIS
*     ReplyTransaction(transaction)
*                           A1
*
*     VOID ReplyTransaction(struct Transaction *);
*
*   FUNCTION
*     This causes a Transaction received from another Entity to be
*     returned to the sender.  You may return data with the
*     Transaction as well.  (See the Transaction structure
*     definition for details.)
*
*   INPUTS
*     transaction -- Pointer to a Transaction structure that was
*                    returned by a previous GetTransaction() call.
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       GetTransaction()
*
******************************************************************************
*
*/
void __asm ReplyTransaction(register __a1 struct Transaction * trans)
{

        register struct NBase *nb = gb;

    if (trans->trans_Type == -1L)   /* unissued */
        return;

    /*
     * There are two possibilities:
     *
     * - Local reply, in which a PutMsg suffices to return the transaction to
     * the proper Entity.
     *
     * - Networked reply, in which life ain't so simple.  <sigh>
     *
     */

    if ((trans->trans_SourceEntity)->entity_Flags & ENTF_LINK)
    {
        /* networked */
        if (trans->trans_Type != TYPE_RESPONSE)
        {

            /*
             * The first thing to do is to create a ResponsePacket structure
             * w/ the response data following the structure.
             */
            struct ResponsePacket *rp;
            struct Buffer *b;
            struct BuffEntry *be;

            ULONG fullsize, fulllimit, datalimit, offset;
            UBYTE *dataarea;
            struct Entity *src;
            src = trans->trans_SourceEntity;

            offset = 0;
            fullsize = trans->trans_RespDataActual;     /* size of data to send */
            if (!fullsize)      /* If no response data needed */
            {
            	if(b = AllocBuffer(sizeof(struct ResponsePacket)))
            	{
            	    be = (struct BuffEntry *)b->buff_list.mlh_Head;
            	    rp = (struct ResponsePacket *)be->be_data;
            	    be->be_length = sizeof(struct ResponsePacket);

		    rp->respack_Filler = 0;
		    rp->respack_Segment = 0;
                    rp->respack_PacketType = PACKET_DATA;
                    rp->respack_Error = trans->trans_Error;
                    rp->respack_NewCommand = trans->trans_Command;
                    rp->respack_Sequence = trans->trans_Sequence;
                    rp->respack_Flags = RESFLAG_LASTSEGMENT;
                    rp->respack_DataLength = 0L;
                    src = trans->trans_SourceEntity;
                    ObtainSemaphore(&nb->RDPCLSemaphore);

                    if (src->entity_connection)
                    {
                        if (src->entity_connection->conn_State == STATE_OPEN)
                        {
                            if (src->entity_connection->conn_SendNxt -
                                src->entity_connection->conn_SendOldest >=
                                src->entity_connection->conn_SendMax)
                            {
                                if (!AddSigBit(src->entity_connection))
                                {
                                    src->entity_LinkCount--;
                                    REntityStatus(src->entity_connection);
                                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                                    FreeBuffer(b);
                                    FreeTransaction(trans);
                                    return;
                                }
                                src->entity_connection->conn_Flags |= CONN_WINDOWSIGNAL;
                                while ((src->entity_connection->conn_SendNxt -
                                        src->entity_connection->conn_SendOldest >=
                                        src->entity_connection->conn_SendMax) &&
                                       (src->entity_connection->conn_State == STATE_OPEN))
                                {
                                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                                    Sleep(src->entity_connection);
                                    ObtainSemaphore(&nb->RDPCLSemaphore);
                                }
                                src->entity_connection->conn_Flags &= !CONN_WINDOWSIGNAL;
                                DelSigBit(src->entity_connection);
                            }
                            rdp_output(src->entity_connection, b);
                        }
                        else
                            FreeBuffer(b);
                    }
                    else
                        FreeBuffer(b);

                    src->entity_LinkCount--;

                    REntityStatus(src->entity_connection);
                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                    FreeTransaction(trans);
                    return;
                }
            }
            ObtainSemaphore(&nb->RDPCLSemaphore);
            if ((trans->trans_SourceEntity)->entity_connection)
                fulllimit = (trans->trans_SourceEntity)->entity_connection->conn_SendMaxSize;     /* Max packet size */
            else
            {
                src->entity_LinkCount--;
                REntityStatus(src->entity_connection);
                ReleaseSemaphore(&nb->RDPCLSemaphore);
                FreeTransaction(trans);
                return;
            }
            ReleaseSemaphore(&nb->RDPCLSemaphore);
            datalimit = fulllimit - sizeof(struct ResponsePacket);      /* Max data area per
                                                                         * packet */
            while (fullsize)
            {
                ULONG thispack;
                thispack = MIN(fullsize, datalimit) + sizeof(struct ResponsePacket);

                if(b = AllocBuffer(thispack))
                {
            	    be = (struct BuffEntry *)b->buff_list.mlh_Head;
            	    rp = (struct ResponsePacket *)be->be_data;
            	    be->be_length = thispack;

		    rp->respack_Flags = 0;
		    rp->respack_Segment = 0;
		    rp->respack_Filler = 0;
                    rp->respack_PacketType = PACKET_DATA;
                    rp->respack_Error = trans->trans_Error;
                    rp->respack_NewCommand = trans->trans_Command;
                    rp->respack_Sequence = trans->trans_Sequence;
                    rp->respack_DataLength = thispack - sizeof(struct ResponsePacket);
                    dataarea = (UBYTE *) (((ULONG) rp) + (ULONG) sizeof(struct ResponsePacket));
                    CopyMem(&(((UBYTE *)trans->trans_ResponseData)[offset]), dataarea, rp->respack_DataLength);
                    if (!(fullsize - thispack + sizeof(struct ResponsePacket)))
                        rp->respack_Flags = RESFLAG_LASTSEGMENT;
                    src = trans->trans_SourceEntity;
                    ObtainSemaphore(&nb->RDPCLSemaphore);
                    if (src->entity_connection)
                    {
                        if (src->entity_connection->conn_State == STATE_OPEN)
                        {
                            if (src->entity_connection->conn_SendNxt -
                                src->entity_connection->conn_SendOldest >=
                                src->entity_connection->conn_SendMax)
                            {
                                if (!AddSigBit(src->entity_connection))
                                {
                                    src->entity_LinkCount--;
                                    REntityStatus(src->entity_connection);
                                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                                    FreeBuffer(b);
                                    FreeTransaction(trans);
                                    return;
                                }
                                src->entity_connection->conn_Flags |= CONN_WINDOWSIGNAL;
                                while ((src->entity_connection->conn_SendNxt -
                                        src->entity_connection->conn_SendOldest >=
                                        src->entity_connection->conn_SendMax) &&
                                       (src->entity_connection->conn_State == STATE_OPEN))
                                {
                                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                                    Sleep(src->entity_connection);
                                    ObtainSemaphore(&nb->RDPCLSemaphore);
                                }
                                src->entity_connection->conn_Flags &= !CONN_WINDOWSIGNAL;
                                DelSigBit(src->entity_connection);
                            }
                            rdp_output(src->entity_connection, b);
                        }
                        else
                            FreeBuffer(b);
                    }
                    else
                        FreeBuffer(b);
                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                }
                else
                {
                    src->entity_LinkCount--;
                    return;
                }
                offset += (thispack - sizeof(struct ResponsePacket));
                fullsize -= (thispack - sizeof(struct ResponsePacket));
            }
            ObtainSemaphore(&nb->RDPCLSemaphore);
            src->entity_LinkCount--;
            REntityStatus(src->entity_connection);
            ReleaseSemaphore(&nb->RDPCLSemaphore);

            FreeTransaction(trans);

        }
    }
    else
    {
        /* local */
        if (trans->trans_Type != TYPE_RESPONSE)
        {
            Forbid();
            ReturnTransaction(trans);
            Permit();
        }
    }
}

/****** nipc.library/WaitTransaction ******************************************
*
*   NAME
*     WaitTransaction -- Waits for a Transaction to complete.
*
*   SYNOPSIS
*     WaitTransaction(transaction)
*                          A1
*
*     VOID WaitTransaction(struct Transaction *);
*
*   FUNCTION
*     Waits for the given transaction to complete, or return as failed.
*
*   INPUTS
*     transaction -- a pointer to a Transaction structure.
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*     - When doing networked transactions, if for any reason the remote
*       machine fails to send a response, (but no network problem exists)
*       and you did not supply a timeout value in trans_Timeout,
*       WaitTransaction() can Wait() forever.
*
*     - If you attempt to WaitTransaction() on a transaction that has
*       already been responded to, WaitTransaction() will return
*       immediately.
*
*     - If the given Transaction has not already completed, WaiTransaction()
*       will wait for it to complete, and then -remove- the Transaction
*       from the source Entity.  (In other words, you don't need to
*       GetTransaction() it off of the Entity.)
*
*   BUGS
*
*   SEE ALSO
*       BeginTransaction(), DoTransaction(), exec.library/Wait()
*
******************************************************************************
*
*/


void __asm WaitTransaction(register __a1 struct Transaction * trans)
{
    struct Entity *src_ent;
    ULONG sigbit;

    src_ent = trans->trans_SourceEntity;
    if (trans->trans_Msg.mn_ReplyPort)
        sigbit = trans->trans_Msg.mn_ReplyPort->mp_SigBit;
    else
        sigbit = src_ent->entity_Port.mp_SigBit;

    if (trans->trans_Type == -1L)       /* unissued transaction */
        return;

    while (trans->trans_Type != TYPE_RESPONSE)
        Wait(1 << sigbit);

    Disable();
    Remove(&(trans->trans_Msg.mn_Node));        /* Pull it from the MsgPort */
    Enable();

}

/****** nipc.library/CheckTransaction ******************************************
*
*   NAME
*     CheckTransaction -- Check whether a transaction has completed or not.
*
*   SYNOPSIS
*     status = CheckTransaction(transaction);
*       D0                          A1
*
*     BOOL CheckTransaction(struct Transaction *);
*
*   FUNCTION
*     Provides a mechanism for determining whether a Transaction previously
*     queued with BeginTransaction() has completed.  The function
*     will return TRUE if the Transaction is complete (or failed), and FALSE
*     if the Transaction is still in progress.
*
*   INPUTS
*     transaction - a pointer to a Transaction structure, which describes
*                   the details of this specific Transaction.
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*       CheckTransaction() will return TRUE if the Transaction has never
*       been sent (unlike CheckIO(), which doesn't take this situation
*       into account).
*
*   BUGS
*
*   SEE ALSO
*       DoTransaction(), BeginTransaction(), WaitTransaction(),
*       AbortTransaction()
*
******************************************************************************
*
*/
BOOL __asm CheckTransaction(register __a1 struct Transaction * trans)
{

    if (trans->trans_Type == -1L)   /* unissued */
        return(TRUE);

    if (trans->trans_Type != TYPE_RESPONSE)
        return (FALSE);
    else
        return (TRUE);
}


/*------------------------------------------------------------------------*/

/* Where a host receives it's side of a FindEntity() call */

BOOL ResolverIn(c, buff)
struct RDPConnection *c;
struct Buffer *buff;
{
    register struct NBase *nb = gb;
    char local[128];
    int offset;
    struct NameReply lnr;
    struct NameRequest *namereq;
    UWORD RDPPort = 0;
    struct BuffEntry *be;
    struct RDPConnection *tmpconn;

    UWORD maxport=0;

    /* Check to see if the other machine has 'crashed'.  If so, dump all
     * links associated with that machine.  Do this by finding the max
     * port # in use (from the remote machine), and comparing the
     * given port number to it.
     */
    tmpconn = (struct RDPConnection *) nb->RDPConnectionList.mlh_Head;
    while (tmpconn->conn_link.mln_Succ)
    {
        if ((tmpconn->conn_TheirPort > maxport) && (tmpconn->conn_TheirAddress == c->conn_TheirAddress))
            maxport = tmpconn->conn_TheirPort;
        tmpconn = (struct RDPConnection *) tmpconn->conn_link.mln_Succ;
    }

    if (c->conn_TheirPort <= maxport)        /* If true, they've likely rebooted */
    {
        tmpconn = (struct RDPConnection *) nb->RDPConnectionList.mlh_Head;
        while (tmpconn->conn_link.mln_Succ)
        {
            if ((tmpconn->conn_TheirPort >= c->conn_TheirPort) && (c->conn_TheirAddress == tmpconn->conn_TheirAddress)
                   && (tmpconn != c) && (tmpconn->conn_linkentity->entity_Flags & ENTF_SERVERLINK))
            {
                struct RDPConnection *t2;
                t2 = (struct RDPConnection *) tmpconn->conn_link.mln_Succ;
                tmpconn->conn_State = STATE_CLOSE;
                /* Send a reset */
                SendFlags(tmpconn,RDPF_RST|RDPF_ACK);
                if ((*tmpconn->conn_Status)(tmpconn))    /* potentially a problem! ! ! */
                    tmpconn = (struct RDPConnection *) t2->conn_link.mln_Pred;
                /* This SHOULDN'T be a problem, since any connection getting deleted cannot be
                 * the one being counted on right now.
                 */

            }
            tmpconn = (struct RDPConnection *) tmpconn->conn_link.mln_Succ;
        }
    }


    be = (struct BuffEntry *) buff->buff_list.mlh_Head;
    namereq = (struct NameRequest *) BuffPointer(buff, 0, &be);

    /* Get ahold of the entity list and look for that name */
    ObtainSemaphore(&nb->RDPCLSemaphore);
    ObtainSemaphore(&nb->ANMPELSemaphore);

    {
        struct Entity *e;
        e = (struct Entity *) gb->ANMPEntityList.mlh_Head;
        while (e->entity_Port.mp_Node.ln_Succ)
        {
            if (e->entity_Port.mp_Node.ln_Name)
            {
                if ((e->entity_Flags & ENTF_PUBLIC) && (!Stricmp(e->entity_Port.mp_Node.ln_Name, (UBYTE *) & namereq->nr_Name)))
                {
                    /*
                     * Create a passive connection, and let them know where it's
                     * at
                     */
                    struct RDPConnection *newcon;
                    struct Entity *le;
                    UBYTE *linkname, *hname;

                    hname = (UBYTE *) AllocMem(strlen((UBYTE *) &namereq->nr_FromHost)+1,MEMF_PUBLIC);
                    if (!hname)
                        break;
                    strcpy(hname, (UBYTE *) &namereq->nr_FromHost);
                    linkname = (UBYTE *) AllocMem(strlen((UBYTE *) & namereq->nr_FromName) + 1, MEMF_PUBLIC);
                    if (!linkname)
                    {
                        FreeMem(hname,strlen((UBYTE *) &namereq->nr_FromHost)+1);
                        break;
                    }
                    strcpy(linkname, (UBYTE *) & namereq->nr_FromName);
                    le = (struct Entity *) AllocMem(sizeof(struct Entity), MEMF_CLEAR | MEMF_PUBLIC);
                    if (!le)
                    {
                        FreeMem(hname,strlen((UBYTE *) &namereq->nr_FromHost)+1);
                        FreeMem(linkname,strlen((UBYTE *) &namereq->nr_FromName)+1);
                        break;
                    }
                    NewList((struct List *) & (le->entity_Partials));
                    InitSemaphore(&le->entity_transsema);
                    NewList((struct List *) & le->entity_translist);
                    le->entity_Flags |= ENTF_LINK|ENTF_SERVERLINK;
                    le->entity_owner = e;
                    le->entity_Port.mp_Node.ln_Name = linkname;
                    le->entity_linkname = hname;
                    ObtainSemaphore(&e->entity_linksema);
                    AddTail((struct List *) & e->entity_linklist, (struct Node *) le);
                    ReleaseSemaphore(&e->entity_linksema);

                    newcon = OpenPassive(-1L, &REntityInput, &REntityStatus);
                    if (!newcon)
                    {
                        struct Buffer *abuff;
                        FreeMem(hname,strlen((UBYTE *) &namereq->nr_FromHost)+1);
                        FreeMem(linkname,strlen((UBYTE *) &namereq->nr_FromName)+1);
                        ObtainSemaphore(&e->entity_linksema);
                        Remove((struct Node *) le);
                        ReleaseSemaphore(&e->entity_linksema);
                        while (abuff = (struct Buffer *) RemHead((struct List *)&le->entity_Partials))
                            FreeBuffer(abuff);
                        FreeMem(le, sizeof(struct Entity));
                        break;
                    }

                    le->entity_connection = newcon;
                    newcon->conn_linkentity = le;   /* back link */
                    RDPPort = newcon->conn_OurPort;
                    ReleaseSemaphore(&nb->ANMPELSemaphore);
                    local[0]=0;
                    offset = 0;
                    if (gb->RealmServer)
                    {
                        stccpy((UBYTE *)&local[0],(UBYTE *)&gb->RealmName,64);
                        offset = strlen(local);
                        local[offset++]=':';
                    }
                    stccpy((UBYTE *)&local[offset],OURHOSTNAME,64);

                    stccpy(&lnr.nr_ThisHost[0],local,128);

                    lnr.nr_PortNumber = RDPPort;
                    if (c->conn_State == STATE_OPEN)
                        rdp_output_lame(c, (UBYTE *) & lnr, sizeof(struct NameReply));
                    FreeBuffer(buff);
                    e->entity_LinkCount++;
                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                    return (TRUE);
                }
            }
            e = (struct Entity *) e->entity_Port.mp_Node.ln_Succ;
        }
    }
    lnr.nr_PortNumber = RDPPort;

    local[0]=0;
    offset = 0;
    if (gb->RealmServer)
    {
        stccpy((UBYTE *)&local[0],(UBYTE *)&gb->RealmName,64);
        offset = strlen(local);
        local[offset++]=':';
    }
    stccpy((UBYTE *)&local[offset],OURHOSTNAME,64);

    stccpy(&lnr.nr_ThisHost[0],local,128);

    if (c->conn_State == STATE_OPEN)
        rdp_output_lame(c, (UBYTE *) & lnr, sizeof(struct NameReply));
    ReleaseSemaphore(&nb->ANMPELSemaphore);

    FreeBuffer(buff);

    ReleaseSemaphore(&nb->RDPCLSemaphore);
    return (TRUE);

}

/*------------------------------------------------------------------------*/

BOOL ResolverStatus(c)
struct RDPConnection *c;
{

    if (c->conn_State == STATE_CLOSE)
    {
        CloseConnection(c);
        return(TRUE);
    }

    return (FALSE);
}

/*------------------------------------------------------------------------*/

BOOL AddSigBit(c)
struct RDPConnection *c;
{

    c->conn_WakeBit = AllocSignal(-1L);
    if (c->conn_WakeBit == -1)
        return(FALSE);
    c->conn_WakeTask = FindTask(0L);
    return(TRUE);
}

/*------------------------------------------------------------------------*/

void DelSigBit(c)
struct RDPConnection *c;
{
    if (c->conn_WakeTask)
    {
        int sigbit;
        Forbid();
        sigbit = c->conn_WakeBit;
        c->conn_WakeTask = 0L;
        c->conn_WakeBit = -1;
        FreeSignal(sigbit);
        Permit();
    }
}

/*------------------------------------------------------------------------*/

void Sleep(c)
struct RDPConnection *c;
{
    Forbid();           /* Yes, I know what I'm doing here! */
    if (c->conn_State != STATE_CLOSE)
        if (c->conn_WakeBit+1)
            Wait(1 << c->conn_WakeBit);
    Permit();           /* Yes yes yes */
}


/*------------------------------------------------------------------------*/

void Wake(c)
struct RDPConnection *c;
{
    Forbid();
    if (c->conn_WakeTask)
        Signal(c->conn_WakeTask, (1 << c->conn_WakeBit));
    Permit();
}

/*------------------------------------------------------------------------*/

void WaitConnect(c)
struct RDPConnection *c;
{

    /* Wait for it either to connect or time out */
    while ((c->conn_State != STATE_OPEN) && (c->conn_State != STATE_CLOSE))
        Sleep(c);
}

/*------------------------------------------------------------------------*/

/*
 * EntityInput handles arriving remote RESPONSES - and guides them back to
 * the original entity
 */
void EntityInput(c, buff)
struct RDPConnection *c;
struct Buffer *buff;
{
    register struct NBase *nb = gb;

    struct ResponsePacket *rp;
    struct BuffEntry *be;
    struct Entity *linker;

  /*  int gcnt=0; */


    ObtainSemaphore(&nb->ANMPELSemaphore);

    rp = (struct ResponsePacket *) BuffPointer(buff, 0L, &be);

    if (rp->respack_PacketType == PACKET_DATA)
    {
        linker = c->conn_linkentity;

        /* sift through all of the transactions, looking for the correct one */
        ObtainSemaphore(&(linker->entity_transsema));
        {
            struct Transaction *is;
            is = (struct Transaction *) linker->entity_translist.mlh_Head;
            while (is->trans_Msg.mn_Node.ln_Succ)
            {
                if (is->trans_Sequence == rp->respack_Sequence)
                {
                    UBYTE *todata;
                    struct Buffer *tb;

                    AddTail((struct List *) & (linker->entity_Partials), (struct Node *) buff);

                    if (!(rp->respack_Flags & RESFLAG_LASTSEGMENT))
                    {
                        ReleaseSemaphore(&linker->entity_transsema);
                        ReleaseSemaphore(&nb->ANMPELSemaphore);
                        return;
                    }

                    buff = (struct Buffer *) RemHead((struct List *) & (linker->entity_Partials));
                    rp = (struct ResponsePacket *) BuffPointer(buff, 0L, &be);

                    while (tb = (struct Buffer *) RemHead((struct List *) & (linker->entity_Partials)))
                    {
                        struct ResponsePacket *trp;

                        trp = (struct ResponsePacket *) BuffPointer(tb, 0L, &be);
                        rp->respack_DataLength += trp->respack_DataLength;

                        ((struct BuffEntry *) tb->buff_list.mlh_Head)->be_offset += sizeof(struct ResponsePacket);
                        ((struct BuffEntry *) tb->buff_list.mlh_Head)->be_length -= sizeof(struct ResponsePacket);

                        buff = (struct Buffer *) MergeBuffer(buff, tb);
                       /* gcnt++; */
                    }

                    /*
                     * Verify that the return data will fit into the allocated
                     * space
                     */
                    todata = is->trans_ResponseData;
                    ((struct BuffEntry *) buff->buff_list.mlh_Head)->be_offset += sizeof(struct ResponsePacket);
                    ((struct BuffEntry *) buff->buff_list.mlh_Head)->be_length -= sizeof(struct ResponsePacket);

/*
                    if (is->trans_Flags & TRANSF_RESPONSETABLE)
                    {
                        ULONG ttb[4], *w, *ww, sz;
                        struct BuffEntry *mbe;
                        ttb[0]=(ULONG) todata;
                        ttb[1]=(ULONG) rp->respack_DataLength;
                        ttb[2]=0L;
                        ttb[3]=0L;
                        sz = CountEntries(buff)*8;
                        w = (ULONG *) AllocMem(sz,MEMF_PUBLIC);
                        ww = w;
                        mbe = (struct BuffEntry *) buff->buff_list.mlh_Head;
                        if (ww)
                        {
                            while(mbe->be_link.mln_Succ)
                            {
                                ww[0] = (ULONG) mbe->be_data;
                                ww[1] = (ULONG) mbe->be_length;
                                ww +=2;
                                mbe = (struct BuffEntry *) mbe->be_link.mln_Succ;
                            }
                            ww[0]=0L;
                            ww[1]=0L;
                            CopyBroken((ULONG *)w,(ULONG *)&ttb,0,0);
                            FreeMem(w,sz);
                        }
                    }
                    */
                    if (TRUE)
                    {
                        if (is->trans_RespDataLength >= rp->respack_DataLength)
                        {
                            if(is->trans_Flags & TRANSF_RESPNIPCBUFF)
                                CopyNIPCBuff((struct NIPCBuff *)buff, (struct NIPCBuff *)todata, 0, 0, MIN(is->trans_RespDataLength, rp->respack_DataLength));
                            else
                                CopyFromBuffer(todata, buff, MIN(is->trans_RespDataLength, rp->respack_DataLength));
                            is->trans_Error = rp->respack_Error;
                        }
                        else
                            is->trans_Error = ENVOYERR_SMALLRESPBUFF;
                    }
                    is->trans_Command = rp->respack_NewCommand;
                    is->trans_RespDataActual = MIN(is->trans_RespDataLength,rp->respack_DataLength);
                    Forbid();
                    Remove((struct Node *) is);
                    ReturnTransaction(is);
                    Permit();
                    FreeBuffer(buff);
                    ReleaseSemaphore(&(linker->entity_transsema));
                    ReleaseSemaphore(&nb->ANMPELSemaphore);
                    return;
                }
                is = (struct Transaction *) is->trans_Msg.mn_Node.ln_Succ;
            }
        }

        ReleaseSemaphore(&(linker->entity_transsema));
    }
    else
    {
        if (rp->respack_PacketType == PACKET_PINGREQUEST)
            RespondToPing(c,(struct PingRequest *)rp);
        else
            if (rp->respack_PacketType == PACKET_PINGRESPONSE)
                GotResponse((struct PingResponse *)rp);
    }
    ReleaseSemaphore(&nb->ANMPELSemaphore);
    FreeBuffer(buff);
}

/*------------------------------------------------------------------------*/

/*
 * REntityInput handles arriving remote REQUESTS - and guides them to the
 * destination entity
 */
void REntityInput(c, buff)
struct RDPConnection *c;
struct Buffer *buff;
{
    struct Buffer *tb;
    struct BuffEntry *be;
    struct RequestPacket *rq;
    struct Transaction *newtrans;
    UBYTE *reqdata;
    UBYTE *resdata;
    ULONG requestsize;

    rq = (struct RequestPacket *) BuffPointer(buff, 0L, &be);
    if (rq->reqpack_PacketType == PACKET_DATA)
    {
        /*
         * If this is merely a _piece_ of the whole transaction, hold onto this
         * piece.
         */
        if (!(rq->reqpack_Flags & REQFLAG_LASTSEGMENT))
        {
            AddTail((struct List *) & (c->conn_linkentity->entity_Partials), (struct Node *) buff);
            return;
        }
        /*
         * If there are any Buffers on the Partials list, this must be the last
         * piece of the puzzle.  Merge them together.
         */

        AddTail((struct List *) & (c->conn_linkentity->entity_Partials), (struct Node *) buff);
        buff = (struct Buffer *) RemHead((struct List *) & (c->conn_linkentity->entity_Partials));
        rq = (struct RequestPacket *) BuffPointer(buff, 0L, &be);

        while(tb = (struct Buffer *) RemHead((struct List *) & (c->conn_linkentity->entity_Partials)))
        {
            struct RequestPacket *trq;
            trq = (struct RequestPacket *) BuffPointer(tb, 0L, &be);
            rq->reqpack_DataLength += trq->reqpack_DataLength;
            ((struct BuffEntry *) tb->buff_list.mlh_Head)->be_offset += sizeof(struct RequestPacket);
            ((struct BuffEntry *) tb->buff_list.mlh_Head)->be_length -= sizeof(struct RequestPacket);
            buff = (struct Buffer *) MergeBuffer(buff, tb);
        }


        newtrans = AllocMem(sizeof(struct Transaction), MEMF_CLEAR | MEMF_PUBLIC);
        if (!newtrans)
        {
            FreeBuffer(buff);
            return;
        }

        /* Make a request buffer area w/ the proper data */
        reqdata = NULL;
        /* If using modify-in-place, the buffer size we want is the bigger of the sizes of
         * the Request Actual, or the Response Allowed.
         */
        if (rq->reqpack_Flags & REQFLAG_INPLACE)
            requestsize = MAX(rq->reqpack_DataLength,rq->reqpack_ResponseDataSize);
        else
            requestsize = rq->reqpack_DataLength;
        if(rq->reqpack_DataLength)
                reqdata = (UBYTE *) AllocMem(requestsize, MEMF_CLEAR | MEMF_PUBLIC);

        if ((!reqdata) && (rq->reqpack_DataLength))
        {
            FreeMem(newtrans, sizeof(struct Transaction));
            FreeBuffer(buff);
            return;
        }

        ((struct BuffEntry *) buff->buff_list.mlh_Head)->be_offset += sizeof(struct RequestPacket);
        ((struct BuffEntry *) buff->buff_list.mlh_Head)->be_length -= sizeof(struct RequestPacket);
        CopyFromBuffer(reqdata, buff, rq->reqpack_DataLength);

        if (rq->reqpack_DataLength)
            newtrans->trans_Flags |= TRANSF_REQBUFFERALLOC;

        /* Make a response data area too */

        if (!(rq->reqpack_Flags & REQFLAG_INPLACE))
        {
            if(rq->reqpack_ResponseDataSize)
                    resdata = (UBYTE *) AllocMem(rq->reqpack_ResponseDataSize, MEMF_CLEAR | MEMF_PUBLIC);
                else
                    resdata = NULL;

            if ((!resdata) && (rq->reqpack_ResponseDataSize))
            {
                FreeMem(reqdata, requestsize);
                FreeMem(newtrans, sizeof(struct Transaction));
                FreeBuffer(buff);
                return;
            }

            if (rq->reqpack_ResponseDataSize)
                newtrans->trans_Flags |= TRANSF_RESPBUFFERALLOC;

            newtrans->trans_RespDataLength = rq->reqpack_ResponseDataSize;
        }
        else /* Modify-in-place transaction */
        {
            resdata = reqdata;
            newtrans->trans_RespDataLength = requestsize;
        }

        newtrans->trans_RequestData = reqdata;
        newtrans->trans_ReqDataLength = requestsize;
        newtrans->trans_ReqDataActual = rq->reqpack_DataLength;
        newtrans->trans_ResponseData = resdata;
        newtrans->trans_RespDataActual = newtrans->trans_RespDataLength;
        newtrans->trans_Command = rq->reqpack_Command;
        newtrans->trans_Sequence = rq->reqpack_Sequence;
        newtrans->trans_SourceEntity = c->conn_linkentity;
        newtrans->trans_Type = TYPE_REQUEST;
        c->conn_linkentity->entity_LinkCount++;
        c->conn_linkentity->entity_Unused=0;
        PutMsg(&(c->conn_linkentity->entity_owner->entity_Port), &newtrans->trans_Msg);
        FreeBuffer(buff);
    }
    else
    {
        if (rq->reqpack_PacketType == PACKET_PINGREQUEST)
        {
            RespondToPing(c,(struct PingRequest *)rq);
            FreeBuffer(buff);
        }
        else
        {
            if (rq->reqpack_PacketType == PACKET_PINGRESPONSE)
            {
                GotResponse((struct PingResponse *)rq);
                FreeBuffer(buff);
            }
        }
    }
}

/*------------------------------------------------------------------------*/

/* Handles the status of the -response- entity */
BOOL EntityStatus(c)
struct RDPConnection *c;
{
    register struct NBase *nb = gb;
    struct Entity *link;
    struct Entity *owner;
    struct Transaction *trn;

    Wake(c);

    /*
     * If the connection has gone to the close state, return all of the
     * transactions associated with this connection.
     */

    if (c->conn_State == STATE_CLOSE)
    {
        ObtainSemaphore(&nb->ANMPELSemaphore);
        link = c->conn_linkentity;
        owner = link->entity_owner;
        ObtainSemaphore(&owner->entity_linksema);
        ObtainSemaphore(&link->entity_transsema);

        /* Return the suckers */
        while (!IsListEmpty((struct List *) & link->entity_translist))
        {
            trn = (struct Transaction *) RemHead((struct List *) & link->entity_translist);
            trn->trans_Error = ENVOYERR_CANTDELIVER;
            ReturnTransaction(trn);
        }

        ReleaseSemaphore(&link->entity_transsema);
        ReleaseSemaphore(&owner->entity_linksema);
        ReleaseSemaphore(&nb->ANMPELSemaphore);
    }

    return (FALSE);
}

/*------------------------------------------------------------------------*/

/* Handles the status of the -request- entity */
BOOL REntityStatus(c)
struct RDPConnection *c;
{
    BOOL returnvalue=FALSE;
    register struct NBase *nb = gb;
    if (c)
    {
        if (c->conn_State > STATE_OPEN)
        {
            /*
             * This stuff is linked with the garbage in DeleteEntity() - to try
             * and prevent deadlocks, because they're both being run in seperate
             * processes, referencing things all over the place.
             */
            struct Entity *le;
            struct Entity *owner;
            struct RDPConnection *oldc;
            struct Buffer *abuff;

            ObtainSemaphore(&nb->ANMPELSemaphore);
            le = c->conn_linkentity;
            owner = le->entity_owner;
            ObtainSemaphore(&owner->entity_linksema);

            if (le->entity_LinkCount)                           /* If there're outstanding xactions, */
            {                                                   /* don't free this thing now. */
                le->entity_Flags |= ENTF_DELAYEDDELETE;         /* Delete me asap */
                oldc = le->entity_connection;
                if (oldc)
                    if (oldc->conn_WakeBit+1)
                        Wake(oldc);
                ReleaseSemaphore(&owner->entity_linksema);
                ReleaseSemaphore(&nb->ANMPELSemaphore);
                return(FALSE);
            }

            /*
             * The rdp connection list is already obtainsemaphore'd when this
             * code is called
             */

            oldc = le->entity_connection;
            if (oldc)
            {
                if (!(oldc->conn_WakeBit+1))
                {
                    le->entity_connection = 0L;
                    CloseConnection(oldc);
                    returnvalue = TRUE;
                }
                else
                {
                    Wake(oldc);
                    le->entity_Flags |= ENTF_DELAYEDDELETE;         /* Delete me asap */
                    ReleaseSemaphore(&owner->entity_linksema);
                    ReleaseSemaphore(&nb->ANMPELSemaphore);
                    return(FALSE);
                }
            }

            Remove(&le->entity_Port.mp_Node);

            if (le->entity_linkname)
                FreeMem(le->entity_linkname, strlen(le->entity_linkname) + 1);

            if (le->entity_Port.mp_Node.ln_Name)
                FreeMem(le->entity_Port.mp_Node.ln_Name, strlen(le->entity_Port.mp_Node.ln_Name) + 1);

            while (abuff = (struct Buffer *) RemHead((struct List *) &le->entity_Partials))
                FreeBuffer(abuff);

            FreeMem(le, sizeof(struct Entity));
            owner->entity_LinkCount--;
            ReleaseSemaphore(&owner->entity_linksema);
            DelayedDelete(owner);
            ReleaseSemaphore(&nb->ANMPELSemaphore);
        }
        else
            Wake(c);
    }
    return(returnvalue);
}


/****** nipc.library/GetEntityName ******************************************
*
*   NAME
*     GetEntityName -- Get the ASCII name associated with an Entity.
*
*   SYNOPSIS
*     status = GetEntityName(entity,stringptr,available);
*                              A0      A1        D0
*
*     BOOL GetEntityName(STRPTR, STRPTR, ULONG);
*
*   FUNCTION
*     Given a pointer to a string, the length of the string data area, and an
*     entity magic-cookie, this function will attempt to return the name
*     associated with the Entity.
*
*   INPUTS
*     entity    - A magic cookie, identifying an Entity.
*     stringptr - A pointer to a section of memory that you'd like the name
*                 copied into.
*     available - The size of the above string area.
*
*   RESULT
*     status    - Either TRUE or FALSE.  Note that even if you've
*                 referenced an entity, over a network - it's
*                 possible that resolving what the correct name
*                 is COULD fail.
*
*   EXAMPLE
*
*   NOTES
*     If the given Entity has no name, the string "UNNAMED ENTITY" will
*     be returned.
*
*   BUGS
*
*   SEE ALSO
*
*
******************************************************************************
*
*/
BOOL __asm GetEntityName(register __a0 void *entity, register __a1 UBYTE * newstr,
                                   register __d0 ULONG spacefree)
{

    /* If there's a name there, return it.  Otherwise, well ... don't. */

    if (((struct Entity *) entity)->entity_Port.mp_Node.ln_Name)
    {
        stccpy(newstr, ((struct Entity *) entity)->entity_Port.mp_Node.ln_Name, spacefree);
        return (TRUE);
    }

    return (FALSE);

}
/*------------------------------------------------------------------------*/

void DelayedDelete(e)
struct Entity *e;
{
 if (e->entity_Flags & ENTF_DELAYEDDELETE)
    DeleteEntity(e);
}


/*------------------------------------------------------------------------*/
/****** nipc.library/GetHostName ******************************************
*
*   NAME
*     GetHostName -- Get the name of a specific machine.
*
*   SYNOPSIS
*     status = GetHostName(entity,stringptr,available)
*       D0                   A0       A1        D0
*
*     BOOL GetHostName(struct Entity *, STRPTR, ULONG);
*
*   FUNCTION
*     Given a pointer to a string, the length of the string data area, and an
*     Entity magic-cookie, this function will attempt to return the ASCII
*     name associated with the host that the Entity is on.
*
*   INPUTS
*     entity    - A magic cookie, identifying an Entity.
*                 (If NULL, the string returned will be either
*                  LocalRealm:LocalHost, or simply LocalHost -- depending on
*                  whether the local machine is in a realm-based environment
*                  or not.)
*
*     stringptr - A pointer to a section of memory that you'd like the name
*                 copied into.
*     available - The size of the above string area.
*
*   RESULT
*     status    - Either TRUE or FALSE.  Note that even if you've referenced
*                 an Entity over a network, it's possible that resolving
*                 the name COULD fail.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/
BOOL __asm GetHostName(register __a0 struct Entity *ent,
                                register __a1 UBYTE *str,
                                register __d0 ULONG availsize)
{

/* Do the case where ent == 0 -- local host and realm case */
    if ((!ent) && (availsize))
    {

        int x;
        x = strlen(gb->LocalHostName)+1;
        if (gb->RealmServer)
            x += strlen(gb->RealmName)+1;

        if (x > availsize)
            return(FALSE);

        if (gb->RealmServer)
        {
            strcpy(str,gb->RealmName);
            x = strlen(str);
            str[x]=':';
            str[x+1]=0;
        }

        strcpy(&str[strlen(str)],gb->LocalHostName);
        return(TRUE);
    }
    if (ent->entity_Flags & ENTF_LINK)
    {
        if (ent->entity_linkname)
            stccpy(str,ent->entity_linkname,availsize);
        else
            return(FALSE);
    }
    else
    {
            char local[128];
            int offset;

            local[0]=0;
            offset = 0;
            if (gb->RealmServer)
            {
                stccpy((UBYTE *)&local[0],(UBYTE *)&gb->RealmName,64);
                offset = strlen(local);
                local[offset++]=':';
            }
            stccpy((UBYTE *)&local[offset],OURHOSTNAME,64);

            stccpy(str,local,availsize);
    }
    return(TRUE);

}

/*------------------------------------------------------------------------*/

void ANMPTimeout()
{
    register struct NBase *nb = gb;
    struct Entity *e,*l;
    struct WakeNode *wn;

    ObtainSemaphore(&nb->WakeLock);
    wn = (struct WakeNode *) nb->WakeFind.mlh_Head;
    while (wn->wn_Node.mln_Succ)
    {
        if (!wn->wn_Time)
            Wake(wn->wn_Connection);
        else
            wn->wn_Time--;
        wn = (struct WakeNode *) wn->wn_Node.mln_Succ;
    }
    ReleaseSemaphore(&nb->WakeLock);

    ObtainSemaphore(&nb->RDPCLSemaphore);
    ObtainSemaphore(&nb->ANMPELSemaphore);
    e = (struct Entity *) gb->ANMPEntityList.mlh_Head;
    while (e->entity_Port.mp_Node.ln_Succ)
    {
        ObtainSemaphore(&e->entity_linksema);
        l = (struct Entity *) e->entity_linklist.mlh_Head;

        while(l->entity_Port.mp_Node.ln_Succ)
        {
            if (l->entity_Flags & ENTF_SERVERLINK)
            {
                /* if the ENTF_KILLQUIET flag is set, entity_MaxUnused defines the maximum
                 * number of seconds a link Entity is allowed to hang around without any
                 * traffic occuring over it.  I recommend something in the range of
                 * portions of hours -- like 30 minutes.  30*60=1800 seconds.
                 * If their links fail, they must reconnect w/ FindEntity().
                 */
                if (e->entity_Flags & ENTF_KILLQUIET)
                {
                    struct Entity *l2;
                    l2 = (struct Entity *) l->entity_Port.mp_Node.ln_Succ;
                    l->entity_Unused++;
                    if (l->entity_Unused > e->entity_MaxUnused)
                    {
                        struct RDPConnection *tmpconn;
                        l->entity_Unused=0;
                        tmpconn = l->entity_connection;
                        if (tmpconn)
                        {
                            tmpconn->conn_State = STATE_CLOSE;
                            /* Send a reset */
                            SendFlags(tmpconn,RDPF_RST|RDPF_ACK);
                            if ((*tmpconn->conn_Status)(tmpconn))
                            {
                                l = (struct Entity *) l2->entity_Port.mp_Node.ln_Pred;
                                continue;
                            }
                        }
                    }
                }
            }
            if (l->entity_Flags & ENTF_LINK)
            {
                struct Transaction *t;
                ObtainSemaphore(&l->entity_transsema);
                t = (struct Transaction *) l->entity_translist.mlh_Head;
                while (t->trans_Msg.mn_Node.ln_Succ)
                {
                    if ((t->trans_Type == TYPE_REQUEST) && (t->trans_Timeout))
                    {
                        t->trans_Timeout--;
                        if (!t->trans_Timeout)
                        {
                            register struct Transaction *tt;
                            tt = (struct Transaction *) t->trans_Msg.mn_Node.ln_Pred;
                            Remove(&t->trans_Msg.mn_Node);
                            t->trans_Error = ENVOYERR_TIMEOUT;
                            ReturnTransaction(t);
                            t = tt;
                        }
                    }
                    t = (struct Transaction *) t->trans_Msg.mn_Node.ln_Succ;
                }
                ReleaseSemaphore(&l->entity_transsema);
            }
            l = (struct Entity *) l->entity_Port.mp_Node.ln_Succ;
        }
        ReleaseSemaphore(&e->entity_linksema);
        e = (struct Entity *) e->entity_Port.mp_Node.ln_Succ;
    }
    ReleaseSemaphore(&nb->ANMPELSemaphore);
    ReleaseSemaphore(&nb->RDPCLSemaphore);
}

/*------------------------------------------------------------------------*/

struct Node *FindNameI(struct List *searchlist, STRPTR target)
{
    struct Node *currnode;
    currnode = searchlist->lh_Head;
    while(currnode->ln_Succ)
    {
        if (currnode->ln_Name)
            if (!Stricmp(currnode->ln_Name,target))
                return(currnode);
        currnode = currnode->ln_Succ;
    }
    return(0L);
}


/****** nipc.library/PingEntity ******************************************
*
*   NAME
*     PingEntity -- Calculate the round-trip time between two Entities
*
*   SYNOPSIS
*     elapsedtime = PingEntity(entity, limit)
*       D0                       A0     D0
*
*     ULONG PingEntity(struct Entity *, ULONG);
*
*   FUNCTION
*
*
*   INPUTS
*     entity    - A magic cookie, identifying an Entity.
*     limit     - Maximum number of microseconds to wait for a response.
*
*   RESULT
*     elapsedtime -
*                 Total number of microseconds elapsed between attempting
*                 to query the Entity, and a response returning.  If -1L,
*                 no response was received in the given timeout interval.
*                 Local Entities will return 0 for elapsed time.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/
ULONG __asm PingEntity(register __a0 struct Entity *re, register __d0 ULONG limit)
{

    struct MsgPort *tmrport;
    register struct NBase *nb = gb;
    ULONG retval=-1L;


    if (re->entity_Flags & ENTF_LINK)
    {
        tmrport = (struct MsgPort *) CreateMsgPort();
        if (tmrport)
        {
            struct PingData *pd;

            pd = (struct PingData *) AllocMem(sizeof(struct PingData),MEMF_CLEAR|MEMF_PUBLIC);
            if (pd)
            {
                struct timerequest *atime;

                pd->pd_SigTask = FindTask(0L);
                pd->pd_SigBit = tmrport->mp_SigBit;

                atime = (struct timerequest *) CreateIORequest(tmrport,sizeof(struct timerequest));
                if (atime)
                {
                    if (!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *) atime,0))
                    {
                        struct PingRequest pr;
                        ULONG freq;

                        atime->tr_node.io_Command = TR_ADDREQUEST;
                        atime->tr_node.io_Flags = 0;
                        atime->tr_time.tv_secs = 0;
                        atime->tr_time.tv_micro = limit;
                        SendIO((struct IORequest *) atime);

                        freq = ReadEClock((struct EClockVal *)&(pd->pd_TimeStamp));
                        pr.pingreq_PacketType = PACKET_PINGREQUEST;
                        pr.pingreq_PingData = pd;
                        pd->pd_link.ln_Type = 0;
                        ObtainSemaphore(&nb->PingListSemaphore);
                        AddTail((struct List *)&(nb->PingList),(struct Node *)pd);
                        ReleaseSemaphore(&nb->PingListSemaphore);
                        rdp_output_lame(re->entity_connection, (UBYTE *) &pr, sizeof(struct PingRequest));
                        while (TRUE)
                        {
                            if (GetMsg(tmrport))
                            {
                                /* We've timed out! */
                                break;
                            }
                            if (pd->pd_link.ln_Type)
                            {
                                /* Cool!  We've received a response ! */
                                /* pd_TimeStamp now has the elapsed time (in ticks) in it ... */
                                Mul64by16((ULONG *)&pd->pd_TimeStamp,1000);
                                Mul64by16((ULONG *)&pd->pd_TimeStamp,1000);
                                retval = Div64by32(pd->pd_TimeStamp[0],pd->pd_TimeStamp[1],freq);
                                AbortIO((struct IORequest *)atime);
                                WaitIO((struct IORequest *)atime);
                                break;
                                /* return # of microseconds elapsed */
                            }
                            Wait(1 << pd->pd_SigBit);
                        }
                        ObtainSemaphore(&nb->PingListSemaphore);
                        Remove((struct Node *)pd);
                        ReleaseSemaphore(&nb->PingListSemaphore);
                        CloseDevice((struct IORequest *)atime);
                    }
                    DeleteIORequest((struct IORequest *)atime);
                }
                FreeMem(pd,sizeof(struct PingData));
            }
            DeleteMsgPort(tmrport);
        }
    }
    else
        retval = 0L;
    return(retval);

}

void RespondToPing(struct RDPConnection *c, struct PingRequest *pr)
{

    pr->pingreq_PacketType = PACKET_PINGRESPONSE;
    rdp_output_lame(c, (UBYTE *) pr, sizeof(struct PingRequest));
}

void GotResponse(struct PingResponse *pr)
{

    register struct NBase *nb = gb;
    struct PingData *pd;

    /* First, find out if this matches with anything in the list */

    ObtainSemaphore(&nb->PingListSemaphore);
    pd = (struct PingData *) nb->PingList.mlh_Head;
    while (pd->pd_link.ln_Succ)
    {
        if (pd == pr->pingres_PingData)
            break;
        pd = (struct PingData *) pd->pd_link.ln_Succ;
    }
    if (pd->pd_link.ln_Succ)
    {
        ULONG gt[2];
        ReadEClock((struct EClockVal *)&gt[0]);
        if (gt[1] < pd->pd_TimeStamp[1])
            gt[0]--;
        gt[1] -= pd->pd_TimeStamp[1];
        gt[0] -= pd->pd_TimeStamp[0];
        pd->pd_TimeStamp[0] = gt[0];
        pd->pd_TimeStamp[1] = gt[1];
        pd->pd_link.ln_Type = 255;
        Signal(pd->pd_SigTask,(1 << pd->pd_SigBit));
    }
    ReleaseSemaphore(&nb->PingListSemaphore);

}


void ReturnTransaction(struct Transaction *t)
{

    t->trans_Type = TYPE_RESPONSE;
    if (t->trans_Msg.mn_ReplyPort)
        PutMsg(t->trans_Msg.mn_ReplyPort,&t->trans_Msg);
    else
        PutMsg(&(t->trans_SourceEntity->entity_Port),&t->trans_Msg);

}

/****** nipc.library/SetEntityAttrsA ****************************************
*
*   NAME
*     SetEntityAttrsA -- change the attributes of an Entity.  (37.106)
*     SetEntityAttrs -- varargs stub for SetEntityAttrsA().  (37.106)
*
*   SYNOPSIS
*     SetEntityAttrsA(entity, tagList)
*                       A0       A1
*
*     VOID SetEntityAttrsA(struct Entity *, struct TagItem *);
*
*     SetEntityAttrs(entity, firsttag, ...)
*
*     VOID SetEntityAttrs(struct Entity *, Tag, ...);
*
*   FUNCTION
*     Change the attributes of a specified Entity, according to
*     the attributes chosen in the tag list.  If an attribute is
*     not provided in the tag list, it's value remains unchanged.
*
*   INPUTS
*     entity    - An Entity, created by CreateEntityA()
*     tagList   - pointer to an array of tags providing optional
*                 extra parameters, or NULL.
*   TAGS
*     ENT_Public -   Set/Reset the public state of an Entity.
*                    TRUE = PUBLIC, FALSE = PRIVATE.
*
*     ENT_Release -  Refuse ownership of this Entity; deallocates
*                    any signal bits created with ENT_AllocSignal.
*                    (Not those passed in with ENT_Signal!)
*                    Use in conjunction with ENT_Inherit to pass an
*                    Entity between processes.
*
*     ENT_Inherit -  Declares ownership of an Entity that has
*                    previously been set with ENT_Release.
*                    If a signal bit is necessary, this call should
*                    be used with ENT_Signal or ENT_AllocSignal.
*
*     ENT_Signal  -  Set the signal bit of the Entity to the bit
*                    value specified by ti_Data.  If a bit was
*                    previously allocated by ENT_AllocSignal, it
*                    will be freed.
*
*     ENT_AllocSignal -
*                    Allocate a signal bit for this Entity.
*                    ti_Data points to a longword where you
*                    wish the bit number to be stored, or NULL.
*                    If a signal bit was previously allocated
*                    by ENT_AllocSignal, it will be freed,
*                    regardless of the success/failure of this
*                    attribute modification.
*
*     ENT_TimeoutLinks -
*                    Defines that any link entities connected
*                    to this Entity should be automatically
*                    removed after ti_Data seconds of inactivity.
*                    0 seconds is defined as meaning "infinite"
*                    timeouts, which is the default on
*                    Entity creation.
*
*   NOTES
*     Attributes not included in the above taglist are unchangable.
*     This function exists only in revision 37.106 and later.
*
*   SEE ALSO
*     GetEntityAttrsA()
*
******************************************************************************
*
*/


VOID __asm SetEntityAttrsA(register __a0 struct Entity *e, register __a1 struct TagItem *taglist)
{
    struct TagItem *tstate;
    struct TagItem *tag;

    if (taglist)
    {
        tstate = taglist;
        while (tag = (struct TagItem *) NextTagItem(&tstate))
        {
            switch (tag->ti_Tag)
            {
                case ENT_Public:
                    if (tag->ti_Data)
                        e->entity_Flags |= ENTF_PUBLIC;
                    else
                        e->entity_Flags &= ~ENTF_PUBLIC;
                    break;
                case ENT_AllocSignal:
                {
                    ULONG bitnum;
                    /* Free old one */
                    e->entity_Port.mp_Flags = PA_IGNORE;
                    if (e->entity_Flags & ENTF_ALLOCSIGNAL)
                        FreeSignal(e->entity_Port.mp_SigBit);

                    bitnum = AllocSignal(-1L);
                    if (bitnum != -1)
                    {
                        e->entity_Port.mp_SigBit = bitnum;
                        e->entity_Port.mp_Flags = PA_SIGNAL;
                    }
                    if (tag->ti_Data)
                        *((ULONG *) tag->ti_Data) = bitnum;
                    break;
                }
                case ENT_Signal:
                {
                    e->entity_Port.mp_Flags = PA_IGNORE;
                    if (e->entity_Flags & ENTF_ALLOCSIGNAL)
                        FreeSignal(e->entity_Port.mp_SigBit);
                    e->entity_Port.mp_SigBit = tag->ti_Data;
                    e->entity_Port.mp_Flags = PA_SIGNAL;
                    break;
                }
                case ENT_Release:
                {
                    e->entity_Port.mp_Flags = PA_IGNORE;
                    if (e->entity_Flags & ENTF_ALLOCSIGNAL)
                        FreeSignal(e->entity_Port.mp_SigBit);
                    break;
                }
                case ENT_Inherit:
                {
                    e->entity_Port.mp_SigTask = FindTask(0L);
                    break;
                }
                case ENT_TimeoutLinks:
                {
                    e->entity_MaxUnused = tag->ti_Data;
                    if (tag->ti_Data)
                        e->entity_Flags |= ENTF_KILLQUIET;
                    else
                        e->entity_Flags &= ~ENTF_KILLQUIET;
                    break;
                }
            }
        }
    }
}





/****** nipc.library/GetEntityAttrsA ****************************************
*
*   NAME
*     SetEntityAttrsA -- request the attributes of an Entity.  (37.106)
*     SetEntityAttrs -- varargs stub for GetEntityAttrsA().  (37.106)
*
*   SYNOPSIS
*     numProcessed = GetEntityAttrsA(entity, tagList)
*                                      A0       A1
*
*     ULONG GetEntityAttrsA(struct Entity *, struct TagItem *);
*
*     numProcessed = GetEntityAttrs(entity, firsttag, ...)
*
*     ULONG GetEntityAttrs(struct Entity *, Tag, ...);
*
*   FUNCTION
*     Retrieve the attributes of a specified Entity, according to
*     the attributes chosen in the tag list.  For each entry in the
*     tag list, ti_Tag indentifies the attribute, and ti_Data is a
*     pointer to the long variable where you wish the result to be
*     stored.
*
*   INPUTS
*     entity    - An Entity, created by CreateEntityA()
*     tagList   - pointer to an array of tags.
*
*   TAGS
*     ENT_NameLength - Declare the size of your ENT_Name buffer.
*                      (MUST be in the same taglist as ENT_Name!)
*
*     ENT_Name  -   Find the name of an Entity; ti_Data points to an
*                   ENT_NameLength sized buffer.  Cannot be used
*                   without ENT_NameLength.
*
*     ENT_Public -  Determine whether an Entity is Public or
*                   not.  (TRUE implies PUBLIC, FALSE implies
*                   PRIVATE.)
*
*     ENT_Signal -  Determine what signal bit (if any) is attached
*                   to the given Entity.  (-1 implies "none",
*                   other signal bit number is stored in the long
*                   pointed to by ti_Data.)
*
*   RESULT
*
*     numProcessed - the number of attributes successfully filled in.
*
*   NOTES
*     Attributes not included in the above taglist are unreadable.
*     This function exists only in revision 37.106 and later.
*
*   SEE ALSO
*     SetEntityAttrsA()
*
******************************************************************************
*
*/

ULONG __asm GetEntityAttrsA(register __a0 struct Entity *e, register __a1 struct TagItem *taglist)
{
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG correct=0;

    if (taglist)
    {
        ULONG namelength=-1;
        tstate = taglist;
        while (tag = (struct TagItem *) NextTagItem(&tstate))
        {
            switch (tag->ti_Tag)
            {
                case ENT_NameLength:
                    namelength = (ULONG) tag->ti_Data;
                    correct++;
                    break;
                case ENT_Name:
                    if (namelength+1)
                    {
                        stccpy((char *) tag->ti_Data,e->entity_Port.mp_Node.ln_Name,namelength);
                        correct++;
                    }
                    break;
                case ENT_Public:
                    if (e->entity_Flags & ENTF_PUBLIC)
                        *((ULONG *) tag->ti_Data) = TRUE;
                    else
                        *((ULONG *) tag->ti_Data) = FALSE;
                    correct++;
                    break;
                case ENT_Signal:
                    if (e->entity_Port.mp_Flags == PA_SIGNAL)
                        *((ULONG *) tag->ti_Data) = e->entity_Port.mp_SigBit;
                    else
                        *((ULONG *) tag->ti_Data) = -1;
                    correct++;
                    break;
            }
        }
    }
    return(correct);
}


