
#include <exec/types.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <dos/dos.h>
#include <diskfont/diskfont.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "segments.h"
#include "utils.h"


/*****************************************************************************/


struct SegmentNode
{
    struct MinNode sn_Link;
    BPTR           sn_Lock;
    BPTR           sn_Segment;
};

typedef BPTR __asm (*LOADSEGFUNC)(register __d1 STRPTR, register __a6 struct Library *);


/*****************************************************************************/


extern struct ExecBase        *SysBase;
extern struct Library         *DOSBase;
extern struct Library         *GfxBase;

static struct MinList          segmentList;
static struct SignalSemaphore  listLock;
static LOADSEGFUNC             realLoadSeg;
static ULONG                   intruder;


/*****************************************************************************/


static BPTR __asm LoadSegPatch(register __d1 STRPTR name)
{
BPTR                lock;
struct SegmentNode *node;
BPTR                result;

    geta4();

    intruder++;

    lock = Lock(name,ACCESS_READ);  /* no problem if this returns NULL */
    result = NULL;

    ObtainSemaphore(&listLock);

    node = (struct SegmentNode *)segmentList.mlh_Head;
    while (node->sn_Link.mln_Succ)
    {
        if (SameLock(lock,node->sn_Lock) == LOCK_SAME)
        {
            result = node->sn_Segment;
            Remove((struct Node *)node);
            UnLock(node->sn_Lock);
            FreeVec(node);
            break;
        }

        node = (struct SegmentNode *)node->sn_Link.mln_Succ;
    }

    ReleaseSemaphore(&listLock);

    UnLock(lock);

    if (!result)
        result = realLoadSeg(name,DOSBase);

    intruder--;

    return(result);
}


/*****************************************************************************/


BOOL AddSegmentNode(STRPTR name, BPTR segment)
{
struct SegmentNode *node;

    if (node = AllocVec(sizeof(struct SegmentNode),MEMF_ANY))
    {
        if (node->sn_Lock = Lock(name,ACCESS_READ))
        {
            node->sn_Segment = segment;

            ObtainSemaphore(&listLock);
            AddTail((struct List *)&segmentList,(struct Node *)node);
            ReleaseSemaphore(&listLock);

            return(TRUE);
        }
        FreeVec(node);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID RemSegmentNode(BPTR segment)
{
struct SegmentNode *node;

    ObtainSemaphore(&listLock);

    node = (struct SegmentNode *)segmentList.mlh_Head;
    while (node->sn_Link.mln_Succ)
    {
        if (node->sn_Segment == segment)
        {
            Remove((struct Node *)node);
            UnLock(node->sn_Lock);
            FreeVec(node);
            UnLoadSeg(segment);
            break;
        }
        node = (struct SegmentNode *)node->sn_Link.mln_Succ;
    }

    ReleaseSemaphore(&listLock);
}


/*****************************************************************************/


VOID InitSegments(VOID)
{
    NewList((struct List *)&segmentList);
    InitSemaphore(&listLock);

    realLoadSeg = (LOADSEGFUNC)SetFunction(DOSBase,-150,LoadSegPatch);
}


/*****************************************************************************/


/* this function must be called under Forbid(), and must not break Forbid() ! */
BOOL TerminateSegments(VOID)
{
LOADSEGFUNC supposedToBeMe;
BOOL        result;

    if (AttemptSemaphore(&listLock))
    {
        if (IsListEmpty((struct List *)&segmentList))
        {
            if (!intruder)  /* nobody is running the LoadSegPatch() */
            {
                supposedToBeMe = (LOADSEGFUNC)SetFunction(DOSBase,-150,realLoadSeg);
                if (supposedToBeMe == (LOADSEGFUNC)LoadSegPatch)
                {
                    result = TRUE;
                }
                else
                {
                    SetFunction(DOSBase,-150,supposedToBeMe);
                }
            }
        }

        ReleaseSemaphore(&listLock);
      }

    return(result);
}


/*****************************************************************************/


enum SegmentType FindSegmentType(BPTR segment)
{
struct DiskFontHeader *dfh;
struct Resident       *tag;

    if (tag = FindRomTag(segment))
    {
        if (tag->rt_Type == NT_LIBRARY)
            return(ST_LIBRARY);

        if (tag->rt_Type == NT_DEVICE)
            return(ST_DEVICE);
    }
    else
    {
        dfh = (struct DiskFontHeader *)((ULONG)BADDR(segment) + 8);
        if ((dfh->dfh_DF.ln_Type == NT_FONT) && (dfh->dfh_FileID = DFH_ID))
            return(ST_FONT);
    }

    return(ST_UNKNOWN);
}
