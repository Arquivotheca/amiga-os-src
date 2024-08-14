/**************************************************************************
**
** amp.c        - NIPC Library API
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: amp.c,v 1.30 92/06/08 09:45:26 kcd Exp Locker: gregm $
**
***************************************************************************/

/* Remember to delete signals on the DelayedDelete -first- */
/* WaitTransaction/AbortTransaction/AllocTransaction should be fixed to allow "Aborting" unsent transactions */
/* Try flipping off MEMF_CLEAR on most things (AllocBuffEntry) */
/* FindEntity has to compare the found IP address with ALL of the sanadev structs, not just
 * the first one, to determine whether it's a local find or not.
 */
/* Possible mem leak - partial transaction fragments should be deleted from link entities
 * on link deletion */
/* Don't delete link entities on servers while any transactions are outstanding */

/* Temporary while we live with the current file formats */
/* Modify the AddSigBit routine in ReplyTransaction to reuse some general signal
 * for the nipc_supervisor task (does any of it run on the supervisor?) */

/* make sure that Realms names (Realm:Machine) work with all amp calls */

//#define OURHOSTNAME ((UBYTE *)&((struct nameentry *)gb->HostList.mlh_Head)->ne_name)
#define OURHOSTNAME ((char *)&(gb->LocalHostName))

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"

#include <envoy/errors.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include "externs.h"

#define MIN(x, y) ((x) < (y) ? (x):(y))

/* This next line is here only temporarily, until I can get access to nipcinternal_protos.h */
extern ULONG CopyBroken(ULONG *st, ULONG *dt, ULONG soff, ULONG doff);


/*------------------------------------------------------------------------*/

/*
 * The exec'ish portion of nipc
 *
 * Because of the unpleasant CHAOS in what's going on with the future of Exec
 * Messages, I consider everything in this file to be entirely TEMPORARY.
 * Who knows how this stuff is going to end up?
 *
 * Remember to verify all routines that return a BOOL - it'd better be a ULONG
 * and not a UWORD. if so, fix them.
 */

/*------------------------------------------------------------------------*/

BOOL InitANMP()
{
    register struct NBase *nb = gb;
    BOOL init;
    NewList((struct List *) & gb->ANMPEntityList);
    NewList((struct List *) & gb->HostList);
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
            if (le->entity_connection)
                CloseConnection(le->entity_connection);
            tmpe = le;
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
        trans->trans_Timeout += (dest_entity->entity_connection->conn_TimeStamp[0]/1024)+1;

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
            trans->trans_Type = TYPE_RESPONSE;
            PutMsg(&trans->trans_SourceEntity->entity_Port, &trans->trans_Msg);
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
            onepacket = TRUE; /* have sent at least one packet */
            thispack = MIN(packsize, datalimit) + sizeof(struct RequestPacket);

            rq = (struct RequestPacket *) AllocMem(thispack, MEMF_PUBLIC | MEMF_CLEAR);
            if (rq)
            {
                if (trans->trans_RequestData == trans->trans_ResponseData)
                    rq->reqpack_Flags |= REQFLAG_INPLACE;
                rq->reqpack_Command = trans->trans_Command;
                rq->reqpack_Sequence = trans->trans_Sequence;
                rq->reqpack_ResponseDataSize = trans->trans_RespDataLength;
                rq->reqpack_DataLength = thispack - sizeof(struct RequestPacket);
                dataarea = (UBYTE *) (((ULONG) rq) + (ULONG) sizeof(struct RequestPacket));
                if (trans->trans_Flags & TRANSF_REQUESTTABLE)
                {
                    ULONG tx[4];
                    tx[0] = (ULONG) dataarea;
                    tx[1] = (ULONG) rq->reqpack_DataLength;
                    tx[2] = 0L;
                    tx[3] = 0L;
                    CopyBroken((ULONG *)trans->trans_RequestData,(ULONG *)&tx,offset,0);
                }
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
                            struct Entity *src_ent;
                            FreeMem(rq, thispack);
                            src_ent = trans->trans_SourceEntity;
                            Remove((struct Node *) trans);      /* Get it off the local list */
                            trans->trans_Error = ENVOYERR_CANTDELIVER;
                            trans->trans_Type = TYPE_RESPONSE;
                            PutMsg(&src_ent->entity_Port, &trans->trans_Msg);
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
                    rdp_output(dest_entity->entity_connection, (UBYTE *) rq, thispack);
                }
                else
                {
                    struct Entity *src_ent;
                    FreeMem(rq, thispack);
                    src_ent = trans->trans_SourceEntity;
                    Remove((struct Node *) trans);      /* Get it off the local list */
                    trans->trans_Error = ENVOYERR_CANTDELIVER;
                    trans->trans_Type = TYPE_RESPONSE;
                    PutMsg(&src_ent->entity_Port, &trans->trans_Msg);
                    ReleaseSemaphore(&dest_entity->entity_transsema);
                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                    return (FALSE);
                }
                FreeMem(rq, thispack);
            }
            else
            {
                struct Entity *src_ent;
                src_ent = trans->trans_SourceEntity;
                Remove((struct Node *) trans);
                trans->trans_Error = ENVOYERR_CANTDELIVER;
                trans->trans_Type = TYPE_RESPONSE;
                PutMsg(&src_ent->entity_Port, &trans->trans_Msg);
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
        PutMsg(&dest_entity->entity_Port, &trans->trans_Msg);
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

void __asm AbortTransaction(register __a0 struct Transaction * trans)
{

    struct Entity *src;
    struct Entity *dest;

    /* If the transaction type isn't marked as a request, don't do anything. */

    src = trans->trans_SourceEntity;
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
        if (trans->trans_Type == TYPE_RESPONSE)
        {
            Permit();
            return;
        }
        Remove((struct Node *) trans);
        Permit();
    }

    trans->trans_Error = ENVOYERR_ABORTED;
    trans->trans_Type = TYPE_RESPONSE;
    PutMsg(&src->entity_Port, &trans->trans_Msg);

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
*                 (default: signal allocated and not returned)
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
    TAllocSignal = TRUE;
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

    eptr = (struct Entity *) entin;

    if (!entin)
        return;                 /* Some moron has passed a NULL in as the entity */

    if (eptr->entity_LinkCount)
    {
        if (eptr->entity_Port.mp_SigTask == FindTask(0L))
            if (eptr->entity_Flags & ENTF_ALLOCSIGNAL)          /* If we can, get rid of the signal bit on the context of the true owner of this entity */
            {
                Forbid();
                eptr->entity_Flags &= !ENTF_ALLOCSIGNAL;
                FreeSignal(eptr->entity_Port.mp_SigBit);
                eptr->entity_Port.mp_Flags = PA_IGNORE;
                Permit();
            }
        eptr->entity_Flags |= ENTF_DELAYEDDELETE;
        return;
    }

    /* Get it off any system lists */
    //if (eptr->entity_Flags & ENTF_PUBLIC)
    if (TRUE)
    {
        ObtainSemaphore(&nb->ANMPELSemaphore);
        Remove((struct Node *) eptr);
        ReleaseSemaphore(&nb->ANMPELSemaphore);
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
//                Forbid();         /* No protection necessary.  We have the RDPCL semaphore locked. */
                oldc = le->entity_connection;
                le->entity_connection = 0L;
//                Permit();
                CloseConnection(oldc);
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

    src = (struct Entity *) src_entity;


    /* If they ask for specific host, rather than NULL for the local one */
    if (hostname)
    {
        struct sanadev *sd;
        dest = ResolveName(hostname);
        if (!dest)
        {
            if (detailerror)
                *detailerror = ENVOYERR_UNKNOWNHOST;
            return ((void *) 0L);
        }

        /* See if that IP address corresponds to any of our devices */
        if (!IsListEmpty((struct List *) & gb->DeviceList))
        {
            sd = (struct sanadev *) gb->DeviceList.lh_Head;
            while (sd->sanadev_Node.mln_Succ)
            {
                if (sd->sanadev_IPAddress == dest)
                {
                    hostname = (UBYTE *) 0L;    /* Yes it does.  Make this a
                                                 * LOCAL transaction */
                    break;
                }
                sd = (struct sanadev *) sd->sanadev_Node.mln_Succ;
            }
        }
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
                *detailerror = ENVOYERR_NORESOLVER;
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

        oldstate = 0;
        while (askc->conn_State <= STATE_OPEN)
        {
            ULONG ret_addr;
            struct NameRequest lnr;
            if ((askc->conn_State == STATE_OPEN) && (oldstate != STATE_OPEN))
            {
                strncpy((UBYTE *) & lnr.nr_Name, portname, 80);
                if (src->entity_Port.mp_Node.ln_Name)
                    strncpy((UBYTE *) & lnr.nr_FromName, src->entity_Port.mp_Node.ln_Name, 80);
                else
                    strncpy((UBYTE *) & lnr.nr_FromName,"UNNAMED ENTITY",80);
                strncpy((UBYTE *) & lnr.nr_FromHost, OURHOSTNAME, 80);
                rdp_output(askc, (UBYTE *) & lnr, sizeof(struct NameRequest));
            }

            oldstate = askc->conn_State;

            ObtainSemaphore(&askc->conn_InSema);
            if (!IsListEmpty((struct List *) & askc->conn_InList))
            {
                struct Buffer *bf;
                struct BuffEntry *be;
                UWORD *addr;
                bf = (struct Buffer *) RemHead((struct List *) & askc->conn_InList);
                addr = (UWORD *) BuffPointer(bf, 0, &be);
                ret_addr = *addr;
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
                        linkent->entity_linkname = (UBYTE *) AllocMem(strlen(hostname) + 1,MEMF_PUBLIC);
                        if (linkent->entity_linkname)                  /* Label this link entity with the foreign host name */
                            strcpy(linkent->entity_linkname,hostname);
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
                            Permit();
                            ObtainSemaphore(&src->entity_linksema);
                            Remove((struct Node *) linkent);
                            ReleaseSemaphore(&src->entity_linksema);
                            if (linkent->entity_linkname)
                                FreeMem(linkent->entity_linkname,strlen(linkent->entity_linkname)+1);
                            if (linkent->entity_Port.mp_Node.ln_Name)
                                FreeMem(linkent->entity_Port.mp_Node.ln_Name, strlen(linkent->entity_Port.mp_Node.ln_Name) + 1);
                            FreeMem(linkent, sizeof(struct Entity));
                            return(0L);
                        }
                        Permit();
                        WaitConnect(linkcon);
                        DelSigBit(linkcon);

                        if (linkcon->conn_State != STATE_OPEN)
                        {

                            CloseConnection(linkcon);
                            ObtainSemaphore(&src->entity_linksema);
                            Remove((struct Node *) linkent);
                            ReleaseSemaphore(&src->entity_linksema);
                            if (linkent->entity_linkname)
                                FreeMem(linkent->entity_linkname,strlen(linkent->entity_linkname)+1);
                            if (linkent->entity_Port.mp_Node.ln_Name)
                                FreeMem(linkent->entity_Port.mp_Node.ln_Name, strlen(linkent->entity_Port.mp_Node.ln_Name) + 1);
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
//                    ReleaseSemaphore(&askc->conn_InSema);
                }
                return ((void *) ret_addr);
            }
            ReleaseSemaphore(&askc->conn_InSema);
            Sleep(askc);
        }
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
        ent->entity_connection = 0L;
        CloseConnection(oldc);
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

    BOOL AllocReq, AllocResp;
    ULONG ReqBuffSize, RespBuffSize;
    struct TagItem *tag;
    struct TagItem *tstate;
    struct Transaction *newtrans;

    AllocReq = AllocResp = FALSE;

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
            }
        }
    }

    if(newtrans = (struct Transaction *) AllocMem(sizeof(struct Transaction), MEMF_CLEAR | MEMF_PUBLIC))
    {
        if (AllocReq)
        {
            newtrans->trans_ReqDataLength = ReqBuffSize;
            newtrans->trans_ReqDataActual = ReqBuffSize;
            if (!(newtrans->trans_RequestData = (UBYTE *) AllocMem(ReqBuffSize, MEMF_PUBLIC)))
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
            if (!(newtrans->trans_ResponseData = (UBYTE *) AllocMem(RespBuffSize, MEMF_PUBLIC)))
            {
                if (AllocReq)
                    FreeMem(newtrans->trans_RequestData, ReqBuffSize);
                FreeMem(newtrans, sizeof(struct Transaction));
                return ((struct Transaction *) 0L);
            }
            newtrans->trans_Flags |= TRANSF_RESPBUFFERALLOC;
        }
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
        FreeMem(oldtrans->trans_RequestData, oldtrans->trans_ReqDataLength);
    if (oldtrans->trans_Flags & TRANSF_RESPBUFFERALLOC)
        FreeMem(oldtrans->trans_ResponseData, oldtrans->trans_RespDataLength);

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
            ULONG fullsize, fulllimit, datalimit, offset;
            UBYTE *dataarea;
            struct Entity *src;
            src = trans->trans_SourceEntity;

            offset = 0;
            fullsize = trans->trans_RespDataActual;     /* size of data to send */
            if (!fullsize)      /* If no response data needed */
            {
                rp = (struct ResponsePacket *) AllocMem(sizeof(struct ResponsePacket), MEMF_PUBLIC | MEMF_CLEAR);
                if (!rp)
                    return;
                rp->respack_Error = trans->trans_Error;
                rp->respack_NewCommand = trans->trans_Command;
                rp->respack_Sequence = trans->trans_Sequence;
                rp->respack_Flags = RESFLAG_LASTSEGMENT;
                rp->respack_DataLength = 0L;
                src = trans->trans_SourceEntity;
                ObtainSemaphore(&nb->RDPCLSemaphore);

                if (src->entity_connection)
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
                                FreeMem(rp,sizeof(struct ResponsePacket));
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
                        rdp_output(src->entity_connection, (UBYTE *) rp, sizeof(struct ResponsePacket));
                    }
                FreeMem(rp, sizeof(struct ResponsePacket));
                src->entity_LinkCount--;
                REntityStatus(src->entity_connection);
                ReleaseSemaphore(&nb->RDPCLSemaphore);
                FreeTransaction(trans);
                return;
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
                rp = (struct ResponsePacket *) AllocMem(thispack, MEMF_PUBLIC | MEMF_CLEAR);
                if (rp)
                {
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
                                    FreeMem(rp,thispack);
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
                            rdp_output(src->entity_connection, (UBYTE *) rp, thispack);
                        }
                    }
                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                    FreeMem(rp, thispack);
                }
                else
                    return;
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
            trans->trans_Type = TYPE_RESPONSE;
            PutMsg(&(trans->trans_SourceEntity->entity_Port), (struct Message *) trans);
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
    sigbit = src_ent->entity_Port.mp_SigBit;

    while (trans->trans_Type != TYPE_RESPONSE)
        Wait(1 << sigbit);

    Forbid();
    Remove(&(trans->trans_Msg.mn_Node));        /* Pull it from the MsgPort */
    Permit();

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
    struct NameReply lnr;
    struct NameRequest *namereq;
    UWORD RDPPort = 0;
    struct BuffEntry *be;

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
            if ((e->entity_Flags & ENTF_PUBLIC) && (!strcmp(e->entity_Port.mp_Node.ln_Name, (UBYTE *) & namereq->nr_Name)))
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
                le->entity_Flags |= ENTF_LINK;
                le->entity_owner = e;
                le->entity_Port.mp_Node.ln_Name = linkname;
                le->entity_linkname = hname;
                ObtainSemaphore(&e->entity_linksema);
                AddTail((struct List *) & e->entity_linklist, (struct Node *) le);
                ReleaseSemaphore(&e->entity_linksema);

                newcon = OpenPassive(-1L, &REntityInput, &REntityStatus);
                if (!newcon)
                {
                    FreeMem(hname,strlen((UBYTE *) &namereq->nr_FromHost)+1);
                    FreeMem(linkname,strlen((UBYTE *) &namereq->nr_FromName)+1);
                    ObtainSemaphore(&e->entity_linksema);
                    Remove((struct Node *) le);
                    ReleaseSemaphore(&e->entity_linksema);
                    FreeMem(le, sizeof(struct Entity));
                    break;
                }

                le->entity_connection = newcon;
                newcon->conn_linkentity = le;   /* back link */
                RDPPort = newcon->conn_OurPort;
                ReleaseSemaphore(&nb->ANMPELSemaphore);
                lnr.nr_PortNumber = RDPPort;
                if (c->conn_State == STATE_OPEN)
                    rdp_output(c, (UBYTE *) & lnr, sizeof(struct NameReply));
                FreeBuffer(buff);
                e->entity_LinkCount++;
                ReleaseSemaphore(&nb->RDPCLSemaphore);
                return (TRUE);
            }
            e = (struct Entity *) e->entity_Port.mp_Node.ln_Succ;
        }
    }
    lnr.nr_PortNumber = RDPPort;
    if (c->conn_State == STATE_OPEN)
        rdp_output(c, (UBYTE *) & lnr, sizeof(struct NameReply));
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
    Wait(1 << c->conn_WakeBit);
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
                struct Entity *returnentity;

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
                else
                {
                    if (is->trans_RespDataLength >= rp->respack_DataLength)
                    {
                        CopyFromBuffer(todata, buff, MIN(is->trans_RespDataLength, rp->respack_DataLength));
                        is->trans_Error = rp->respack_Error;
                    }
                    else
                        is->trans_Error = ENVOYERR_SMALLRESPBUFF;
                }
                is->trans_Command = rp->respack_NewCommand;
                is->trans_RespDataActual = MIN(is->trans_RespDataLength,rp->respack_DataLength);
                Forbid();
                is->trans_Type = TYPE_RESPONSE;
                Remove((struct Node *) is);
                returnentity = is->trans_SourceEntity;
                PutMsg(&returnentity->entity_Port, &is->trans_Msg);
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

    rq = (struct RequestPacket *) BuffPointer(buff, 0L, &be);

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
    if(rq->reqpack_DataLength)
            reqdata = (UBYTE *) AllocMem(rq->reqpack_DataLength, MEMF_CLEAR | MEMF_PUBLIC);

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
            FreeMem(reqdata, rq->reqpack_DataLength);
            FreeMem(newtrans, sizeof(struct Transaction));
            FreeBuffer(buff);
            return;
        }

        if (rq->reqpack_ResponseDataSize)
            newtrans->trans_Flags |= TRANSF_RESPBUFFERALLOC;
    }
    else /* Modify-in-place transaction */
    {
        resdata = reqdata;
        rq->reqpack_ResponseDataSize = rq->reqpack_DataLength;
    }

    newtrans->trans_RequestData = reqdata;
    newtrans->trans_ReqDataLength = rq->reqpack_DataLength;
    newtrans->trans_ReqDataActual = newtrans->trans_ReqDataLength;
    newtrans->trans_ResponseData = resdata;
    newtrans->trans_RespDataLength = rq->reqpack_ResponseDataSize;
    newtrans->trans_RespDataActual = newtrans->trans_RespDataLength;
    newtrans->trans_Command = rq->reqpack_Command;
    newtrans->trans_Sequence = rq->reqpack_Sequence;
    newtrans->trans_SourceEntity = c->conn_linkentity;
    newtrans->trans_Type = TYPE_REQUEST;
    c->conn_linkentity->entity_LinkCount++;
    PutMsg(&(c->conn_linkentity->entity_owner->entity_Port), &newtrans->trans_Msg);
    FreeBuffer(buff);

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
            struct Entity *src_ent;
            trn = (struct Transaction *) RemHead((struct List *) & link->entity_translist);
            trn->trans_Error = ENVOYERR_CANTDELIVER;
            src_ent = trn->trans_SourceEntity;
            trn->trans_Type = TYPE_RESPONSE;
            PutMsg(&src_ent->entity_Port, &trn->trans_Msg);
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
            /*
             * The rdp connection list is already obtainsemaphore'd when this
             * code is called
             */
            ObtainSemaphore(&nb->ANMPELSemaphore);
            le = c->conn_linkentity;
            owner = le->entity_owner;
            ObtainSemaphore(&owner->entity_linksema);
            Remove(&le->entity_Port.mp_Node);

            Forbid();
            oldc = le->entity_connection;
            le->entity_connection = 0L;
            Permit();
            if (oldc)
            {
                CloseConnection(oldc);
                returnvalue=TRUE;
            }

            if (le->entity_LinkCount)                           /* If there're outstanding xactions, */
            {                                                   /* don't free this thing now. */
                ReleaseSemaphore(&owner->entity_linksema);
                ReleaseSemaphore(&nb->ANMPELSemaphore);
                le->entity_Flags |= ENTF_DELAYEDDELETE;         /* Delete me asap */
                return(returnvalue);
            }

            if (le->entity_linkname)
                FreeMem(le->entity_linkname, strlen(le->entity_linkname) + 1);

            if (le->entity_Port.mp_Node.ln_Name)
                FreeMem(le->entity_Port.mp_Node.ln_Name, strlen(le->entity_Port.mp_Node.ln_Name) + 1);

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
        strncpy(newstr, ((struct Entity *) entity)->entity_Port.mp_Node.ln_Name, spacefree);
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

    if (ent->entity_Flags & ENTF_LINK)
    {
        if (ent->entity_linkname)
            strncpy(str,ent->entity_linkname,availsize);
        else
            return(FALSE);
    }
    else
    {
        strncpy(str,OURHOSTNAME,availsize);
    }
    return(TRUE);

}

/*------------------------------------------------------------------------*/

void ANMPTimeout()
{
    register struct NBase *nb = gb;
    struct Entity *e,*l;
    ObtainSemaphore(&nb->ANMPELSemaphore);
    e = (struct Entity *) gb->ANMPEntityList.mlh_Head;
    while (e->entity_Port.mp_Node.ln_Succ)
    {
        ObtainSemaphore(&e->entity_linksema);
        l = (struct Entity *) e->entity_linklist.mlh_Head;

        while(l->entity_Port.mp_Node.ln_Succ)
        {
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
                            t->trans_Type = TYPE_RESPONSE;
                            PutMsg(&(t->trans_SourceEntity->entity_Port),
                                     &t->trans_Msg);
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
}

/*------------------------------------------------------------------------*/

struct Node *FindNameI(struct List *searchlist, STRPTR target)
{
    struct Node *currnode;
    currnode = searchlist->lh_Head;
    while(currnode->ln_Succ)
    {
        if (!Stricmp(currnode->ln_Name,target))
            return(currnode);
        currnode = currnode->ln_Succ;
    }
    return(0L);
}
