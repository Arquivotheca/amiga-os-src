/*
 *  jcc_custom.c  -  Custom library initialization goes here...
 *
 *  This contains the custom LibInit and LibExpunge routines for
 *  the library we are making...
 *
 */
#include <exec/types.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <exec/semaphores.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "dos.h"

#include "jccbase.h"

/* The prototypes for the custom library init and expunge. */
BOOL __saveds __asm CustomLibInit(register __a6 struct JCCLibrary *libbase);
void __saveds __asm CustomLibExpunge(register __a6 struct JCCLibrary *libbase);

/* This is for libraries base we need in our library */
#define JCCBase	((struct JCCLibrary *)getreg(REG_A6))
#define SysBase	(JCCBase->jl_SysBase)

struct SignalSemaphore MyDataSemaphore;

/*
 * This is the custom LibInit code.
 *
 * This is where the library specific initialization code should be.
 *
 *  *** NOTE ***  Note that it is best to use a little stack
 *                as possible as you are being called from some
 *                other process's context.
 *
 *                Also note that the *FIRST* thing that must happen
 *                in this routine is to set up SysBase as shown...
 */
BOOL __saveds __asm CustomLibInit(register __a6 struct JCCLibrary *libbase)
{
	/* Get SysBase set up... this MUST happen *FIRST* */
	libbase->jl_SysBase = (struct ExecBase *)(* ((ULONG *)4));

	/* Initialize our semaphore */
	InitSemaphore(&MyDataSemaphore);

	if (!(libbase->jl_UtilityBase = (struct Library *)
				OpenLibrary("utility.library", 0L)))
	{
		Alert(AG_OpenLib|AO_UtilityLib);
		return(FALSE);
	}
	return(TRUE);
}


/*
 * This is the custom LibExpunge code.
 *
 * This is where the library specific expunge code should be placed.
 *
 *  *** NOTE ***  This routine *MUST* *NEVER* *BREAK* *FORBID*
 *                That is, it must not do *ANYTHING* that could
 *                cause a Wait() to happen...
 *
 *                Also note that it is best to use a little stack as possible as
 *		  you are being called from some other process's context.
 */
void __saveds __asm CustomLibExpunge(register __a6 struct JCCLibrary *libbase)
{
	/* Close the libraries it uses: */
	if (libbase->jl_UtilityBase)
	{
		CloseLibrary((struct Library *)libbase->jl_UtilityBase);
		libbase->jl_UtilityBase = NULL;
	}
}

