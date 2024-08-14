/* createproc.c */

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

#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "klib_protos.h"
#include "blib_protos.h"
#include "crp_protos.h"
#include "env_protos.h"
#include "path_protos.h"
#include "cli_protos.h"

#define SAME		0		/* for string compares */

#define TOBPTR(x)	(((LONG) (x)) >> 2)

extern UBYTE b_prompt[];
extern UBYTE doslibname[];

/* mappings from tags to flags in the pr_Flags field */
struct TagItem flagmap[] = {
	{ NP_FreeSeglist,PRF_FREESEGLIST },
	{ NP_Cli,	 PRF_FREECLI },
	{ NP_CloseOutput,PRF_CLOSEOUTPUT },
	{ NP_CloseInput, PRF_CLOSEINPUT },
/*	{ NP_CloseError, PRF_CLOSEERROR }, */
	{ NP_Arguments,  PRF_FREEARGS },
	{ TAG_DONE }
};

struct Process * ASM
CreateNewProc (REG(d1) struct TagList *tags)
{
	struct Process *np,*currproc = MYPROC;
	BPTR seglist,entry;
	LONG stack;
	char *name,*args = NULL;
	struct CommandLineInterface *cli = NULL;
	struct TagItem *tag;
	struct CliProcList *cpl; 
	char opened_input = FALSE;
	char opened_output = FALSE;
	char is_process = (currproc->pr_Task.tc_Node.ln_Type == NT_PROCESS);

/*kprintf("In CreateNewProc, is_process = 0x%lx\n",is_process);*/
	if (getTagData(NP_Cli,0,tags))
	{
		cli = makecli(currproc,tags);
		if (!cli)
			return NULL;
	}

	/* stacksize is in bytes */
	stack = getTagData(NP_StackSize,4000,tags);
	name  = (char *) getTagData(NP_Name,(LONG) "New Process",tags);

	/* these are mutually exclusive!!!  Also, ONE of these MUST be used! */
	seglist = getTagData(NP_Seglist,NULL,tags);
	entry   = getTagData(NP_Entry,NULL,tags);

	/* This call allocates process & stack, and sets up the appropriate */
	/* fields, and copies the name into the process.		    */
	/* inits most set fields in the process, sets up stack ptrs, pushes */
	/* the stacksize onto the stack, sets pr_ReturnAddr.		    */

	np = MakeProc(&stack,name,strlen(name));
	if (!np)
		goto cleanup;

	/* This builds the segarray (array of seglists) and links it in.    */
	/* It allocates and builds a fake seglist if needed (for NP_Entry). */
	/* use default segarray in ROM */
	if (!AddSegArray(np,seglist,entry))
		goto cleanup;

/*kprintf("stack is now %ld, seglist = 0x%lx\n",stack,
((LONG *)BADDR(np->pr_SegList))[3]);
kprintf("First instruction is 0x%lx\n",
((UWORD *)BADDR(((LONG *)BADDR(np->pr_SegList))[3]))[2]);*/


	/* flags - CNP() always has a currentdir, unless speced as NULL */
	np->pr_Flags = packBoolTags(PRF_FREECURRDIR |
/*				    PRF_CLOSEERROR | */
				    PRF_CLOSEOUTPUT |
				    PRF_CLOSEINPUT,
				    tags,flagmap);

	np->pr_Task.tc_Node.ln_Pri =
		getTagData(NP_Priority,currproc->pr_Task.tc_Node.ln_Pri,tags);
	if (cli)
		cli->cli_DefaultStack = stack>>2;	/* longwords */

	/* inherited fields */
	if (is_process)
	{
		LONG wp = (LONG) currproc->pr_WindowPtr;

		/* don't copy windowptr from parent unless 0 or -1! */
		if (wp != 0 && wp != -1)
			wp = 0;

		np->pr_ConsoleTask = (void *) getTagData(NP_ConsoleTask,
					  (LONG) currproc->pr_ConsoleTask,tags);
		np->pr_WindowPtr   = (void *) getTagData(NP_WindowPtr,wp,tags);
		np->pr_FileSystemTask = currproc->pr_FileSystemTask;

		/* should we copy the parents local vars? */
		if (getTagData(NP_CopyVars,TRUE,tags))
		{
			if (!copyvars(np,currproc))  /* fail on lack of mem */
				goto cleanup;
		}
	} else {
		np->pr_ConsoleTask = (void *) getTagData(NP_ConsoleTask,NULL,
							 tags);
		np->pr_WindowPtr   = (void *) getTagData(NP_WindowPtr,NULL,
							 tags);
		np->pr_FileSystemTask = taskrootstruct()->rn_BootProc;
					/* evil, FIX! */
	}

	np->pr_CLI = TOBPTR(cli);

	/* set the currentdir and homedir */
	/* Check for failure? FIX */
	tag = findTagItem(NP_CurrentDir,tags);
	np->pr_CurrentDir = tag ? (BPTR) tag->ti_Data :
				  (is_process ? copydir(currproc->pr_CurrentDir) :
						NULL);
	/* if no current dir, don't free it on exit!! */
	/* this is so it can be used to spawn WB programs, that leave */
	/* the currentdir set to wa_Lock.  NewShell can use it too.   */
	if (!np->pr_CurrentDir)
		np->pr_Flags &= ~PRF_FREECURRDIR;

	tag = findTagItem(NP_HomeDir,tags);
	np->pr_HomeDir = tag ? (BPTR) tag->ti_Data :
			       (is_process ? copydir(currproc->pr_HomeDir) :
					     NULL);
	
	/* set the input streams */
	/* FIX! check for Open failure! */
	/* FIX!!!  save in second place in process because of MANX!!!! */

	/* we GUARANTEE it's safe to Open("NIL:",...) from a Task!!!!! */

	tag = findTagItem(NP_Input,tags);
	if (tag)
		np->pr_CIS = tag->ti_Data;
	else {
		np->pr_CIS = findstream("NIL:",MODE_OLDFILE);
		opened_input = TRUE;
	}
/*kprintf("input stream = 0x%lx\n",np->pr_CIS);*/
	if (cli)
	{
		cli->cli_StandardInput = np->pr_CIS;
		cli->cli_CurrentInput  = np->pr_CIS;
	}

	/* output */
	/* FIX! check for Open failure! */
	tag = findTagItem(NP_Output,tags);
	if (tag)
		np->pr_COS = tag->ti_Data;
	else {
		np->pr_COS = findstream("NIL:",MODE_NEWFILE);
		opened_output = TRUE;
	}
/*kprintf("output stream = 0x%lx\n",np->pr_COS);*/
	if (cli)
	{
		cli->cli_StandardOutput = np->pr_COS;
		cli->cli_CurrentOutput  = np->pr_COS;
	}

/*	np->pr_CES = getTagData(NP_Error,stderr(),tags);	*/

	np->pr_ExitCode = (void *) getTagData(NP_ExitCode,NULL,tags);
	np->pr_ExitData = getTagData(NP_ExitData,NULL,tags);

	/* set up arguments - startup code stuffs into a0/d0 */
	/* pr_Arguments is already NULL */
	tag = findTagItem(NP_Arguments,tags);
	if (tag)
	{
		args = AllocVecPub(strlen((char *) tag->ti_Data)+1);
		if (!args)
			goto cleanup;
		strcpy(args,(char *) tag->ti_Data);
		np->pr_Arguments = args;
	}

	if (!seglist) {
		/* find the fake seglist ptr - we always want to free it */
		seglist = ((LONG *) BADDR(np->pr_SegList))[3];
	}

	Forbid(); /* locking so we can set the entry in the cpl array */

	if (cli) {
		cli->cli_Module = seglist;
		np->pr_TaskNum  = find_tnum(&cpl);
		if (!np->pr_TaskNum)
		{
			/* must have failed an alloc */
			Permit();

			goto cleanup;
		}
	}

#ifdef DEATH
	/* should we wait for synchronous reply? */
	if (getTagData(NP_Synchronous,FALSE,tags))
	{
		???
	}
#endif


/*kprintf("Adding New Process at 0x%lx... %s\n",np,np->pr_Task.tc_Node.ln_Name);
kprintf("flags = 0x%lx\n",np->pr_Flags);
kprintf("Execution begins at 0x%lx\n", ((LONG *) BADDR(seglist+1)));*/

	/* CNP_ActiveCode sets up a0/d0 and calls seglist in segarray[3] */
	if (!AddTask(&(np->pr_Task),(void *) CNP_ActiveCode,
				    (void *) deactcode))
	{
		/* AddTask failed!!!! */
		Permit();
		goto cleanup;
	}
/*kprintf("Added it\n");*/

	/* set the tasknum of our new process (cpl is live from find_tnum() */
	if (cli)
		cpl->cpl_Array[np->pr_TaskNum + 1 - cpl->cpl_First] =
							   &(np->pr_MsgPort);
	Permit();

	return np;

cleanup:
	if (np)
	{
		if (opened_input)
			endstream(np->pr_CIS);		/* NULL is ok */
		if (opened_output)
			endstream(np->pr_COS);
/*			endstream(np->pr_CES);	*/
		freevars(np);
		freeVec(args);			/* NULL is OK */
		freeobj(np->pr_CurrentDir);	/* NULL is OK */
		freeobj(np->pr_HomeDir);
		freeVec((void *) BADDR(np->pr_SegList));
	}

	/* if we failed, don't deallocate their path, if supplied */
	if (findTagItem(NP_Path,tags))
		cli->cli_CommandDir = NULL;

	/* if we failed to allocate cli, it returns immediately   */
	freecli(cli,np ? np->pr_TaskNum : 0);	/* NULL is ok */
	if (np)
	{
		/* frees np itself */
		FreeEntry((struct MemList *) np->pr_Task.tc_MemEntry.lh_Head);
	}
	return NULL;
}

struct CommandLineInterface * __regargs
makecli (struct Process *currproc, struct TagItem *tags)
{
	struct CommandLineInterface *cli,*mycli = (void *)BADDR(currproc->pr_CLI);
	UBYTE *ptr,*optr;
	struct TagItem *tag;

	/* Careful!  Old process may not be CLI or even process! */

	/* most things inited to NULL, faillevel set */
	cli = AllocCli(tags);
	if (!cli)
		return NULL;

	/* set up ptrs and copy data */
	ptr  = BADDR(cli->cli_Prompt);
	optr = NULL;
	if (mycli)
		optr = (UBYTE *) BADDR(mycli->cli_Prompt);
	if (!optr)
		optr = (UBYTE *) b_prompt;	/* make sure we get prompt */
	copyMem(optr,ptr,*optr+1);

	ptr  = BADDR(cli->cli_CommandName);
	tag  = findTagItem(NP_CommandName,tags);
	if (tag)
	{
			*ptr = strlen((char *) tag->ti_Data);
		copyMem((void *) tag->ti_Data,ptr+1,*ptr);
	} else {
		if (mycli)
		{
			optr = (UBYTE *) BADDR(mycli->cli_CommandName);
			if (optr)
				copyMem(optr,ptr,*optr+1);
		}
	}

	ptr  = BADDR(cli->cli_SetName);
	tag  = findTagItem(NP_CurrentDir,tags);
/* FIX!! do something about tasks! */
	if (tag && currproc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
	{
		*ptr = 0;
		/* FIX! should we do something different on error? */
		/* FIX! use SetDirName */
		if (NameFromLock(tag->ti_Data,ptr+1,SETMAX*4-1))
			*ptr = strlen(ptr+1);
	} else {
		optr = NULL;
		if (mycli)
			optr = (UBYTE *) BADDR(mycli->cli_SetName);
		if (!optr)
			optr = "";	/* should do NameFromLock! FIX! */
		copyMem(optr,ptr,*optr+1);
	}

	/* path */
	tag = findTagItem(NP_Path,tags);
	if (tag)
	{
		cli->cli_CommandDir = tag->ti_Data;
	} else {
		if (mycli)
			cli->cli_CommandDir = copylist(mycli->cli_CommandDir);
	}

	/* copy some stuff */
	if (mycli)
	{
		cli->cli_FailLevel   = mycli->cli_FailLevel;
	}
	/* allow background to default to DOSTRUE, interactive to false, etc */

	return cli;
}

struct CommandLineInterface * __regargs
AllocCli (struct TagItem *tags)
{
	struct CommandLineInterface *cli;

	cli = AllocVecPubClear(sizeof(struct CommandLineInterface));
	if (!cli)
		return NULL;

	/* set up ptrs */
	if (!(cli->cli_Prompt =	TOBPTR(AllocVecPubClear(
				    getTagData(ADO_PromptLen,
					       PROMPTMAX*4,tags)))))
		goto cleanup;

	if (!(cli->cli_CommandName = TOBPTR(AllocVecPubClear(
					getTagData(ADO_CommNameLen,
						   NAMEMAX*4,tags)))))
		goto cleanup;

	if (!(cli->cli_CommandFile = TOBPTR(AllocVecPubClear(
					getTagData(ADO_CommFileLen,
						   FILEMAX*4,tags)))))
		goto cleanup;

	if (!(cli->cli_SetName = TOBPTR(AllocVecPubClear(
					getTagData(ADO_DirLen,SETMAX*4,tags)))))
		goto cleanup;

	cli->cli_FailLevel   = CLI_INITIAL_FAIL_LEVEL;
	cli->cli_Background  = DOSTRUE;			/* default */

	return cli;

	/* freevec(0) is safe! */
cleanup:
	freeVec(BADDR(cli->cli_CommandFile));
	freeVec(BADDR(cli->cli_CommandName));
	freeVec(BADDR(cli->cli_Prompt));
	freeVec(cli);

	return NULL;
}

void __regargs
freecli (struct CommandLineInterface *cli, LONG tnum)
{
	if (cli)
	{
		freepath(cli->cli_CommandDir);

		/* kill before freeing cli struct */
		if (tnum)
			kill_cli_num(tnum);

		/* I had better have allocated it! */
		/* FIX? */
		/* high = *(((LONG *) cli)-1) + (LONG) cli;
		   if (BADDR(cli->cli_SetName) >= (LONG) cli &&
		       BADDR(cli->cli_SetName) < high)
			ARGH! I didn't allocate it!
		*/
		freeVec(BADDR(cli->cli_SetName));
		freeVec(BADDR(cli->cli_CommandFile));
		freeVec(BADDR(cli->cli_CommandName));
		freeVec(BADDR(cli->cli_Prompt));
		freeVec(cli);

	}
}

void __regargs
freepath (BPTR path)
{
	struct Path *p,*next;

	/* passed in as BPTR */
	p = BADDR(path);

	/* if we're started from a task, cli_CommandDir WILL be NULL */
	while (p)
	{
		next = (struct Path *) BADDR(p->path_Next);
		freeobj(p->path_Lock);
		freeVec(p);

		p = next;
	}
}

/* MUST be callable from a task! */

struct MsgPort * ASM 
createtaskproc (REG(a0) char *name, REG(a1) BPTR seg, REG(a2) BPTR gvec,
		REG(d0) LONG stacksize, REG(d1) LONG namelen,
		REG(d2) LONG pri, REG(d3) LONG task)
{
	struct Process *np,*currproc = MYPROC;
	LONG stack = stacksize;

	/* This call allocates process & stack, and sets up the appropriate */
	/* fields, and copies the name into the process.		    */
	/* inits most set fields in the process, sets up stack ptrs, pushes */
	/* the stacksize onto the stack, sets pr_ReturnAddr.		    */

	np = MakeProc(&stack,name,namelen);
	if (!np)
		return NULL;

	/* This builds the segarray (array of seglists) and links it in.    */
	/* Only for createproc, not for createtask */
	if (!task)
	{
		if (!AddSegArray(np,seg,NULL))
			goto cleanup;
		// segarray is added to tc_MemList, so it gets freed on RemTask

	} else {
		np->pr_SegList = seg;
		np->pr_GlobVec = BADDR(gvec);
	}

	np->pr_Task.tc_Node.ln_Pri = pri;

	/* inherited fields */
	if (currproc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
	{
		np->pr_ConsoleTask    = currproc->pr_ConsoleTask;
		np->pr_WindowPtr      = currproc->pr_WindowPtr;
		np->pr_FileSystemTask = currproc->pr_FileSystemTask;
	} else {
		/* leave others NULL */
		np->pr_FileSystemTask = taskrootstruct()->rn_BootProc;
					/* evil, FIX! */
	}

	/* can fail now */
	if (!AddTask(&(np->pr_Task),(void *) (task ? activecode :
						((LONG *) BADDR(seg+1))),
				    (void *) deactcode))
	{
		goto cleanup;
	}

	return &(np->pr_MsgPort);

cleanup:
	freeVec((void *) BADDR(np->pr_SegList));
	FreeEntry((struct MemList *) np->pr_Task.tc_MemEntry.lh_Head);

	return NULL;
}

struct MsgPort * ASM 
createproc (REG(d1) char *name, REG(d2) LONG pri, REG(d3) BPTR seg,
	    REG(d0) LONG stacksize)
{
	return createtaskproc(name,seg,NULL,stacksize,strlen(name),pri,FALSE);
}

struct MsgPort * ASM 
createtask (REG(d1) BPTR segarray, REG(d2) LONG stacksize, REG(d3) LONG pri,
	    REG(d0) BSTR bname, REG(a0) BPTR gvec)
{
	char *name = BADDR(bname);

	return createtaskproc(name+1,segarray,gvec,stacksize*4,*name,pri,TRUE);
}

/* version of rootstruct that's safe to call from a task (doesn't use dosbase)*/

struct RootNode * __regargs
taskrootstruct ()
{
	struct DosLibrary *DOSBase;

	DOSBase = (void *) OpenLibrary(doslibname,0);
	/* it MUST succeed */
	CloseLibrary((struct Library *) DOSBase);
	/* safe since we know we're open at least once */

	return DOSBase->dl_Root;
}

/* formerly contained in DEACT in doslib.asm */

LONG __regargs
CleanupProc (struct Process *p, LONG returncode)
{
	LONG flags = p->pr_Flags;
	LONG *segarray;

/*kprintf("Process %s ending, flags = 0x%lx\n",p->pr_Task.tc_Node.ln_Name,flags);*/

	/* call pr_ExitCode if it exists */
	if (p->pr_ExitCode)
	{
		/* function ptr, cal with d0 = returncode, d1 = pr_ExitData */
		/* icky function ptr cast - returns new returncode */
		returncode = (*((LONG (* __regargs)(LONG,LONG)) p->pr_ExitCode))
						(returncode,p->pr_ExitData);
	}

	/* should we free the seglist?  (segarray[3]) */
	segarray = (LONG *) BADDR(p->pr_SegList);

	if ((flags & PRF_FREESEGLIST) && segarray[3])
		unloadseg(segarray[3]);

	/* and free the array itself - always */
// Segarray is on the tc_MemEntry list now...
//	freeVec((char *) segarray);

	/* should we unlock to currentdir? */
	if (flags & PRF_FREECURRDIR)
		freeobj(p->pr_CurrentDir);

	/* always free homedir (may be NULL) */
	freeobj(p->pr_HomeDir);

	/* free local variables */
	freevars(p);

	/* clean up filehandles */
	if (flags & PRF_CLOSEOUTPUT)
		endstream(p->pr_COS);

	if (flags & PRF_CLOSEINPUT)
		endstream(p->pr_CIS);

/*
	if (flags & PRF_CLOSEERROR)
		endstream(p->pr_CES);
*/

	/* free arguments if passed in */
	if (flags & PRF_FREEARGS)
		freeVec(p->pr_Arguments);

	/* cli and related structures */
	if (flags & PRF_FREECLI)
	{
		struct CommandLineInterface *cli = BADDR(p->pr_CLI);

		p->pr_CLI = NULL;
		freecli(cli,p->pr_TaskNum); /* NULL ok */
	}

	/* free any private global vector */
	if (p->pr_GlobVec != dosbase()->dl_GV)
		freeVec(((char *) p->pr_GlobVec) - NEGATIVE_GLOBAL_SIZE*4);

	/* exec frees everything on the task mementry list: process, stack */
	/* also Segarray */
	return returncode;
}
