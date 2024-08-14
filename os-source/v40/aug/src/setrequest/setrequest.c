#include	<exec/libraries.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	"setrequest_rev.h"

/******************************************************************************/

/* This is the command template. */
#define TEMPLATE  "OFF/S,ON/S" VERSTAG

#define	OPT_OFF		0
#define	OPT_ON		1
#define	OPT_COUNT	2

LONG cmd_makevf(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
	LONG		rc;

	rc=RETURN_FAIL;
	if (DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",37))
	{
	struct	RDargs		*rdargs;
		LONG		opts[OPT_COUNT];
		LONG		x;

		for (x=0;x<OPT_COUNT;opts[x++]=0);

		rdargs = ReadArgs(TEMPLATE, opts, NULL);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(),NULL);
		}
		else
		{
		struct	Process	*pr;

			pr=(struct Process *)FindTask(NULL);

			if (opts[OPT_OFF]) pr->pr_WindowPtr=(APTR)-1;
			if (opts[OPT_ON]) pr->pr_WindowPtr=(APTR)0;

			rc=RETURN_OK;

			FreeArgs(rdargs);
		}

		CloseLibrary((struct Library *)DOSBase);
	}
	return(rc);
}
