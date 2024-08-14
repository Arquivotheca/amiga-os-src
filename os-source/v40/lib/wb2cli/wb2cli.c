/*
 * $Id: wb2cli.c,v 1.4 92/02/11 12:06:18 mks Exp $
 *
 * $Log:	wb2cli.c,v $
 * Revision 1.4  92/02/11  12:06:18  mks
 * Forgot to set prompt to NULL...
 * 
 * Revision 1.3  92/02/11  12:04:50  mks
 * Added Prompt String to the data that is copied from the shell...
 *
 * Revision 1.2  91/07/24  19:51:39  mks
 * Cleaned up a bit...
 *
 * Revision 1.1  91/07/24  18:59:04  mks
 * Added default stack argument and fixed the fact that the default
 * stack was 0.  This was causing major crashes.
 *
 * Revision 1.0  91/07/24  17:03:06  mks
 * Initial revision
 *
 */

/*
 * WB2CLI - By Michael Sinz
 *             Operating System Development Group
 *             Commodore-Amiga, Inc.
 *
 * This is the magic code needed to take a started workbench process
 * and convert it into a CLI process with a copy of the workbench path.
 *
 * After that point, the process will look much like a background CLI.
 */

#include	<exec/types.h>
#include	<exec/ports.h>
#include	<exec/nodes.h>
#include	<exec/memory.h>
#include	<workbench/startup.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>

#include	<clib/exec_protos.h>
#include	<clib/dos_protos.h>

#include	<pragmas/exec_pragmas.h>
#include	<pragmas/dos_pragmas.h>

#include	"wb2cli.h"

LONG __asm WB2CLI(register __a0 struct WBStartup *wbmsg,register __d0 ULONG DefaultStack,register __a1 struct DosLibrary *DOSBase)
{
struct	ExecBase		*SysBase = (*((struct ExecBase **) 4));
struct	Process			*process;
struct	CommandLineInterface	*cli;
struct	MsgPort			*wbport;
struct	Process			*wbproc;
struct	CommandLineInterface	*wbcli;
	char			*prompt=NULL;
	ULONG			*wbpath=NULL;
	ULONG			*lastpath;
	ULONG			*tmp;

	process=(struct Process *)FindTask(NULL);
	cli=BADDR(process->pr_CLI);

	/*
	 * Check if we already have a CLI or no WBStartup
	 */
	if ((!cli) && (wbmsg)) if (cli=AllocDosObjectTagList(DOS_CLI,NULL))
	{
		/*
		 * Set the default stack for the CLI...
		 */
		cli->cli_DefaultStack=((DefaultStack+3)>>2);

		/*
		 * Connect the CLI to the process
		 */
		process->pr_CLI=MKBADDR(cli);

		/*
		 * Tell DOS that it will free the CLI...
		 */
		process->pr_Flags|=PRF_FREECLI;

		/*
		 * At this point, we "worked"  We now try to get the path
		 * but it may not be there or it may be NULL or it
		 * may be impossible to copy it...
		 */
		Forbid();	/* This is needed for current systems */

		/*
		 * Now, find our launching process (usually Workbench)
		 * There are lots of checks here to make sure it is safe.
		 *
		 * Check for a reply port
		 */
		if (wbport=wbmsg->sm_Message.mn_ReplyPort)
		{	/*
			 * Check that it is a PA_SIGNAL
			 */
			if (((wbport->mp_Flags)&PF_ACTION) == PA_SIGNAL)
			{	/*
				 * Check to make sure there is a SigTask
				 */
				if (wbproc=wbport->mp_SigTask)
				{	/*
					 * Make sure it is a process
					 */
					if (wbproc->pr_Task.tc_Node.ln_Type==NT_PROCESS)
					{	/*
						 * Make sure the process is a CLI
						 */
						if (wbcli=BADDR(wbproc->pr_CLI))
						{	/*
							 * Get the pointer to the path
							 */
							wbpath=BADDR(wbcli->cli_CommandDir);

							/*
							 * Also copy the prompt
							 */
							prompt=BADDR(wbcli->cli_Prompt);
						}
					}
				}
			}
		}

		/*
		 * Did we get a prompt to copy?
		 */
		if (prompt) SetPrompt(&(prompt[1]));

		/*
		 * Did we get a path to copy?
		 */
		if (wbpath)
		{
			/*
			 * Cute trick to point as first pointer...
			 */
			lastpath=&(cli->cli_CommandDir);

			/*
			 * Now, start the copy...
			 *
			 * It is very important to do this as given here
			 * as the system will then deallocate the path and
			 * it must be in the right form.
			 *
			 * Also, we check for NULL path entries and do not
			 * copy them (they are used in workbench as "to be
			 * released" entries)
			 */
			while (wbpath) if (wbpath[1])
			{	/*
				 * Allocate the next entry in the lock list
				 */
				if (tmp=AllocVec(8,MEMF_CLEAR|MEMF_PUBLIC))
				{	/*
					 * If the lock did not DupLock, we
					 * abort the rest of the path copy
					 */
					if (!(tmp[1]=DupLock(wbpath[1])))
					{
						FreeVec(tmp);
						tmp=NULL;
					}

					/*
					 * Advance the workbench path pointer
					 * to the next entry...
					 */
					wbpath=BADDR(wbpath[0]);
				}

				/*
				 * Now, we have the next lock element...
				 * so link it in if it exists.  If not,
				 * we just stop copying...
				 */
				if (tmp)
				{
					lastpath[0]=MKBADDR(tmp);
					lastpath=tmp;
				}
				else wbpath=NULL;
			}
			else wbpath=BADDR(wbpath[0]);
		}

		Permit();
	}

	return((LONG)cli);
}
