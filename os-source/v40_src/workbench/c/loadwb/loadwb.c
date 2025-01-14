/*
 * $Id: loadwb.c,v 38.5 91/10/11 08:32:08 mks Exp $
 *
 * $Log:	loadwb.c,v $
 * Revision 38.5  91/10/11  08:32:08  mks
 * Added the wbstart file directly to the source code
 * 
 * Revision 38.4  91/10/11  08:30:00  mks
 * Moved the FreeArgs() call to before the closing of dos.library
 *
 * Revision 38.3  91/07/24  14:34:47  mks
 * Changed to use IoErr() values for error strings
 * such that it is now localized.
 *
 * Revision 38.2  91/07/15  15:02:54  mks
 * Added keyboard shortcut for CleanUp if the cleanup option is used.
 *
 * Revision 38.1  91/06/24  11:40:52  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include <internal/commands.h>
#include "loadwb_rev.h"

/******************************************************************************/

#include        <exec/ports.h>
#include        <exec/libraries.h>
#include        <dos/dos.h>
#include        <dos/dosextens.h>

#include	<clib/wb_protos.h>
#include	<pragmas/wb_pragmas.h>

/*
 * These bits are from the bitfield that is used to start workbench.
 * Old LoadWB commands only set WBARGF_DEBUGON  New LoadWB may set
 * more bits.
 */

/* wb_Argument bits */
#define	WBARGB_DEBUGON	0		/* Turn on debugging */
#define	WBARGB_DELAYON	1		/* Delay start until ready... */
#define	WBARGB_CLEANUP	2		/* Auto CleanUp on initial open */
#define	WBARGB_NEWPATH	3		/* Install new path */

#define	WBARGF_DEBUGON	(1L << WBARGB_DEBUGON)
#define	WBARGF_DELAYON	(1L << WBARGB_DELAYON)
#define	WBARGF_CLEANUP	(1L << WBARGB_CLEANUP)
#define	WBARGF_NEWPATH	(1L << WBARGB_NEWPATH)

/******************************************************************************/

/*
 * This pragma is the entry point into workbench that starts it.
 * It is a private LVO and thus is not listed in the public LVO
 */
#pragma libcall WorkbenchBase StartWorkbench 2a 1002
BOOL StartWorkbench(ULONG, ULONG);

/*
 * This prototype is for the PatchWorkbench() function
 */
VOID __asm PatchWorkbench(register __a0 struct Library *);

extern char __far CleanUpKey[2];

/* This is the command template. */
#define TEMPLATE  "-DEBUG/S,DELAY/S,CLEANUP/S,NEWPATH/S" CMDREV

#define	OPT_DEBUG	WBARGB_DEBUGON
#define	OPT_DELAY	WBARGB_DELAYON
#define	OPT_CLEANUP	WBARGB_CLEANUP
#define	OPT_NEWPATH	WBARGB_NEWPATH
#define	OPT_COUNT	4

LONG cmd_loadwb(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
struct	Library		*WorkbenchBase;
struct	RDargs		*rdargs;
struct	Process		*proc;
	LONG		opts[OPT_COUNT];
	LONG		rc;

	rc=RETURN_FAIL;
	if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
	{
		memset((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts, NULL);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(),NULL);
		}
		else if (WorkbenchBase=OpenLibrary("workbench.library",36L))
		{
		BPTR	*cmdpath=NULL;
		BPTR	*lastpath=NULL;
		BPTR	*nextpath;
		BPTR	oldpath;

			rc=RETURN_OK;

			/*
			 * Note:  If CleanUp is selected, we add the
			 * keyboard option as needed
			 */
			if (opts[OPT_CLEANUP]) CleanUpKey[0]='.';

			/*
			 * Note:  The following path copy code is as follows
			 * for compatibility reasons.  As long as we need
			 * to support the current (2.00) workbench.library
			 * it will have to remain the same.  (Yuck!)
			 *
			 * One possible method would be to have LoadWB also
			 * revision check workbench and do the "Right" thing
			 * in that case.  Currently, that does not look like
			 * a good solution.
			 */
			oldpath=((proc=(struct Process *)(FindTask(NULL)))->pr_CLI) << 2;
			if (oldpath) oldpath=((struct CommandLineInterface *)oldpath)->cli_CommandDir;

			while ((oldpath) && (rc==RETURN_OK))
			{
				if (nextpath=AllocMem(8,MEMF_PUBLIC|MEMF_CLEAR))
				{
					if (!cmdpath) cmdpath=nextpath;
					else lastpath[0]=MKBADDR(nextpath);

					lastpath=nextpath;	/* For loop */
					nextpath=BADDR(oldpath);/* Current */
					oldpath=nextpath[0];	/* Next element */

					/*
					 * Try to make a copy of the path...
					 */
					if (!(lastpath[1]=DupLock(nextpath[1])))
					{
						rc=RETURN_FAIL;
					}
				}
				else rc=RETURN_FAIL;
			}

			if (rc==RETURN_OK)
			{
			ULONG	flags=NULL;
			short	i;
			APTR	oldcon;

				/*
				 * Set up the flags for the arguments
				 */
				for (i=0;i<OPT_COUNT;i++) if (opts[i]) flags |= (1L << i);

				/*
				 * Clear pr_CurrentDir for Pre-2.04 kickstart
				 * Clear pr_ConsoleTask for Pre-2.04 kickstart
				 */
				oldpath=CurrentDir(NULL);
				oldcon=proc->pr_ConsoleTask;
				proc->pr_ConsoleTask=NULL;

				/*
				 * Now call the magic workbench patching
				 * code...  This installs any patches
				 * to workbench in such a way that
				 * they will only be done once and
				 * be removed when completed...
				 */
				PatchWorkbench(WorkbenchBase);

				flags=StartWorkbench(flags,MKBADDR(cmdpath));
				proc->pr_ConsoleTask=oldcon;
				CurrentDir(oldpath);

				if (flags)
				{
					/* Workbench started, so we clear the path */
					cmdpath=NULL;
				}
				else rc=RETURN_WARN;
			}
			else PrintFault(IoErr(),NULL);

			/*
			 * Free the path if I still have it...
			 */
			while (cmdpath)
			{
				if (cmdpath[1]) UnLock(cmdpath[1]);
				nextpath=BADDR(cmdpath[0]);
				FreeMem(cmdpath,8);
				cmdpath=nextpath;
			}

			CloseLibrary(WorkbenchBase);
		}
		else PrintFault(ERROR_INVALID_RESIDENT_LIBRARY,NULL);

		if (rdargs) FreeArgs(rdargs);

		CloseLibrary((struct Library *)DOSBase);
	}
	else
	{
		OPENFAIL;
	}
	return(rc);
}
