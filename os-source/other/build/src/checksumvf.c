/*
 * $Id: ChecksumVF.c,v 40.2 93/02/25 15:26:01 mks Exp $
 *
 * $Log:	ChecksumVF.c,v $
 * Revision 40.2  93/02/25  15:26:01  mks
 * No longer uses the internal (private) include
 * 
 * Revision 40.1  93/02/18  15:39:25  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.4  91/10/18  15:37:07  mks
 * Added CDTV option
 *
 * Revision 1.3  91/10/17  10:22:46  mks
 * Cleaned up formatting so it will print nice
 *
 * Revision 1.2  91/10/15  10:19:43  mks
 * Added standard version string to the commands
 *
 * Revision 1.1  91/10/11  08:26:28  mks
 * Moved FreeArgs() call to before the CloseLibrary(DOSBase)
 *
 * Revision 1.0  91/09/16  14:53:15  mks
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

#include	"checksumvf_rev.h"

/******************************************************************************/

#include	"vf.h"

/*
 * This command checksums the VF and adds ROM constants...
 */

#define	ROM_LOC_VER	(3)
#define	ROM_LOC_VEC1	(full_size-1)
#define	ROM_LOC_VEC2	(full_size-2)
#define	ROM_LOC_VEC3	(full_size-3)
#define	ROM_LOC_VEC4	(full_size-4)
#define	ROM_LOC_SIZE	(full_size-5)
#define	ROM_LOC_SUM	(full_size-6)

/* This is the command template. */
#define TEMPLATE  "VERSION/N,REVISION/N,SUPERKICK/S,CDTV/S" VERSTAG

#define	OPT_VERSION	0
#define	OPT_REVISION	1
#define	OPT_SUPERKICK	2
#define	OPT_CDTV	3
#define	OPT_COUNT	4

#define	FIT_BUFFER	1
#define	ROM_MAX		(0x0007FFE0 - FIT_BUFFER)
#define	ROM_MAX_KICK	(0x0007F000 - FIT_BUFFER)
#define	ROM_MAX_CDTV	(0x0003FFE0 - FIT_BUFFER)

LONG cmd_checksumvf(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
struct	RDArgs		*rdargs;
struct	VF_Sem		*vf;
	LONG		opts[OPT_COUNT];
	LONG		rc;
	ULONG		version;
	ULONG		revision;
	ULONG		full_size=FULL_SIZE;	/* Default FULL_SIZE */

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
			 * Defaults
			 */
			version=37;
			revision=0;

			if (opts[OPT_VERSION]) version=*(ULONG *)opts[OPT_VERSION];
			if (opts[OPT_REVISION]) revision=*(ULONG *)opts[OPT_REVISION];
			if (opts[OPT_CDTV]) full_size=(256*1024/4);

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
			ULONG	size;
			ULONG	sum;
			ULONG	tmp;
			ULONG	loop;

				rc=RETURN_OK;

				/*
				 * Check if we fit
				 */
				size=vf->LastPos*4;
				if ( ((opts[OPT_SUPERKICK]) && (size>ROM_MAX_KICK)) ||
				     (size>ROM_MAX) ||
				     ((opts[OPT_CDTV]) && (size>ROM_MAX_CDTV))		)
				{
					VPrintf("Warning!  Size of $%06lx is too large\n",&size);
					rc=RETURN_WARN;
				}

				/*
				 * Set the ROM version/revision
				 */
				vf->Memory[ROM_LOC_VER]=(version << 16) | revision;

				/*
				 * Set up the auto-vectors
				 */
				vf->Memory[ROM_LOC_VEC1]=0x001E001F;
				vf->Memory[ROM_LOC_VEC2]=0x001C001D;
				vf->Memory[ROM_LOC_VEC3]=0x001A001B;
				vf->Memory[ROM_LOC_VEC4]=0x00180019;

				/*
				 * Set up the ROM SIZE values
				 */
				vf->Memory[ROM_LOC_SIZE]=full_size*4;

				/*
				 * Now, do the checksum
				 */
				vf->Memory[ROM_LOC_SUM]=0;
				sum=0;

				for (loop=0;loop<full_size;loop++)
				{
					tmp=sum+vf->Memory[loop];
					if (tmp<sum) tmp++;
					sum=tmp;
				}
				vf->Memory[ROM_LOC_SUM]=~sum;

				/*
				 * Set the VF to be FULL
				 */
				vf->LastPos=full_size;

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
