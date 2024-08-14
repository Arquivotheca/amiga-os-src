/* fixed error checking after AllocEntry */

#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/memory.h"
#include "exec/ports.h"
#include "exec/tasks.h"
#include "clib/exec_protos.h"

extern struct Library *SysBase;
#include "pragmas/exec_pragmas.h"

void NewList( struct List *list );
struct Task *CreateTask( char *name, LONG pri, APTR initPC, ULONG stackSize );

/*
extern APTR AllocMem();
extern struct Task *FindTask();
*/

/*
******* amiga.lib/CreateTask ***************************
*
*   NAME
*	CreateTask -- Create task with given name, priority, stacksize
*
*   SYNOPSIS
*	CreateTask( name, pri, initPC, stackSize)
*
*	task=(struct Task *)CreateTask(char *, LONG, funcEntry, ULONG);
*
*   FUNCTION
*	This function simplifies program creation of subtasks by
*	dynamically allocating and initializing required structures
*	and stack space, and adding the task to Exec's task list
*	with the given name and priority.  A tc_MemEntry list is provided
*	so that all stack and structure memory allocated by CreateTask
*	is automatically deallocated when the task is removed.
*
*	An Exec task may not call dos.library functions or any function
*	which might cause the loading of a disk-resident library, device,
*	or file (since such functions are indirectly calls to dos.library).
*	Only AmigaDOS Processes may call AmigaDOS; see the DOS CreateProc()
*	call for more information.
*
*	If other tasks or processes will need to find this task by name,
*	provide a complex and unique name to avoid conflicts.
*
*	If your compiler provides automatic insertion of stack-checking
*	code, you may need to disable this feature when compiling subtask
*	code since the stack for the subtask is at a dynamically allocated
*	location.  If your compiler requires 68000 registers to contain
*	particular values for base relative addressing, you may need to
*	save these registers from your main process, and restore them
*	in your initial subtask code.
*
*	The function entry initPC is generally provided as follows:
*
*	In C:
*	extern void functionName();
*	char *tname = "unique name";
*	task = CreateTask(tname, 0L, functionName, 4000L);
*
*	In assembler:
*		PEA	startLabel
*
*   INPUTS
*
*	name - a null terminated string.
*	pri - an Exec task priority between -128 and 127 (commonly 0)
*	funcEntry - the address of the first executable instruction
*		    of the subtask code.
*	stackSize - size in bytes of stack for the subtask.  Don't cut it
*		    too close - system function stack usage may change.
*
*   SEE ALSO
*	DeleteTask, exec/FindTask
*
***************************************************************
*/


/*
 *  Create a task with given name, priority, and stack size.
 *  It will use the default exception and trap handlers for now.
 */
/* the template for the mementries.  Unfortunately, this is hard to
 * do from C: mementries have unions, and they cannot be statically
 * initialized...
 *
 * In the interest of simplicity I recreate the mem entry structures
 * here with appropriate sizes.  We will copy this to a local
 * variable and set the stack size to what the user specified,
 * then attempt to actually allocate the memory.
 */

#define ME_TASK 	0
#define ME_STACK	1
#define NUMENTRIES	2

struct FakeMemEntry {
    ULONG fme_Reqs;
    ULONG fme_Length;
};

struct FakeMemList {
    struct Node fml_Node;
    UWORD	fml_NumEntries;
    struct FakeMemEntry fml_ME[NUMENTRIES];
} TaskMemTemplate = {
    { 0 },						/* Node */
    NUMENTRIES, 					/* num entries */
    {							/* actual entries: */
	{ MEMF_PUBLIC | MEMF_CLEAR, sizeof( struct Task ) },    /* task */
	{ MEMF_CLEAR,	0 }					/* stack */
    }
};




struct Task *
CreateTask( char *name, LONG pri, APTR initPC, ULONG stackSize )
{
    struct Task *newTask;
    struct FakeMemList fakememlist;
    struct MemList *ml;

    /* round the stack up to longwords... */
    stackSize = (stackSize +3) & ~3;

    /*
     * This will allocate two chunks of memory: task of PUBLIC
     * and stack of PRIVATE
     */
    fakememlist = TaskMemTemplate;
    fakememlist.fml_ME[ME_STACK].fme_Length = stackSize;

    ml = (struct MemList *) AllocEntry( (struct MemList *)&fakememlist );

    /* NOTE ! - AllocEntry returns with bit 31 set if it fails ! */
    if( ((ULONG)ml) & 0x80000000 ) {
	return( NULL );
    }

    /* set the stack accounting stuff */
    newTask = (struct Task *) ml->ml_ME[ME_TASK].me_Addr;

    newTask->tc_SPLower = ml->ml_ME[ME_STACK].me_Addr;
    newTask->tc_SPUpper = (APTR)((ULONG)(newTask->tc_SPLower) + stackSize);
    newTask->tc_SPReg = newTask->tc_SPUpper;

    /* misc task data structures */
    newTask->tc_Node.ln_Type = NT_TASK;
    newTask->tc_Node.ln_Pri = pri;
    newTask->tc_Node.ln_Name = name;

    /* add it to the tasks memory list */
    NewList( &newTask->tc_MemEntry );
    AddHead( &newTask->tc_MemEntry, ml );

    /* add the task to the system -- use the default final PC */
    AddTask( newTask, initPC, 0 );
    return( newTask );
}
