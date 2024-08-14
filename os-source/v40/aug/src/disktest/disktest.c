/*
******* DiskTest **************************************************************
*
*   NAME
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULTS
*
*   NOTES
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

#include	<string.h>

#include	"disktest_rev.h"

#define EXECBASE (*(struct ExecBase **)4)

/******************************************************************************/

/* Default file name */
#define	DEFAULT_FILE	"DiskTest.TMP"
#define	DEFAULT_OUTPUT	"CON:////DiskTest/CLOSE"
#define	DEFAULT_MINSIZE	1L
#define	DEFAULT_MAXSIZE	32768L
#define	MAX_SIZE	512000L

#define TEMPLATE  "FILE/K,MAXSIZE/N/K,MINSIZE/N/K,OUTPUT/K,CHIP/S,STOPERROR/S" VERSTAG

#define	OPT_FILE	0
#define	OPT_MAXSIZE	1
#define	OPT_MINSIZE	2
#define	OPT_OUTPUT	3
#define	OPT_CHIP	4
#define	OPT_STOPERROR	5
#define	OPT_COUNT	6

ULONG doRandom(ULONG range,ULONG *seed);

ULONG cmd(void)
{
struct	ExecBase	*SysBase;
struct	Library		*DOSBase;
struct	Process		*proc;
struct	RDArgs		*rdargs=NULL;
struct	WBStartup	*msg=NULL;
struct	RDArgs		*myRDArgs=NULL;
	BPTR		output=NULL;
	ULONG		rc=RETURN_FAIL;
	LONG		opts[OPT_COUNT];
	ULONG		GoodBytes=0;
	ULONG		BadBytes=0;
	ULONG		MinSize=DEFAULT_MINSIZE;
	ULONG		MaxSize=DEFAULT_MAXSIZE;
	ULONG		Seed=0x17485392;
	char		*filename=DEFAULT_FILE;
	ULONG		done=FALSE;
	ULONG		MemType=MEMF_PUBLIC|MEMF_CLEAR;

	SysBase = EXECBASE;

	proc=(struct Process *)FindTask(NULL);

	if (!(proc->pr_CLI))
	{
		WaitPort(&(proc->pr_MsgPort));
		msg=(struct WBStartup *)GetMsg(&(proc->pr_MsgPort));
	}

	if (DOSBase = OpenLibrary("dos.library",37))
	{
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

		/*
		 * If all is OK, start up the system as needed...
		 */
		if (RETURN_OK==rc)
		{
		char	*outfile=NULL;

			if (msg) outfile=DEFAULT_OUTPUT;
			if (opts[OPT_OUTPUT]) outfile=(char *)opts[OPT_OUTPUT];

			if (outfile) if (!(output=Open(outfile,MODE_NEWFILE)))
			{
				if (!msg) PrintFault(IoErr(),NULL);
				rc=RETURN_FAIL;
			}
		}

		if (RETURN_OK==rc)
		{
			if (opts[OPT_MINSIZE]) MinSize=*((ULONG *)opts[OPT_MINSIZE]);
			if (opts[OPT_MAXSIZE]) MaxSize=*((ULONG *)opts[OPT_MAXSIZE]);
			if ((MinSize>MaxSize) || (MaxSize>MAX_SIZE))
			{
				if (!msg) PrintFault(ERROR_BAD_NUMBER,NULL);
				rc=RETURN_FAIL;
			}
		}

		if (opts[OPT_FILE]) filename=(char *)opts[OPT_FILE];
		if (opts[OPT_CHIP]) MemType|=MEMF_CHIP;

		while ((RETURN_OK==rc) && (!done))
		{
		UBYTE	*buffer1;
		UBYTE	*buffer2;
		UBYTE	*rom;
		ULONG	size;
		ULONG	offset1;
		ULONG	offset2;
		BPTR	testfile;
		ULONG	tmp[4];

			size=doRandom(MaxSize-MinSize+1,&Seed)+MinSize;
			offset1=doRandom(16,&Seed);
			offset2=doRandom(16,&Seed);
			rom=(char *)(doRandom(512*1024-size-1,&Seed)+0x00F80000);

			if (SetSignal(0,0) & SIGBREAKF_CTRL_C)
			{
				PrintFault(ERROR_BREAK,NULL);
				done=TRUE;

				tmp[0]=GoodBytes+BadBytes;
				tmp[1]=GoodBytes;
				tmp[2]=BadBytes;
				if (output) VFPrintf(output,"\n%ld bytes written: %ld good, %ld bad\n",tmp);
				else VPrintf("\n%ld bytes written: %ld good, %ld bad\n",tmp);
			}
			else
			{
				buffer1=AllocVec(size+offset1,MemType);
				buffer2=AllocVec(size+offset2,MemType);

				if (buffer1 && buffer2)
				{
					CopyMem(rom,&buffer1[offset1],size);
					if (testfile=Open(filename,MODE_NEWFILE))
					{
						if (size==Write(testfile,&buffer1[offset1],size))
						{
							Close(testfile);
							if (testfile=Open(filename,MODE_OLDFILE))
							{
								if (size==Read(testfile,&buffer2[offset2],size))
								{
								ULONG	bad=0;

									tmp[0]=size;
									tmp[1]=(ULONG)&buffer1[offset1];
									tmp[2]=(ULONG)&buffer2[offset2];
									if (output) VFPrintf(output,"%9ld bytes written from $%08lx and read to $%08lx\n",tmp);
									else VPrintf("%9ld bytes written from $%08lx and read to $%08lx\n",tmp);

									/* Check the data */
									while (size--)
									{
										if (buffer1[offset1]==buffer2[offset2]) GoodBytes++;
										else
										{
											if (!bad)
											{
												tmp[0]=(ULONG)&buffer1[offset1];
												tmp[1]=(ULONG)buffer1[offset1];
												tmp[2]=(ULONG)&buffer2[offset2];
												tmp[3]=(ULONG)buffer2[offset2];
												if (output) VFPrintf(output,"Address $%08lx (%02lx) and $%08lx (%02lx) does not match!\n",tmp);
												else VPrintf("Address $%08lx (%02lx) and $%08lx (%02lx) does not match!\n",tmp);
											}
											bad++;
										}
										offset1++;
										offset2++;
									}
									BadBytes+=bad;
									if (bad)
									{
										if (output) VFPrintf(output,"There were %ld bad bytes\n",&bad);
										else VPrintf("There were %ld bad bytes\n",&bad);

										if (opts[OPT_STOPERROR])
										{
											Close(testfile);
											break; // while - don't free bufs!
										}
									}
								}
								else
								{
									PrintFault(IoErr(),NULL);
									rc=RETURN_FAIL;
								}
							}
							else
							{
								PrintFault(IoErr(),NULL);
								rc=RETURN_FAIL;
							}
						}
						else
						{
							PrintFault(IoErr(),NULL);
							rc=RETURN_FAIL;
						}

						if (output) Flush(output);
						else Flush(Output());

						if (output) Write(output,".",1);
						else Write(Output(),".",1);

						Close(testfile);

						if (output) Write(output,".",1);
						else Write(Output(),".",1);

						DeleteFile(filename);

						if (output) Write(output,".\n",2);
						else Write(Output(),".\n",2);
					}
					else
					{
						PrintFault(IoErr(),NULL);
						rc=RETURN_FAIL;
					}
				}
				else
				{
					PrintFault(ERROR_NO_FREE_STORE,NULL);
					rc=RETURN_FAIL;
				}

				FreeVec(buffer1);
				FreeVec(buffer2);
			}
		}

		if (output) Close(output);

		if (rdargs) FreeArgs(rdargs);
		FreeVec(myRDArgs);

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

ULONG doRandom(ULONG range,ULONG *seed)
{
ULONG	result;

	result=*seed;

	result-=(result << 8) + (result >> 8) + (result >> 16) + (result << 16) - 1;

	*seed=result;

	return(result % range);
}
