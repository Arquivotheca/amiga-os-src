/* execute.c */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <utility/tagitem.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/dostags.h"

#include <string.h>
#include <stddef.h>

#include "libhdr.h"
#include "cli_init.h"
#include "fault.h"

#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "klib_protos.h"
#include "blib_protos.h"
#include "cli_protos.h"
#include "crp_protos.h"

/* sizeof, but returns longs */
#define longsize(x)	((sizeof(x)+3) >> 2)

#define SAME		0		/* for string compares */

#define TOBPTR(x)	(((LONG) (x)) >> 2)


/* BCPL dos used BPTR to the buffers in the filehandles.  I want to use CPTRs
 * for speed reasons, but am leaving them a BPTRs until I can boot the new
 * dos to reduce possible problems using handles created by the old dos.
 * NOTE: must also change setting of the buffer!!! (marked by "FIX!")
 * ALSO NOTE: must change bcplio.c also!!!!
 * Can't do it: Execute/Run/NewCLI(?) use it also.  ARGH
 */

#ifdef NEWDOS
#define BUFFER(fh)	((UBYTE *) (fh)->fh_Buf)
#define TOBUFFER(buf)	(buf)
#else
#define BUFFER(fh)	((UBYTE *) BADDR((fh)->fh_Buf))
#define TOBUFFER(buf)	(TOBPTR(buf))
#endif

/*
// EXECUTE may be called in a variety of ways.

// EXECUTE( 0, instream, 0 )          invokes a background CLI (RUN)
// EXECUTE( "", windowstream, 0 )     invokes a new CLI (NEWCLI) or new shell
// Execute( "execute s:xxx-startup", windowstream, 0)
//				      invokes a newcli/shell with startup file
// EXECUTE( "command", 0, outstream ) executes command with output
//                                    to outstream
// EXECUTE( "command", 0, 0 )         executes command, output to * 

// Change: if output for run itself is redirected, the initial CLI
// may be closed, as run doesn't do a findoutput("*") in that case.
// Now uses the resident seglist
*/

extern char shellname[];
extern char bootshell[];

/*
 * This used to be c:Run, and Execute() would invoke it.  I have split off the
 * two functions, leaving little in Run (which now calls Execute()).
 */

LONG ASM
execute (REG(d1) char *comm,
	 REG(d2) BPTR instr,
	 REG(d3) BPTR outstr)
{
	return (internal_run(comm,bootshell,
			     RUN_EXECUTE,instr,outstr,NULL) == 0 ?
		DOSTRUE : FALSE);
}

/* does NOT continue reading from instr after done with command! */

LONG ASM
system (REG(d1) char *comm,
	REG(d2) struct TagItem *tags)
{
	BPTR in,out,asynch;
	char *shell;

/* FIX!!!!!! if NULL is specified, don't use input() (check cli_init_run)! */
	in     = getTagData(SYS_Input,input(),tags);
	out    = getTagData(SYS_Output,output(),tags);
	asynch = getTagData(SYS_Asynch,FALSE,tags);
	if (getTagData(SYS_UserShell,FALSE,tags))
	{
		/* user shell specified.  Use "Shell" off the resident list */
		shell = shellname;	/* "shell" */
	} else {
		/* did he specify a specific shell to send it to? */
		shell = (char *) getTagData(SYS_CustomShell,(LONG) bootshell,
					    tags);
	}

/*	err    = getTagData(SYS_Error,stderr(),tags);	*/

	/* if comm is NULL, this is Run calling us - make it look like Execute */
	/* Run should specify Usershell */
	return internal_run(comm,shell,
			    comm ? (asynch ? RUN_SYSTEM_ASYNCH : RUN_SYSTEM) :
				   RUN_EXECUTE,
			    in,out,tags);
}

/* these tags aren't allowed to be passed through */
Tag NotAllowedTags[] = {
	NP_Seglist,
	NP_FreeSeglist,
	NP_Entry,
	NP_Input,
	NP_Output,
	NP_CloseInput,
	NP_CloseOutput,
	NP_CurrentDir,		/* handled via packet */
	NP_HomeDir,		/* shell stuffs this  */
	NP_Cli,
	TAG_END,
};

/* this function is too big */

LONG __regargs
internal_run (char *command, char *shell, LONG type,
	      BPTR instr, BPTR outstr, struct TagItem *tags)
{
	struct Process *tid  = MYPROC;
	struct MsgPort *task;			/* the new process */
	BYTE pri = tid->pr_Task.tc_Node.ln_Pri;	/* use my current priority */
	LONG rc = -1, tnum;			/* -1 == error */
	struct DosPacket  *pkt = NULL;
	struct FileHandle *cvec;			/* no init needed */
	struct Segment *seg;
	char *svec;					/* buffer for fake fh */
	LONG len = command ? strlen(command) : 0;
	BPTR newcurrdir = NULL;
	/* put tags on stack, so we can modify seglist */
	/* We want the cli_init_run to do the funky stuff */
	struct TagItem *filtered_tags = NULL,*tag;
	/* can't move the first 3 items! */
	struct TagItem crptags[] = {
		{ NP_Seglist,NULL },	/* code depends on this being first */
		{ NP_WindowPtr, 0 },	/* and this being second */
		{ NP_Priority,0 },	/* and this third */
		{ NP_StackSize,800*4 },	/* and this fourth */
		{ NP_Path, NULL },
		{ NP_Name,(LONG) "Background CLI" },
		{ NP_FreeSeglist,FALSE },
		{ NP_Input,NULL },	/* set up by cli_init_run */
		{ NP_Output,NULL },
		{ NP_CloseInput,FALSE },
		{ NP_CloseOutput,FALSE },
/*		{ NP_Error,NULL },
		{ NP_CloseError,FALSE }, */
		{ NP_CurrentDir,NULL },
		{ NP_HomeDir,NULL },	/* shell sets cdir/homedir */
		{ NP_Cli,TRUE },
		{ TAG_MORE,NULL},	/* pass through tags after filter */
		{ TAG_END,NULL},	/* not really needed? */
	};
/*
 *  We need to be able to supply the input channel sperately from the command
 *  channel.  The way we'll do this is to look like a execute file to the
 *  shell (building a fake filehandle).  The shell uses cli_StandardInput
 *  as the input filehandle for invoked processes.  It uses cli_CurrentInput
 *  to read commands from.  What we'll do is set StandardInput to our input
 *  channel, and CurrentInput to our fake filehandle.  We also need to flag
 *  the shell (via the message and the rc from cli_init_run) that it should
 *  exit when it hits eof on the CurrentInput instead of continuing to read
 *  commands from StandardInput.
 */

/*kprintf("Execute/System(\"%s\",0x%lx,0x%lx, shell = %s, type = %ld)\n",
command,instr,outstr,shell,type);*/

	/* build fake fh if not Run */
	if (command)
	{
		/* safe, since cvec will be freeVec()ed, and it has no
		   actendread */
		cvec = (struct FileHandle * )
			AllocVecPubClear(sizeof(struct FileHandle) + 1 + len);
		if (!cvec)
			goto err;

		/* Create a suitable SCB */
		svec = (char *) ((LONG) cvec) + sizeof(struct FileHandle);
		copyMem(command,svec,len);	/* Copy over command string */
		svec[len] = '\n';		/* Terminate it with a '*N' */

		cvec->fh_Buf   = TOBPTR(svec);	/* must be on LW boundary */
		cvec->fh_End   = len+1;		/* Pos is 0 */

		/* some programs invoke at pri 20, and other stupid ones break */
		/* old execute happened to cause it to always be run at pri=0  */
		/* We do NOT limit it for Run! */
		if (type == RUN_EXECUTE)
			pri = 0;

	} else {
		/* This is C:Run.  It built a fake FH already for me. */
		/* The input/output streams will be NIL: or *. */
		cvec = (struct FileHandle *) BADDR(instr);
	}

	/* Why? FIX! */
/*	if (pri <= -123) goto err; */

	/* do this BEFORE creating process in case of failure! */
	if (!(pkt = AllocDosObject(DOS_STDPKT,NULL))) goto err;

	/* unfortunately, we MUST create Boot shell processes as BCPL */
	/* this is because we can have Shell-Seg installed as CLI     */
	/* No longer true.  I put it on the list as BootCLI, so a     */
	/* resident of CLI is ignored.  So I always use CreateNewProc */

	/* find the appropriate shell */
	seg = searchsegs(shell,NULL,TRUE);
	if (!seg)
		goto err;
	crptags[0].ti_Data = seg->seg_Seg;

	/* for synchronous calls, copy windowptr, else 0 */
	if (type != RUN_SYSTEM_ASYNCH)
		crptags[1].ti_Data = (LONG) tid->pr_WindowPtr;

	/* we can't put his taglist ahead of ours!!! */
	if (tag = findTagItem(NP_WindowPtr,tags))
		crptags[1].ti_Tag = TAG_IGNORE;

	/* Set the priority of the new task */
	crptags[2].ti_Data = pri;
	if (tag = findTagItem(NP_Priority,tags))	/* he speced, ignore */
		crptags[2].ti_Tag = TAG_IGNORE;

	/* use the same stacksize for the shell as the current process */
/*	crptags[3].ti_Data = tid->pr_StackSize*4;*/
/*
if (command)
kprintf("Would have set shell stack to %ld bytes for command %s\n",
tid->pr_StackSize*4,command);
else
kprintf("Would have set shell stack to %ld bytes for Run\n",
tid->pr_StackSize*4);
*/
	/* cli_init_run will check for path, and assume it's set   */
	/* if it finds one.					   */
	/* we set path to NULL for createnewproc, since it's	   */
	/* handled in cli_init_run.				   */

	if (!(filtered_tags = cloneTagItems(tags)))
		goto err;			 /* ran out of mem */

	/* handle tags that have to be passed via the packet */
	if (tag = findTagItem(NP_CurrentDir,tags))
		newcurrdir = tag->ti_Data;

	/* tricky calc so I don't need to know the size of crptags */
	/* TAG_END is last, TAG_MORE is second to last		   */ 
	crptags[sizeof(crptags)/sizeof(crptags[0]) - 2].ti_Data = 
		(LONG) filtered_tags;

	filterTagItems(filtered_tags,NotAllowedTags,TAGFILTER_NOT);

	task = (void *) CreateNewProc(crptags);
	if (!task)
		goto err;
	tnum = ((struct Process *) task)->pr_TaskNum;

/*kprintf("sending packet...\n");
{ int i; struct CommandLineInterface *clip;
clip = BADDR(((struct Process *) task)->pr_CLI);
if (clip) {
for (i  = 0; i < sizeof(*clip)/4; i++) {
kprintf("0x%lx ",((long *) clip)[i]);
if ((i+1)%4 == 0) kprintf("\n");}
if ((i+2)%4 != 0) kprintf("\n");
}
kprintf("\n");
}*/
	task = &(((struct Process *) task)->pr_MsgPort);

	/* make sure he only sees his stacksize, not the one we gave to CNP */
	/* note: we only pass HIS tags to cli_init_run!!! Therefore he      */
	/* can't see nay of my tags */
	
	/* send the startup packet to the new process. */

	/* The Run case uses arg3 to find out if redirection occurred */

	pkt->dp_Type = type;		/* RUN_EXECUTE or RUN_SYSTEM */
	pkt->dp_Res1 = 0;		/* MUST BE 0 */
	pkt->dp_Res2 = 0;		/* MUST BE 0 */
	pkt->dp_Arg1 = tid->pr_CLI;
	pkt->dp_Arg2 = instr;		/* for execute/system, not run */
	pkt->dp_Arg3 = outstr;		/* for execute/system, not run */
	pkt->dp_Arg4 = TOBPTR(cvec);	/* Command fh - all cases */
	pkt->dp_Arg5 = newcurrdir ? newcurrdir :
				    copydir(currentdir(FALSE,NULL));
	pkt->dp_Arg6 = (command != NULL); /* boolean - !is_run */
	pkt->dp_Arg7 = (LONG) filtered_tags;	/* so c_i_run can look */

	SendPkt(pkt,task,&(tid->pr_MsgPort));

	/* wait for packet to come back - immediate for run & sys_asynch */
	taskwait();

	/* for c:Run */
	if (!command) writef("[CLI %N]\n",tnum);

	/* for Execute and System synch, tell console handler to send	*/
	/* signals to this task again.					*/
	if (command != NULL && type != RUN_SYSTEM_ASYNCH && instr != NULL)
	{
		struct FileHandle *fh = BADDR(instr);

		if (fh->fh_Type) {
			sendpkt3(fh->fh_Type,ACTION_CHANGE_SIGNAL,
				 fh->fh_Arg1,(LONG) &(tid->pr_MsgPort),0);
		}
		/* ignore result */
	}

	/* get both return codes */
	/* For Execute(), if we ran it, always return success */
	/* if we got to here for Run, we have succeeded */
	/* run comes in via System, but with RUN_EXECUTE and NULL command */
	rc = ((type == RUN_EXECUTE && command) ? 0 : pkt->dp_Res1);

	/* don't lose things if shell startup failed! */
	/* if it succeeded, cli_init_run clears arg4 and arg5 */
	/* shell startup failed.  Always return -1 so caller cleans up */
	if (pkt->dp_Arg4) {
		rc = -1;			/* failure! */
		endstream(pkt->dp_Arg4);	/* cvec */
	}

	/* cli_init_run doesn't CurrentDir until after all failures! */
	/* it clears arg5 if it sets up the currentdir */
	freeobj(pkt->dp_Arg5);	/* even if passed in! (NULL is ok) */

	/* set secondary result after any other dos calls that hit it */
	setresult2(pkt->dp_Res2);

/*kprintf("execute returned (%ld,%ld)\n",rc,pkt->dp_Res2);*/
	/* cvec is freed by endstream - lucky! */
	cvec = NULL;  /* either owned by new shell or closed on error */

	/* the cli removes it's own tnum from array on exit */
err:
	if (filtered_tags) freeTagItems(filtered_tags);
	if (pkt) FreeDosObject(DOS_STDPKT,pkt);

	/* freeVec(NULL) is safe */
	freeVec(cvec);

	/* only writes if command succeeded, therefore it's ok to kill res2 */
	if (!command && rc)
		writes(getstring(STR_CANT_CREATE_PROC));

	return rc;
}

/* Must be called under Forbid()! */
/* find a free task number, creating if needed */

LONG __regargs
find_tnum (struct CliProcList **clip)
{
	struct CliProcList *cpl, *next;
	struct RootNode *rootnode = rootstruct();
	LONG tnum;

	/* check each small array of task numbers */
	for (cpl = (struct CliProcList *) rootnode->rn_CliList.mlh_Head;
	     cpl && cpl->cpl_Node.mln_Succ;
	     cpl = next)
	{
		/* check each entry in the small array */
		for (tnum = cpl->cpl_First;
		     tnum < cpl->cpl_First + (LONG) cpl->cpl_Array[0];
		     tnum++)
		{
			/* if the entry is NULL we can use it */
			if (cpl->cpl_Array[tnum+1 - cpl->cpl_First] == NULL)
				goto got_task;
		}
		/* tnum is now first+array[0] - 21 for example */
		/* tnum will be the number of the first entry of next array */

		/* grab the next small array - if no more, allocate one */
		next = (struct CliProcList *) cpl->cpl_Node.mln_Succ;
		if (next->cpl_Node.mln_Succ == NULL)
		{
		    /* this is the code that handles any number of CLI's! */
		    /* add new array to task array - same size as last */
		    /* array[0] has number of entries after it, +1 for [0] */
		    /* *4 because they're longwords, plus the struct */
		    next = AllocVecPubClear((((LONG) cpl->cpl_Array[0])+1) * 4 +
					     sizeof(struct CliProcList));
		    if (next)
		    {
			/* initialize the new array portion */
			/* array starts after the CliProcList struct */
			next->cpl_Array = (struct MsgPort **)
					  (((LONG) next) +
					   sizeof(struct CliProcList));
			next->cpl_Array[0] = cpl->cpl_Array[0];
			next->cpl_First = tnum;
			AddTail((struct List *) &(rootnode->rn_CliList),
				(struct Node *) &(next->cpl_Node));

			/* and go round loop again - this time WILL succeed */
		    }
		    /* else we fall out of the loop and error */
		} /* try next table */
	} /* for cpl... */

got_task:
	/* if tnum is non-zero, cpl is live - if tnum == 0, cpl == NULL */
	*clip = cpl;

	return tnum;
}

void __regargs
kill_cli_num (LONG tnum)
{
	struct CliProcList *cl;

	for (cl = (struct CliProcList *) rootstruct()->rn_CliList.mlh_Head;
	     cl && cl->cpl_Node.mln_Succ;
	     cl = (struct CliProcList *) cl->cpl_Node.mln_Succ)
	{
		/* is the entry in this small array? */
		if (tnum <= cl->cpl_First + ((LONG) cl->cpl_Array[0]) - 1)
		{
			cl->cpl_Array[tnum + 1 - cl->cpl_First] = NULL;
			return;
		}
	}
}

