#define NULL 0L
#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dosextens.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "spawn.h"

#define fh_Interact fh_Port
#define fh_Process  fh_Type
#define ACTION_CLOSE 1007
#define ACTION_ALOHA 543210
#undef STATIC
#define STATIC 
long serr;

STATIC char *mempos(s, n, c)
register char *s;
register int n;
register char c;
{
	while (--n >= 0)
	{
		if (*s == c) return(s);
		++s;
	}
	return(NULL);
}

/* free BCPL allocated storage */
STATIC VOID FreeBCPL(bptr)
BPTR bptr;
{
	LONG *lptr;

	if (!bptr) return;
	lptr = (LONG *)BADDR((bptr - 1));
	FreeMem(lptr, *lptr);
}

/* cleanup after spawn */
STATIC VOID CleanupSpawn(si)
register struct SPAWNINFO *si;
{
	FreeBCPL(si->fh1.fh_Buf);
	FreeSignal((LONG)si->port.mp_SigBit);
	FreeMem(si, (LONG)sizeof(*si));
}

/*
 * Return a packet
 */
STATIC VOID ReturnPkt(pkt, Res1, Res2)
register struct DosPacket *pkt;
LONG Res1, Res2;
{
	pkt->dp_Res1 = Res1;
	pkt->dp_Res2 = Res2;
	PutMsg(pkt->dp_Port, pkt->dp_Link);
}

/*
 * Abort a packet
 */
STATIC VOID AbortPkt(pkt, place)
register struct DosPacket *pkt;
char *place;
{
/*	printf("%s: Unexpected packet type %ld\n", place, pkt->dp_Type);*/
	ReturnPkt(pkt, 0L, ERROR_ACTION_NOT_KNOWN);
}

extern void LaunchTask();

#define BREAKSIGS (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D)

/*
 * Handle signals
 */
STATIC VOID SignalCLI(si, sigs)
register struct SPAWNINFO *si;
register LONG sigs;
{
	sigs |= SetSignal(0L, BREAKSIGS);
	if (sigs & SIGBREAKF_CTRL_C)
	{
		si->breaksig = 'C';
		if (si->proc) Signal(&si->proc->pr_Task, BREAKSIGS);
	}
	if ((sigs & SIGBREAKF_CTRL_D) && !si->breaksig)
		si->breaksig = 'D';
}

STATIC struct DosPacket *GetPkt(si)
register struct SPAWNINFO *si;
{
	struct Message *msg;

	SignalCLI(si, 0L);
	while ((msg = GetMsg(&si->port)) == NULL)
		SignalCLI(si, Wait((1L << si->port.mp_SigBit) | BREAKSIGS) );
	return((struct DosPacket *)msg->mn_Node.ln_Name);
}

/*
 * Handle common requests, return unprocessed packet
 */
STATIC struct DosPacket *HandlePkt(si, pkt, echo)
register struct SPAWNINFO *si;
register struct DosPacket *pkt;
int echo;
{
	register struct FileHandle *fh;
	struct Process *opener;
	
	switch (pkt->dp_Type)
	{
	case MODE_NEWFILE:
	case MODE_OLDFILE:
		opener = (struct Process *)pkt->dp_Port->mp_SigTask;
		if (si->proc != opener)
		{	/* disallow opens from new processes */
			ReturnPkt(pkt, 0L, ERROR_OBJECT_NOT_FOUND);
			return(NULL);
		}
		fh = (struct FileHandle *)BADDR(pkt->dp_Arg1);
		fh->fh_Interact = si->fh1.fh_Interact;
		fh->fh_Process  = si->fh1.fh_Process;
		fh->fh_Pos = fh->fh_End = -1;
#if 0
		fh->fh_Func1 = si->fh1.fh_Func1;
		fh->fh_Func2 = si->fh1.fh_Func2;
		fh->fh_Func3 = si->fh1.fh_Func3;
		fh->fh_Arg1 = si->fh1.fh_Arg1;
		fh->fh_Arg2 = si->fh1.fh_Arg2;
#endif
		++si->opencount;
		ReturnPkt(pkt, -1L, 0L);
		break;
	case ACTION_CLOSE:
		--si->opencount;
		ReturnPkt(pkt, -1L, 0L);
		break;
	case ACTION_WRITE:
		if (echo)
		{
			register char *src;
			register LONG n;

			src = (char *)pkt->dp_Arg2;
			n = pkt->dp_Arg3;

			/* Output with no command loaded is either a
			   prompt or a "command not found" message.
			   We assume it is a prompt unless it begins 
			   with "Unknown or Unable" (ick !!!) */

			if (!si->CLI->cli_Module && 
			    (si->CLI->cli_ReturnCode > 9 || 
			    strncmp("Unknown",src,7) == 0 ||
			    strncmp("Unable",src,6) ==0)) 
				si->badoutput = 1;

			Write(Output(), src, n);
		}
		ReturnPkt(pkt, pkt->dp_Arg3, 0L);
		break;
#if 0
	case ACTION_WAIT_CHAR:
		/* behave as if char were immediately available */
		ReturnPkt(pkt, -1L, 0L);
		break;
#endif
	case ACTION_READ:
		/* if a module is loaded get input */
		if (si->CLI->cli_Module)
		{
			pkt->dp_Arg3 = Read(Input(),(UBYTE *)pkt->dp_Arg2,
				(LONG)pkt->dp_Arg3);
			ReturnPkt(pkt, pkt->dp_Arg3, 0L);
			break;
		}
				
	default:
		return(pkt);
	}
	return(NULL);
}

/*
 * Wait for first read or unusual packet
 */
STATIC struct DosPacket *WaitRead(si)
register struct SPAWNINFO *si;
{
	struct DosPacket *pkt;

	if ((pkt = si->queuedpkt) != NULL)
	{
		si->queuedpkt = NULL;
		return(pkt);
	}

	for (;;)
	{
		if ((pkt = HandlePkt(si, GetPkt(si), TRUE)) != NULL)
			return(pkt);
	}
}

/*
 * Spawn a CLI task
 */
STATIC struct SPAWNINFO *SpawnCLI()
{
	register struct SPAWNINFO *si;
	register struct FileHandle *fh;
	struct MsgPort *pid;
	int sigbit;
	register struct DosPacket *pkt;
	struct Process *pr;

	pr = (struct Process *)FindTask((char *)NULL);
	
#if 0
	/* test for redirect to NIL NOT ALLOWED */
	fh = (struct FileHandle *)BADDR(pr->pr_COS);
	if (fh->fh_Type == NULL) return(NULL);
	fh = (struct FileHandle *)BADDR(pr->pr_CIS);
	if (fh->fh_Type == NULL) return(NULL);
#endif

	/* create spawninfo structure */
	si = (struct SPAWNINFO *)
	     AllocMem((LONG)sizeof(struct SPAWNINFO), MEMF_PUBLIC|MEMF_CLEAR);
	if (si == NULL) return(NULL);

	/* initialize message */
	si->msg.mn_Node.ln_Type = NT_MESSAGE;
	si->msg.mn_Node.ln_Name = (char *)&si->dp;
	si->msg.mn_Length = sizeof(*si) - sizeof(struct Message);

	/* initialize packet */
	si->dp.dp_Link = &si->msg;
	si->dp.dp_Port = &si->port;
	si->dp.dp_Type = ACTION_ALOHA;

	/* initialize segment */
	si->jmp = 0x4EF9;
	si->launcher = LaunchTask;

	/* initialize port */
	si->port.mp_Node.ln_Type = NT_MSGPORT;
	si->port.mp_Flags = PA_SIGNAL;
	si->port.mp_SigTask = (struct Task *)pr;
	
	if ((sigbit = AllocSignal(-1L)) < 0)
	{
		FreeMem(si, (LONG)sizeof(*si));
		return(NULL);
	}
	si->port.mp_SigBit = sigbit;
	NewList(&si->port.mp_MsgList);

	/* initialize file handles */
		
	memset((char *)&si->fh1,0, sizeof(struct FileHandle));
	si->fh1.fh_Process  = &si->port;
	si->fh1.fh_Pos      = -1;
	si->fh1.fh_End      = -1;
	si->fh1.fh_Interact = (void *)-1;	/*TRUE*/
#if 0
	fh = (struct FileHandle *)BADDR(pr->pr_COS);
	si->fh1.fh_Funcs = fh->fh_Funcs;
	fh = (struct FileHandle *)BADDR(pr->pr_CIS);
	si->fh1.fh_Func2 = fh->fh_Func2;
	si->fh1.fh_Func3 = fh->fh_Func3;
#endif

	fh = (struct FileHandle *)BADDR(serr);
	si->fh1.fh_Funcs = fh->fh_Funcs;
	si->fh1.fh_Func2 = fh->fh_Func2;
	si->fh1.fh_Func3 = fh->fh_Func3;

	/* call the launcher process */
	/* from this point on there is no easy way to clean up */
	/* since the subprocess is active and will be hung */
	pid = CreateProc("Launcher", 0L, ((LONG)&si->nextseg) >> 2, 2000L);
	if (pid == NULL)
	{	CleanupSpawn(si); return(NULL); }
	PutMsg(pid, &si->msg);

#if 1
	/* wait for the first open */
	while ((pkt = GetPkt(si))->dp_Type != MODE_NEWFILE)
	{
		if (pkt->dp_Type == ACTION_ALOHA)
		{	/* premature death */
			CleanupSpawn(si);
			return(NULL);
		}
		AbortPkt(pkt, "SpawnCLI(open):");

	}

	/* get process info */
	si->proc = (struct Process *)pkt->dp_Port->mp_SigTask;

	/* handle open */
	HandlePkt(si, pkt, FALSE);
#endif

	while ((pkt = GetPkt(si))->dp_Type != ACTION_WRITE)
	{
		if (pkt->dp_Type == ACTION_ALOHA)
		{	/* premature death */
			CleanupSpawn(si);
			return(NULL);
		}
		AbortPkt(pkt, "SpawnCLI(write):");
	}

	/* get process info */
	si->proc = (struct Process *)pkt->dp_Port->mp_SigTask; 
	si->CLI = (struct CommandLineInterface *)BADDR(si->proc->pr_CLI);
	
	/* change the prompt to an empty string */
	*(char *)BADDR(si->CLI->cli_Prompt) = 0;

	if (si->CLI->cli_DefaultStack < 3750) 
		si->CLI->cli_DefaultStack = 3750;
	 
	/* skip the current packet (write of the first prompt) */
	HandlePkt(si, pkt, FALSE);

	/* wait for the first command */
	si->queuedpkt = WaitRead(si);

	/* return the spawn info */
	return(si);
}

/*
 * perform a read operation, return NULL normally or unprocessed packet
 * a newline is automatically appended to the source
 */
STATIC struct DosPacket *HandleRead(si, src, srclen, addnl)
register struct SPAWNINFO *si;
register char *src;
register int srclen;
int addnl;
{
	register struct DosPacket *pkt;
	register char *dest;
	register LONG n;

	if (srclen < 0) srclen = strlen(src);

	if (srclen == 1 && *src == '\x1C')
	   {
           addnl = FALSE;
           srclen = 0;
           }

	while (srclen >= 0)
	{
		pkt = WaitRead(si);
		if (pkt->dp_Type != ACTION_READ) return(pkt);
		dest = (char *)pkt->dp_Arg2;
		for (n = 0; srclen > 0 && n < pkt->dp_Arg3; --srclen, ++n)
			*dest++ = *src++;
		if (srclen == 0 && (!addnl || n < pkt->dp_Arg3))
		{
			if (addnl)
			{
				*dest = '\n';
				++n;
			}
			srclen = -1;
		}
		ReturnPkt(pkt, n, 0);
	}
	return(NULL);
}

/*
 * execute command in spawned CLI
 */
STATIC int ExecCLI(si, cmd)
register struct SPAWNINFO *si;
char *cmd;
{
	register struct DosPacket *pkt;
	
	/* reset bad output indicator */
	si->badoutput = 0;

	/* send the command to the CLI */
	while ((pkt = HandleRead(si, cmd, -1, TRUE)) != NULL)
		AbortPkt(pkt, "ExecCLI(read)");

	/* wait for command to complete */
	si->queuedpkt = WaitRead(si);

	/* return -1 if command didn't load,
	          0 if the command is still executing
		  the return code otherwise */
	
	return((si->badoutput)	     ? -1 :
	       (si->CLI->cli_Module) ? 0
				     : (int)si->CLI->cli_ReturnCode);
}

/*
 * terminate the spawned CLI
 */
STATIC VOID KillCLI(si)
register struct SPAWNINFO *si;
{
	register struct DosPacket *pkt;

	/* send a break if a command is still in progress */
	while (si->CLI->cli_Module)
	{
		Signal(&si->proc->pr_Task, BREAKSIGS);

		/* send eof */
		if ((pkt = HandleRead(si, "", 0, FALSE)) != NULL)
			AbortPkt(pkt, "KillCLI(eof)");

		/* wait for next prompt */
		si->queuedpkt = WaitRead(si);
	}

	/* send an endcli command */
	while ((pkt = HandleRead(si, "EndCLI;\x85", -1, TRUE)) != NULL)
		AbortPkt(pkt, "KillCLI(EndCLI)");

	/* wait for ALOHA */
	for (;;)
	{
		while ((pkt = HandlePkt(si, GetPkt(si), FALSE)) == NULL)
			;

		if (pkt->dp_Type == ACTION_ALOHA)
			break;

		AbortPkt(pkt, "KillCLI(Aloha)");
	}

	/* terminate */
	CleanupSpawn(si);
}

/*
 * return current status of CLI break signals
 */
STATIC int BreakCLI(si)
struct SPAWNINFO *si;
{
	return((int)(si->breaksig));
}

system(cmd)
char *cmd;
{
struct SPAWNINFO *si;
int i;

if ((serr = Open("*",MODE_OLDFILE)) == 0) return(-1);
if ((si = SpawnCLI()) == NULL) 
	{
	Close(serr);
	return(-1);
	}
i = ExecCLI(si, cmd);
KillCLI(si);
Close(serr);
return(i);
}

/*
 * CLI launching process
 */
STATIC VOID LaunchTask()
{
	register struct Process *proc, *parent;
	register struct SPAWNINFO *si;
	struct CommandLineInterface *cli;
	LONG retcode;
	long savestack;
	APTR savedtask;
	BPTR saveddir;
	struct DosLibrary *DOSBase;
		
	DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",0);
	
	/* locate our task */
	proc = (struct Process *)FindTask((char *)NULL);

	/* get the startup message */
	(VOID)WaitPort(&proc->pr_MsgPort);
	si = (struct SPAWNINFO *)GetMsg(&proc->pr_MsgPort);

	/* make ourselves look like the parent CLI */
	parent = (struct Process *)si->port.mp_SigTask;
	saveddir = proc->pr_CurrentDir;
	proc->pr_CurrentDir = parent->pr_CurrentDir;
	savedtask = proc->pr_FileSystemTask;
	proc->pr_FileSystemTask = parent->pr_FileSystemTask;
	proc->pr_CLI = parent->pr_CLI;
	cli = (struct CommandLineInterface *)BADDR(proc->pr_CLI);
	savestack = cli->cli_DefaultStack;
	if (savestack < 3750)
		cli->cli_DefaultStack = 3750;
	
	/* start up a CLI, wait for it to finish */
	retcode = Execute("", ((LONG)&si->fh1) >> 2, proc->pr_COS);

	/* restore old values */
	proc->pr_CurrentDir = saveddir;
	proc->pr_FileSystemTask = savedtask;
	proc->pr_CLI = NULL;
	cli->cli_DefaultStack = savestack;

	/* Forbid so we can fade into sunset unmolested */
	Forbid();

	/* notify parent task */
	ReturnPkt(&si->dp, retcode, 0L);

        CloseLibrary((struct Library *)DOSBase);
}

