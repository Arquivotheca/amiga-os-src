
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/io.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>

#include "mpegplayerbase.h"


/***************************************************************************/


#define ME_TASK 	0
#define ME_STACK	1
#define NUMENTRIES	2

struct FakeMemEntry
{
    ULONG fme_Reqs;
    ULONG fme_Length;
};

struct FakeMemList
{
    struct Node fml_Node;
    UWORD	fml_NumEntries;
    struct FakeMemEntry fml_ME[NUMENTRIES];
} TaskMemTemplate = {
    { 0 },
    NUMENTRIES,
    {
	{ MEMF_REVERSE | MEMF_CLEAR, sizeof( struct Task ) },
	{ MEMF_REVERSE | MEMF_CLEAR,	0 }					/* stack */
    }
};


struct Task *MakeTask(struct MPEGPlayerLib *MPEGPlayerBase,
                      STRPTR name, LONG pri, APTR initPC, ULONG stackSize)
{
struct Task        *newTask;
struct FakeMemList  fakememlist;
struct MemList     *ml;
APTR                result;

    fakememlist = TaskMemTemplate;
    fakememlist.fml_ME[ME_STACK].fme_Length = stackSize;

    if (ml = (struct MemList *) AllocEntry((struct MemList *)&fakememlist))
    {
        newTask                  = (struct Task *) ml->ml_ME[ME_TASK].me_Addr;
        newTask->tc_SPLower      = ml->ml_ME[ME_STACK].me_Addr;
        newTask->tc_SPUpper      = (APTR)((ULONG)(newTask->tc_SPLower) + stackSize);
        newTask->tc_SPReg        = newTask->tc_SPUpper;
        newTask->tc_Node.ln_Type = NT_TASK;
        newTask->tc_Node.ln_Pri  = pri;
        newTask->tc_Node.ln_Name = name;
        newTask->tc_UserData     = MPEGPlayerBase;

        NewList(&newTask->tc_MemEntry);
        AddHead(&newTask->tc_MemEntry,ml);

        if (result = AddTask( newTask, initPC, 0 ))
            return(newTask);

        FreeEntry(ml);
    }

    return(NULL);
}


/*****************************************************************************/


VOID SyncSignal(struct MPEGPlayerLib *MPEGPlayerBase, struct Task *task,
                ULONG sigMask)
{
    SetSignal(0,SIGF_SINGLE);
    Signal(task,sigMask);
    Wait(SIGF_SINGLE);
}
