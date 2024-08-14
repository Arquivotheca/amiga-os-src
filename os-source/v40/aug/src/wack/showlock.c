/*
 * Amiga Grand Wack
 *
 * showlock.c -- Show semaphores on target machine.
 *
 * $Id: showlock.c,v 39.5 93/06/02 15:51:33 peter Exp $
 *
 * Code to display semaphores, owners, and waiters on the target Amiga.
 *
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

#include "sys_proto.h"
#include "parse_proto.h"
#include "special_proto.h"
#include "showlock_proto.h"
#include "ops_proto.h"
#include "io_proto.h"

#define NAMESIZE 100

extern APTR CurrAddr;

#define SEMFMT0 "Address  NestCount    Owner  Queue  Waiters ... <Name>\n"
#define SEMFMT1 "%08lx     %4ld %8lx   %4ld  "

void
semHeader( void )
{
    PPrintf(SEMFMT0);
}

ULONG
dumpSemReq( APTR addr, struct SemaphoreRequest *sr )
{
    PPrintf(" %8lx ", sr->sr_Waiter);
    return (0);
}

ULONG
dumpSem( APTR addr, struct SignalSemaphore *sem, char *name )
{
    /* dump semaphore */
    PPrintf(SEMFMT1, addr, sem->ss_NestCount, sem->ss_Owner, sem->ss_QueueCount);

    /* print out waiters */
    WalkMinList(CADDR(addr, sem, &sem->ss_WaitQueue),
		sizeof(struct SemaphoreRequest), dumpSemReq, FALSE);

    PPrintf("%s\n", name);
    return (0);
}


STRPTR
ShowSem( char *arg )
{
    struct SignalSemaphore	*ss;

    if ( parseAddress( arg, (ULONG *) &ss ) )
    {
	semHeader();
	ShowSemNoHeader( ss );
    }
    else
    {
	BadSyntax();
    }

    return( NULL );
}

void
ShowSemNoHeader( APTR addr )
{
    doNode(addr, sizeof (struct SignalSemaphore), dumpSem);
}

void
ShowSemListNoHeader( APTR addr )
{
    WalkList(addr, sizeof (struct SignalSemaphore), dumpSem, FALSE);
}

STRPTR
ShowSemList( char *arg )
{
    APTR semlist;

    if ( parseAddress( arg, (ULONG *) &semlist ) )
    {
	semHeader();
	ShowSemListNoHeader(semlist);
    }
    return( NULL );
}
