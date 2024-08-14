/************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                       - Talin & Joe Pearce                            *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* node.c      - MidiNode management functions
*
************************************************************************/

#include <exec/memory.h>
#include <string.h>
#include <stddef.h>

#include "camdlib.h"

#define Static static

#define MinMsgQueueSize   32  /* minimum Msg Queue size (MidiMsg's) (includes padding) */
#define MinSysExQueueSize 64  /* minimum Sys/Ex Queue size (bytes) (includes padding) */

#define IsMsgQueue(mi)   ((mi)->MsgQueue)         /* is this a receiver? */
#define IsSysExQueue(mi) ((mi)->SysExQueue)       /* does it have a sys/ex Queue */

    /* external (msg.asm) */
extern void __asm StripSysEx (register __a1 struct XMidiNode *);
extern void notify_external(void);


    /* private */
static void AddMidi (struct XMidiNode *);
static void RemMidi (struct XMidiNode *);


/*========================================================================*/
/*                      PUBLIC INTERFACE FUNCTIONS                        */
/*========================================================================*/

/****** camd.library/SetMidiAttrs ******************************************
*
*   NAME
*       SetMidiAttrs -- Set the attributes of a MidiNode
*
*   SYNOPSIS
*       BOOL SetMidiAttrsA (struct MidiNode *mi, struct TagItem *tags)
*                                       a0          a1
*
*       BOOL SetMidiAttrs (struct MidiNode *mi, ...)
*
*   FUNCTION
*       Sets the attributes of a MidiNode, using a Tag List.
*
*       The first form of the function expects a tag array pointer or NULL.
*       The second form permits the tag items to exist on the caller's
*       stack.  In both cases, the final tag item must be TAG_END.
*
*   INPUTS
*       mi - a pointer to the MidiNode
*
*       TagList   - optional pointer to tag array.  May be NULL.  For
*                   OS v1.3, there are restrictions on the tag array
*                   contents.  See NOTE below.
*
*   TAGS
*         MIDI_Name         STRPTR - ti_Data points to the new name of
*                               the node (generally the Application name)
*
*         MIDI_SignalTask   struct Task * - the task to be signaled whenever
*                   a MidiMsg or Participant change occurs. This is set
*                   by CreateMidi to the current task as a default
*
*         MIDI_RecvHook     struct Hook * - this hook will be called when
*                   new MidiMsgs arrive, if the buffer was empty. If the buffer
*                   was not yet empty, then it is simply added onto the end.
*
*         MIDI_PartHook     struct Hook * - this hook will be called whenever
*                   any of the clusters that this node is linked to either
*                   adds or removes a member.
*
*         MIDI_RecvSignal   BYTE - the signal to send whenever an incoming
*                   MidiMsg arrives in the buffer, or -1 to send no signal
*
*         MIDI_PartSignal   BYTE - the signal to send whenever a cluster
*                   to which this node is linked has a participant change,
*                   or -1 to send no signal
*
*         MIDI_MsgQueue     ULONG - ti_Data specifies the size of the
*                   MsgQueue for this MidiNode. An additional pad MidiMsg
*                   is allocated for overflow protection. It can also be
*                   set to zero to indicate that no buffer should be allocated
*                   (A send-only MidiNode)
*
*         MIDI_SysExSize   ULONG - ti_Data specifies the size of the SysExQueue
*                   in bytes. Like the MsgQueue, an additional byte is
*                   allocated to allow for overflow protection.
*
*         MIDI_TimeStamp   ULONG * - if non-NULL, ti_Data is a pointer to
*                   a longword which is to be used as the source for time
*                   stamps of incoming MidiMsgs for this MidiNode. It is
*                   assumed that the longword pointed to will be updated by
*                   some other mechanism -- for example, the longword could
*                   point to one of the fields in a PlayerInfo structure.
*
*         MIDI_ErrFilter   UWORD - specified the ErrFilter for this MidiNode
*
*         MIDI_ClientType  UWORD - specified the Client Type for this MidiNode
*                   See camd.h for more detail.
*
*         MIDI_Image       struct Image * - pointer to an Intuition Image
*                   structure representing a glyph or icon that is symbolic
*                   if this application. Will be used for a future "patch
*                   bay" application. It is suggested that images be
*                   approximately 32 wide x 32 high, for consistency.
*
*   RESULTS
*       TRUE if all changes were made successfully or FALSE on failure.
*       When FALSE is returned, an error code will be returned if a TagItem
*	with an ti_Tag of MIDI_ErrorCode was provided. The CME_ error code
*	will be put in the ti_Data field.
*
*   NOTE
*       Under 1.3 only a restricted tag array may be passed into this
*       funciton. Specifically, the only special tag values that are
*       supported are TAG_END (or TAG_DONE) and TAG_IGNORE.  All others
*       (e.g. TAG_SKIP, TAG_MORE) are treated as TAG_END.  The full range of
*       TAG list operations are permitted under 2.0.
*
*       This function MUST be called by a Process (not a Task).  It makes a
*       few DOS calls.
*
*       Don't call WaitMidi() with a MidiNode that has no ReceiveSignal.
*
*   EXAMPLES
*       mi = SetMidiAttrs (mi,
*               MIDI_MsgQueue, 2048, MIDI_SysExSize, 10000L, TAG_END);
*
*       Modifies a MidiNode to have space to receive 2048 MidiMsg's and 10000
*       bytes of Sys/Ex data.  Note the 'L' on the end of 10000.  This
*       forces correct alignment of the tag items on the stack for 16 bit
*       integer compilation.
*
*   SEE ALSO
*       CreateMidi(), DeleteMidi()
*       2.0 tag docs.
****************************************************************************
*
*/

BOOL __saveds __asm LIBSetMidiAttrsA (
register __a0 struct XMidiNode *mi,
register __a1 struct TagItem *taglist)
{
    ULONG       err = 0;
    LONG        msgsize = mi->Public.mi_MsgQueueSize,
                sysexsize = mi->Public.mi_SysExQueueSize;
    struct TagItem *tags = taglist;
    struct TagItem *tlist = tags;

        /* parse all of the tags and set the appropriate attributes */

	/* note: if setting of attrs is made a subroutine accessed by
	 * SetMidiAttrs, CreateMidi and DeleteMidi, then these locks
	 * are only needed by the SetMidiAttrs glue.
	 */

    LockLinks();
    LockMidiList();

        /* !!! this doesn't seem to handle failures very completely */
        /* !!! make this whole function look nicer! */

    while (tags = next_tag(&tlist)) {
        ULONG tdata = tags->ti_Data;

        switch (tags->ti_Tag) {
            case MIDI_Name:         mi->Public.mi_Node.ln_Name      = (char *)tdata; break;
            case MIDI_RecvHook:     mi->Public.mi_ReceiveHook       = (struct Hook *)tdata; break;
            case MIDI_PartHook:     mi->Public.mi_ParticipantHook   = (struct Hook *)tdata; break;
            case MIDI_SignalTask:   mi->Public.mi_SigTask           = (struct Task *)tdata; break;
            case MIDI_RecvSignal:   mi->Public.mi_ReceiveSigBit     = tdata; break;
            case MIDI_PartSignal:   mi->Public.mi_ParticipantSigBit = tdata; break;
            case MIDI_ErrFilter:    mi->Public.mi_ErrFilter         = tdata; break;
            case MIDI_ClientType:   mi->Public.mi_ClientType        = tdata; break;
            case MIDI_Image:        mi->Public.mi_Image             = (struct Image *)tdata; break;
            case MIDI_TimeStamp:    mi->Public.mi_TimeStamp         = (ULONG *)tdata; break;

            case MIDI_MsgQueue:
                    msgsize = tdata;
                    if (msgsize == 0) ;
                    else if (msgsize < MinMsgQueueSize) msgsize = MinMsgQueueSize;
                    else msgsize++;         /* pad requested size for overflow */
                    break;

            case MIDI_SysExSize:
                    sysexsize = tdata;
                    if (sysexsize == 0) ;
                    else if (sysexsize < MinSysExQueueSize) sysexsize = MinSysExQueueSize;
                    else sysexsize++;       /* pad requested size for overflow */
                    break;
        }
    }

        /* make sure we don't get any errors we aren't looking for */

    mi->ErrFlags &= mi->Public.mi_ErrFilter;

        /* Now, check to see if any of the buffer sizes changed. */

    if ((msgsize != mi->Public.mi_MsgQueueSize) ||
        (mi->MsgQueue != NULL) != (msgsize != NULL) )
    {
        MidiMsg *new_queue = NULL;

            /* First, attempt to allocate the new buffer before we lock
                the list...
            */

        if (msgsize)
        {
                new_queue = AllocMem (msgsize * sizeof (MidiMsg), MEMF_PUBLIC|MEMF_CLEAR);
                if (!new_queue) err = CME_NoMem;
        }

        if (!err)
        {
            if (mi->MsgQueue)
            {   /* REM: if new queue exists, copy old data over... */
                /* REM: Even more efficient would be to defer FreeMem until
                        after we unlock */

                FreeMem (mi->MsgQueue, mi->Public.mi_MsgQueueSize * sizeof (MidiMsg));
            }

            if (new_queue)
            {
                mi->MsgQueueHead = mi->MsgQueueTail = new_queue;
                /* this computation makes use of C pointer math... remember to
                   multiply by sizeof (MidiMsg) if this gets turned into assembly */
                mi->MsgQueueEnd = new_queue + msgsize;

            }

            mi->MsgQueue = new_queue;
            mi->Public.mi_MsgQueueSize = msgsize;
        }
    }

	/* If there is no MsgQueue, then a SysExQueue is pretty useless...
	 * Therefore, set sysezsize to 0.
	 */

    if (mi->MsgQueue == NULL) sysexsize = 0;

    if ((sysexsize != mi->Public.mi_SysExQueueSize) ||
        (mi->SysExQueue != NULL) != (sysexsize != NULL) )
    {
        UBYTE *new_queue = NULL;

            /* First, attempt to allocate the new buffer before we lock
                the list...
            */

        if (sysexsize)
        {   new_queue = AllocMem (sysexsize, MEMF_PUBLIC|MEMF_CLEAR);
            if (!new_queue) err = CME_NoMem;
        }

        if (!err)
        {
                /* REM: The flag keeps application-supplied buffers from
                        being de-allocated...
                */

            if (mi->SysExQueue && mi->SysFlags & MIF_MySXBuf)
            {   /* REM: if new queue exists, copy old data over... */
                /* REM: Even more efficient would be to defer FreeMem until
                        after we unlock */

                FreeMem (mi->SysExQueue, mi->Public.mi_SysExQueueSize);
            }

            if (new_queue)
            {
                mi->SysExQueueHead = mi->SysExQueueTail = new_queue;
                mi->SysExQueueEnd = new_queue + sysexsize;

                /* REM: This flag indicates that it is not an application-
                        supplied buffer */

                mi->SysFlags |= MIF_MySXBuf;
            }

            mi->SysExQueue = new_queue;
            mi->Public.mi_SysExQueueSize = sysexsize;
        }
    }


    UnlockMidiList();

    notify_external();

    UnlockLinks();

    if (err) {
        SetErrorCode (MIDI_ErrorCode, taglist, err);
        return FALSE;
    }

    return TRUE;
}

/***************************************************************
*
*   CreateMidi
*
***************************************************************/

/****** camd.library/CreateMidi ******************************************
*
*   NAME
*       CreateMidi -- Create a MidiNode.
*
*   SYNOPSIS
*       struct MidiNode *CreateMidiA (struct TagItem *TagsList)
*                                               a0
*
*       struct MidiNode *CreateMidi (Tag tag1, ...)
*
*   FUNCTION
*       Creates a MidiNode structure with the desired attributes.
*
*       The first form of the function expects a tag array pointer or NULL.
*       The second form permits the tag items to exist on the caller's
*       stack.  In both cases, the final tag item must be TAG_END.
*
*   INPUTS
*       TagList   - optional pointer to tag array.  May be NULL.  For
*                   OS v1.3, there are restrictions on the tag array
*                   contents.  See NOTE below.
*
*   TAGS
*	    See SetMidiAttrsA
*
*   RESULTS
*       A pointer to a MidiNode structure on success or NULL on failure.
*       When FALSE is returned, an error code will be returned if a TagItem
*	with an ti_Tag of MIDI_ErrorCode was provided. The CME_ error code
*	will be put in the ti_Data field.
*
*   NOTE
*       Under 1.3 only a restricted tag array may be passed into this
*       funciton. Specifically, the only special tag values that are
*       supported are TAG_END (or TAG_DONE) and TAG_IGNORE.  All others
*       (e.g. TAG_SKIP, TAG_MORE) are treated as TAG_END.  The full range of
*       TAG list operations are permitted under 2.0.
*
*       This function MUST be called by a Process (not a Task).  It makes a
*       few DOS calls.
*
*       Don't call WaitMidi() with a send-only MidiNode.
*
*   EXAMPLES
*       mi = CreateMidi (
*               MIDI_MsgQueue, 2048, MIDI_SysExSize, 10000L, TAG_END);
*
*       Creates a MidiNode with space to receive 2048 MidiMsg's and 10000
*       bytes of Sys/Ex data.  Note the 'L' on the end of 10000.  This
*       forces correct alignment of the tag items on the stack for 16 bit
*       integer compilation.
*
*   SEE ALSO
*       DeleteMidi(), SetMidiAttrs()
*       2.0 tag docs.
****************************************************************************
*
*/

struct XMidiNode * __saveds __asm LIBCreateMidiA (
register __a0 struct TagItem *tags)
{
    register struct XMidiNode *mi;
    int err = 0;

   /* --------------------------------------
      Allocate memory for MidiNode structure
      -------------------------------------- */

    if ( !(mi = AllocMem (sizeof *mi, MEMF_PUBLIC | MEMF_CLEAR)) )
    {
        err = CME_NoMem;
        goto clean;
    }

      /* init a few fields */
    mi->Public.mi_SigTask = FindTask(NULL);
    mi->Public.mi_ReceiveSigBit = mi->Public.mi_ParticipantSigBit = -1;
    mi->Public.mi_TimeStamp = &CamdBase->TimeLess;

    NewList((struct List *)&mi->Public.mi_InLinks);
    NewList((struct List *)&mi->Public.mi_OutLinks);

    if (!SetMidiAttrsA (mi,tags)) goto clean;


   AddMidi (mi);
   mi->SysFlags |= MIF_Ready;

   /* --------------------------
      Done initializing MidiNode
      -------------------------- */
   return mi;

   /* ----------------
      Clean exit label
      ---------------- */
clean:
   DeleteMidi (mi);
   SetErrorCode (MIDI_ErrorCode, tags, err);
   return NULL;
}

/***************************************************************
*
*   GetMidiAttrsA
*
***************************************************************/

/****** camd.library/GetMidiAttrsA ******************************************
*
*   NAME
*       GetMidiAttrsA -- Get the attributes of a MidiNode
*
*   SYNOPSIS
*       ULONG GetMidiAttrsA (struct MidiNode *mi, struct TagItem *attrs)
*                                       a0          a1
*   FUNCTION
*       Gets attributes of a MidiNode.
*
*   INPUTS
*       mi - a pointer to the MidiNode
*
*       attrs - Attributes to get, terminated with TAG_DONE. For each entry
*	    in the tag list, ti_Tag identifies the attribute, and ti_Data
*	    is a pointer to the long variable where you wish the result
*	    to be stored.
*
*   RESULTS
*	Count of attributes understood.
*
*   NOTE
*	See SetMidiAttrs() for tags understood.o
*
*   EXAMPLES
*
*   SEE ALSO
*       SetMidiAttrs()
****************************************************************************
*
*/

ULONG __saveds __asm LIBGetMidiAttrsA (
register __a0 struct XMidiNode *mi,
register __a1 struct TagItem *attrs)
{
    struct TagItem *tags = attrs;
    struct TagItem *tlist = tags;
    ULONG count = 0;

        /* parse all of the tags and set the appropriate attributes */

    LockLinks();
    LockMidiList();

    while (tags = next_tag(&tlist)) {
        ULONG tdata = tags->ti_Data;

        switch (tags->ti_Tag) {
            case MIDI_Name:
		*(char **)tdata = mi->Public.mi_Node.ln_Name;
		count++;
		break;

            case MIDI_RecvHook:
		*(struct Hook **)tdata = mi->Public.mi_ReceiveHook;
		count++;
		break;

            case MIDI_PartHook:
		*(struct Hook **)tdata = mi->Public.mi_ParticipantHook;
		count++;
		break;

            case MIDI_SignalTask:
		*(struct Task **)tdata = mi->Public.mi_SigTask;
		count++;
		break;

            case MIDI_RecvSignal:
		*(LONG *)tdata = mi->Public.mi_ReceiveSigBit;
		count++;
		break;

            case MIDI_PartSignal:
		*(LONG *)tdata = mi->Public.mi_ParticipantSigBit;
		count++;
		break;

            case MIDI_ErrFilter:
		*(LONG *)tdata = mi->Public.mi_ErrFilter;
		count++;
		break;

            case MIDI_ClientType:
		*(LONG *)tdata = mi->Public.mi_ClientType;
		count++;
		break;

            case MIDI_Image:
		*(struct Image **)tdata = mi->Public.mi_Image;
		count++;
		break;

            case MIDI_TimeStamp:
		*(ULONG **)tdata = mi->Public.mi_TimeStamp;
		count++;
		break;

            case MIDI_MsgQueue:
		*(ULONG *)tdata = mi->Public.mi_MsgQueueSize;
		count++;
		break;

            case MIDI_SysExSize:
		*(ULONG *)tdata = mi->Public.mi_SysExQueueSize;
		count++;
		break;
        }
    }

    UnlockMidiList();
    UnlockLinks();

    return count;
}

/***************************************************************
*
*   DeleteMidi
*
***************************************************************/

/****** camd.library/DeleteMidi ******************************************
*
*   NAME
*       DeleteMidi -- Delete a MidiNode.
*
*   SYNOPSIS
*       void DeleteMidi (struct MidiNode *mn)
*                               a0
*
*   FUNCTION
*       Deletes the specified MidiNode.  A sys/ex queue allocated by
*       CreateMidi() is freed.  The following actions are automatically
*       performed by this function:
*
*       A client-supplied sys/ex queue attached with SetSysExQueue() will
*       not be freed by this function.
*
*   INPUTS
*       mn - MidiNode to delete. Can be NULL.
*
*   RESULTS
*       None
*
*   SEE ALSO
*       CreateMidi(), ClearSysExQueue()
****************************************************************************
*
*/

void __saveds __asm LIBDeleteMidi (register __a0 struct XMidiNode *mi)
{
    static struct TagItem free_queue_tags[] = {
        MIDI_MsgQueue, 0,
        MIDI_SysExSize, 0,
        TAG_DONE
    };
    struct Node *next_link, *succ_link;

    if (mi) {
            /* remove whatever links there may be */

        for (next_link = (struct Node *)mi->Public.mi_InLinks.mlh_Head;
                succ_link = next_link->ln_Succ; next_link = succ_link)
        {
                RemoveMidiLink( (struct MidiLink *)
                        ((char *)next_link - offsetof(struct MidiLink,ml_OwnerNode)) );
        }

        for (next_link = (struct Node *)mi->Public.mi_OutLinks.mlh_Head;
                succ_link = next_link->ln_Succ; next_link = succ_link)
        {
                RemoveMidiLink( (struct MidiLink *)
                        ((char *)next_link - offsetof(struct MidiLink,ml_OwnerNode)) );
        }

            /* RemMidi() if AddMidi was done */
        if (mi->SysFlags & MIF_Ready) RemMidi (mi);

            /* free queues */
        SetMidiAttrsA (mi, free_queue_tags);

        FreeMem (mi, sizeof *mi);
    }
}


/***************************************************************
*
*   FlushMidi
*
***************************************************************/

/****** camd.library/FlushMidi ******************************************
*
*   NAME
*       FlushMidi -- Dispose of all pending messages.
*
*   SYNOPSIS
*       void FlushMidi (struct MidiNode *mi)
*                               a0
*
*   FUNCTION
*       Disposes of all messages waiting to be received by GetMidi() and
*       GetSysEx().  Also clears pending errors.
*
*   INPUTS
*       mi - MidiNode to flush.
*
*   RESULTS
*       None
****************************************************************************
*
*/

void __saveds __asm LIBFlushMidi (register __a0 struct XMidiNode *mi)
{
   LockMidiList();

   mi->MsgQueueHead = mi->MsgQueueTail; /* harmless if no MsgQueue */
   mi->SysExQueueHead = mi->SysExQueueTail;     /* harmless if no SysExQueue */
   mi->SysFlags &= ~MIF_InSysEx;
   mi->ErrFlags = 0;

   UnlockMidiList();
}


/***************************************************************
*
*   FindMidi
*
***************************************************************/

/****** camd.library/FindMidi ******************************************
*
*   NAME
*       FindMidi -- Find a MidiNode by name
*
*   SYNOPSIS
*       struct MidiNode *FindMidi (name)
*                                  a0
*
*   FUNCTION
*       Find a MidiNode by name. If node of that exists, the function
*       returns NULL. Midi links (CD_Linkages) must be locked when called.
*
*   INPUTS
*       name - name of MidiNode to find.
*
*   RESULTS
*       result - A MidiNode or NULL.
*
****************************************************************************
*
*/

struct MidiNode * __saveds __asm LIBFindMidi (register __a0 char *name)
{
    return (struct MidiNode *)
        FindName((struct List *)&CamdBase->MidiList,name);
}


/***************************************************************
*
*   NextMidi
*
***************************************************************/

/****** camd.library/NextMidi ******************************************
*
*   NAME
*       NextMidi -- Get next MidiNode
*
*   SYNOPSIS
*       struct MidiNode *NextMidi (struct MidiNode *last)
*                                  a0
*
*   FUNCTION
*       Returns the next MidiNode on CAMD MidiNode list. If last is NULL,
*	returns the first MidiNode. Returns NULL if no more MidiNodes.
*	Midi links (CD_Linkages) must be locked when called.
*
*   INPUTS
*       last - previous MidiNode or NULL to get first MidiNode
*
*   RESULTS
*       next - next MidiNode or NULL
*
****************************************************************************
*
*/

struct MidiNode * __saveds __asm LIBNextMidi (
register __a0 struct MidiNode *last
)
{   struct MidiNode *mi;

    mi = (last == NULL ?
	(struct MidiNode *)CamdBase->MidiList.mlh_Head :
	(struct MidiNode *)last->mi_Node.ln_Succ);

    if (mi->mi_Node.ln_Succ == NULL) return NULL;
    return mi;
}


/*========================================================================*/
/*                      PRIVATE INTERFACE FUNCTIONS                       */
/*========================================================================*/


/* !!! could fold these functions into their clients */

/***************************************************************
*
*   AddMidi
*
*   FUNCTION
*       Adds the specified MidiNode structure the
*       MidiList.  MICount is incremented.  If it advances from
*       0 to 1, the Alarm and Unit subsystems are activated.
*
*   INPUTS
*       mi - pointer to initialized MidiNode to add.
*
*   RESULTS
*       None.
*
***************************************************************/

Static
void AddMidi (struct XMidiNode *mi)
{
    /* if (IsMsgQueue(mi)) !!! old test */ {
	LockLinks();
        LockMidiList();
        AddTail ((struct List *)&CamdBase->MidiList, (struct Node *)mi);
        UnlockMidiList();
	UnlockLinks();
    }
}


/***************************************************************
*
*   RemMidi
*
*   FUNCTION
*       Removes the specified MidiNode from the
*       MidiList.  Decrements MICount.  If MICount becomes
*       0, the Alarm and Unit subsystems are shut down.
*
*   INPUTS
*       mi - pointer to initialized MidiNode to add.
*
*   RESULTS
*       None
*
***************************************************************/

Static
void RemMidi (struct XMidiNode *mi)
{
    /* if (IsMsgQueue(mi)) !!! old test */ {
	LockLinks();
        LockMidiList();
        Remove ((struct Node *)mi);     /* Remove from midi list */
        UnlockMidiList();
	UnlockLinks();
    }
}
