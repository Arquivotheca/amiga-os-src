/*
 * $Id: KickWordMix.c,v 1.1 93/03/10 10:42:59 mks Exp $
 *
 * $Log:	KickWordMix.c,v $
 * Revision 1.1  93/03/10  10:42:59  mks
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
#include	<dos/stdio.h>

#include	"KickWordMix_rev.h"

/******************************************************************************/

/* This is the command template. */
#define TEMPLATE  "FILE/A" VERSTAG

#define	OPT_FILE	0
#define	OPT_COUNT	1

LONG cmd(void)
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
		BPTR	file;

			if (file=Open((char *)opts[OPT_FILE],MODE_OLDFILE))
			{
			UWORD	*buffer;
			UWORD	*tmp;
			UWORD	*end;
			ULONG	size;

				Seek(file,0,OFFSET_END);
				size=Seek(file,0,OFFSET_BEGINNING);
				if (buffer=AllocVec(size,MEMF_PUBLIC))
				{
					end=buffer+(size >> 1);
					if (size==Read(file,buffer,size))
					{
						Seek(file,0,OFFSET_BEGINNING);
						SetVBuf(file,NULL,BUF_FULL,16384);

						for (tmp=buffer;tmp<end;tmp+=2)
						{
							FWrite(file,tmp,2,1);
						}

						for (tmp=buffer+1;tmp<end;tmp+=2)
						{
							FWrite(file,tmp,2,1);
						}

						rc=RETURN_OK;
					}
					else PrintFault(IoErr(),NULL);

					FreeVec(buffer);
				}
				else PrintFault(IoErr(),NULL);
				Close(file);
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
