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
* Copyright 1992 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* link.c      - MidiLink management functions
*
************************************************************************/

#include <dos/dosextens.h>      /* for pr_Result2 in Process */
#include <exec/memory.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>

#include "camdlib.h"

static void NotifyPartChanges(struct MidiCluster *mc, struct MidiLink *initiator, ULONG action);
static struct MidiCluster *open_cluster(char *name);

extern void notify_external(void);

#if 0
static int OpenSupport(void);
static void CloseSupport(void);
#endif

#define FAIL(x) { error = (x); goto exitit; }

/***************************************************************
*
*   AddMidiLink
*
***************************************************************/

/****** camd.library/AddMidiLink ******************************************
*
*   NAME
*       AddMidiLink -- Create a MidiLink to a MidiCluster.
*
*   SYNOPSIS
*       struct MidiLink *AddMidiLinkA (struct MidiNode *mi, LONG type,
*                                               a0		d0
*		struct TagItem *TagsList)
*			a1
*
*	struct MidiLink *AddMidiLink(struct MidiNode *mi, LONG type,
*			Tag type1, ...)
*
*   FUNCTION
*       Creates a MidiLink structure and connects it to a MidiCluster.
*
*       The first form of the function expects a tag array pointer or NULL.
*       The second form permits the tag items to exist on the caller's
*       stack.  In both cases, the final tag item must be TAG_END.
*
*   INPUTS
*	mi - MidiNode that the MidiLink should communicate through.
*
*	type - Type of link, either MLTYPE_Receiver or MLTYPE_Sender.
*
*       TagList   - optional pointer to tag array.  May be NULL.  For
*                   OS v1.3, there are restrictions on the tag array
*                   contents.  See NOTE below.
*
*   TAGS
*	See SetMidiLinkAttrsA
*
*   RESULTS
*       A pointer to a MidiLink structure on success or NULL on failure.
*       When FALSE is returned, an error code will be returned if a TagItem
*	with an ti_Tag of MLINK_ErrorCode was provided. The CME_ error code
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
*   EXAMPLES
*       ml = AddMidiLink (mi,MLTYPE_Receiver,
*               MLINK_Name, "app.in", MLINK_Location,"in.0", TAG_END);
*
*       Creates a receive MidiLink named "app.in" to the MidiCluster named
*	"in.0", where all bytes are recevied through the MidiNide mi.
*
*   SEE ALSO
*       RemoveMidiLinkA(), SetMidiLinkAttrs()
*       2.0 tag docs.
****************************************************************************
*
*/

struct MidiLink * __saveds __asm LIBAddMidiLinkA(
register __a0 struct MidiNode *mi,
register __d0 LONG type,
register __a1 struct TagItem *tags)
{
    struct MidiLink             *ml;
    struct TagItem              *ti;
    ULONG                       error;

        /* Allocate the MidiLink */
    if ( !(ml = AllocMem (sizeof *ml, MEMF_PUBLIC|MEMF_CLEAR)) ) FAIL (CME_NoMem);

    ml->ml_ChannelMask = ~0;                    /* initialize link */
    ml->ml_EventTypeMask = ~0;
    ml->ml_MidiNode = mi;
    ml->ml_Flags = type ? MLF_Sender : 0;
    ml->ml_Node.ln_Type = NT_USER;

        /* set creation-only tag attibutes */

    if (ti = find_tag(tags,MLINK_Private))
        ml->ml_Flags |= (ti->ti_Data ? MLF_PrivateLink : 0);
    if (ti = find_tag(tags,MLINK_Priority)) ml->ml_Node.ln_Pri = ti->ti_Data;
    if ((ti = find_tag(tags,MLINK_Parse)) && ti->ti_Data != FALSE)
        ml->ml_ParserData = (APTR)AllocParser();

#if 0
    if (ti = find_tag(tags,MLINK_DeviceLink))
        ml->ml_Flags |= (ti->ti_Data ? MLF_DeviceLink : 0);
#endif

    LockLinks();                                /* lock lists */
    LockMidiList();                             /* lock MIDI flow */

        /* Add to list of links for this node */

    AddTail(type ? (struct List *)&mi->mi_OutLinks :
        (struct List *)&mi->mi_InLinks,
        (struct Node *)&ml->ml_OwnerNode);

    SetMidiLinkAttrsA(ml,tags);                 /* set other tag attrs */

    UnlockMidiList();                           /* restore MIDI flow */

#if 0
        /* if was an application link and no devices linked, try to link */

    if ( !(ml->ml_Flags & MLF_DeviceLink) && CamdBase->DeviceLinkCount == 0 ) {
        if (error = OpenSupport()) {
            RemoveMidiLink (ml);
	    UnlockLinks();                      /* allow linkmods */
            goto exitit;
        }
    }
#endif

    UnlockLinks();                              /* allow linkmods */

    return ml;

exitit:
    SetErrorCode (MLINK_ErrorCode, tags, error);
    /* !!! needs to free link! */
    return NULL;
}

/***************************************************************
*
*   RemoveMidiLink
*
***************************************************************/

/****** camd.library/RemoveMidiLink **************************************
*
*   NAME
*       RemoveMidiLink -- Removes a MidiLink from a MidiCluster.
*
*   SYNOPSIS
*       void RemoveMidiLink (struct MidiLink *ml)
*                                  a0
*   FUNCTION
*       Removes a MidiLink structure from a MidiCluster.
*
*   INPUTS
*	ml - MidiLink to remove. Can be NULL.
*
*   SEE ALSO
*       AddMidiLinkA()
****************************************************************************
*
*/

void __saveds __asm LIBRemoveMidiLink (register __a0 struct MidiLink *ml)
{
    static struct TagItem removal_tags[] = {
        MLINK_Location,         NULL,   /* no location                                  */
        TAG_DONE
    };

    if (ml != NULL)
    {
	LockLinks();                                /* lock links */
	LockMidiList();                             /* halt flow of events */

        /* Call SetMidiLinkAttrs to set the Location to NULL. This has
                the effect of unlinking us from the Cluster
        */

	SetMidiLinkAttrsA(ml,removal_tags);
	if (ml->ml_ParserData) FreeParser(ml->ml_ParserData);
	Remove((struct Node *)&ml->ml_OwnerNode);

	UnlockMidiList();                           /* restore flow of events */
	UnlockLinks();                              /* unlock linkages */

	FreeMem (ml, sizeof *ml);
    }
}

/****** camd.library/SetMidiLinkAttrs ******************************************
*
*   NAME
*       SetMidiLinkAttrs -- Set the attributes of a MidiNode
*
*   SYNOPSIS
*       BOOL SetMidiLinkAttrsA (struct MidiLink *ml, struct TagItem *tags)
*                                       a0          a1
*
*       BOOL SetMidiLinkAttrs (struct MidiLink *mi, Tag type1, ...)
*
*   FUNCTION
*       Sets the attributes of a MidiLink, using a Tag List.
*
*       The first form of the function expects a tag array pointer or NULL.
*       The second form permits the tag items to exist on the caller's
*       stack.  In both cases, the final tag item must be TAG_END.
*
*   INPUTS
*       ml - a pointer to the MidiLink
*
*       TagList   - optional pointer to tag array.  May be NULL.  For
*                   OS v1.3, there are restrictions on the tag array
*                   contents.  See NOTE below.
*
*   TAGS
*       MLINK_Name         STRPTR - ti_Data points to the new name of
*                             the node
*
*       MLINK_Location     STRPTR - ti_Data points to name of MidiCluster
*			      to link with
*
*	MLINK_ChannelMask  UWORD - ti_Data contains mask of which MIDI
*			      channels to listen for (defaults to ~0)
*
*	MLINK_EventMask    UWORD - ti_Data contains mask of which types
*			      of MIDI events to listen for (defaults to ~0)
*
*	MLINK_UserData     CPTR - ti_Data points to user definable data
*
*	MLINK_Comment      STRPTR - ti_Data points to a comment string. The
*			      highest priority MidiLink in a MidiCluster
*			      has its comment field copied to the
*			      MidiCluster's comment field
*
*	MLINK_PortID       UBYTE - ti_Data contains value to copy into any
*			      MidiMsg's arriving at MidiNode through this
*			      MidiLink (defaults to 0)
*
*	MLINK_Private      BOOL - if ti_Data contains TRUE, then this link
*			      requests to not be shown by patch editors, etc.
*
*	MLINK_Priority     BYTE - ti_Data contains priotity of the MidiLink
*
*	MLINK_SysExFilter  ULONG - ti_Data contains three 1-byte manufacturor
*			      numbers to filter SysEx messages with
*
*	MLINK_SysExFilterX ULONG - ti_Data contains one 3-byte manufacturor
*			      number to filter SysEx messages with
*
*	MLINK_Parse        BOOL - if ti_Data contains true, allocate a parser
*			      for the MidiLink so raw MIDI streams can be
*			      sent though the link
*
*	MLINK_ErrorCode    ULONG * - ti_Data points to an error code buffer
*
*   RESULTS
*       TRUE if all changes were made successfully or FALSE on failure.
*       When FALSE is returned, an error code will be returned if a TagItem
*	with an ti_Tag of MLINK_ErrorCode was provided. The CME_ error code
*	will be put in the ti_Data field.
*
*   NOTE
*       Under 1.3 only a restricted tag array may be passed into this
*       funciton. Specifically, the only special tag values that are
*       supported are TAG_END (or TAG_DONE) and TAG_IGNORE.  All others
*       (e.g. TAG_SKIP, TAG_MORE) are treated as TAG_END.  The full range of
*       TAG list operations are permitted under 2.0.
*
*   EXAMPLES
*       mi = SetMidiLinkAttrs (ml,
*               MLINK_Location, "out.0", MLINK_Priority, -5L, TAG_END);
*
*       Modifies a MidiLink so it will be connected to the MidiCluster named
*	"out.0" and makes the MidiLink's priority -5. Note the 'L' on the end
*	of -5. This forces correct alignment of the tag items on the stack
*	for 16 bit integer compilation.
*
*   SEE ALSO
*       AddMidiLink(), RemoveMidiLink()
*       2.0 tag docs.
****************************************************************************
*
*/

BOOL __saveds __asm LIBSetMidiLinkAttrsA(
register __a0 struct MidiLink *ml,
register __a1 struct TagItem *tags)
{   struct MidiCluster          *mc,
                                *new_mc;
    struct TagItem              *tlist = tags;

    LockLinks();                                /* lock links */
    LockMidiList();                             /* halt flow of events */
                                                /* (fortunately, locks nest) */
    while (tags = next_tag(&tlist))             /* for each tag */
    {   ULONG tdata = tags->ti_Data;            /* avoid deref each time */

        switch (tags->ti_Tag) {

        case MLINK_Location:

        if (tdata == NULL) new_mc = NULL;
        else new_mc = open_cluster( (char *)tdata );    /* !!! doesn't handle low mem failure from open_cluster() very well */

        mc = ml->ml_Location;                   /* pointer to old cluster */
        if (mc == new_mc) break;                /* if same cluster, no change */

            /* if it's already set to a cluster, then detach from the cluster */

        if (mc)
        {   Remove(&ml->ml_Node);               /* remove from list */
	    mc->mcl_Participants--;
            if ( !(ml->ml_Flags & MLF_PrivateLink) )
                mc->mcl_PublicParticipants--;   /* subtract 1 from part.count */

            NotifyPartChanges(mc,ml,CACT_Unlink);           /* notify other participants */

#if 0
            if (ml->ml_Flags & MLF_DeviceLink) CamdBase->DeviceLinkCount--;
            else CamdBase->AppLinkCount--;
#endif
        }

            /* now, set to new cluster */

	if ( (ml->ml_Location = new_mc) != NULL )
        {
            Enqueue( (ml->ml_Flags & MLF_Sender) ?
                &new_mc->mcl_Senders :
                &new_mc->mcl_Receivers, &ml->ml_Node);

	    new_mc->mcl_Participants++;
            if ( !(ml->ml_Flags & MLF_PrivateLink) )
                new_mc->mcl_PublicParticipants++; /* add 1 to participant count */

            NotifyPartChanges(new_mc,ml,CACT_Link);       /* notify other participants */

#if 0
            if (ml->ml_Flags & MLF_DeviceLink) CamdBase->DeviceLinkCount++;
            else CamdBase->AppLinkCount++;
#endif
        }

        break;

        case MLINK_SysExFilter:
	ml->ml_SysExFilter.sxf_Packed = tdata & 0x00ffffff;
	if (ml->ml_SysExFilter.sxf_ID3) ml->ml_SysExFilter.sxf_Mode = 3;
	else if (ml->ml_SysExFilter.sxf_ID2) ml->ml_SysExFilter.sxf_Mode = 2;
	else if (ml->ml_SysExFilter.sxf_ID1) ml->ml_SysExFilter.sxf_Mode = 1;
        else ml->ml_SysExFilter.sxf_Mode = 0;
	break;

        case MLINK_SysExFilterX:
	ml->ml_SysExFilter.sxf_Packed =
	    (ULONG)SXFM_3Byte << 24 | (tdata & 0x00ffffff);
	break;

        case MLINK_ChannelMask: ml->ml_ChannelMask = tdata; break;
        case MLINK_EventMask:   ml->ml_EventTypeMask = tdata; break;
        case MLINK_UserData:    ml->ml_UserData = (APTR)tdata; break;
        case MLINK_Comment:     ml->ml_ClusterComment = (char *)tdata; break;
        case MLINK_PortID:      ml->ml_PortID = tdata; break;
        case MLINK_Name:        ml->ml_Node.ln_Name = (char *)tdata; break;
        }
    }

#if 0
    if ( !(ml->ml_Flags & MLF_DeviceLink) && CamdBase->AppLinkCount == 0)
    {
        CloseSupport();
    }
#endif

    UnlockMidiList();

    notify_external();

    UnlockLinks();                                /* unlock links */

    return TRUE;
}


/***************************************************************
*
*   GetMidiLinkAttrsA
*
***************************************************************/

/****** camd.library/GetMidiLinkAttrsA *****************************************
*
*   NAME
*       GetMidiLinkAttrsA -- Get attributes of a MidiLink
*
*   SYNOPSIS
*       ULONG GetMidiLinkAttrsA (struct MidiLink *ml, struct TagItem *attrs)
*                                       a0             a1
*
*   FUNCTION
*       Gets an attribute of a MidiLink.
*
*   INPUTS
*       mi - a pointer to the MidiLink
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
*	See SetMidiLinkAttrs() for tags understood.o
*
*   EXAMPLES
*
*   SEE ALSO
*       SetMidiLinkAttrs()
****************************************************************************
*
*/

ULONG __saveds __asm LIBGetMidiLinkAttrsA(
register __a0 struct MidiLink *ml,
register __a1 struct TagItem *attrs)
{   struct TagItem              *tlist = attrs,
				*tags;
    ULONG			count = 0;

    LockLinks();                                /* lock links */
    LockMidiList();                             /* halt flow of events */
                                                /* (fortunately, locks nest) */

    while (tags = next_tag(&tlist))             /* for each tag */
    {   ULONG tdata = tags->ti_Data;            /* avoid deref each time */

        switch (tags->ti_Tag) {

        case MLINK_Location:
	    *(char **)tdata = ml->ml_Location->mcl_Node.ln_Name;
	    count++;
	    break;

        case MLINK_SysExFilter:
	    *(ULONG *)tdata = (ml->ml_SysExFilter.sxf_Packed & 0xff000000 ?
		-1 : ml->ml_SysExFilter.sxf_Packed);
	    count++;
	    break;

        case MLINK_SysExFilterX:
	    *(ULONG *)tdata = ((ml->ml_SysExFilter.sxf_Packed & 0xff000000) ==
		((ULONG)SXFM_3Byte << 24) ?
		ml->ml_SysExFilter.sxf_Packed & 0x00ffffff : -1);
	    count++;
	    break;

        case MLINK_ChannelMask:
	    *(ULONG *)tdata = ml->ml_ChannelMask;
	    count++;
	    break;

        case MLINK_EventMask:
	    *(ULONG *)tdata = ml->ml_EventTypeMask;
	    count++;
	    break;

        case MLINK_UserData:
	    *(void **)tdata = ml->ml_UserData;
	    count++;
	    break;

        case MLINK_Comment:
	    *(char **)tdata = ml->ml_ClusterComment;
	    count++;
	    break;

        case MLINK_PortID:
	    *(ULONG *)tdata = ml->ml_PortID;
	    count++;
	    break;

        case MLINK_Name:
	    *(char **)tdata = ml->ml_Node.ln_Name;
	    count++;
	    break;
        }
    }

    UnlockMidiList();
    UnlockLinks();                                /* unlock links */

    return count;
}


/* ============================================================================ *
                               Cluster Management
 * ============================================================================ */


static int notify_linklist (
struct MidiCluster *,
struct MidiLink *initiator,
LONG mltype,
ULONG action);

/*      Internal function to open a cluster

        This function finds a cluster if it exists, otherwise it will create one.
*/

static struct MidiCluster *open_cluster(char *name)
{   struct MidiCluster  *m;
    char                        len = strlen(name) + 1;

    if (m = FindCluster(name)) return m;

    if ( !(m = AllocMem(sizeof *m + len,MEMF_PUBLIC|MEMF_CLEAR)) ) return NULL;

    NewList(&m->mcl_Receivers);
    NewList(&m->mcl_Senders);
        m->mcl_Receivers.lh_Type = m->mcl_Senders.lh_Type = NT_USER;
    m->mcl_Node.ln_Name = (char *)(m + 1);
    CopyMem(name,m+1,len);

    AddTail((struct List *)&CamdBase->ClusterList,&m->mcl_Node);

    return m;
}

        /* This function is called each time a link is added or removed from a
                cluster. It has two functions:
                1. It informs all the other participants that a change has been
                        made, if they requested that information.
                2. It deletes the cluster if there are no more links left.

           !!! should have a name that reflects #2.  maybe sync_cluster() or update_cluster()
        */

static
void NotifyPartChanges (
struct MidiCluster *mc,
struct MidiLink *initiator,
ULONG action)
{
    int partcount = 0;

    partcount += notify_linklist (mc, initiator, MLTYPE_Sender, action);
    partcount += notify_linklist (mc, initiator, MLTYPE_Receiver, action);

    if (!partcount) {
        size_t len = strlen ( (char *)(mc + 1) ) + 1;

	Remove (&mc->mcl_Node);
        FreeMem (mc, sizeof *mc + len);
    }
}

static
int notify_linklist (
struct MidiCluster *mc,
struct MidiLink *initiator,
LONG mltype,
ULONG action)
{
    struct MidiNode *mi;
    struct MidiLink *ml;
    int partcount = 0;

    for (ml = NULL; ml = NextClusterLink (mc, ml, mltype); ) {
        partcount++;
        if (ml != initiator) {
            ml->ml_Flags |= MLF_PartChange;

            mi = ml->ml_MidiNode;

            if (mi->mi_ParticipantHook)
				CallHook (mi->mi_ParticipantHook, (APTR)ml, (ULONG)CMSG_Link, action);
            if (mi->mi_ParticipantSigBit >= 0)
                Signal (mi->mi_SigTask, 1L << mi->mi_ParticipantSigBit);
        }
    }

    return partcount;
}


/***************************************************************
*
*   NextCluster
*
***************************************************************/

/****** camd.library/NextCluster ******************************************
*
*   NAME
*       NextCluster -- Get next MidiCluster
*
*   SYNOPSIS
*       struct MidiCluster *NextCluster (struct MidiCluster *last)
*                                  		a0
*
*   FUNCTION
*       Returns the next MidiCluster on CAMD MidiCluster list. If last
*	is NULL, returns the first MidiCluster. Returns NULL if no more
*	MidiClusters. Midi links (CD_Linkages) must be locked when called.
*
*   INPUTS
*       last - previous MidiCluster or NULL to get first MidiCluster
*
*   RESULTS
*       next - next MidiCluster or NULL
*
****************************************************************************
*
*/

struct MidiCluster * __saveds __asm LIBNextCluster(
register __a0 struct MidiCluster *mc)
{   if (mc) mc = (struct MidiCluster *)mc->mcl_Node.ln_Succ;
    else mc = (struct MidiCluster *)CamdBase->ClusterList.mlh_Head;

    return mc->mcl_Node.ln_Succ ? mc : NULL;
}

/***************************************************************
*
*   FindCluster
*
***************************************************************/

/****** camd.library/FindCluster ******************************************
*
*   NAME
*       FindCluster -- Find a MidiCluster by name
*
*   SYNOPSIS
*       struct MidiCluster *FindCluster (name)
*                                  a0
*
*   FUNCTION
*       Find a MidiCluster by name. If node of that exists, the function
*       returns NULL. Midi links (CD_Linkages) must be locked when called.
*
*   INPUTS
*       name - name of MidiCluster to find.
*
*   RESULTS
*       result - A MidiCluster or NULL.
*
****************************************************************************
*
*/

struct MidiCluster * __saveds __asm LIBFindCluster(
register __a0 char *name)
{   return (struct MidiCluster *)
        FindName((struct List *)&CamdBase->ClusterList,name);
}

/***************************************************************
*
*   NextClusterLink
*
***************************************************************/

/****** camd.library/NextClusterLink ***************************************
*
*   NAME
*       NextClusterLink -- Get next MidiLink of one type in a MidiCluster
*
*   SYNOPSIS
*       struct MidiLink *NextClusterLink (struct MidiCluster *mc,
*                                  		a0
*			struct MidiLink *last, LONG type)
*				a1		d0
*
*   FUNCTION
*       Returns the next MidiLink of a particular type a MidiCluster's list
*	of MidiLinks. If last is NULL, returns the first MidiLink. Returns
*	NULL if no more MidiLinks. Midi links (CD_Linkages) must be locked
*	when called.
*
*   INPUTS
*       last - previous MidiLink or NULL to get first MidiLink
*
*   RESULTS
*       next - next MidiLink or NULL
*
****************************************************************************
*
*/

struct MidiLink * __saveds __asm LIBNextClusterLink(
register __a0 struct MidiCluster *mc,
register __a1 struct MidiLink *ml,
register __d0 LONG type)
{   if (ml) ml = (struct MidiLink *)ml->ml_Node.ln_Succ;
    else ml = (struct MidiLink *)(type!=MLTYPE_Receiver ? mc->mcl_Senders.lh_Head : mc->mcl_Receivers.lh_Head);

    return (ml->ml_Node.ln_Succ) ? ml : NULL;
}

/***************************************************************
*
*   NextMidiLink
*
***************************************************************/

/****** camd.library/NextMidiLink ***************************************
*
*   NAME
*       NextMidiLink -- Get next MidiLink of one type in a MidiNode
*
*   SYNOPSIS
*       struct MidiLink *NextMidiLink (struct MidiNode *mi,
*                                  		a0
*			struct MidiLink *last, LONG type)
*				a1		d0
*
*   FUNCTION
*       Returns the next MidiLink of a particular type a MidiNode's list
*	of MidiLinks. If last is NULL, returns the first MidiLink. Returns
*	NULL if no more MidiLinks. Midi links (CD_Linkages) must be locked
*	when called.
*
*   INPUTS
*       last - previous MidiLink or NULL to get first MidiLink
*
*   RESULTS
*       next - next MidiLink or NULL
*
****************************************************************************
*
*/

struct MidiLink * __saveds __asm LIBNextMidiLink(
register __a0 struct MidiNode *mi,
register __a1 struct MidiLink *ml,
register __d0 LONG type)
{
    struct MinNode *n;

    n = ml ? ml->ml_OwnerNode.mln_Succ
      : type != MLTYPE_Receiver ? mi->mi_OutLinks.mlh_Head : mi->mi_InLinks.mlh_Head;

    return n->mln_Succ ? (struct MidiLink *)((char *)(n) - offsetof (struct MidiLink, ml_OwnerNode)) : NULL;
}

/***************************************************************
*
*   MidiLinkConnected
*
***************************************************************/

/****** camd.library/MidiLinkConnected ***************************************
*
*   NAME
*       MidiLinkConnected -- Determine if MidiLink has a connection
*
*   SYNOPSIS
*       BOOL MidiLinkConnected (struct MidiLink *ml)
*                                      a0
*
*   FUNCTION
*       Returns TRUE if a MidiLink can currently send or receive to anyone.
*	For a MidiLink of type MLTYPE_Sender, then there must be at least
*	one MidiLink of type MLTYPE_Receiver linked to the same MidiCluster,
*	and visa versa.
*
*   INPUTS
*       ml - MidiLink to check
*
*   RESULTS
*       result - TRUE if there is a connection, FALSE otherwise
*
****************************************************************************
*
*/

BOOL __saveds __asm LIBMidiLinkConnected (register __a0 struct MidiLink *ml)
{   struct MidiCluster  *mc = ml->ml_Location;
    struct MinNode      *n;

    n = (ml->ml_Flags & MLF_Sender) ? mc->mcl_Receivers.lh_Head : mc->mcl_Senders.lh_Head ;

    return (n->mln_Succ != NULL);
}


#if 0
/* -------------------- open/close support subsystems */

static
int OpenSupport (void)
{
    int err;

    if (err = OpenTimer()) goto clean;
    /* if (err = OpenUnits()) goto clean; */

    return 0;

clean:
    CloseSupport();
    return err;
}

static
void CloseSupport (void)
{
    CloseTimer();
    /* CloseUnits(); */
}
#endif
