/*
 * $Id: DumpVF.c,v 40.2 93/02/25 15:26:31 mks Exp $
 *
 * $Log:	DumpVF.c,v $
 * Revision 40.2  93/02/25  15:26:31  mks
 * No longer uses the internal (private) include
 * 
 * Revision 40.1  93/02/18  15:39:49  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.4  91/10/18  15:26:05  mks
 * Very minor code cleanup
 *
 * Revision 1.3  91/10/17  10:24:44  mks
 * Cleaned up formatting so it will print nice
 *
 * Revision 1.2  91/10/15  10:19:35  mks
 * Added standard version string to the commands
 *
 * Revision 1.1  91/10/11  08:27:28  mks
 * Moved FreeArgs() call to before the CloseLibrary(DOSBase)
 *
 * Revision 1.0  91/09/16  14:53:46  mks
 * Initial revision
 *
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/alerts.h>

#include <string.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include <utility/tagitem.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>

#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/dosasl.h>
#include <dos/rdargs.h>

#define DOSLIB	   "dos.library"
#define DOSVER	   37L

#define OPENFAIL { Result2(ERROR_INVALID_RESIDENT_LIBRARY); }
#define EXECBASE (*(struct ExecBase **)4)

#define THISPROC    ((struct Process *)(EXECBASE->ThisTask))
#define Result2(x)  THISPROC->pr_Result2 = x

/******************************************************************************/

#include	<exec/ports.h>
#include	<exec/libraries.h>
#include	<exec/semaphores.h>

#include	<dos/doshunks.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>

#include	"dumpvf_rev.h"

/******************************************************************************/

#include	"vf.h"

/* This is the command template. */
#define TEMPLATE  "LINES/N" VERSTAG

#define	OPT_LINES	0
#define	OPT_COUNT	1

LONG cmd_dumpvf(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
struct	RDArgs		*rdargs;
struct	VF_Sem		*vf;
	LONG		opts[OPT_COUNT];
	LONG		rc;
	ULONG		lines;

	rc=RETURN_FAIL;
	if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
	{
		memset((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts, NULL);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(),NULL);
		}
		else
		{
			lines=4;
			if (opts[OPT_LINES]) lines=*(ULONG *)opts[OPT_LINES];
			if (lines<1) lines=1;
			if ((lines*8) > FULL_SIZE) lines=FULL_SIZE/8;

			/*
			 * We need to make sure that no one gets
			 * in here for a bit...
			 */
			Forbid();
			if (vf=(struct VF_Sem *)FindSemaphore(VF_NAME))
			{
				ObtainSemaphore((struct SignalSemaphore *)vf);
			}
			Permit();

			if (vf)
			{
			ULONG	pos=0;

				while (lines--)
				{
					VPrintf("%08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",&(vf->Memory[pos]));
					pos+=8;
				}

				ReleaseSemaphore((struct SignalSemaphore *)vf);
			}
			else PutStr("Can not find VF Semaphore.\n");
			FreeArgs(rdargs);
		}

		CloseLibrary((struct Library *)DOSBase);
	}
	else
	{
		OPENFAIL;
	}
	return(rc);
}
