/*
 * $Id: wb2cli.h,v 1.4 91/07/24 20:03:29 mks Exp $
 *
 * $Log:	wb2cli.h,v $
 * Revision 1.4  91/07/24  20:03:29  mks
 * Autodoc cleanup
 * 
 * Revision 1.2  91/07/24  18:58:42  mks
 * Forgot to fix the example to use default stack.
 *
 * Revision 1.1  91/07/24  18:57:46  mks
 * Added the defaultstack to the prototype and documentation.
 *
 * Revision 1.0  91/07/24  17:03:24  mks
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

/*
******* WB2CLI ***************************************************************
*
*   NAME
*	WB2CLI								(V37)
*
*   SYNOPSIS
*	status = WB2CLI(wbmsg,defaultstack,dosbase)
*	  D0             A0    D0           A1
*
*	LONG WB2CLI(struct WBStartup *,ULONG,struct DosLibrary *)
*
*   FUNCTION
*	This function will take a workbench started application and change
*	it into a CLI-like application.  This will include a path as copied
*	from the Workbench.  The CLI default stack is used to pick the stack
*	size for the processes run as CLIs.  (This is in SYNC mode)
*
*	You would pass in the Workbench message pointer and a pointer
*	to an open dos.library.  (The DosLibrary pointer is used to call
*	dos.library without the need of globals.)
*
*	This *REQUIRES* V37 dos.library and exec.library.
*	It may seem to work with V36 but certain bugs in V36 ROM will
*	make it unstable.
*
*   NOTES
*	Assembly language programmers:  This routine is _WB2CLI
*
*	If you call WB2CLI() and are already a CLI, it will return as if
*	it worked even though it did nothing.  (After all, you were a CLI)
*	For this reason, you do not need to check if you are a CLI
*	started program before you call the function.
*
*	One word of interest:  You should check the startup code of your
*	program.  It is important that on exit, the code replies the
*	WBStartup message if it has it and does not look at pr_CLI again.
*	The startup code that I have looked at was correct.  If the
*	message is not replied, the program's seglist is never unloaded.
*
*   EXAMPLE
*	void main(int argc, char *argv[])
*	{
*	struct  DosLibrary  *DOSBase;
*
*	    |* We need V37 to do this *|
*	    if (DOSBase=(struct DosLibrary *)OpenLibrary("dos.library",37L))
*	    {
*	        |* NOTE:  Most of the time you will ignore the error *|
*	        if (WB2CLI((struct WBStartup *)argv,4000,DOSBase))
*	        {
*	            |* We now are workbench started CLI *|
*	            |* (Complete with path *|
*	        }
*	        else
*	        {
*	            |* We did not become a CLI but we still *|
*	            |* can run.  Just no path *|
*	        }
*
*	        |* More stuff *|
*
*	        CloseLibrary((struct Library *)DOSBase);
*	    }
*	}
*
*   INPUTS
*	wbmsg -- The Workbench message that you were started with.  If NULL
*	         this routine will do nothing...
*
*	defaultstack -- The size of the CLI default stack in bytes.
*	                You *MUST* supply a value of at least 4000.
*
*	dosbase -- Pointer to dos.library (must be open)
*
*   RESULTS
*	A magically transformed process that has a CLI structure added.
*
*	status -- Returns TRUE if it worked.  Usually can be ignored
*	          as your program will continue to work as if you did not
*	          call this function.
*
*   SEE ALSO
*	None
*
*   BUGS
*
******************************************************************************
*/

#include	<dos/dosextens.h>
#include	<workbench/startup.h>

LONG __asm WB2CLI(register __a0 struct WBStartup *,register __d0 ULONG,register __a1 struct DosLibrary *);
