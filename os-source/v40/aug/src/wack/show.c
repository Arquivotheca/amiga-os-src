/*
 * Amiga Grand Wack
 *
 * show.c -- Routines to show many low-level system structures.
 *
 * $Id: show.c,v 39.5 93/08/19 16:35:49 peter Exp $
 *
 * Here is the code to show many of the low-level system structures of
 * the target Amiga.  Most of the stuff here is Exec level stuff
 * (nodes, libraries, ports, tasks, etc.), plus processes.
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

#include "symbols.h"
#include "std.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern APTR LinkLibBase;

#include "sys_proto.h"
#include "main_proto.h"
#include "showd_proto.h"
#include "link_proto.h"
#include "show_proto.h"
#include "io_proto.h"
#include "parse_proto.h"

#define LTOB(b_ptr) (b_ptr << 2)

extern APTR CurrAddr;
extern APTR SpareAddr;
extern long FrameSize;

#define NAMESIZE 100

/* Read the string at address 'addr' into buffer 'buffer'.
 * No more than 'maxlen' bytes will be transferred, including the
 * trailing \0.  Returns the actual number of bytes transferred.
 */
unsigned long
GetString( APTR addr, char *buffer, unsigned long maxlen )
{
    unsigned long len = 0;

    if ( addr == 0 )
    {
	buffer[ 0 ] = 0;
    }
    else
    {
	len = ReadString( addr, buffer, maxlen );
	buffer[ maxlen - 1 ] = 0;
    }
    return( len );
}


unsigned long
GetBSTR( APTR addr, char *buffer, unsigned long maxlen )
{
    unsigned long len = 0;

    if ( addr == 0 )
    {
	buffer[ 0 ] = 0;
    }
    else
    {
	len = ReadByte( addr ) + 1;
	if ( len > maxlen )
	{
	    len = maxlen;
	}
	ReadBlock( ( (UBYTE *) addr ) + 1, buffer, len-1 );
	buffer[ len - 1 ] = 0;
    }
    return( len );
}


void
GetName( APTR nameAddr, char *nameStr )
{
    GetString( nameAddr, nameStr, NAMESIZE );
}


/***********************************************************************
*
*   Show Node
*
************************************************************************
*    Address NT  Pri Name
*  C XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/
#define NFMT0 "  Address   NT  Pri Name\n"
#define NFMT1 "%08lx  %2ld %4ld \"%s\"\n"
#define NFMT2 "%08lx  %2ld %4ld --\n"

void
ShowNode( struct Node *n )
{
    struct Node node;
    char name[NAMESIZE];

    ReadBlock( n, &node, sizeof( struct Node ) );
    if ( ( node.ln_Name ) && ValidAddress( node.ln_Name ) )
    {
	GetName( node.ln_Name, name );
	PPrintf( NFMT1, n, node.ln_Type, node.ln_Pri, name );
    }
    else
    {
	PPrintf( NFMT2, n, node.ln_Type, node.ln_Pri );
    }
}


STRPTR
ShowNodes( char *arg )
{
    struct Node *node;
    struct Node *succ;
    struct Node *thisnode;
    struct Node **nodearray;
    long i;

    if ( parseAddress( arg, (ULONG *)&thisnode ) )
    {
	LinkLock();
	node = thisnode;
	while ( succ = ReadPointer( node ) )
	{
	    /* move to list header */
	    node = succ;
	}

	nodearray = ObtainNodeArray( (struct List *) (((ULONG *)node) - 1), 100 );
	LinkUnlock();
	if ( nodearray )
	{
	    PPrintf( NFMT0 );
	    for ( i = 1; i <= (long) nodearray[ 0 ]; i++ )
	    {
		if ( nodearray[ i ] == thisnode )
		{
		    PPrintf("* ");
		}
		else
		{
		    PPrintf("  ");
		}
		ShowNode( nodearray[ i ] );
	    }
	    ReleaseNodeArray( nodearray );
	}
    }

    return( NULL );
}


/***********************************************************************
*
*   Show all Tasks
*
************************************************************************
* Address NT  Pri Stat  SigWait  SPReg Name
* XXXXXX  DD DDDD SSSS XXXXXXXX XXXXXX SSSSSSSSSSSSSSSSS
***********************************************************************/
#define TFMT0 "Address   NT  Pri Stat  SigWait  SPReg   Name\n"
#define TFMT1 "%08lx  %2ld %4ld %s %08lx %08lx \"%s\""

static char *states[] =
{
    "????",
    " add",
    " run",
    "redy",
    "wait",
    "expt",
    "remv"
};


void
dumpTaskSummary( struct Task *task )
{
    struct Task tc;
    char   name[ NAMESIZE ];
    UBYTE  state;

    ReadBlock( task, &tc, sizeof( struct Task ) );

    if ( ( tc.tc_State < TS_INVALID ) || ( tc.tc_State > TS_REMOVED ) )
    {
	state = 0;
    }
    else
    {
	state = tc.tc_State;
    }

    GetName( tc.tc_Node.ln_Name, name );

    PPrintf( TFMT1, task, tc.tc_Node.ln_Type, tc.tc_Node.ln_Pri,
	    states[ state ], tc.tc_SigWait, tc.tc_SPReg, name );

    if ( tc.tc_Node.ln_Type == NT_PROCESS )
    {
	struct Process pr;
	ReadBlock( task, &pr, sizeof( struct Process ) );
	if ( pr.pr_CLI )
	{
	    PPrintf( " (" );
	    PrintBSTR( &(((struct CommandLineInterface *)LTOB(pr.pr_CLI))->cli_CommandName) );
	    PPrintf( ")" );
	}
    }
    NewLine();
}


STRPTR
ShowTasks( void )
{
    struct ExecBase *ebptr;
    struct ExecBase eb;
    struct Node **readyarray;
    struct Node **waitarray;
    long i;

    LinkLock();
    ebptr = ReadPointer( ABSSYSBASE );
    ReadBlock( ebptr, &eb, sizeof( struct ExecBase ) );
    readyarray = ObtainNodeArray( &ebptr->TaskReady, 100 );
    waitarray = ObtainNodeArray( &ebptr->TaskWait, 100 );
    LinkUnlock();
    if ( ( readyarray ) && ( waitarray ) )
    {
	PPrintf( TFMT0 );
	dumpTaskSummary( eb.ThisTask );
	for ( i = 1; i <= (long)readyarray[ 0 ]; i++ )
	{
	    dumpTaskSummary( (struct Task *)readyarray[ i ] );
	}
	for ( i = 1; i <= (long)waitarray[ 0 ]; i++ )
	{
	    dumpTaskSummary( (struct Task *)waitarray[ i ] );
	}
    }
    ReleaseNodeArray( readyarray );
    ReleaseNodeArray( waitarray );

    return( NULL );
}


/***********************************************************************
*
*   Show all Devices
*
************************************************************************
*  Address NT  Pri Name
*  XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/
#define DFMT0 "Address   NT  Pri Ver Rev Name\n"
#define DFMT1 "%08lx  %2ld %4ld%4ld%4ld \"%s\"\n"

void
ShowDeviceHead( struct Node *device )
{
    struct Library dev; 
    char name[ NAMESIZE ];

    ReadBlock( device, &dev, sizeof( struct Library ) );

    GetName( dev.lib_Node.ln_Name, name );

    PPrintf( DFMT1, device, dev.lib_Node.ln_Type, dev.lib_Node.ln_Pri,
	dev.lib_Version, dev.lib_Revision, name );
}


STRPTR
ShowDeviceList( void )
{
    struct ExecBase *ebptr;
    struct Node **devicearray;
    long i;

    ebptr = ReadPointer( ABSSYSBASE );
    if ( devicearray = ObtainNodeArray( &ebptr->DeviceList, 32 ) )
    {
	PPrintf( DFMT0 );
	for ( i = 1; i <= (long)devicearray[ 0 ]; i++ )
	{
	    ShowDeviceHead( devicearray[ i ] );
	}
	ReleaseNodeArray( devicearray );
    }
    return( NULL );
}


/***********************************************************************
*
*   Show all Resources
*
************************************************************************
*  Address NT  Pri Name
*  XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/

STRPTR
ShowResourceList( void )
{
    struct ExecBase *ebptr;
    struct Node **resourcearray;
    long i;

    ebptr = ReadPointer( ABSSYSBASE );
    if ( resourcearray = ObtainNodeArray( &ebptr->ResourceList, 32 ) )
    {
	PPrintf( DFMT0 );
	for ( i = 1; i <= (long)resourcearray[ 0 ]; i++ )
	{
	    ShowDeviceHead( resourcearray[ i ] );
	}
	ReleaseNodeArray( resourcearray );
    }
    return( NULL );
}


/***********************************************************************
*
*   Show all Libraries
*
************************************************************************
*  Address NT  Pri Name
*  XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/

STRPTR
ShowLibraryList( void )
{
    struct ExecBase *ebptr;
    struct Node **libraryarray;
    long i;

    ebptr = ReadPointer( ABSSYSBASE );
    if ( libraryarray = ObtainNodeArray( &ebptr->LibList, 32 ) )
    {
	PPrintf( DFMT0 );
	for ( i = 1; i <= (long)libraryarray[ 0 ]; i++ )
	{
	    ShowDeviceHead( libraryarray[ i ] );
	}
	ReleaseNodeArray( libraryarray );
    }
    return( NULL );
}


/***********************************************************************
*
*   Show all Ports
*
************************************************************************
*  Address NT  Pri Flags SigTask/Bit MsgList Name
*  XXXXXX  DD  DDD  XX   XXXXXX DDD  XXXXXX SSSSSSSSSSSSSSSSSSSSSSSSSSSSS
***********************************************************************/

#define PFMT0 "Address   NT  Pri Flags SigTask / Bit MsgList   Name\n"
#define PFMT1 "%08lx  %2ld %4ld  %02lx   %08lx %3ld  %08lx \"%s\"\n"

void
ShowPortHead( struct Node *port )
{
    struct MsgPort p;
    char name[ NAMESIZE ];

    ReadBlock( port, &p, sizeof( struct MsgPort ) );
    GetName( p.mp_Node.ln_Name, name );
    PPrintf( PFMT1, port, p.mp_Node.ln_Type, p.mp_Node.ln_Pri,
	p.mp_Flags, p.mp_SigTask, p.mp_SigBit,
	20 + (ULONG) port, name );
}


STRPTR
ShowPortList( void )
{
    struct ExecBase *ebptr;
    struct Node **portarray;
    long i;

    ebptr = ReadPointer( ABSSYSBASE );
    if ( portarray = ObtainNodeArray( &ebptr->PortList, 32 ) )
    {
	PPrintf( PFMT0 );
	for ( i = 1; i <= (long)portarray[ 0 ]; i++ )
	{
	    ShowPortHead( portarray[ i ] );
	}
	ReleaseNodeArray( portarray );
    }
    return( NULL );
}


/***********************************************************************
*
*   Show all Interrupts
*
************************************************************************
*  IV   Data   Code   Node NT Pri Name
*  DD XXXXXX XXXXXX XXXXXX DD DDD SSSSSSSSSSSSSSSSSSSSSSSSSSSSS
***********************************************************************/

#define IFMT0 "IV   Data     Code     Node   NT Name\n"
#define IFMT1 "%2ld %08lx %08lx %08lx %2ld \"%s\"\n"

STRPTR
ShowIntVects( void )
{
    struct ExecBase eb;
    long i;
    struct Node node;
    char name[ NAMESIZE ];
    char *n;

    LinkLock();
    ReadBlock( ReadPointer( ABSSYSBASE ), &eb, sizeof( struct ExecBase ) );
    PPrintf( IFMT0 );
    for ( i = 0; i < 16; i++ )
    {
        name[ 0 ] = 0;
	n = name;
	if ( eb.IntVects[ i ].iv_Node != 0 )
	{
	    ReadBlock( eb.IntVects[i].iv_Node, &node, sizeof( struct Node ) );
	    GetName( node.ln_Name, name );
	}
	else
	{
	    if ( eb.IntVects[ i ].iv_Code != 0 )
	    {
		n = "server-chain";
	    }
	}

	PPrintf( IFMT1, i, eb.IntVects[ i ].iv_Data, eb.IntVects[ i ].iv_Code,
		eb.IntVects[ i ].iv_Node, node.ln_Type, n );
    }
    LinkUnlock();

    return( NULL );
}


/***********************************************************************
*
*   Show all Memory Regions
*
************************************************************************
***********************************************************************/
#define FMT_REGION "%8lx attr %04lx lower %8lx upper %8lx first %8lx free %ld.\n"

void
ShowRegion( struct MemHeader *region )
{
    struct MemHeader mem;

    ReadBlock( region, &mem, sizeof( struct MemHeader ) );

    PPrintf( FMT_REGION, region, mem.mh_Attributes, 
	    mem.mh_Lower, mem.mh_Upper, mem.mh_First, mem.mh_Free );
}

STRPTR
ShowRegionList( void )
{
    struct ExecBase eb;
    struct MemHeader *node;
    struct MemHeader *succ;

    ReadBlock( ReadPointer( ABSSYSBASE ), &eb, sizeof ( struct ExecBase ) );
    for (node = (struct MemHeader *)eb.MemList.lh_Head;
	succ = ReadPointer( node );
	node = succ )
    {
	ShowRegion( node );
    }

    return( NULL );
}


/***********************************************************************
*
*   Show all Memory Chunks
*
************************************************************************
***********************************************************************/
#define	FMT_TITLE \
"   -----addr--ALLOCATED---size--------   -----addr-----FREE-----size--------\n"
#define	FMT_END   "   %8lx-%-8lx %8lx #%-8ld\n"
#define FMT_CHUNK "   %8lx-%-8lx %8lx #%-8ld  %8lx-%-8lx %8lx #%-10ld\n"

void
ShowMemory( struct MemRegion *region )
{
    ULONG *chunk;
    struct MemHeader mem;
    long size;
    long lastEnd;

    ShowRegion( region );

    ReadBlock( region, &mem, sizeof ( struct MemHeader ) );
    lastEnd = (long) mem.mh_Lower;

    PPrintf( FMT_TITLE );
    for ( chunk = (ULONG *) mem.mh_First; chunk; chunk = ReadPointer( chunk ) )
    {
	size = ReadLong( chunk + 1 );
	PPrintf( FMT_CHUNK, lastEnd, ( (long)chunk ) - 1,
	    ( (long) chunk ) - lastEnd, ( (long) chunk ) - lastEnd,
	    chunk, ( (long) chunk ) + size - 1, size, size );
	lastEnd = ( (long) chunk ) + size;
    }
    if ( ( ( (long)mem.mh_Upper ) - lastEnd ) != 0 )
    {
	PPrintf( FMT_END, lastEnd, ( (long)mem.mh_Upper ) - 1,
	    ( (long)mem.mh_Upper ) - lastEnd, ( (long)mem.mh_Upper ) - lastEnd );
    }
    PPrintf( FMT_TITLE );
}


STRPTR
ShowMemoryList( void )
{
    struct ExecBase *ebptr;
    struct Node **memoryarray;
    long i;

    LinkLock();
    ebptr = ReadPointer( ABSSYSBASE );
    if ( memoryarray = ObtainNodeArray( &ebptr->MemList, 100 ) )
    {
	for ( i = 1; i <= (long)memoryarray[ 0 ]; i++ )
	{
	    ShowMemory( memoryarray[ i ] );
	}
	ReleaseNodeArray( memoryarray );
    }
    LinkUnlock();

    return( NULL );
}


/***********************************************************************
*
*   Show Resident Modules
*
************************************************************************
***********************************************************************/
#define RMFMT0 "Address   Pri Type Ver Name\n"
#define RMFMT1 "%08lx  %4ld%4ld%4ld %s"

void
ShowModule( struct Resident *m )
{
    struct Resident r;
    char name[ NAMESIZE ];
    int len;

    ReadBlock( m, &r, sizeof( struct Resident ) );
    GetName( r.rt_IdString, name );

    /* Strip trailing newline character, which sometimes is and sometimes
     * isn't present.  This will allow us to have consistent output from
     * inconsistent RomTag IdStrings.
     */
    if ( len = strlen( name ) )
    {
	if ( name[ len - 1 ] == '\n' )
	{
	    name[ len - 1 ] = '\0';
	}
    }

    /* Most RomTags have a trailing ^M in the string, which we should
     * also nuke for cleaner output.
     */
    if ( len = strlen( name ) )
    {
	if ( name[ len - 1 ] == 0x0D )
	{
	    name[ len - 1 ] = '\0';
	}
    }

    PPrintf( RMFMT1, m, r.rt_Pri, r.rt_Type, r.rt_Version, name );
    if ( ( strlen( name ) ) && ( name[ strlen( name ) - 1 ] != '\n' ) )
    {
	NewLine();
    }
}


STRPTR
ShowResModules( void )
{
    struct ExecBase eb;
    long i;
    ULONG mods[100];
    APTR modbase;

    ReadBlock( ReadPointer( ABSSYSBASE ), &eb, sizeof( struct ExecBase ) );
    PPrintf( RMFMT0 );

    modbase = (APTR) eb.ResModules;


    while( modbase )
    {
	ReadBlock( modbase, &mods[0], sizeof( mods ) );
	modbase = 0;
    
	for ( i = 0; mods[ i ] != 0; i++ )
	{
	    if( mods[ i ] & ( 1<<31 ) )
	    {
		/* link field */
		modbase = (APTR) ( mods[ i ] & ~(1<<31) );
		break;
	    }
	
	    ShowModule( (struct Resident *)mods[ i ] );
	}
    }

    return( NULL );
}


/***********************************************************************
*
*   ShowTask
*
***********************************************************************/

long
compareTaskName( struct Task *task, char *arg )
{
    struct Task tc;
    struct Process pr;
    char name[ NAMESIZE ];
    long found = FALSE;

    ReadBlock( task, &tc, sizeof( struct Task ) );
    GetName( tc.tc_Node.ln_Name, name );
    if ( strstr( name, arg ) )
    {
	found = TRUE;
    }
    else if ( tc.tc_Node.ln_Type == NT_PROCESS )
    {
	ReadBlock( task, &pr, sizeof( struct Process ) );
	if ( pr.pr_CLI )
	{
	    GetBSTR( (APTR)(ReadLong( &(((struct CommandLineInterface *)LTOB(pr.pr_CLI))->cli_CommandName) ) << 2),
		name, NAMESIZE );
	    if ( strstr( name, arg ) )
	    {
		found = TRUE;
	    }
	}
    }
    return( found );
}


struct Task *
findTask( char *arg )
{
    struct ExecBase *ebptr;
    struct Task *task;
    struct Node **readyarray;
    struct Node **waitarray;
    long i;
    long found;

    LinkLock();
    ebptr = ReadPointer( ABSSYSBASE );
    readyarray = ObtainNodeArray( &ebptr->TaskReady, 100 );
    waitarray = ObtainNodeArray( &ebptr->TaskWait, 100 );
    task = ReadPointer( &ebptr->ThisTask);
    LinkUnlock();
    found = compareTaskName( task, arg );
    if ( ( readyarray ) && ( waitarray ) )
    {
	i = 1;
	while ( ( !found ) && ( i <= (long)readyarray[ 0 ] ) )
	{
	    task = (struct Task *)readyarray[ i ];
	    found = compareTaskName( task, arg );
	    i++;
	}
	i = 1;
	while ( ( !found ) && ( i <= (long)waitarray[ 0 ] ) )
	{
	    task = (struct Task *)waitarray[ i ];
	    found = compareTaskName( task, arg );
	    i++;
	}
    }
    if ( !found )
    {
	task = NULL;
    }
    ReleaseNodeArray( readyarray );
    ReleaseNodeArray( waitarray );

    return( task );
}


#define	tft0 "'%s' at %lx (in %s state at priority %ld)\n"
#define tft1 "Type %ld  Flags %02lx  IDNestCnt %ld  TDNestCnt %ld\n"
#define tft2 "Signal:  Alloc %08lx  Wait  %08lx  Recvd %08lx  Except %08lx\n"
#define tft3 "Trap:    Data  %08lx  Code  %08lx  Alloc %04lx  Able %04lx\n"
#define	tft4 "Except:  Data  %08lx  Code  %08lx\n"
#define	tft5 "Stack:   Reg   %08lx  Lower %08lx  Upper %08lx\n"
#define tft6 "CPURegs: PC    %08lx  SR    %04lx\n"  

void
dumpTask( struct Task *task )
{
    struct Task tc;
    char name[100];
    UBYTE state;
    long sp;
    short reg;

    LinkLock();
    ReadBlock( task, &tc, sizeof( struct Task ) );

    if ( ( tc.tc_State < TS_INVALID ) || ( tc.tc_State > TS_REMOVED ) )
    {
	state = 0;
    }
    else
    {
	state = tc.tc_State;
    }

    GetName( tc.tc_Node.ln_Name, name );
    PPrintf( tft0, name, task, states[ state ], tc.tc_Node.ln_Pri );

    PPrintf( tft1, tc.tc_Node.ln_Type, tc.tc_Flags, tc.tc_IDNestCnt,
	tc.tc_TDNestCnt );

    PPrintf( tft2, tc.tc_SigAlloc, tc.tc_SigWait, tc.tc_SigRecvd,
	tc.tc_SigExcept );

    PPrintf( tft3, tc.tc_TrapData, tc.tc_TrapCode, tc.tc_TrapAlloc,
	tc.tc_TrapAble );

    PPrintf( tft4, tc.tc_ExceptData, tc.tc_ExceptCode );

    PPrintf( tft5, tc.tc_SPReg, tc.tc_SPLower, tc.tc_SPUpper );

    if ( ( tc.tc_State == TS_READY ) || ( tc.tc_State == TS_WAIT ) )
    {
	sp = (long)tc.tc_SPReg;
	PPrintf( tft6, ReadPointer( (APTR) sp ), ReadWord( (APTR)( sp + 4 ) ) );

	PPrintf( "D: " );
	for ( reg = 0; reg < 8; reg++)
	{
	    PPrintf( "%08lx ", ReadPointer( (APTR)( sp + 6 + reg * 4 ) ) );
	}

	PPrintf( "\nA: " );
	for ( reg = 8; reg < 15; reg++ )
	{
	    PPrintf( "%08lx ", ReadPointer( (APTR)( sp + 6 + reg * 4 ) ) );
	}
    }
    NewLine();
    LinkUnlock();
}


STRPTR
ShowTask( char *arg )
{
    struct Task *task;

    if ( ( parseAddress( arg, (unsigned long *)&task ) ) ||
	( task = findTask( arg ) ) )
    {
	dumpTask( task );
    }
    else
    {
	PPrintf("Task not found\n");
    }

    return( NULL );
}


/***********************************************************************
*
*   ShowProcess
*
***********************************************************************/

#define pft0 "SegList      %08lx  StackSize    %08lx  GlobVec  %08lx\n"
#define pft1 "TaskNum      %8ld  StackBase    %08lx  Result2  %8ld\n"
#define pft2 "CurrentDir   %08lx  CIS          %08lx  COS      %08lx\n"
#define pft3 "ConsoleTask  %08lx  FileSysTask  %08lx  CLI      %08lx\n"
#define pft4 "ReturnAddr   %08lx  PktWait      %08lx\n"


void
dumpProcess( struct Process *proc )
{
    struct Process pr;
    struct Task tc;

    ReadBlock( proc, &tc, sizeof( struct Task ) );
    if ( tc.tc_Node.ln_Type != NT_PROCESS )
    {
	PPrintf( "(NOT A PROCESS!)\n" );
	dumpTask( (struct Task *)proc );
	return;
    }

    LinkLock();
    dumpTask( (struct Task *)proc );
    PPrintf( "Process Dependent ------------------------------------------\n" );
    ReadBlock( proc, &pr, sizeof( struct Process ) );
    if ( pr.pr_CLI )
    {
	PPrintf( "cli_CommandName: " );
	PrintBSTR( &(((struct CommandLineInterface *)LTOB(pr.pr_CLI))->cli_CommandName) );
	NewLine();
    }
    PPrintf( pft0, LTOB( pr.pr_SegList ), pr.pr_StackSize, pr.pr_GlobVec );
    PPrintf( pft1, pr.pr_TaskNum, LTOB( pr.pr_StackBase ), pr.pr_Result2 );
    PPrintf( pft2, LTOB( pr.pr_CurrentDir ), LTOB( pr.pr_CIS ), LTOB( pr.pr_COS ) );
    PPrintf( pft3, pr.pr_ConsoleTask, pr.pr_FileSystemTask, LTOB( pr.pr_CLI ) );
    PPrintf( pft4, pr.pr_ReturnAddr, pr.pr_PktWait );
    LinkUnlock();
}


STRPTR
ShowProcess( char *arg )
{
    struct Process *proc;

    if ( ( parseAddress( arg, (unsigned long *)&proc ) ) ||
	( proc = (struct Process *)findTask( arg ) ) )
    {
	dumpProcess( proc );
    }
    else
    {
	PPrintf( "Process not found\n" );
    }

    return( NULL );
}


/***********************************************************************
*
*   Show ExecBase
*
***********************************************************************/

#define ebf0 "SoftVer %3ld\n"
#define ebfa "ChkSums: Library %s  SysBase %s  LowMem %s  Critical %s\n" 
#define ebf1 "ColdCapture  %08lx  CoolCapture  %08lx  WarmCapture  %08lx\n"
#define ebf2 "SysStkUpper  %08lx  SysStkLower  %08lx  MaxLocMem    %08lx\n"
#define ebf3 "DebugEntry   %08lx  DebugData    %08lx  AlertData    %08lx\n"

#define ebf4 "ThisTask     %08lx  SysFlags         %04lx\n"
#define ebf5 "IdleCount    %8ld  DispCount    %8ld\n"
#define ebf6 "Quantum         %5ld  Elapsed         %5ld\n"
#define ebf7 "IDNestCnt        %4ld  TDNestCnt        %4ld\n"
#define ebf8 "TrapCode     %08lx  ExceptCode   %08lx  ExitCode     %08lx\n"
#define ebf9 "SigAlloc     %08lx  TrapAlloc        %04lx\n"
#define ebfA "AttnFlags CPU: %s FPU: %s\n"

STRPTR
ShowExecBase( void )
{
    struct ExecBase eb;
    char *ok1;
    char *ok2;
    char *ok3;
    char *ok4;
    char *cpu;
    char *fpu;

    LinkLock();
    ReadBlock( ReadPointer( ABSSYSBASE ), &eb, sizeof( struct ExecBase ) );
    SpareAddr = ReadPointer( ABSSYSBASE );
    ok1 = ok2 = ok3 = ok4 = "???";
    if ( (long)( ReadLong( ABSSYSBASE ) + eb.ChkBase ) == -1 )
    {
	ok2 = "ok ";
    }
    PPrintf( ebf0, eb.SoftVer );
    PPrintf( ebfa, ok1, ok2, ok3, ok4 );
    PPrintf( ebf1, eb.ColdCapture, eb.CoolCapture, eb.WarmCapture );
    PPrintf( ebf2, eb.SysStkUpper, eb.SysStkLower, eb.MaxLocMem );
    PPrintf( ebf3, eb.DebugEntry, eb.DebugData, eb.AlertData );
    PPrintf( ebf4, eb.ThisTask, eb.SysFlags );
    PPrintf( ebf5, eb.IdleCount, eb.DispCount );
    PPrintf( ebf6, eb.Quantum, eb.Elapsed );
    PPrintf( ebf7, eb.IDNestCnt, eb.TDNestCnt );
    PPrintf( ebf8, eb.TaskTrapCode, eb.TaskExceptCode, eb.TaskExitCode );
    PPrintf( ebf9, eb.TaskSigAlloc, eb.TaskTrapAlloc );
    cpu = "68000";

    if ( eb.AttnFlags & AFF_68040 )
    {
	cpu = "68040";
    }
    else if ( eb.AttnFlags & AFF_68030 )
    {
	cpu = "68030";
    }
    else if ( eb.AttnFlags & AFF_68020 )
    {
	cpu = "68020";
    }
    else if ( eb.AttnFlags & AFF_68010 )
    {
	cpu = "68010";
    }

    fpu = "none";

    if ( eb.AttnFlags & AFF_68882 )
    {
	fpu = "68882";
    }
    else if ( eb.AttnFlags & AFF_68881 )
    {
	fpu = "68881";
    }
    else if ( ( eb.AttnFlags & ( AFF_68881 | AFF_68882 | AFF_FPU40 | AFF_68040 ) ) ==
	( AFF_FPU40 | AFF_68040 ) )
    {
	fpu = "68040";
    }

    PPrintf( ebfA, cpu, fpu );
    LinkUnlock();

    return( NULL );
}





