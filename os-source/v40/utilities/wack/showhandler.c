
/* $Id: showhandler.c,v 1.3 91/04/24 20:53:57 peter Exp $	*/
/* showhandler.c : wack "show dos handler" functions.
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

#include "exec/semaphores.h"

#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "symbols.h"
#include "special.h"
#include "wack.h"

#include "dosmagic.h"

static ULONG HandlerAddr;
static ULONG GlobalBase;

SetHandler()
{
    struct Process pr;

    HandlerAddr = CurrAddr;

    GetBlock (HandlerAddr, &pr, sizeof (pr));
    GlobalBase = (ULONG) pr.pr_GlobVec;

    NewLine();
}

ShowHandler()
{
    int numents;
    int i;
    ULONG *globals;

    NewLine();


    numents = GetMemL( GlobalBase ) + 1;

    globals = (ULONG *) calloc( numents, sizeof( ULONG ) );
    if( !globals ) {
	printf( "Out of memory\n" );
	return;
    }

    GetBlock( GlobalBase, globals, numents * sizeof( ULONG ) );

    printf( "process 0x%lx, globals 0x%lx\n", HandlerAddr, GlobalBase );

    printf( "iob 0x%lx, unit %ld, bitmap 0x%lx\n",
	globals[FH3_DISK_DEV], globals[FH3_UNITNO], globals[FH3_BITMAP] << 2 );

    cfree( globals );
}

ReadGlobal( number )
int number;
{
    return( GetMemL( GlobalBase + 4*number ) );
}


ShowGlobal( argStr )
char *argStr;
{
    long global;

    if( sscanf( argStr, " %d ", &global) == 1 ) {
	printf( " => %lx\n", ReadGlobal( global ) );
    }
}

ShowUG( argStr )
char *argStr;
{
    long global;

    if( sscanf( argStr, " %d ", &global ) == 1 ) {
	printf( " => %lx\n", ReadGlobal( UG + global ) );
    }
}


ShowBitMap()
{
    ULONG bitmap = ReadGlobal( FH3_BITMAP ) << 2;
    LONG bitmapsize = (ReadGlobal( FH3_BITMAP_UPB ) + 1 ) << 2;

    ShowMem( bitmap, bitmapsize );

}

FindGlobal( argStr )
char *argStr;
{
    long routine;
    ULONG *globals;
    ULONG *ptr;
    int i;
    long numents;
    int success = 0;

    if( sscanf( argStr, " %x ", &routine ) != 1 ) {
	printf( "\n(findglobal <value>) -- search global list for value\n" );
	return;
    }

    numents = GetMemL( GlobalBase ) + 1;

    globals = (ULONG *) calloc( numents, sizeof( ULONG ) );
    if( !globals ) {
	printf( "Out of memory\n" );
	return;
    }

    GetBlock( GlobalBase, globals, numents * sizeof( ULONG ) );

    success = 0;
    for( i = 0, ptr = globals; i < numents; i++, ptr++ ) {
	if( *ptr == routine ) {
	    printf( "global %ld/0x%lx @ 0x%lx = 0x%lx\n",
		i, i, GlobalBase + (i << 2), routine );
	    success = 1;
	}
    }

    if( ! success ) {
	printf( "global not found\n" );
    }


    cfree( globals );
}
