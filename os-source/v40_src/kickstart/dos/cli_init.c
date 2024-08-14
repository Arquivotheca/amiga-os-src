/* cli_init.c */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/filehandler.h"
#include "dos/dostags.h"
#include <utility/tagitem.h>
#include <intuition/preferences.h>
#include <devices/timer.h>
#include <libraries/expansionbase.h>

#include <internal/librarytags.h>

/* Private ROM routine... */
struct Library *TaggedOpenLibrary(LONG);
#pragma syscall TaggedOpenLibrary 32a 001

#include <string.h>
#include <stddef.h>

#include "libhdr.h"
#include "cli_init.h"

/*#include <clib/alib_protos.h>*/
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include "device_protos.h"
#include "klib_protos.h"
#include "blib_protos.h"
#include "cli_protos.h"
#include "path_protos.h"
#include "env_protos.h"
#include "crp_protos.h"
#include "fault.h"

#define SAME		0		/* for string compares */

#define TOBPTR(x)	(((LONG) (x)) >> 2)


/* external strings that need to be long-aligned */

extern char b_trackname[];	/* contains "\x11trackdisk.device\0" */
extern char b_prompt[];		/* "\4%N> " */
extern char b_filesysname[];	/* "File System" */
extern char b_porthandler[];	/* "l:port-handler */

extern char ramname[];
extern char shellname[];
extern char consolename[];

       char bootshell[] = "BootShell";

#ifdef STRAP_DOESNT
extern struct DosEnvec disk_envvec;	/* must be LW alligned */
#endif

extern LONG run_seglist;
extern LONG newcli_seglist;


/*
void pm(char *string);
void pm(char *string)
{
kprintf("%s\n",string);
}
*/

#define pm(x)

LONG __regargs
cli_init (pkt)
	struct DosPacket *pkt;
{
	LONG *syssegl;
	struct Process *tid;
	struct CommandLineInterface *clip;
	struct MsgPort *task;
	struct RootNode *rootnode;
	struct DosInfo *info;
	struct DosList *bootnode;
	struct CliProcList *cl;
	struct ExpansionBase *eb;
	char *prompt    = &b_prompt[0];
#ifdef STRAP_DOESNT
	char *trackname = &b_trackname[0];
	LONG unit;
#endif

pm("in cli_init");
	eb	= (void *) TaggedOpenLibrary(OLTAG_EXPANSION);
	syssegl = (LONG *) BADDR(seglist());
	tid     = MYPROC;
	clip    = cli();
/*
    // bootnode has been stashed in rootnode at startup
    // The boot node comes over in the fhseg slot
*/
	info = AllocVecPubClear(sizeof(struct DosInfo));

	/* initialize the dos device list semaphore */
	InitSemaphore(&(info->di_DevLock));
	InitSemaphore(&(info->di_EntryLock));
	InitSemaphore(&(info->di_DeleteLock));

	/* used to count of getmem allocating a extra longword!! */
	cl = AllocVecPubClear(TASKTABLESIZE*4 + 4 + sizeof(struct CliProcList));
	if (!cl)
no_mem:		Alert(AN_StartMem|AT_DeadEnd);

	/* Setup my taskid and global task table entry */
	cl->cpl_Array = (struct MsgPort **) (((LONG) cl) + sizeof(*cl));
	cl->cpl_Array[0] = (struct MsgPort *) TASKTABLESIZE;
	cl->cpl_Array[1] = &(tid->pr_MsgPort);
	cl->cpl_First = 1;
	tid->pr_TaskNum = 1;

	/* Wake up intuition - leaves it open forever - no checking */
	/* need to put it into global vector! */
	store_IBase(TaggedOpenLibrary(OLTAG_INTUITION));

	rootnode = rootstruct();

	/* get current time if it was set by BattClock */
	datstamp(&(rootnode->rn_Time));
	if (rootnode->rn_Time.ds_Days <= 1)
	{
		/* not set, make sure it's 0 so the FS will set it */
		rootnode->rn_Time.ds_Days	= 0;
		rootnode->rn_Time.ds_Minute	= 0;
		rootnode->rn_Time.ds_Tick	= 0;
	}
	rootnode->rn_TaskArray		= TOBPTR(cl->cpl_Array);
	rootnode->rn_Info		= TOBPTR(info);
	rootnode->rn_ConsoleSegment	= syssegl[4];	/* aka CLI */
	rootnode->rn_RestartSeg		= syssegl[7];
	rootnode->rn_ShellSegment	= syssegl[8];	/* aka Shell */


	/* set up new unlimited CLI stuff */
	NewList((struct List *) &(rootnode->rn_CliList));
	AddHead((struct List *) &(rootnode->rn_CliList),(struct Node *) cl);

	/* If the boot device is not specified, use DF0   */
	/* If it is specified, add it to the device chain */
	/* can't write over rn_FileHandlerSegment until we read it */

	bootnode = (struct DosList *) rootnode->rn_FileHandlerSegment;
	rootnode->rn_FileHandlerSegment = syssegl[6];
pm("have bootnode");
#ifdef STRAP_DOESNT
	if (bootnode == NULL)
		bootnode = createnode(0,trackname);
#endif
	/* Put bootnode on the devicelist, even if we created it above -- bart*/
	/* carries all the other bootnodes onto the list as well (!) */
	info->di_DevInfo = TOBPTR(bootnode);

	/* Make inactive device node entries */
pm("making node entries");
	make_node_entries(syssegl);

	/* Start up the filesystem tasks. */
	/* Start the boot device - FIX! check for failure */
pm("starting up bootnode");
	task = startup( bootnode, syssegl, 1, eb );

	/* Must be set before doing assigns! */
	tid->pr_FileSystemTask = (APTR) task;
	rootnode->rn_BootProc  = task;

	make_assign_entry("L");
	make_assign_entry("FONTS");
	make_assign_entry("DEVS");
	make_assign_entry("LIBS");
	make_assign_entry("S");
	make_assign_entry("C");
	make_task_assignment( "SYS", ":" );

	/* latebinding to avoid having it stuck on the boot: of a2090 users */
	assignlate( "ENVARC", "sys:prefs/env-archive" );
pm("assigns done");

	/* set the current directory of the boot shell - used to be left 0 */
	/* ignore failure of locateobj - worst that happens is we get 0 */
	CurrentDir(locateobj(":"));

	/* At this point the rest of the boot list should be startup'ed */
	{	struct DosList *nextnode;

		nextnode = (struct DosList *) BADDR(bootnode->dol_Next);
		while (nextnode)
		{
			(void) startup(nextnode, syssegl, 0, eb);
			nextnode = (struct DosList *)BADDR(nextnode->dol_Next);
		}
	}
pm("startups done");

#ifdef STRAP_DOESNT
	/* Start up the standard floppy handlers. */
	for (unit = 0; unit <= MAXUNITS; unit++)
	{
		struct MsgPort *task;
		struct DosList *node;
		struct FileSysStartupMsg *bootstartup;

		/* don't restart the floppy if it was the boot unit */
		bootstartup = (struct FileSysStartupMsg *)
			      BADDR(bootnode->dol_misc.dol_handler.dol_Startup);

		/* null terminated BSTR's! */
		/* checks unit and trackdisk.device */
		if (unit == bootstartup->fssm_Unit &&
		    strcmp(trackname+1,
			   (char *) BADDR(bootstartup->fssm_Device)+1) == SAME)
			continue;

		node = createnode( unit, trackname );
		if (!node)
			continue;

		task = startup( node, syssegl, 0 );
		if (task)
			continue;

		/* No disk unit ... remove structure */
		if (node)
		{ 
/* FIX! use RemDosEntry,etc! */
			   info->di_DevInfo = node->dol_Next;
			   freeVec((char *) BADDR(node->dol_misc.dol_handler.
							dol_Startup));
			   freeVec(node);
		}
	}
#endif

	/* build the segment list */
	{
		LONG ok = TRUE;	/* this avoids lots of tst/bra pairs */

		ok &= makeseg( "CLI",     rootnode->rn_ConsoleSegment, -1);
/* fix! string also used in execute.c */
		ok &= makeseg( bootshell, rootnode->rn_ShellSegment, -1);
		ok &= makeseg( shellname, rootnode->rn_ShellSegment, -1);
/*		ok &= makeseg( "Restart", rootnode->rn_RestartSeg, -1);	*/
		ok &= makeseg( "FileHandler", 	syssegl[6], -1);
		ok &= makeseg( consolename,	syssegl[5], -1);
		ok &= makeseg( ramname,		syssegl[9], -1);

		if (!ok)
			goto no_mem;
	}

	/* do some cli init (all we can before AFTERDOS) */
	/* ROM shell always clears cli_Commandname */
	clip->cli_FailLevel      = CLI_INITIAL_FAIL_LEVEL;
	clip->cli_DefaultStack   = CLI_INITIAL_STACK;
	copyMem(prompt,BADDR(clip->cli_Prompt),*prompt+1);

	/* Initialize ramlib 					     */
	/* Patches vectors in exec to search for modules on disk.    */
	/* We do this before we start spawning off handler processes */
	/* but after the dos library data structures are all set up. */
	/* This also inits all other AFTERDOS items. */
	InitCode(RTF_AFTERDOS, 0);

	/* Read the preferences - don't put up requester for devs: */
	/* should we leave them on?  we REALLY want to have devs:! FIX! */
	tid->pr_WindowPtr = (APTR) -1;
	readprefs();
	tid->pr_WindowPtr = 0;

	/* Sign on message */
	{
		BPTR outs;

		outs = findoutput("CON:///-1/AmigaDOS/Auto/NoClose/Smart");
		if (!outs)
			goto no_mem;
		tid->pr_ConsoleTask = (APTR) (((struct FileHandle *)
				              BADDR(outs))->fh_Type);
		selectoutput(outs);
		selectinput(findinput("*"));
	}
	clip->cli_StandardInput = input();
	clip->cli_CurrentInput  = NULL;

	/* did the bootscreen turn off the startup-sequence? */
	if (!eb || !(eb->Flags & EBF_DOSFLAG))
		clip->cli_CurrentInput   = findinput(":S/startup-sequence");
	if (eb)
	{
		/* hack to force screen to open for 1.3 WB disks */
		/* keeps bitplane stealers from breaking	 */
		/* \r forces a flush				 */
		if (!(eb->Flags & EBF_SILENTSTART))
		{
			/* stupid sierra games are broken by cdir != NULL */
			freeobj(CurrentDir(NULL));
			wrch('\r');
		}
		/* CloseLibrary((struct Library *) eb); don't need to close! */
	}

	if (clip->cli_CurrentInput == NULL)
		clip->cli_CurrentInput = clip->cli_StandardInput;

	clip->cli_StandardOutput = output();
	clip->cli_CurrentOutput  = clip->cli_CurrentOutput;

pm("exiting cli_init");
	/* ROM shell always clears cli_SetName */
	return 0;
}

#ifdef STRAP_DOESNT
/* Create a file system node for unit DFn */

struct DosList * __regargs
createnode (unit,devname)
	LONG unit;
	char *devname;	/* CPTR to NULL terminated BSTR - xxx.device */
{
	struct FileSysStartupMsg *arg;
	struct DosList *node = NULL;
	char name[4];

	arg = AllocVecPubClear(sizeof(struct FileSysStartupMsg));
	if (arg)
	{
		arg->fssm_Unit    = unit;
		arg->fssm_Device  = TOBPTR(devname);	/* xxxx.device */
		arg->fssm_Environ = TOBPTR(&disk_envvec);
		/* flags = 0 */

		strcpy(name,"DF0");
		name[2] += unit;	/* unit = 0..3 */

		/* safe - no locking - no apps up yet */
		node = adddevice(name);
		if (node)
			node->dol_misc.dol_handler.dol_Startup = TOBPTR(arg);
		/* loses arg if adddevice fails! FIX! */
	}
	return node;
}
#endif

/* Create a new file system task instance, or return zero */
/* used to take the current proc's seglist as a parm to get klib/blib - REJ */

struct MsgPort * __regargs
startup (node,syssegl,boottime,eb)
	struct DosList *node;
	LONG *syssegl;
	LONG boottime;
	struct ExpansionBase *eb;
{
	char buf[32];				 /* FIX! maxfilename + 2 */
	char *name = (char *) BADDR(node->dol_Name);
	struct FileSysStartupMsg *start = (void *)
				BADDR(node->dol_misc.dol_handler.dol_Startup);
	struct DosEnvec *env = NULL;
	struct MsgPort *rc;

	if ((LONG) start > 64*4)   /* can't be in the exception table.... */
		env = (void *) BADDR(start->fssm_Environ);

	/* if not specified, use default FS IF (and only if) it's DOS\0 or 1 */
	if (node->dol_misc.dol_handler.dol_SegList == NULL)
	{
		/* make sure the entry exists!!!! */
		if (env && (env->de_TableSize < 16 ||
			    (((env->de_DosType & 0xffffff00) == ID_DOS_DISK) &&
			     ((env->de_DosType & 0xff) <= 127))))
		{
/*
if (env->de_TableSize < 16)
kprintf("Small Table! (%ld, %s)\n",env->de_TableSize,name+1);
*/
			node->dol_misc.dol_handler.dol_SegList = syssegl[6];
		} else {
			/* we have no handler for this partition/drive! */
			/* should we search somewhere? FIX! */
			return NULL;
		}
	}

	/* if not specified, use default FH stack */
	if (node->dol_misc.dol_handler.dol_StackSize <= 0)
		node->dol_misc.dol_handler.dol_StackSize = FH_STACK;

	/* dol_Name MUST be null-terminated BSTR! */
	/* add : to end to make into a legal device name */
	mysprintf(buf,"%s:",(LONG) name+1);

	/* forbid to provide locking for the CC0: test */
	Forbid();
	if (boottime || mystricmp(buf,"CC0:") != SAME ||
	    (eb->Flags & EBF_START_CC0 ? 1 : (eb->Flags |= EBF_START_CC0,0)))
		rc = deviceproc(buf);
	else
		rc = NULL;
	Permit();

	return rc;
}

/* make initial device list entry */
/* if it fails, assign it to the root of the disk (?) */

LONG __regargs
make_task_assignment (name,dir)
	char *name;
	char *dir;
{
	BPTR lock;

	if (!(lock = locateobj(dir)))
		lock = locateobj(":");

	return assign(name,lock);
}

LONG __regargs
make_assign_entry (name)
	char *name;
{
	return make_task_assignment(name,name);
}

/* make node entry for inactive devices */

void __regargs
make_node_entry (name,startup,seglist,handler,stksize,pri,gv)
	char *name;
	LONG startup;
	BPTR seglist;
	char *handler;	/* cptr to BSTR */
	LONG stksize;
	LONG pri;
	LONG gv;
{
	struct DosList *node;

/*kprintf("making node %s\n",name);*/
	/* no locking needed - no apps running yet */
	node = adddevice(name);
	if (node)
	{
		node->dol_misc.dol_handler.dol_Handler   = TOBPTR(handler);
		node->dol_misc.dol_handler.dol_StackSize = stksize;
		node->dol_misc.dol_handler.dol_Priority  = pri;
		node->dol_misc.dol_handler.dol_SegList   = seglist;
		node->dol_misc.dol_handler.dol_Startup   = startup;
		node->dol_misc.dol_handler.dol_GlobVec   = gv;
	}
}

/* make all the node entries */
#define make1(name,startup,hname,stacksize,gv) \
	make_node_entry(name,startup,0,hname,stacksize,0,gv);

/* also needs to be changed in doslib.asm! */
/* C handlers specify stack in BYTES!!!!! (bcpl uses longs) */
#define makec(name,startup,segl) \
	make_node_entry(name,startup,segl,0,CON_STACK<<2,5,-1);

/* gv was TOBPTR(dosbase()->dl_GV) */

void __regargs
makes(name,startup,stacksize)
	char *name;
	LONG startup;
	LONG stacksize;
{
	make1(name,startup,b_porthandler,stacksize,TOBPTR(dosbase()->dl_GV));
}

void __regargs
make_node_entries (syssegl)
	LONG *syssegl;
{
	/* Note that for BCPL tasks stacksize is in LONGs instead of bytes! */
	/* in reverse order of importance */
	makes("PRT", 2, PRT_STACK );
	makes("PAR", 1, PAR_STACK );
	makes("SER", 0, SER_STACK );
	makec("RAW", 1, syssegl[5]);
	makec("CON", 0, syssegl[5]);
	/* RAM is now a C module, since it's in ROM */
	make_node_entry("RAM",0,syssegl[9],0,RAM_STACK*4,10,-1);
}

/* -1: system, 0: loaded cmd */

LONG ASM
makeseg (REG(d1) char *name,REG(d2) BPTR seg,REG(d3) LONG system)
{
	/* struct segment has 4 chars of seg_name at end */
	LONG upb = sizeof(struct Segment)-4 + strlen(name)+1;
	struct Segment *segment;
	BPTR *lv_list;			/* note: BPTR *!!! */

	segment = AllocVecPubClear(upb);
	if (!segment)
		return FALSE;

	segment->seg_UC  = system;	/* -1 is not a command */
	segment->seg_Seg = seg;

	/* copy name to a bstr */
/*	CtoB(name,(((LONG) segment) + (offsetof(struct Segment,seg_Name))));*/

/* lattice offsetof() sucks rocks - I'll cheat - FIX! */
	CtoB(name,(CPTR) (((LONG) segment) +
		   (sizeof(*segment) - sizeof(segment->seg_Name))));

	lv_list = (BPTR *)
	     &(((struct DosInfo *) BADDR(rootstruct()->rn_Info))->di_NetHand);

	Forbid();
	segment->seg_Next = *lv_list;
	*lv_list = TOBPTR(segment);
	Permit();

	return DOSTRUE;
}


/* evil routine - ignore errors */

void __regargs
readprefs ()
{
	struct Preferences prefs;
	BPTR fh;

	fh = findinput("devs:system-configuration");
	if (fh)
	{
		if (read(fh,(char *) &prefs, sizeof(prefs)) == sizeof(prefs))
		{
			MySetPrefs(&prefs,sizeof(prefs));
		}
		endstream(fh);
	} else
		MySetPrefs(&prefs,0);	/* intuition ALWAYS wants this called */
}

/* DOWNCODE! */

ULONG __regargs
MaxCli ()
{
	struct CliProcList *cli;
	ULONG max = 0,curr;

	for (cli = (struct CliProcList *) rootstruct()->rn_CliList.mlh_Head;
	     cli->cpl_Node.mln_Succ;
	     cli = (struct CliProcList *) cli->cpl_Node.mln_Succ)
	{
		for (curr = 0;
		     curr < (LONG) cli->cpl_Array[0];
		     curr++)
		{
			if (cli->cpl_Array[curr])
				max = cli->cpl_First + curr;
		}
	}

	return max;
}

/* DOWNCODE! */

struct Process * ASM
FindCli (REG(d1) ULONG num)
{
	struct CliProcList *cli;
	
	if (num)
	{
	  for (cli = (struct CliProcList *) rootstruct()->rn_CliList.mlh_Head;
	       cli->cpl_Node.mln_Succ;
	       cli = (struct CliProcList *) cli->cpl_Node.mln_Succ)
	  {
		if (num < cli->cpl_First + (LONG) cli->cpl_Array[0])
		{
			if (cli->cpl_Array[num+1 - cli->cpl_First])
				 return (struct Process *)
			          (((LONG) cli->cpl_Array[num+1-cli->cpl_First])
			           - sizeof(struct Task));
			else
				return NULL;
		}
	  }
	}

	return NULL;
}

/* stuff from the old run.b, used in cli.c */
/* the names have been changed to protect the innocent */

/*
// This function is called as initialization for new process
// It returns a value as follows:
// >0         The function QPKT, to be called by the CLI
// <0         Flags used as follows (odd values for compatibility)
// Bit 31     Set to indicate flags
// Bit  3     Set to indicate asynch system call - return pkt early - Added REJ
// Bit  2     Set to indicate the System call (stop cli at EOF from
//	      cli_CurrentInput).  Added REJ.
// Bit  1     Set if user provided input stream
// Bit  0     Set if RUN provided output stream
*/

/* This is really part of the standard C= CLI/Shell. */
/* Because of l:shell-seg, we MUST return valid pointers to BCPL routines!!! */
/* cli_init_run and cli_init_newcli MAY be called from BCPL!! See run.c.     */

LONG __regargs
cli_init_run (struct DosPacket *pkt)
{
	struct Process *tid = MYPROC;
	struct CommandLineInterface *clip  = cli();
	struct CommandLineInterface *oclip = (void *) BADDR(pkt->dp_Arg1);
	BPTR scb = pkt->dp_Arg4;		/* command stream */
	LONG is_run = (pkt->dp_Arg6 == NULL);
	LONG type = pkt->dp_Type;
	BPTR outs = NULL;
	LONG interactive = FALSE;
	LONG rc = 0x80000000;				/* Set flag word */

/*
    // Two cases here. If this is RUN then use new
    // io streams to *. If this is execute then the
    // user must have provided two io streams already;
    // input in arg2 and output in arg3. If input is
    // zero then we run non-interactively.
    // If arg3 is zero we use *, possibly derived from arg2

    // There is now a third case: System.  For system, arg4 has the command
    // stream, and arg2/3 have the input/output filehandles.  We will stuff
    // arg4 into standardinput, arg2/3 into currentinput/output, and set the
    // new bit in flags to indicate system().
    //
    // We tell System() from Execute() by the dp_Type field - RUN_EXECUTE (-1)
    // for Execute(), and RUN_SYSTEM (-2) for System().  BCPL run will
    // pass in a positive ptr.
    // 
    // Asynchronous System() calls will have RUN_SYSTEM_ASYNCH (-3).  They
    // will return the packet immediately, but otherwise work like normal
    // RUN_SYSTEM processes.
    //
    // dp_Arg7 contains tags (if any) used to create this process in CNP.
*/

/*kprintf("in cli_init_run(), type = %ld, is_run = %ld\n", pkt->dp_Type,is_run);*/

	/* make evil assumptions about where the startup packet goes to! */
	/* TRICKY! EVIL casting! */
#ifdef WRONG
	if (!copyvars(tid,(struct Process *)
		(((LONG) ((struct Message *) (pkt->dp_Link))->mn_ReplyPort) -
		 sizeof(struct Task))))
	{
		/* ran out of mem */
		goto error;
	}
#endif
	clip->cli_CurrentInput  = scb;
	clip->cli_StandardInput = scb;		/* Default for both */

	/* What if someone has a NULL pr_ConsoleTask that we inherited? */
	/* We should set it to something safe before opening "*", so we */
	/* don't end up with pr_COS == NULL */

	if (!is_run)
	{
		BPTR instream = pkt->dp_Arg2;
		struct FileHandle *fh = BADDR(instream);

		/* system() or execute() */
		/* pr_ConsoleTask is inherited from parent by default */
		/* what if the file is non-interactive??? FIX! */
		/* ignore instream if it's NULL (ends up as a nil: fh) */
		if (instream != NULL)
		{
			clip->cli_StandardInput = instream;

			/* See if we might possibly have an interactive stream*/
			interactive = isinteractive(instream);

			/* Make the instream the current console task */
/*
 * Lattice/SAS lmk does Execute("",fakefh,NULL).  They then expect it to open
 * "*" on the fakefh's msgport even if the FH isn't interactive.  They guarantee
 * that the fh is right after the msgport, which does not normally happen, so
 * we can key off it.  This is safe since it always used to do this in all
 * cases.
 */
/*
 * Runback and many WB programs use Execute("command",NILfh,NILfh).  We must
 * make that clear ConsoleTask like it used to.
 */

/*
kprintf("setting consoletask for execute/system, %s inter, fh%s=msgport+1\n",
interactive ? "" : "not", 
(void *)
(((LONG) ((struct MsgPort *) ((struct FileHandle *) BADDR(instream))->fh_Type))
+ sizeof(struct MsgPort))
==
BADDR(instream) ? "=" : "!");
kprintf(" (msgport = 0x%lx, fh = 0x%lx, ConsoleTask was 0x%lx)\n",
((struct FileHandle *) BADDR(instream))->fh_Type, BADDR(instream),
tid->pr_ConsoleTask);
*/
			if (interactive || !fh->fh_Type ||
			    (((LONG) fh) ==
			     (((LONG) fh->fh_Type) + sizeof(struct MsgPort))))
			{
				tid->pr_ConsoleTask = (APTR) fh->fh_Type;
				if (fh->fh_Type) {
				    sendpkt3(fh->fh_Type,ACTION_CHANGE_SIGNAL,
					     fh->fh_Arg1,
					     (LONG) &(tid->pr_MsgPort),0);
				}
			}

			/* means the shell will NOT close StandardInput */
			rc |= 2;		/* Set bit 1 */
		} else {
			/* System/Execute called with NULL input stream */
			/* provide one to make it look like a batch.	*/

			instream = findstream("NIL:",MODE_OLDFILE);
			if (instream)
				clip->cli_StandardInput = instream;

			/* we DON'T set the flag, which means the shell */
			/* will close it in cleanup.			*/
		}
		outs = pkt->dp_Arg3;
	}

	/* note: for Run, outs == NULL here */
	if (outs == NULL)
	{
		/* causes the shell to close StandardOutput on cleanup */
		rc |= 1;	/* Set bit 0 (we're providing output stream) */

		/* if he redirected output of run to somewhere that's */
		/* interactive, open * on it for run output.  If he   */
		/* redirected to NIL:, redirect to NIL: and clear     */
		/* pr_ConsoleTask.				      */
		if (is_run)
		{
			/* arg3 is the output stream for the Run command */

			if (oclip->cli_StandardOutput == pkt->dp_Arg3) {
				/* output not redirected */
				goto open_star;

			} else if (isinteractive(pkt->dp_Arg3)) {
				/* output redirected to interactive fs */
				tid->pr_ConsoleTask = (APTR)
					((struct FileHandle *)
						BADDR(pkt->dp_Arg3))->fh_Type;
				goto open_star;

			} else if (((struct FileHandle *)
				    BADDR(pkt->dp_Arg3))->fh_Type) {
			    /* output was redirected to a non-interactive fs */
			    /* open * on normal ConsoleTask output */
				goto open_star;

			} else {
				/* output was redirected to NIL: */
				/* else we fall out, and end up opening NIL: */
				tid->pr_ConsoleTask = NULL;
			}
		} else {
open_star:		/* not run - system or execute */
/*if (!is_run)
kprintf("opening * for execute/system, consoletask = 0x%lx\n",
tid->pr_ConsoleTask);*/
			outs = findoutput("*");
/*kprintf("opened *, result = 0x%lx %s\n",outs,
((struct FileHandle *) BADDR(outs))->fh_Type ? "" : "(actually nil:)");*/
		}
	}
	if (outs == NULL)
		outs = findoutput("NIL:");
	if (outs == NULL)
	{
error:		/* boy is memory tight! */
		returnpkt(pkt,0,getresult2());
		setresult2((LONG) tid);
		return (LONG) deletetask;   /* might get called by shell-seg */
	}

	/* set background for System() Asynch, Run, and Execute(), but not */
	/* for System() Synch! */
	/* this is the first cut - modified by RUN_SYSTEM below */
	clip->cli_Background = (interactive && isinteractive(outs) ?
				FALSE : DOSTRUE);

	/* set flag to tell shell not to read commands from StandardInput */
	if (type == RUN_SYSTEM)
	{
		clip->cli_Background = FALSE;
		rc |= 4;
	} else if (type == RUN_SYSTEM_ASYNCH)
	{
		clip->cli_Background = DOSTRUE;	/* probably redundant */
		rc &= ~2;
		rc |= (1 | 4 | 8);	/* return pkt early, close files */
	}

	/* Initialise CD to SYS: unless spawned from another CLI */
	/* old currentdir was NULL (started by createtask) */
	/* must do AFTER all possible failures (see execute.c for reason) */
	/* (shell unlocks currentdir on failure, and we want system() to) */
	CurrentDir(pkt->dp_Arg5);
	pkt->dp_Arg5 = NULL;		/* so it can't be unlocked by System */
	pkt->dp_Arg4 = NULL;		/* ditto, for command scb */

	run_init_clip(clip, outs, oclip, pkt);

	/* null out the "current program" entry */
	((LONG *) BADDR(seglist()))[3] = NULL; 

	setresult2((LONG) pkt);	/* to be passed to qpkt */

/*kprintf("Setting background to %ld\n",clip->cli_Background);
kprintf("CurrentInput = 0x%lx, StandardInput = 0x%lx\n",
clip->cli_CurrentInput,clip->cli_StandardInput);*/

	/* might get called by shell-seg */
/*
kprintf("returning 0x%lx\n",is_run ? (LONG) bcpl_qpkt : rc);
{ int i; for (i  = 0; i < sizeof(*clip)/4; i++) {
kprintf("0x%lx ",((long *) clip)[i]);
if ((i+1)%4 == 0) kprintf("\n");}
if ((i+2)%4 != 0) kprintf("\n");
}
*/
	return (is_run ? (LONG) bcpl_qpkt : rc);
}

/*
// Initialise the CLI globals. We copy what we can from an existing
// CLI if possible, otherwise we create new versions as required.
// We pass the CLI structure pointer clip, the output stream, and
// the parents CLI structure (or zero).
*/

void __regargs
run_init_clip (struct CommandLineInterface *clip, BPTR outs,
	       struct CommandLineInterface *oclip,
	       struct DosPacket *pkt)
{
	UBYTE temp[256];
	UBYTE *prompt, *setname;
	LONG defstk = 0;
	BPTR cmddir = NULL;
	struct TagItem *tags = (void *) pkt->dp_Arg7;
	struct TagItem *tag  = NULL;

	/* RUN_xxxx are all less than 0 */
	if (pkt->dp_Type < 0)		/* fix! addresses >2gig! */
	{
		defstk = getTagData(NP_StackSize,0,tags) >> 2;
		tag = findTagItem(NP_Path,tags);
		if (tag)
			cmddir = tag->ti_Data;
	}

	if (oclip == NULL)
	{
		prompt = "\4%N> ";
		setname= "\4SYS:";
		if (defstk == 0)
			defstk = CLI_INITIAL_STACK;
	} else {
		prompt = (char *) BADDR(oclip->cli_Prompt);
		setname= (char *) BADDR(oclip->cli_SetName);
		if (defstk == 0)
			defstk = oclip->cli_DefaultStack;

		/* don't copy if he speced it - CNP set it up */
		if (!tag)
			cmddir = copylist(oclip->cli_CommandDir);
	}

	/* if NFL fails, use default current dir name */
	/* probably overkill - doesn't cause requesters */
	if (!myNameFromLock(currentdir(FALSE,NULL),temp,sizeof(temp)))
		BtoC(setname,temp);		/* put C version in temp */
	SetCurrentDirName(temp);

	clip->cli_StandardOutput = outs;
	clip->cli_CurrentOutput  = outs;
	
	clip->cli_FailLevel      = CLI_INITIAL_FAIL_LEVEL;
	((char *) BADDR(clip->cli_CommandFile))[0] = '\0';
	clip->cli_DefaultStack   = defstk;
	clip->cli_CommandDir     = cmddir;
	clip->cli_Module	 = 0;
	BtoC(prompt,temp);
	SetPrompt(temp);
}

/* copy a path list */

/* DOWNCODE! */

BPTR __regargs
copylist (BPTR p)
{
	BPTR list = NULL;		/* fake terminating node */
	BPTR *liste = &list;		/* ptr to link of previous node */
	LONG *node;

	/* funky bptr-list following! */
	while (p = (BPTR) BADDR(p))
	{
		node = AllocVecPubClear(2*4);	/* used to be getvec(1)! */
		if (node == NULL)
			break;		/* No more memory */
		/* node[0] == 0   */
		node[1] = copydir(((LONG *)p)[1]);
		*liste  = TOBPTR(node);
		liste	= node;
		p       = ((LONG *)p)[0];
	}

	return list;
}


/* stuff from the old newcli.b, used in cli.c */
/* the names have been changed to protect the innocent */

LONG __regargs
cli_init_newcli (struct DosPacket *pkt)
{
	struct Process *tid = MYPROC;
	struct CommandLineInterface *clip = cli();
	struct CommandLineInterface *clio = (void *) BADDR(pkt->dp_Arg4);
	BPTR scb;
	char *prompt  = (char *) BADDR(clio->cli_Prompt);
/*	char *setname = (char *) BADDR(clio->cli_SetName); */
	char *filename;
	char temp[256];

	/* Set up currentdir and standard tasks */
	currentdir(TRUE,pkt->dp_Arg1);
	tid->pr_FileSystemTask = (APTR) pkt->dp_Arg5;
	tid->pr_WindowPtr = NULL;		/* Always use WB window */

	/* Set up new CLI io streams. The first is to the new window. */
	/* it must be passed in as a BSTR, I'm afraid */
	filename = (char *) BADDR(((LONG *) BADDR(pkt->dp_Arg2))[0]);

	/* don't overwrite cli_CommandDir, free duplicate path if it exists */
	/* careful!  2.0-2.02 shell duplicates value into dp_Arg3! */
	/* MUST do this before any possible exit */
	if (!clip->cli_CommandDir)
		clip->cli_CommandDir = pkt->dp_Arg3;
	else if (pkt->dp_Arg3 && pkt->dp_Arg3 != clip->cli_CommandDir)
		freepath(pkt->dp_Arg3);

	/* can't convert in place */
	BtoC(filename,temp);
/*kprintf("opening %s\n",temp);*/
	scb = findinput(temp);

	if (scb && !(isinteractive(scb))) /* FIX! remove isinter... test!? */
	{				  /* see pr_ConsoleTask below!! */
/*kprintf("not interactive\n");*/
		endstream(scb);
		setresult2(ERROR_BAD_STREAM_NAME);
		scb = NULL;
	}

	if (!scb)			/* Panic - no window */
	{
/*kprintf("no scb\n");*/
error:
/*kprintf("can't make newcli\n");*/
		/* unset currentdir so it doesn't get unlocked */
		currentdir(TRUE,NULL);
		returnpkt(pkt,0,getresult2());
		setresult2((LONG) tid);
		return (LONG) deletetask;   /* might get called by shell-seg */
	}

	clip->cli_StandardInput = scb;
	selectinput(clip->cli_StandardInput);

	/* Set the default current input */
	clip->cli_CurrentInput = clip->cli_StandardInput;

	/* Patch consoletask so that subsequent references */
	/* to * refer to the new window. */
	/* NOTE: scb must be interactive (test is above) */
	tid->pr_ConsoleTask = (APTR) 
			      ((struct FileHandle *) BADDR(scb))->fh_Type;

	/* Now use it */
	/* Pipes?  where whould output go?  Parent's output! */
	clip->cli_StandardOutput = findoutput("*");
	if (clip->cli_StandardOutput == NULL)
	{
/*kprintf("can't open \"*\"\n");*/
		/* very wierd */
		endread();
		goto error;
	}

	selectoutput(clip->cli_StandardOutput);
	clip->cli_CurrentOutput = clip->cli_StandardOutput;
	clip->cli_CurrentInput  = clip->cli_StandardInput;

	/* Set up a possible initial command file */
	/* FIX! should we allow FH's to be passed?? */
	filename = (char *) BADDR(((LONG *) BADDR(pkt->dp_Arg2))[1]);
	if (filename)
	{
		/* must be passed in as BSTR, I'm afraid */

		/* can't convert in place */
		BtoC(filename,temp);
		scb = findinput(temp);

		if (!scb)
		{
			filename = &temp[0];	/* need ptr to array of args */
			VPrintf(getstring(STR_CANT_OPEN_FROM),
				(LONG *) &filename);
		} else
			clip->cli_CurrentInput = scb;
	} else {
		/* test default file */
		scb = finddefault("s:shell-startup");
		if (scb != NULL)
			clip->cli_CurrentInput = scb;
	}

	/* Set up other CLI values */
	clip->cli_Background = FALSE;
	clip->cli_ReturnCode = 0;
	clip->cli_Result2    = 0;
	clip->cli_Module     = NULL;
	clip->cli_FailLevel  = clio->cli_FailLevel;
	((char *) BADDR(clip->cli_CommandFile))[0] = '\0';
	clip->cli_DefaultStack = clio->cli_DefaultStack;

	/* if NFL fails, use the parent's current dir name */
	/* probably overkill */
	if (myNameFromLock(currentdir(FALSE,NULL),temp,sizeof(temp)))
		SetCurrentDirName(temp);
	else {
		char *setname = (char *) BADDR(clio->cli_SetName);
		copyMem(setname,BADDR(clip->cli_SetName),*setname+1);
	}

	copyMem(prompt,BADDR(clip->cli_Prompt),*prompt+1);

	VPrintf(getstring(STR_NEW_CLI_PROCESS), &(tid->pr_TaskNum));

	/* null out the "current program" entry */
	((LONG *) BADDR(seglist()))[3] = NULL;

	setresult2((LONG) pkt);

	return (LONG) bcpl_qpkt;	/* might get called by shell-seg! */
}

