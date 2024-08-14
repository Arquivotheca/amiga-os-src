
/*

v37.3 revs - io_main.c - show cli command name if present
	     io_torture.asm - allow NT_MESSAGE if LN_PRED is NULL
	     add safe pull-out logic
v37.4 revs - make io_torture and io_torture.par
v37.5 revs - fix taskname() routine
v37.6 revs - fix taskname() more
v37.7 revs - use debug.lib and ddebug.lib (not RawPrintf asm modules)

Current plan:
	SendIO() - Check that message is free.  Check for ReplyPort.
	Be sure request is not linked into a list.

	BeginIO() - Check that message is free.  Check for ReplyPort.
	Be sure request is not linked into a list.

	OpenDevice() - Mark message as free.  If error, trash IO_DEVICE,
	IO_UNIT and LN_TYPE.
	Be sure request is not linked into a list.

	CloseDevice() - Check that message is free.  Trash IO_DEVICE,
	IO_UNIT.


Possible future tortures include:
	1> Randomly changing a QUICK request to non-quick.
	2> Randomly failing an IORequest.
	3> Tracking the use of AbortIO() and WaitIO().  Perhaphs
	   insisting on a WaitIO() on a request before
	   reuse of an IORequest is allowed.

Mistakes not checked for:
	Add/RemDevice passed trash.
	Device not ready for callers or expunge at AddDevice time.

*/

#include "exec/types.h"
#include "exec/execbase.h"
#include "exec/nodes.h"
#include "exec/io.h"
#include "exec/devices.h"

#include "dos/dos.h"
#include "dos/dosextens.h"

#include "clib/exec_protos.h"
#include "clib/dos_protos.h"

#include "io_torture_rev.h"


VOID kprintf( STRPTR fmt, ... );
VOID dprintf( STRPTR fmt, ... );

/*
#define PARALLEL
*/

#ifdef PARALLEL
#define RawPrintf dprintf
extern VOID dprintf(STRPTR,...);
#else
#define RawPrintf kprintf
extern VOID kprintf(STRPTR,...);
#endif

char *vers=VERSTAG;
UBYTE *intname = "Int|Except";
UBYTE *taskname(struct Task *task);

/* Globally visible so the assembly code can see them */
APTR OldSendIO;
APTR OldDoIO;
APTR OldOpenDevice;
APTR OldCloseDevice;

APTR RemSendIO;
APTR RemDoIO;
APTR RemOpenDevice;
APTR RemCloseDevice;

void main()
{
extern struct ExecBase *SysBase;
extern APTR SetFunction();

extern void MySendIO();
extern void MyDoIO();
extern void MyOpenDevice();
extern void MyCloseDevice();

extern LVOSendIO;
extern LVODoIO;
extern LVOOpenDevice;
extern LVOCloseDevice;

printf("\nio_torture installed.  Exec device IO usage will be tracked.\n");
printf("If an IORequest is reused while still active, io_torture will print\n");
printf("a warning message on the serial port (parallel for io_torture.par).\n");
printf("io_torture does not currently check for another common mistake:\n");
printf("After virtually all uses of AbortIO(IORequest), there should be a\n");
printf("call to WaitIO(IORequest).  AbortIO() asks the device to finish\n");
printf("the I/O as soon as possible; this may or may not happen instantly.\n");
printf("AbortIO() does not wait for or remove the replied message.\n");

	Forbid();
	OldOpenDevice	=SetFunction(SysBase, &LVOOpenDevice,	MyOpenDevice);
	OldCloseDevice	=SetFunction(SysBase, &LVOCloseDevice,	MyCloseDevice);
	OldSendIO	=SetFunction(SysBase, &LVOSendIO,	MySendIO);
	OldDoIO		=SetFunction(SysBase, &LVODoIO,		MyDoIO);

thewait:
	Wait(SIGBREAKF_CTRL_C);

	RemOpenDevice 	= SetFunction(SysBase, &LVOOpenDevice,	OldOpenDevice);
	RemCloseDevice	= SetFunction(SysBase, &LVOCloseDevice,	OldCloseDevice);
	RemSendIO	= SetFunction(SysBase, &LVOSendIO,	OldSendIO);
	RemDoIO		= SetFunction(SysBase, &LVODoIO,	OldDoIO);

	if((RemOpenDevice  != MyOpenDevice)||
	   (RemCloseDevice != MyCloseDevice)||
	   (RemSendIO != MySendIO)||
	   (RemDoIO != MyDoIO))
		{
		SetFunction(SysBase, &LVOOpenDevice, RemOpenDevice);
		SetFunction(SysBase, &LVOCloseDevice, RemCloseDevice);
		SetFunction(SysBase, &LVOSendIO, RemSendIO);
		SetFunction(SysBase, &LVODoIO, RemDoIO);
		printf("io_torture can't uninstall yet - another SetFunction present\n");
		goto thewait;
		}	
	Permit();


	/* Exiting is somewhat unsafe, since someone may be threaded
	   into one of our calls */
	printf("\nFreed io_torture test.  Exiting in 30 seconds.\n");
	Delay(30*50);
}



void DisplayError(Request,FunctionName,Stack)
ULONG *Stack;
char * FunctionName;
struct IOStdReq *Request;
{
       RawPrintf("\r\nIORequest %lx reused! Task $%lx \"%s\" during %s\r\n",Request,
		FindTask(0L),taskname(FindTask(0L)),FunctionName);
       RawPrintf("io_Command = %ld ",(ULONG)Request->io_Command);
       RawPrintf("io_Data = %lx ",Request->io_Data);
       RawPrintf("io_Length = %lx\r\n",Request->io_Length);
       RawPrintf("io_Device = %lx, (%s) - Stack %lx. Stack dump:\r\n",
		Request->io_Device,
		((struct Node *)Request->io_Device)->ln_Name,Stack);
       RawPrintf("SF:%08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\r\n",
		Stack[0],Stack[1],Stack[2],Stack[3],Stack[4],Stack[5],
		Stack[6],Stack[7]);
       RawPrintf("SF:%08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\r\n",
		Stack[8],Stack[9],Stack[10],Stack[11],Stack[12],Stack[13],
		Stack[14],Stack[15],Stack[16]);
}


UBYTE *taskname(struct Task *task)
    {
    struct CommandLineInterface *cli;
    struct Process *pr;
    UBYTE  *s, *name;
    APTR stack;

    stack = (APTR)&stack;

    if((stack > SysBase->SysStkLower)&&(stack < SysBase->SysStkUpper))
		name = intname;
    else
	{
    	pr = (struct Process *)task;
    	name = task->tc_Node.ln_Name;
    	if((task->tc_Node.ln_Type == NT_PROCESS)&&(pr->pr_CLI))
	    {
	    cli = (struct CommandLineInterface *)(BADDR(pr->pr_CLI));
	    if(cli->cli_CommandName)
		{
	    	s = (UBYTE *)(BADDR(cli->cli_CommandName)); 
	    	if(s[0]) name = &s[1];
		}
	    }
	}
    return(name);
    }


