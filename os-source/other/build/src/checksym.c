/*
 * $Id: CheckSym.c,v 40.2 93/02/25 15:26:45 mks Exp $
 *
 * $Log:	CheckSym.c,v $
 * Revision 40.2  93/02/25  15:26:45  mks
 * No longer uses the internal (private) include
 * 
 * Revision 40.1  93/02/18  15:40:03  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.8  92/11/03  08:56:43  mks
 * Now sets result to WARN...
 *
 * Revision 1.7  92/11/02  11:50:52  mks
 * Added the QUICK/S option to no display the header information
 *
 * Revision 1.6  91/11/14  20:10:41  mks
 * Added support for $ addresses as for the ALL/S switch
 *
 * Revision 1.5  91/10/17  18:14:01  mks
 * Now automatically handles both symbols and addesses
 *
 * Revision 1.4  91/10/17  17:13:48  mks
 * Fixed bug where an exact address match would not show the label
 *
 * Revision 1.3  91/10/17  17:08:08  mks
 * Changed the loadsym_rev.h to checksym_rev.h
 *
 * Revision 1.2  91/10/17  17:02:54  mks
 * Added the /A switch to the ADDRESS parameter
 *
 * Revision 1.1  91/10/17  15:45:23  mks
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

#include	<exec/semaphores.h>

#include	<dos/dos.h>

#include	"checksym_rev.h"

/******************************************************************************/

#include	"sym.h"

/*
 * This command uses the symbol table loaded by LoadSym
 * to check an address against a symbol.
 */

/* This is the command template. */
#define TEMPLATE  "ARG/M,QUICK/S,ALL/S" VERSTAG

#define	OPT_ADDRESS	0
#define	OPT_QUICK	1
#define	OPT_ALL		2
#define	OPT_COUNT	3

LONG cmd_checksym(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
	LONG		rc;

	rc=RETURN_FAIL;
	if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
	{
	struct	RDArgs	*rdargs;
		LONG	opts[OPT_COUNT];

		memset((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts, NULL);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(),NULL);
		}
		else
		{
		struct	Sym_Sem	*sym;
			ULONG	tmp[3];

			Forbid();
			if (sym=(struct Sym_Sem *)FindSemaphore(SYM_NAME)) ObtainSemaphore((struct SignalSemaphore *)sym);
			Permit();

			if ((sym) && (sym->Memory))
			{
			char	**hex;

				rc=RETURN_OK;

				tmp[0]=(ULONG)(sym->Symbols);
				if (!opts[OPT_QUICK]) VPrintf("Symbols loaded: %s\n\n",tmp);

				if (opts[OPT_ADDRESS]) for (hex=(char **)opts[OPT_ADDRESS];(*hex);hex++)
				{
				char	*symbol="Not found";
				char	*p;
				ULONG	*mem;
				ULONG	offset;
				ULONG	val;
				ULONG	tp;
				ULONG	c;

					val=0;
					mem=sym->Memory;
					p=*hex;

					if (*p=='$') p++;	/* Support $hex */
					while (*p)
					{
						c=(ULONG)*p;
						if ((c>='a') && (c<='f')) c-=32;
						c-='0';
						if (c>9)
						{
							c-=7;
							if (c<10) c=16;
						}

						if (c<16)
						{
							val=(val << 4) + c;
							p++;
						}
						else
						{
							val=0;
							p=&p[strlen(p)];
						}
					}

					offset=524288;
					while (*mem)
					{
						c=*mem++;
						tp=mem[c];
						if (val)
						{
							if (tp<=val) if ((val-tp)<offset)
							{
								symbol=(char *)mem;
								offset=val-tp;
							}
						}
						else
						{
							if (!strcmp((char *)mem,*hex))
							{
								val=tp;
								symbol=(char *)mem;
								offset=0;
							}
						}
						mem=&mem[c+1];
					}


					if (offset!=524288)
					{
						tmp[0]=val;
						tmp[1]=(ULONG)symbol;
						tmp[2]=offset;
						VPrintf("Address $%08lx @ %s + $%lx\n",tmp);
					}
					else
					{
						tmp[0]=(ULONG)(*hex);
						tmp[1]=(ULONG)symbol;
						VPrintf("<%s> - %s\n",tmp);
						rc=RETURN_WARN;
					}
				}

				if (opts[OPT_ALL])
				{
				ULONG	*mem;
				ULONG	c;

					mem=sym->Memory;

					while ((*mem) && (rc==RETURN_OK))
					{
						c=*mem++;
						tmp[0]=mem[c];
						tmp[1]=(ULONG)mem;
						VPrintf("$%08lx @ %s\n",tmp);
						mem=&mem[c+1];

						if (SetSignal(0L,0L) & SIGBREAKF_CTRL_C)
						{
							PutStr("***BREAK\n");
							rc=RETURN_FAIL;
						}
					}
				}
			}
			else PutStr("No symbols loaded\n");

			if (sym) ReleaseSemaphore((struct SignalSemaphore *)sym);
		}

		CloseLibrary((struct Library *)DOSBase);
	}
	else
	{
		OPENFAIL;
	}
	return(rc);
}
