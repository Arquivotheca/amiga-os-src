/*
 ******************************************************************************
 *
 * $Id: maprom.c,v 39.6 92/11/13 13:43:32 mks Exp $
 *
 * $Log:	maprom.c,v $
 * Revision 39.6  92/11/13  13:43:32  mks
 * Added an autodoc...
 * 
 * Revision 39.5  92/08/26  17:35:57  mks
 * Have to set BURST mode too!
 *
 * Revision 39.4  92/08/26  10:43:19  mks
 * Fixed the cache setting issues...
 *
 * Revision 39.3  92/08/26  10:33:23  mks
 * Cleaned up to work better...
 *
 * Revision 39.2  92/06/24  15:34:20  mks
 * To make it work in 1meg FAST RAM machines it now loads kickstart
 * into MEMF_CHIP...
 *
 * Revision 39.1  92/06/05  09:44:59  mks
 * First RCS checkin
 *
 ******************************************************************************
 */

/*
******* MapROM ****************************************************************
*
*   NAME
*	MapROM - A Developer Tool for testing Kickstarts with the 3640 board
*
*   SYNOPSIS
*	MapROM <kickstartfile> [FORCE]
*
*   FUNCTION
*	MapROM is a tool that uses a feature of the A3640 (the Commodore
*	68040 CPU board in the A4000 and A3000/040 systems) to let developers
*	load disk-based kickstarts.
*
*	MapROM will check to see if you are already running the kickstart
*	that is contained in the file and if you have the memory required
*	for the kickstart.  If the kickstart file is different than the
*	current kickstart, it will replace it and cause a system reboot.
*	Kickstarts loaded with MapROM do not go away until a full power down.
*
*	The FORCE option can be used to force a reload of a kickstart image
*	even if it is the same one.
*
*	MapROM does *NOT* use the MMU of the 68040 CPU but some special
*	hardware on the A3640 card.
*
*   INPUTS
*	FILE/A	- Required file name.  It should be the name of the
*		  developer kickstart file for your machine.  The A4000
*		  and A3000 kickstarts are *different* and can not be
*		  interchanged.
*
*	FORCE/S	- An option to FORCE the reloading of the kickstart even
*		  if it matches.
*
*   RESULTS
*	If the kickstart is new and has loaded successfully, the system will
*	reboot and use that kickstart.  If not, MapROM quietly exits.
*
*   NOTES
*	In order for this tool to work correctly with other system tools
*	you should place it in the main C: directory.  Various developer
*	tools will look for it there.  (This is SYS:C)
*
*	Also, in order to use this tool, it *MUST* be run *ABSOLUTELY FIRST*
*	in the Startup-Sequence.  It *MUST* come before SetPatch as it
*	must not have the MMU turned on.  Since MapROM will either work
*	or quietly exit, there is no problem with doing this.
*
*	The standard setup is:
*
*	IF EXISTS C:MapROM
*		MapROM DEVS:Kickstart
*	ENDIF
*
*   SEE ALSO
*	Your developer non-disclosure agreement.  This is a *developer only*
*	tool and is not to be released to the public in any way.
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
#include	<dos/dosasl.h>
#include	<dos/notify.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	<string.h>

#define EXECBASE (*(struct ExecBase **)4)

/******************************************************************************/

#include "maprom_rev.h"

#define	ROM_ADDR	(0x00F80000)
#define	ROM_SIZE	(512*1024)

#define TEMPLATE  "FILE/A,FORCE/S" VERSTAG

#define	OPT_FILE	0
#define	OPT_FORCE	1
#define	OPT_COUNT	2

void __asm DoMap(register __a0 ULONG *);

ULONG cmd(void)
{
struct	ExecBase	*SysBase = EXECBASE;
struct	Library		*DOSBase;
	long		opts[OPT_COUNT];
struct	RDArgs		*rdargs;
	ULONG		rc=RETURN_FAIL;

	if (DOSBase = OpenLibrary("dos.library",37))
	{
		memset((char *)opts, 0, sizeof(opts));

		if (!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) PrintFault(IoErr(),NULL);
		else if (SetSignal(0,0) & SIGBREAKF_CTRL_C) PrintFault(ERROR_BREAK,NULL);
		else rc=RETURN_OK;

		if (RETURN_OK==rc)
		{
		ULONG	*buffer;

			if (buffer=AllocMem(ROM_SIZE,MEMF_PUBLIC|MEMF_CHIP))
			{
			BPTR	file;

				if (file=Open((char *)opts[OPT_FILE],MODE_OLDFILE))
				{
					if (16==Read(file,buffer,16))
					{
					ULONG	*rom=(ULONG *)ROM_ADDR;

						/*
						 * Check if the file
						 * really is a kickstart file
						 */
						if ((rom[0] == buffer[0]) &&
						    (rom[1] == buffer[1]) &&
						    (rom[2] == buffer[2]))
						{
							/*
							 * Check version...
							 */
							if ((rom[3] != buffer[3]) || (opts[OPT_FORCE]))
							{
								Seek(file,0,OFFSET_BEGINNING);
								if (ROM_SIZE==Read(file,buffer,ROM_SIZE))
								{
								ULONG	CacheSettings;

									Forbid();
									/*
									 * We now remap (but it may not
									 * if mapping hardware is not there)
									 * We also delay for a bit to make
									 * sure things settle down...
									 */
									Delay(150);	/* 3 seconds */

									/*
									 * Turn off caches (except I cache) and remap...
									 */
									CacheSettings=CacheControl(CACRF_EnableI | CACRF_IBE,~0);
									DoMap(buffer);

									/*
									 * We only return here if
									 * no mapping hardware.
									 * Restore cache settings
									 */
									CacheControl(CacheSettings,~0);

									Permit();
								}
							}
						}
						else
						{
							PutStr("Invalid Kickstart file.\n");
							rc=RETURN_FAIL;
						}
					}
					Close(file);
				}
				FreeMem(buffer,ROM_SIZE);
			}
		}

		if (rdargs) FreeArgs(rdargs);
		CloseLibrary(DOSBase);
	}

	return(rc);
}
