/* $Id: showlock.c,v 1.5 91/04/24 20:54:05 peter Exp $	*/
/* showlock.c : wack SignalSemaphores, Layer locks, IBase locks.
 */

#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/interrupts.h"
#include "exec/memory.h"
#include "exec/ports.h"
#include "exec/libraries.h"
#include "exec/devices.h"
#include "exec/tasks.h"
#include "exec/resident.h"
#include "exec/execbase.h"

#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "exec/semaphores.h"

#include "symbols.h"
#include "special.h"

#define NAMESIZE 100

extern APTR CurrAddr;

#define SEMFMT0 "\nAddress  NestCount    Owner  Queue  Waiters ... <Name>\n"
#define SEMFMT1 "%08lx     %4d %8lx   %4d  "

semHeader()
{
    printf(SEMFMT0);
}

ULONG
dumpSemReq(addr, sr)
APTR	addr;
struct SemaphoreRequest *sr;
{
    printf(" %8lx ", sr->sr_Waiter);
    return (0);
}

ULONG
dumpSem(addr, sem, name)
APTR	addr;
struct SignalSemaphore *sem;
char	*name;
{
    /* dump semaphore */
    printf(SEMFMT1, addr, sem->ss_NestCount, sem->ss_Owner, sem->ss_QueueCount);

    /* print out waiters */
    WalkMinList(CADDR(addr, sem, &sem->ss_WaitQueue),
		sizeof(struct SemaphoreRequest), dumpSemReq);

    printf("%s\n", name);
    return (0);
}


/* builds callers to dumpT(addr, ptr) where T is not a node	*/
#define SHOWONE(T) \
	Show/**/T(arg) char *arg; \
	{ struct T *w;ULONG dump/**/T(); if (AddrArg(arg, &w)) { printf("\n");\
	doMinNode(w, sizeof (struct T), dump/**/T); }\
	else puts("\nbad syntax"); }
ShowSem( arg )
char	*arg;
{
    struct SignalSemaphore	*ss;

    if ( AddrArg( arg, &ss ) )
    {
	semHeader();
	showSem( ss );
    }
    else puts("\nbad syntax.");
}
showSem(addr)
{
    struct SignalSemaphore sem;

    doNode(addr, sizeof (struct SignalSemaphore), dumpSem);
}

showSemList(addr)
APTR addr;
{
    WalkList(addr, sizeof (struct SignalSemaphore), dumpSem);
}

ShowSemList()
{
    semHeader();
    showSemList(CurrAddr);
}

#if 0	/* should come from dump screen, ... */
ShowLayerInfoSemList()
{
    struct Layer_Info	linfo;

    WalkList(CADDR(CurrAddr, &linfo, &linfo->gs_List), 
	sizeof (struct SignalSemaphore), dumpSem);
}
#endif
