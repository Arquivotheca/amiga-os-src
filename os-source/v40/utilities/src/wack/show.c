/* $Id: show.c,v 1.6 92/08/27 12:08:54 peter Exp $	*/

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


#define LTOB(b_ptr) (b_ptr << 2)

long GetMemL();
extern APTR CurrAddr;
extern APTR SpareAddr;
extern struct Task *LastTask;
extern long FrameSize;

/* #define FMT0 "%06lx  pri=%-3d  '%s'\n"
   #define FMT1 "%06lx %-7s  pri=%-3d sp=%06lx  '%s'\n" */

#define NAMESIZE 100

GetName (nameAddr, nameStr)
    APTR nameAddr;
    char *nameStr;
{
    if (nameAddr == 0) {
	nameStr[0] = 0;
    }
    else {
	GetBlock (nameAddr, nameStr, NAMESIZE);
	nameStr[NAMESIZE - 1] = 0;
    }
}

/***********************************************************************
*
*   Show Node
*
************************************************************************
*    Address NT  Pri Name
*  C XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/
#define NFMT0 "\n  Address NT  Pri Name\n"
#define NFMT1 "%lc %06lx  %2ld %4ld \"%s\"\n"

ShowNode(n)
    struct Node *n;
{
    struct Node node;
    char    name[NAMESIZE];

    GetBlock (n, &node, sizeof (node));
    GetName (node.ln_Name, name);

    Print (NFMT1, ((APTR) n == CurrAddr) ? '*' : ' ', n,
	    node.ln_Type, node.ln_Pri, name);
}

ShowNodes()
{
    ULONG *n1;

    Forbid();
    n1 = (ULONG *) CurrAddr;
    NewLine ();

    while (GetMemL (n1)) {	/* move to list header */
	n1 = (ULONG *)GetMemL (n1);
    }

    Print (NFMT0);
    for (n1 = (ULONG *)GetMemL (n1 - 1); (ULONG *)GetMemL (n1); n1 = (ULONG *)GetMemL (n1)) {
	ShowNode (n1);
    }
    Permit();
}


/***********************************************************************
*
*   Show all Tasks
*
************************************************************************
* Address NT  Pri Stat  SigWait  SPReg Name
* XXXXXX  DD DDDD SSSS XXXXXXXX XXXXXX SSSSSSSSSSSSSSSSS
***********************************************************************/
#define TFMT0 "\nAddress NT  Pri Stat  SigWait  SPReg Name\n"
#define TFMT1 "%06lx  %2ld %4ld %s %08lx %06lx \"%s\"\n"

static char *states[] = {
    "????",
    " add",
    " run",
    "redy",
    "wait",
    "expt",
    "remv"
};

ShowTaskHead(task)
    struct Task *task;
{
    struct Task tc;
    char   name[NAMESIZE];
    UBYTE  state;

    GetBlock (task, &tc, sizeof (tc));

    if (tc.tc_State < TS_INVALID | tc.tc_State > TS_REMOVED)
	state = 0;
    else
	state = tc.tc_State;

    GetName (tc.tc_Node.ln_Name, name);
    Print (TFMT1, task, tc.tc_Node.ln_Type, tc.tc_Node.ln_Pri,
	    states[state], tc.tc_SigWait, tc.tc_SPReg, name);
}


ShowTaskList(l)
    struct Node *l;
{
    struct Node *node;

    for (node = l; GetMemL (node); (node = (struct Node *) GetMemL (node)))
	ShowTaskHead (node);
}


ShowTasks()
{
    struct ExecBase eb;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));

    Print(TFMT0);
    ShowTaskHead (eb.ThisTask);
    ShowTaskList (eb.TaskReady);
    ShowTaskList (eb.TaskWait);
    Permit();
}


/***********************************************************************
*
*   Show all Devices
*
************************************************************************
*  Address NT  Pri Name
*  XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/
#define DFMT0 "\nAddress NT  Pri Ver Rev Name\n"
#define DFMT1 "%06lx  %2ld %4ld%4ld%4ld \"%s\"\n"

ShowDeviceHead(device)
    struct Node *device;
{
    struct Library dev; 
    char name[NAMESIZE];

    GetBlock (device, &dev, sizeof (dev));

    GetName (dev.lib_Node.ln_Name, name);

    Print (DFMT1, device, dev.lib_Node.ln_Type, dev.lib_Node.ln_Pri,
	dev.lib_Version, dev.lib_Revision, name);
}

ShowDeviceList()
{
    struct ExecBase eb;
    long    i;
    struct Node *node;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));
    Print (DFMT0);
    for (node = (struct Node *) eb.DeviceList.lh_Head; GetMemL (node);
	    node = (struct Node *) GetMemL (node)) {
	                    ShowDeviceHead (node);
    }
    Permit();
}

/***********************************************************************
*
*   Show all Resources
*
************************************************************************
*  Address NT  Pri Name
*  XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/

ShowResourceList()
{
    struct ExecBase eb;
    long    i;
    struct Node *node;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));
    Print (DFMT0);
    for (node = (struct Node *) eb.ResourceList.lh_Head; GetMemL (node);
	    node = (struct Node *) GetMemL (node)) {
	                    ShowDeviceHead (node);
    }
    Permit();
}

/***********************************************************************
*
*   Show all Libraries
*
************************************************************************
*  Address NT  Pri Name
*  XXXXXX  DD DDDD SSSSSSSSSSSSSSSSS
***********************************************************************/

ShowLibraryList()
{
    struct ExecBase eb;
    long   i;
    struct Node *node;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));
    Print (DFMT0);
    for (node = (struct Node *) eb.LibList.lh_Head; GetMemL (node);
	    node = (struct Node *) GetMemL (node)) {
	                    ShowDeviceHead (node);
    }
    Permit();
}

/***********************************************************************
*
*   Show all Ports
*
************************************************************************
*  Address NT  Pri Flags SigTask/Bit MsgList Name
*  XXXXXX  DD  DDD  XX   XXXXXX DDD  XXXXXX SSSSSSSSSSSSSSSSSSSSSSSSSSSSS
***********************************************************************/

#define PFMT0 "\nAddress NT  Pri Flags SigTask/Bit MsgList Name\n"
#define PFMT1 "%06lx  %2ld %4ld  %02lx   %06lx %3ld  %06lx \"%s\"\n"

ShowPortHead(port)
    struct Node *port;
{
    struct MsgPort p;
    char name[NAMESIZE];

    GetBlock (port, &p, sizeof (p));
    GetName (p.mp_Node.ln_Name, name);
    Print (PFMT1, port, p.mp_Node.ln_Type, p.mp_Node.ln_Pri,
		 p.mp_Flags, p.mp_SigTask, p.mp_SigBit,
		 20 + (ULONG) port, name);
}

ShowPortList()
{
    struct ExecBase eb;
    long    i;
    struct Node *node;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));
    Print(PFMT0);
    for (node = (struct Node *) eb.PortList.lh_Head; GetMemL (node);
	    node = (struct Node *) GetMemL (node)) {
	                    ShowPortHead (node);
    }
    Permit();
}


/***********************************************************************
*
*   Show all Interrupts
*
************************************************************************
*  IV   Data   Code   Node NT Pri Name
*  DD XXXXXX XXXXXX XXXXXX DD DDD SSSSSSSSSSSSSSSSSSSSSSSSSSSSS
***********************************************************************/

#define IFMT0 "\nIV   Data   Code   Node NT Name\n"
#define IFMT1 "%2ld %06lx %06lx %06lx %2ld \"%s\"\n"

ShowIntVects()
{
    struct ExecBase eb;
    long    i;
    struct Node node;
    char    name[NAMESIZE];
    char    *n;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));
    Print (IFMT0);
    for (i = 0; i < 16; i++) {
        name[0] = 0;
	n = name;
	if (eb.IntVects[i].iv_Node != 0) {
	    GetBlock (eb.IntVects[i].iv_Node, &node, sizeof (node));
	    GetName (node.ln_Name, name);
	}
	else {
	    if (eb.IntVects[i].iv_Code != 0) {
	    n = "server-chain";
	    }
	}

	Print (IFMT1, i, eb.IntVects[i].iv_Data, eb.IntVects[i].iv_Code,
		eb.IntVects[i].iv_Node, node.ln_Type, n);
    }
    Permit();
}


/***********************************************************************
*
*   Show all Memory Regions
*
************************************************************************
***********************************************************************/
#define FMT_REGION "%8lx attr %04lx lower %8lx upper %8lx first %8lx free %ld.\n"

ShowRegion(region)
    struct MemHeader *region;
{
    struct MemHeader  mem;

    GetBlock (region, &mem, sizeof (struct MemHeader));

    Print (FMT_REGION, region, mem.mh_Attributes, 
	    mem.mh_Lower, mem.mh_Upper, mem.mh_First, mem.mh_Free);
}

ShowRegionList()
{
    struct ExecBase eb;
    long    i;
    struct Node *node;

    GetBlock (GetMemL (4), &eb, sizeof (eb));
    NewLine ();
    for (node = (struct Node *) eb.MemList.lh_Head; GetMemL (node);
	    node = (struct Node *) GetMemL (node)) {
	                    ShowRegion (node);
    }
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

ShowMemory(region)
    struct MemRegion *region;
{
    ULONG 	*chunk;
    struct MemHeader    mem;
    long    size, lastEnd;
    ShowRegion (region);

    GetBlock (region, &mem, sizeof (struct MemHeader));
    lastEnd = (long) mem.mh_Lower;

    Print(FMT_TITLE);
    for (chunk = (ULONG *) mem.mh_First; chunk; chunk = (ULONG *) GetMemL (chunk)) {
	size = (long) GetMemL (chunk+1);
	Print (FMT_CHUNK, lastEnd, ((long)chunk)-1,
		((long)chunk)-lastEnd, ((long)chunk)-lastEnd,
		chunk, ((long) chunk)+size-1, size, size);
	lastEnd = ((long) chunk)+size;
    }
    if ((((long)mem.mh_Upper)-lastEnd) != 0)
	Print (FMT_END, lastEnd, ((long)mem.mh_Upper)-1,
		((long)mem.mh_Upper)-lastEnd, ((long)mem.mh_Upper)-lastEnd);
    Print(FMT_TITLE);
}

ShowMemoryList()
{
    struct ExecBase eb;
    long    i;
    struct Node *node;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));
    NewLine ();
    for (node = (struct Node *) eb.MemList.lh_Head; GetMemL (node);
	    node = (struct Node *) GetMemL (node)) {
	                    ShowMemory (node);
    }
    Permit();
}


/***********************************************************************
*
*   Show Resident Modules
*
************************************************************************
***********************************************************************/
#define RMFMT0 "\nAddress Pri Type Ver Name\n"
#define RMFMT1 "%06lx  %4ld%4ld%4ld %s"

ShowModule(m)
    struct Resident *m;
{
    struct Resident r;
    char name[NAMESIZE];

    GetBlock (m, &r, sizeof (r));
    GetName (r.rt_IdString, name);

    Print (RMFMT1, m, r.rt_Pri, r.rt_Type, r.rt_Version, name);
}

ShowResModules()
{
    struct ExecBase eb;
    long    i;
    struct Node *node;
    ULONG  mods[100];
    ULONG  modbase;

    GetBlock (GetMemL (4), &eb, sizeof (eb));
    Print (RMFMT0);

    modbase = (ULONG) eb.ResModules;


    while( modbase ) {
	GetBlock (modbase, &mods[0], sizeof (mods));
	modbase = 0;
    
	for (i = 0; mods[i] != 0; i++) {
	    if( mods[i] & (1<<31) ) {
		/* link field */
		modbase = mods[i] & ~(1<<31);
		break;
	    }
	
	    ShowModule (mods[i]);
	}
    }
}


/***********************************************************************
*
*   ShowTask
*
***********************************************************************/

#define	tft0 "\n'%s' in %s state at priority %ld\n"
#define tft1 "Type %ld  Flags %02lx  IDNestCnt %ld  TDNestCnt %ld\n"
#define tft2 "Signal:  Alloc %08lx  Wait  %08lx  Recvd %08lx  Except %08lx\n"
#define tft3 "Trap:    Data  %08lx  Code  %08lx  Alloc %04lx  Able %04lx\n"
#define	tft4 "Except:  Data  %08lx  Code  %08lx\n"
#define	tft5 "Stack:   Reg   %08lx  Lower %08lx  Upper %08lx\n"
#define tft6 "CPURegs: PC    %08lx  SR    %04lx\n"  

ShowTask()
{
    struct Task tc;
    char    name[100];
    UBYTE state;
    long    sp;
    short   reg;

    LastTask = (struct Task *)CurrAddr;

    Forbid();
    GetBlock (CurrAddr, &tc, sizeof (tc));

    if (tc.tc_State < TS_INVALID | tc.tc_State > TS_REMOVED)
	state = 0;
    else
	state = tc.tc_State;

    GetName (tc.tc_Node.ln_Name, name);
    Print (tft0, name, states[state], tc.tc_Node.ln_Pri, (long) CurrAddr);

    Print (tft1, tc.tc_Node.ln_Type, tc.tc_Flags, tc.tc_IDNestCnt,
	    tc.tc_TDNestCnt);

    Print (tft2, tc.tc_SigAlloc, tc.tc_SigWait, tc.tc_SigRecvd,
	    tc.tc_SigExcept);

    Print (tft3, tc.tc_TrapData, tc.tc_TrapCode, tc.tc_TrapAlloc,
	    tc.tc_TrapAble);

    Print (tft4, tc.tc_ExceptData, tc.tc_ExceptCode);

    Print (tft5, tc.tc_SPReg, tc.tc_SPLower, tc.tc_SPUpper);

    if (tc.tc_State == TS_READY | tc.tc_State == TS_WAIT) {
	sp = (long) tc.tc_SPReg;
	Print (tft6, GetMemL (sp), GetMem (sp + 4));

	Print ("D: ");
	for (reg = 0; reg < 8; reg++) {
	    Print ("%08lx ", GetMemL (sp + 6 + reg * 4));
	}

	Print ("\nA: ");
	for (reg = 8; reg < 15; reg++) {
	    Print ("%08lx ", GetMemL (sp + 6 + reg * 4));
	}
    }
    NewLine ();
    Permit();
/*    do_decode (); */
}


/***********************************************************************
*
*   ShowProcess
*
***********************************************************************/

#define pft0 "SegList      %06lx  StackSize    %06lx  GlobVec  %06lx\n"
#define pft1 "TaskNum      %6ld  StackBase    %06lx  Result2  %6ld\n"
#define pft2 "CurrentDir   %06lx  CIS          %06lx  COS      %06lx\n"
#define pft3 "ConsoleTask  %06lx  FileSysTask  %06lx  CLI      %06lx\n"
#define pft4 "ReturnAddr   %06lx  PktWait      %06lx\n"

ShowProcess()
{
    struct Process  pr;
    struct Task tc;

    GetBlock (CurrAddr, &tc, sizeof (tc));
    if (tc.tc_Node.ln_Type != NT_PROCESS) {
	Print ("\n(NOT A PROCESS!)");
        ShowTask();
	return;
    }

    Forbid();
    ShowTask();
    Print ("Process Dependent ------------------------------------------\n");
    GetBlock (CurrAddr, &pr, sizeof (pr));
    if (pr.pr_CLI)
    {
	Print( "cli_CommandName: " );
	PrintBSTR( &(((struct CommandLineInterface *)LTOB(pr.pr_CLI))->cli_CommandName) );
	Print("\n");
    }
    Print (pft0, LTOB(pr.pr_SegList), pr.pr_StackSize, pr.pr_GlobVec);
    Print (pft1, pr.pr_TaskNum, LTOB(pr.pr_StackBase), pr.pr_Result2);
    Print (pft2, LTOB(pr.pr_CurrentDir), LTOB(pr.pr_CIS), LTOB(pr.pr_COS));
    Print (pft3, pr.pr_ConsoleTask, pr.pr_FileSystemTask, LTOB(pr.pr_CLI));
    Print (pft4, pr.pr_ReturnAddr, pr.pr_PktWait);
    Permit();
}

/* Display stack pointer of last task looked at, and put it in the spare
 * address.
 */
ShowSP()
{
    struct Task tc;

    if ( LastTask )
    {
	Forbid();
	GetBlock (LastTask, &tc, sizeof (tc));

	SpareAddr = (APTR) tc.tc_SPReg;
	Permit();

	printf(" of task/process %08lx is at %08lx\n", LastTask, SpareAddr );
    }
    else
    {
	printf(" of what task?\n");
    }
}

/* Starting at the current address, find anything that looks like a ROM
 * address.  Useful in stack-backtrace.
 */
FindROMAddrs()
{
    unsigned long addr = (long) CurrAddr;
    unsigned long data;

    Print (" from %08lx\n", addr );
    while ( ( addr - ((unsigned long)CurrAddr) ) < FrameSize )
    {
	data = GetMemL( addr );
	/* Looks like a ROM address */
	if ( ( data & 0xFFF00000 ) == 0x00f00000 )
	{
	    Print ("%08lx\n", data );
	}

	/* Go to next word */
	addr += 2;
    }
    CurrAddr = (APTR) addr;
}

/***********************************************************************
*
*   Show ExecBase
*
***********************************************************************/

#define ebf0 "SoftVer %3ld\n"
#define ebfa "ChkSums: Library %s  SysBase %s  LowMem %s  Critical %s\n" 
#define ebf1 "ColdCapture  %06lx  CoolCapture  %06lx  WarmCapture  %06lx\n"
#define ebf2 "SysStkUpper  %06lx  SysStkLower  %06lx  MaxLocMem    %06lx\n"
#define ebf3 "DebugEntry   %06lx  DebugData    %06lx  AlertData    %06lx\n"

#define ebf4 "ThisTask     %06lx  SysFlags       %04lx\n"
#define ebf5 "IdleCount  %8ld  DispCount  %8ld\n"
#define ebf6 "Quantum       %5ld  Elapsed       %5ld\n"
#define ebf7 "IDNestCnt      %4ld  TDNestCnt      %4ld\n"
#define ebf8 "TrapCode     %06lx  ExceptCode   %06lx  ExitCode  %06lx\n"
#define ebf9 "SigAlloc   %08lx  TrapAlloc      %04lx\n"
#define ebfA "AttnFlags %s\n"
ShowExecBase()
{
    struct ExecBase eb;
    struct Node *node;
    char   *ok1,
           *ok2,
           *ok3,
           *ok4;
    char   *cpu;

    Forbid();
    GetBlock (GetMemL (4), &eb, sizeof (eb));
    SpareAddr = (APTR) GetMemL (4);
    NewLine ();
    ok1 = ok2 = ok3 = ok4 = "???";
    if ((long)(GetMemL (4) + eb.ChkBase) == -1) ok2 = "ok ";
    Print (ebf0, eb.SoftVer);
    Print (ebfa, ok1, ok2, ok3, ok4);
    Print (ebf1, eb.ColdCapture, eb.CoolCapture, eb.WarmCapture);
    Print (ebf2, eb.SysStkUpper, eb.SysStkLower, eb.MaxLocMem);
    Print (ebf3, eb.DebugEntry, eb.DebugData, eb.AlertData);
    Print (ebf4, eb.ThisTask, eb.SysFlags);
    Print (ebf5, eb.IdleCount, eb.DispCount);
    Print (ebf6, eb.Quantum, eb.Elapsed);
    Print (ebf7, eb.IDNestCnt, eb.TDNestCnt);
    Print (ebf8, eb.TaskTrapCode, eb.TaskExceptCode, eb.TaskExitCode);
    Print (ebf9, eb.TaskSigAlloc, eb.TaskTrapAlloc);
    switch (eb.AttnFlags & 3) {
	case 1: 			
		cpu = "CPU=68010";
		break;
	case 3:
		cpu = "CPU=68020";
		break;
	default:
		cpu = "CPU=68000";
    }
    Print (ebfA, cpu);
    Permit();
}
