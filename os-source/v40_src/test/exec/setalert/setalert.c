/*
 * $Id: setalert.c,v 40.3 93/05/18 16:57:38 mks Exp $
 */

/*
******* SetAlert **************************************************************
*
*   NAME
*	SetAlert                                                  Requires 2.04
*
*   SYNOPSIS
*	SetAlert - A tool to let you set the Alert TimeOut in V39 and later
*	system releases.
*
*   FUNCTION
*	As of V39, EXEC alerts are able to timeout and that timeout can
*	be set.  This timeout also survives reboots as best as possible.
*	SetAlert can also be used to determine what the current setting is.
*	SetAlert can be used from the Workbench and the options can be
*	stored in the tooltypes of the icon.
*
*   INPUTS
*	SECONDS/N    Number of seconds that alerts should remain visible
*	NONE/S       Alerts should never be displayed
*	FOREVER/S    Alerts should not timeout
*	QUIET/S      Do not display status information
*
*   RESULTS
*	The alert timeout will be set as asked for and the setting will
*	be displayed unless the QUIET option is used.
*
*   NOTES
*	The alert timeout is actualy a count of VBlanks that the alert is
*	displayed.  SetAlert will try to convert this value as best as
*	possible to seconds by using the VBlank frequency stored in
*	EXEC.  This is the default VBlank frequency and if the alert
*	happens to show up in a different display frequency from that
*	default the delay will be slightly different.
*
*	A delay of 0 means never display the alert
*
*   SEE ALSO
*	"A master's secrets are only as good as the
*	 master's ability to explain them to others."  -  Michael Sinz
*
*   BUGS
*	None?
*
******************************************************************************
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
#include	<intuition/intuition.h>
#include	<workbench/startup.h>
#include	<workbench/workbench.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	<clib/intuition_protos.h>
#include	<pragmas/intuition_pragmas.h>

#include	<clib/icon_protos.h>
#include	<pragmas/icon_pragmas.h>

#include	<clib/locale_protos.h>
#include	<pragmas/locale_pragmas.h>

#include	<clib/utility_protos.h>
#include	<pragmas/utility_pragmas.h>

#include	<string.h>

#include	"setalert_rev.h"

/*
 * Localization stuff...
 */
#define		CATCOMP_NUMBERS
#include	"setalert_strings.h"
STRPTR __asm	GetString(register __a0 struct LocaleInfo *,register __d0 LONG);

#define EXECBASE (*(struct ExecBase **)4)

static char Template[]="SECONDS/N,NONE/S,FOREVER/S,QUIET/S" VERSTAG;

#define	OPT_SECONDS	0
#define	OPT_NONE	1
#define	OPT_FOREVER	2
#define	OPT_QUIET	3
#define	OPT_COUNT	4

#define	LocaleBase	(LocaleInfo.li_LocaleBase)
#define	Catalog		(LocaleInfo.li_Catalog)
#define	GET_STRING(x)	GetString(&LocaleInfo,(x))

/*
 * This is the main entry point for the program.  It does all of the
 * setup work and allocates the global data on the stack, etc.
 */
ULONG cmd(void)
{
struct	ExecBase	*SysBase=EXECBASE;
struct	Library		*DOSBase;
struct	Library		*IntuitionBase;
struct	Library		*UtilityBase;
struct	Process		*proc;
struct	RDArgs		*rdargs=NULL;
struct	WBStartup	*msg=NULL;
struct	RDArgs		*myRDArgs=NULL;
	ULONG		rc=RETURN_FAIL;
struct	LocaleInfo	LocaleInfo;	/* Locale information */

	proc=(struct Process *)FindTask(NULL);

	if (!(proc->pr_CLI))
	{
		WaitPort(&(proc->pr_MsgPort));
		msg=(struct WBStartup *)GetMsg(&(proc->pr_MsgPort));
	}

	if (LocaleBase=OpenLibrary("locale.library",38))
	{
		Catalog=OpenCatalogA(NULL,"setalert.catalog",NULL);
	}

	DOSBase=OpenLibrary("dos.library",37);
	IntuitionBase=OpenLibrary("intuition.library",37);
	UtilityBase=OpenLibrary("utility.library",37);

	if (DOSBase && IntuitionBase && UtilityBase)
	{
	long	opts[OPT_COUNT];

		memset((char *)opts, 0, sizeof(opts));

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

									if (rdargs=ReadArgs(Template, opts, myRDArgs))
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
			if (!(rdargs = ReadArgs(Template, opts, NULL))) PrintFault(IoErr(),NULL);
			else if (SetSignal(0,0) & SIGBREAKF_CTRL_C) PrintFault(ERROR_BREAK,NULL);
			else rc=RETURN_OK;
		}

		if (RETURN_OK==rc)
		{
		ULONG	out;
		LONG	value;

			if (opts[OPT_SECONDS]) SysBase->LastAlert[3]=UMult32(*(ULONG *)opts[OPT_SECONDS],(ULONG)SysBase->VBlankFrequency);

			if (opts[OPT_NONE]) SysBase->LastAlert[3]=0;

			if (opts[OPT_FOREVER]) SysBase->LastAlert[3]=-1;

			value=SysBase->LastAlert[3];

			/*
			 * For pre-V39 systems, pretend no alert timeout
			 */
			if (SysBase->LibNode.lib_Version<39) value=-1;

			out=MSG_NOALERTS;
			if (value)
			{
				out=MSG_FOREVER;
				if (value!=-1)
				{
					out=MSG_SHOWTIME;
					value=UDivMod32(value+(ULONG)(SysBase->VBlankFrequency)-1,(ULONG)(SysBase->VBlankFrequency));
				}
			}

			if (!opts[OPT_QUIET])
			{
				if (msg)
				{
				struct	EasyStruct	es = {sizeof(struct EasyStruct),NULL,"SetAlert",NULL,NULL};

					es.es_TextFormat=GET_STRING(out);
					es.es_GadgetFormat=GET_STRING(MSG_CONTINUE);
					EasyRequestArgs(NULL,&es,NULL,&value);
				}
				else
				{
					VPrintf(GET_STRING(out),&value);
					PutStr("\n");
				}
			}
		}

		if (rdargs) FreeArgs(rdargs);
		if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);

	}
	else if (!msg)
	{
		if (!DOSBase) DOSBase=OpenLibrary("dos.library",0);
		if (DOSBase)
		{
			Write(Output(),"Requires Kickstart 2.04 (37.175) or later.\n",43);
		}
	}

	/*
	 * We do this in order to not crash under 1.3...
	 * We do not run under 1.3, but that is another issue.
	 */
	if (UtilityBase)	CloseLibrary(UtilityBase);
	if (IntuitionBase)	CloseLibrary(IntuitionBase);
	if (DOSBase)		CloseLibrary(DOSBase);

	/*
	 * Close the locale (if there is one)
	 */
	if (LocaleBase)
	{
		CloseCatalog(Catalog);
		CloseLibrary(LocaleBase);
	}

	if (msg)
	{
		Forbid();
		ReplyMsg((struct Message *)msg);
	}

	return(rc);
}
