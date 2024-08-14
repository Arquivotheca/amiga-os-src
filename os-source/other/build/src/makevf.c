/*
 * $Id: MakeVF.c,v 40.7 93/03/09 15:13:59 mks Exp $
 *
 * $Log:	MakeVF.c,v $
 * Revision 40.7  93/03/09  15:13:59  mks
 * CDGS code...
 * 
 * Revision 40.6  93/03/04  11:47:54  mks
 * Added TAG keyword and made it do the split tag and
 * APPEND only does an APPEND version of CLEAR.
 *
 * Revision 40.5  93/03/04  10:31:41  mks
 * Changed APPEND to make room for the version longword
 *
 * Revision 40.4  93/03/03  14:10:27  mks
 * Changed the keyword to Append...
 *
 * Revision 40.3  93/03/03  12:07:41  mks
 * Added the second-half parameter required for multi-512K kickstart
 * builds...
 *
 * Revision 40.2  93/02/25  15:25:31  mks
 * No longer uses the internal (private) include
 *
 * Revision 40.1  93/02/18  15:38:55  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.8  91/12/13  19:55:58  mks
 * Fixed the VF Clear...  (major broken before)
 *
 * Revision 1.7  91/12/11  09:37:39  mks
 * Added the making of the VF structure in F-space for those
 * cases where the DIRECT keyword was used...
 *
 * Revision 1.6  91/12/06  07:19:17  mks
 * Added support for Direct mapping of the load space
 *
 * Revision 1.5  91/10/18  15:26:54  mks
 * Very minor code cleanup
 *
 * Revision 1.4  91/10/17  10:26:15  mks
 * Cleaned up formatting so it will print nice
 *
 * Revision 1.3  91/10/15  10:19:47  mks
 * Added standard version string to the commands
 *
 * Revision 1.2  91/10/11  08:25:15  mks
 * Moved FreeArgs() call to before the CloseLibrary(DOSBase)
 *
 * Revision 1.1  91/10/10  19:55:59  mks
 * Added the clearing of the global hunk counter
 *
 * Revision 1.0  91/09/16  14:54:06  mks
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

#include	<dos/dos.h>
#include	<dos/dosextens.h>

#include	"makevf_rev.h"

/******************************************************************************/

#include	"vf.h"

/*
 * This command can make/remove/clear a virtual F space needed for build...
 */

/* This is the command template. */
#define TEMPLATE  "REMOVE/S,CLEAR/S,DIRECT/S,APPEND/S,TAG/S" VERSTAG

#define	OPT_REMOVE	0
#define	OPT_CLEAR	1
#define	OPT_DIRECT	2
#define	OPT_APPEND	3
#define	OPT_TAG		4
#define	OPT_COUNT	5

LONG cmd_makevf(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
struct	RDArgs		*rdargs;
struct	VF_Sem		*vf;
	LONG		opts[OPT_COUNT];
	LONG		rc;
	LONG		loop;

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
			/*
			 * We need to make sure that no one gets
			 * in here for a bit...
			 */
			Forbid();
			if (vf=(struct VF_Sem *)FindSemaphore(VF_NAME))
			{
				/*
				 * If the option to REMOVE the semaphore
				 * is set, we will remove it from the
				 * public list before we continue
				 * to optain it.  This makes sure that others
				 * will not try to obtain after we have...
				 */
				if (opts[OPT_REMOVE]) RemSemaphore((struct SignalSemaphore *)vf);
				ObtainSemaphore((struct SignalSemaphore *)vf);
			}

			/*
			 * If we are not removing the semaphore
			 * and we don't have one, we make one...
			 */
			if ((!opts[OPT_REMOVE]) && (!vf))
			{
				if (opts[OPT_DIRECT])
				{
					vf=VF_FPOS;	/* F-space VF structure */
					vf->Memory=(ULONG *)(0x00F00000);
				}
				else if (vf=AllocVec(VF_SIZE,MEMF_PUBLIC|MEMF_CLEAR))
				{
					vf->Memory=(ULONG *)&(vf->Memory_Start);
				}

				if (vf)
				{
					InitSemaphore((struct SignalSemaphore *)vf);
					strcpy(vf->SemName,VF_NAME);
					vf->Sem.ss_Link.ln_Name=vf->SemName;
					vf->Sem.ss_Link.ln_Pri=-127;	/* Bottom of list */
					AddSemaphore((struct SignalSemaphore *)vf);
					ObtainSemaphore((struct SignalSemaphore *)vf);		/* Get it for us */
				}
			}

			Permit();

			/*
			 * Ok, at this point, if vf is not NULL we
			 * have a semaphore and are ready to play with it...
			 */
			if (vf)
			{
				/*
				 * Check if VF matches the DIRECT mode
				 */
				if (opts[OPT_DIRECT])
				{
					if (VF_FPOS!=vf) PutStr("VF Semaphore is not in direct!\n");
				}
				else
				{
					rc=0;

					if (opts[OPT_CLEAR] || opts[OPT_APPEND])
					{
						if (opts[OPT_CLEAR]) vf->LastHunk=0;
						vf->LastPos=0;
						loop=(512*1024/4);

						if (vf->Memory==(ULONG *)(0x00F00000)) loop-=(sizeof(struct VF_Sem)/4);

						while (loop)
						{
							loop--;
							vf->Memory[loop]=0xFFFFFFFF;
						}
					}

					if (opts[OPT_TAG])
					{
						vf->Memory[0]=0x11144EF9;	/* fill...   JMP    */
						vf->Memory[1]=0x00F80002;	/*           F80002 */
						vf->Memory[2]=0x0000FFFF;	/* Diagnostic value */
						vf->Memory[3]=0xFFFFFFFF;	/* Empty version #  */

						if (vf->LastPos<4) vf->LastPos=4;
					}
				}

				/*
				 * We are done with it now, so release it
				 */
				ReleaseSemaphore((struct SignalSemaphore *)vf);

				/*
				 * Finally, if we are going to remove
				 * it, just call FreeVec()...
				 */
				if (opts[OPT_REMOVE]) if (VF_FPOS!=vf) FreeVec(vf);
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
