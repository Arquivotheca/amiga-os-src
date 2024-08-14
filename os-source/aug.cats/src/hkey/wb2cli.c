#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

/* prototype */
LONG __asm WB2CLI (register __a0 struct WBStartup * wbmsg,
		    register __d0 ULONG DefaultStack,
		    register __a1 struct Library * DOSBase);


/*****************************************************************************/


LONG __asm WB2CLI (register __a0 struct WBStartup * wbmsg,
		    register __d0 ULONG DefaultStack,
		    register __a1 struct Library * DOSBase)
{
    struct Process *process;
    struct CommandLineInterface *cli;
    struct MsgPort *wbport;
    struct Process *wbproc;
    struct CommandLineInterface *wbcli;
    ULONG *wbpath = NULL;
    ULONG *lastpath;
    ULONG *tmp;

    process = (struct Process *) FindTask (NULL);
    cli = BADDR (process->pr_CLI);

    /*
     * Check if we already have a CLI or no WBStartup
     */
    if ((!cli) && (wbmsg))
	if (cli = AllocDosObjectTagList (DOS_CLI, NULL))
	{

	    /*
	     * Set the default stack for the CLI...
	     */
	    cli->cli_DefaultStack = ((DefaultStack + 3) >> 2);

	    /*
	     * Connect the CLI to the process
	     */
	    process->pr_CLI = MKBADDR (cli);

	    /*
	     * Tell DOS that it will free the CLI...
	     */
	    process->pr_Flags |= PRF_FREECLI;

	    /*
	     * At this point, we "worked"  We now try to get the path but it
	     * may not be there or it may be NULL or it may be impossible to
	     * copy it...
	     */
	    Forbid ();		/* This is needed for current systems */

	    /*
	     * Now, find our launching process (usually Workbench) There are
	     * lots of checks here to make sure it is safe.
	     *
	     * Check for a reply port
	     */
	    if (wbport = wbmsg->sm_Message.mn_ReplyPort)
	    {			/* Check that it is a PA_SIGNAL */
		if (((wbport->mp_Flags) & PF_ACTION) == PA_SIGNAL)
		{		/* Check to make sure there is a SigTask */
		    if (wbproc = wbport->mp_SigTask)
		    {		/* Make sure it is a process */
			if (wbproc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
			{	/* Make sure the process is a CLI */
			    if (wbcli = BADDR (wbproc->pr_CLI))
			    {	/* Get the pointer to the path */
				wbpath = BADDR (wbcli->cli_CommandDir);
			    }
			}
		    }
		}
	    }

	    /*
	     * Did we get a path to copy?
	     */
	    if (wbpath)
	    {

		/*
		 * Cute trick to point as first pointer...
		 */
		lastpath = &(cli->cli_CommandDir);

		/*
		 * Now, start the copy...
		 *
		 * It is very important to do this as given here as the system
		 * will then deallocate the path and it must be in the right
		 * form.
		 *
		 * Also, we check for NULL path entries and do not copy them (they
		 * are used in workbench as "to be released" entries)
		 */
		while (wbpath)
		    if (wbpath[1])
		    {		/* Allocate the next entry in the lock list */
			if (tmp = AllocVec (8, MEMF_CLEAR | MEMF_PUBLIC))
			{	/* If the lock did not DupLock, we abort the
				 * rest of the path copy */
			    if (!(tmp[1] = DupLock (wbpath[1])))
			    {
				FreeVec (tmp);
				tmp = NULL;
			    }

			    /*
			     * Advance the workbench path pointer to the next
			     * entry...
			     */
			    wbpath = BADDR (wbpath[0]);
			}

			/*
			 * Now, we have the next lock element... so link it in
			 * if it exists.  If not, we just stop copying...
			 */
			if (tmp)
			{
			    lastpath[0] = MKBADDR (tmp);
			    lastpath = tmp;
			}
			else
			    wbpath = NULL;
		    }
		    else
			wbpath = BADDR (wbpath[0]);
	    }

	    Permit ();
	}

    return ((LONG) cli);
}
