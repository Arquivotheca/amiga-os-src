/*
******* MapBoard **************************************************************
*
*   NAME
*	MapBoard - A 68040 Expansion Board ROM mapper
*
*   SYNOPSIS
*	MapBoard will let you map the ROM of an I/O board as cacheable
*	at specific addresses on 68040 systems.
*
*   FUNCTION
*	MapBoard will use the MMU to map sections of an I/O board's
*	address space as cacheable such that things like device drivers
*	can run in the CPU instruction caches.  No memory should be
*	used by this program.
*
*	This program only is useful for 68040 based systems running
*	V37.10 or later of the 68040.library.
*
*	All options are also available from icon tooltypes in either
*	project or tool icons.
*
*   INPUTS
*	MANUFACTURE/N	- The Manufacture autoconfig ID for the board
*
*	PRODUCT/N	- The Product autoconfig ID for the board
*
*	OFFSET/N	- The address offset from the start of the board
*			  where caching should begin.  This is rounded up
*			  to the nearest page address.
*
*	SIZE/N		- The length in bytes of the cacheable area.
*			  This will be rounded to the page size as
*			  to only cache pages that are completely
*			  contained within the area from OFFSET to
*			  OFFSET+SIZE
*
*	A2091/S		- Do the Commodore-Amiga A2091 hard drive
*			  controller ROM.  This makes the A2091
*			  run over 10 times faster on the A4000
*			  when talking to non-DMA memory.
*
*   RESULTS
*	The MMU tables are modified to show the specific pages as
*	cacheable.  MapBoard does not (currently) produce any error
*	messages.
*
*   WARNING
*	Specifying invalid SIZE or OFFSET can cause the hardware device
*	to fail.  MapBoard does not any specific knowledge of the hardware
*	you are mapping (other than the A2091) and thus can not error
*	check your input.
*
*   NOTES
*	Since this tool modifies the existing MMU tables, toys that play
*	with the MMU can cause it to not work.  MapBoard will correctly
*	ignore invalid MMU tables as best it can.  The tables must be
*	4K page size and be fully valid 68040 tables.
*
*	Also, since Enforcer and such tools install a new MMU table
*	when they run, you will need to run MapBoard after starting
*	Enforcer.  If you quit Enforcer and then restart Enforcer
*	you will need to run MapBoard again.  If you run MapBoard
*	before Enforcer, Enforcer will correctly restore the MMU
*	settings when it quits.  So, if you ever do run Enforcer and
*	turn it off you may wish to have MapBoard run BEFORE you
*	run Enforcer and after each time you start Enforcer.
*
*   SEE ALSO
*	"A master's secrets are only as good as the
*	 master's ability to explain them to others."  -  Michael Sinz
*
*   BUGS
*	None?
*
*******************************************************************************
*/

#include	<exec/types.h>
#include	<exec/execbase.h>
#include	<exec/tasks.h>
#include	<exec/memory.h>
#include	<exec/alerts.h>
#include        <exec/ports.h>
#include        <exec/libraries.h>
#include	<exec/semaphores.h>
#include        <dos/dos.h>
#include        <dos/dosextens.h>
#include	<dos/rdargs.h>
#include	<devices/timer.h>
#include	<workbench/startup.h>
#include	<workbench/workbench.h>
#include	<libraries/configvars.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	<clib/icon_protos.h>
#include	<pragmas/icon_pragmas.h>

#include	<clib/expansion_protos.h>
#include	<pragmas/expansion_pragmas.h>

#include	<strings.h>

#define EXECBASE (*(struct ExecBase **)4)

#define TEMPLATE  "MANUFACTURE/N,PRODUCT/N,OFFSET/N,SIZE/N,A2091/S"

#define	OPT_MAN		0
#define	OPT_PROD	1
#define	OPT_OFFSET	2
#define	OPT_SIZE	3
#define	OPT_A2091	4
#define	OPT_COUNT	5

void __asm CacheMap(register __a6 struct ExecBase *,register __a0 struct ConfigDev *,register __d0 ULONG,register __d1 ULONG);

ULONG cmd(void)
{
struct	ExecBase	*SysBase;
struct	Library		*DOSBase;
struct	Process		*proc;
struct	RDArgs		*rdargs=NULL;
struct	WBStartup	*msg=NULL;
struct	RDArgs		*myRDArgs=NULL;
	ULONG		rc=RETURN_FAIL;
	LONG		opts[OPT_COUNT];

	SysBase = EXECBASE;

	proc=(struct Process *)FindTask(NULL);

	if (!(proc->pr_CLI))
	{
		WaitPort(&(proc->pr_MsgPort));
		msg=(struct WBStartup *)GetMsg(&(proc->pr_MsgPort));
	}

	if (DOSBase = OpenLibrary("dos.library",37))
	{
	struct	Library		*Lib68040;

		memset((char *)opts, 0, sizeof(opts));

		/*
		 * Do the option parsing
		 * If from Workbench, use icon tool types
		 * If from CLI, use ReadArgs
		 */
		if (msg)
		{
		struct	Library	*IconBase;
		struct	WBArg	*wbArg;

			/*
			 * Started from Workbench so do icon magic...
			 *
			 * What we will do here is try all of the tooltypes
			 * in the icon and keep only those which do not cause
			 * errors in the RDArgs.
			 */
			if (wbArg=msg->sm_ArgList) if (IconBase=OpenLibrary("icon.library",0))
			{
			struct	DiskObject	*diskObj;
				BPTR		tmplock;

				/*
				 * Use project icon if it is there...
				 */
				if (msg->sm_NumArgs > 1) wbArg++;

				tmplock=CurrentDir(wbArg->wa_Lock);
				if (diskObj=GetDiskObject(wbArg->wa_Name))
				{
				char	**ToolTypes;

					if (ToolTypes=diskObj->do_ToolTypes)
					{
					char	*TotalString;
					ULONG	totalSize=3;

						while (*ToolTypes)
						{
							totalSize+=strlen(*ToolTypes)+1;
							ToolTypes++;
						}

						if (TotalString=AllocVec(totalSize,MEMF_PUBLIC))
						{
						char	*CurrentPos=TotalString;
						ULONG	currentLength;

							ToolTypes=diskObj->do_ToolTypes;
							do
							{
								*CurrentPos='\0';
								if (*ToolTypes) strcpy(CurrentPos,*ToolTypes);
								currentLength=strlen(CurrentPos);
								CurrentPos[currentLength+0]='\n';
								CurrentPos[currentLength+1]='\0';

								if (rdargs) FreeArgs(rdargs);
								rdargs=NULL;
								memset((char *)opts, 0, sizeof(opts));

								if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);
								if (myRDArgs=AllocDosObject(DOS_RDARGS,NULL))
								{
									myRDArgs->RDA_Source.CS_Buffer=TotalString;
									myRDArgs->RDA_Source.CS_Length=strlen(TotalString);

									if (rdargs=ReadArgs(TEMPLATE, opts, myRDArgs))
									{
										CurrentPos[currentLength]=' ';
										CurrentPos+=currentLength+1;
									}
								}
							} while (*ToolTypes++);
							FreeVec(TotalString);
						}
					}

					FreeDiskObject(diskObj);
				}

				CurrentDir(tmplock);
				CloseLibrary(IconBase);
			}
			rc=RETURN_OK;
		}
		else
		{
			/*
			 * Started from CLI so do standard ReadArgs
			 */
			if (!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) PrintFault(IoErr(),NULL);
			else if (SetSignal(0,0) & SIGBREAKF_CTRL_C) PrintFault(ERROR_BREAK,NULL);
			else rc=RETURN_OK;
		}

		if (RETURN_OK==rc) if (Lib68040=OpenLibrary("68040.library",0))
		{
		struct	Library	*ExpansionBase;

			if (ExpansionBase=OpenLibrary("expansion.library",0))
			{
			struct	ConfigDev	*cd=NULL;
				LONG		man=0;
				LONG		prod=0;
				LONG		offset=0;
				LONG		size=0;

				if (opts[OPT_MAN]) man=*(LONG *)opts[OPT_MAN];
				if (opts[OPT_PROD]) prod=*(LONG *)opts[OPT_PROD];
				if (opts[OPT_OFFSET]) offset=*(LONG *)opts[OPT_OFFSET];
				if (opts[OPT_SIZE]) size=*(LONG *)opts[OPT_SIZE];

				/*
				 * User defined case ...
				 */
				while (cd=FindConfigDev(cd,man,prod))
				{
					/* Another board, so do it... */
					CacheMap(SysBase,cd,offset,size);
				}

				/*
				 * A2091 hard-coded case...
				 *
				 * A2091 has two versions,
				 * 514/2 and 514/3 so...
				 */
				if (opts[OPT_A2091])
				{
					for (prod=2;prod<=3;prod++) while (cd=FindConfigDev(cd,514,prod))
					{
						/*
						 * Another board, we need to
						 * map 16K at the $2000 offset
						 */
						CacheMap(SysBase,cd,0x2000,0x4000);
					}
				}

				CloseLibrary(ExpansionBase);
			}
			CloseLibrary(Lib68040);
		}
		else if (!msg) PutStr("Requires a 68040 and 68040.library\n");

		if (rdargs) FreeArgs(rdargs);
		if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);

		CloseLibrary(DOSBase);
	}
	else
	{
		if (!msg) if (DOSBase=OpenLibrary("dos.library",0))
		{
			Write(Output(),"Requires Kickstart 2.04 (37.175) or later.\n",43);
			CloseLibrary(DOSBase);
		}
	}

	if (msg)
	{
		Forbid();
		ReplyMsg((struct Message *)msg);
	}

	return(0);
}
