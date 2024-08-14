/*
 * $Id: SumKick.c,v 40.2 93/02/25 15:26:40 mks Exp $
 *
 * $Log:	SumKick.c,v $
 * Revision 40.2  93/02/25  15:26:40  mks
 * No longer uses the internal (private) include
 * 
 * Revision 40.1  93/02/18  15:39:59  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 39.2  92/09/15  15:13:32  mks
 * Added the "double" keyword
 *
 * Revision 39.1  92/09/15  14:44:10  mks
 * First release of the checksum builder...
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

#include	<dos/dos.h>
#include	<dos/dosextens.h>

#include	"sumkick_rev.h"

/******************************************************************************/

/*
 * This command saves the VF into a file
 */

/* This is the command template. */
#define TEMPLATE  "FILE/A,DOUBLE/S" VERSTAG

#define	OPT_FILE	0
#define	OPT_DOUBLE	1
#define	OPT_COUNT	2

#define	KS_SIZE	(512*1024)

LONG cmd_savevf(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
struct	RDArgs		*rdargs;
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
		else
		{
		UBYTE	*buffer;

			if (buffer=AllocVec(KS_SIZE,MEMF_PUBLIC))
			{
			BPTR	file;

				if (file=Open((char *)opts[OPT_FILE],MODE_OLDFILE))
				{
					if (KS_SIZE==Read(file,buffer,KS_SIZE))
					{
					ULONG	index;
					ULONG	tmp[2];
					UWORD	Sum0;
					UWORD	Sum1;

						index=0;
						Sum0=0;
						Sum1=0;
						while (index<KS_SIZE)
						{
							Sum0+=buffer[index++];
							Sum0+=buffer[index++];
							Sum1+=buffer[index++];
							Sum1+=buffer[index++];
						}

						if (opts[OPT_DOUBLE])
						{
							Sum0+=Sum0;
							Sum1+=Sum1;
						}

						tmp[0]=Sum0;
						tmp[1]=Sum1;
						VPrintf("Chip 0 sum $%04lx\nChip 1 sum $%04lx\n",tmp);
					}
					else PrintFault(IoErr(),NULL);
					Close(file);
				}
				else PrintFault(IoErr(),NULL);
				FreeVec(buffer);
			}
			else PrintFault(IoErr(),NULL);
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
