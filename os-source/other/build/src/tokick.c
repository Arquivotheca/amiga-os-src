/*
 * $Id: ToKick.c,v 40.4 93/05/25 14:38:56 mks Exp $
 *
 * $Log:	ToKick.c,v $
 * Revision 40.4  93/05/25  14:38:56  mks
 * Added support to make SuperKickstart disks from a devs:kickstart file
 * This is for A3000 kickstarts
 * 
 * Revision 40.3  93/03/03  15:34:41  mks
 * Added the StripSuper keyword...
 *
 * Revision 40.2  93/02/25  15:26:36  mks
 * No longer uses the internal (private) include
 *
 * Revision 40.1  93/02/18  15:39:54  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 39.1  92/04/23  15:04:00  mks
 * Initial release
 *
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/alerts.h>

#include <devices/trackdisk.h>

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

#include	"tokick_rev.h"

/******************************************************************************/

/* This is the command template. */
#define TEMPLATE  "FILE/A,TO/A,STRIPSUPER/S" VERSTAG

#define	OPT_FILE	0
#define	OPT_TO		1
#define	OPT_STRIP	2
#define	OPT_COUNT	3

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
		ULONG	*buffer;
		ULONG	len;
		char	*name;
		LONG	disk=-1;

			name=(char *)opts[OPT_TO];
			len=strlen(name);
			if ((len==4) && (name[3]==':'))
			{
				if ( ((name[0]=='D') || (name[0]=='d')) &&
				     ((name[1]=='F') || (name[1]=='f')) )
				{
					disk=name[2]-'0';
					if (disk>3) disk=-1;
				}
			}

			if (disk>=0)
			{
				/* Do SuperKickstart disk... */
				if (buffer=AllocVec(2*11*80*512,MEMF_PUBLIC|MEMF_CLEAR))
				{
				BPTR	file;

					buffer[0]='KICK';
					buffer[1]='SUP0';

					if (file=Open((char *)opts[OPT_FILE],MODE_OLDFILE))
					{
						Seek(file,0,OFFSET_END);
						len=Seek(file,0,OFFSET_BEGINNING);
						if (KS_SIZE==Read(file,&(buffer[0x40400/4]),KS_SIZE))
						{
							len-=KS_SIZE;
							if (len==Read(file,&(buffer[0xC0400/4]),len))
							{
							struct	MsgPort	*port;

								/* Set bonus size */
								buffer[3]=(len+511) & (~511);

								if (port=CreateMsgPort())
								{
								struct	IOExtTD	*io;

									if (io=(struct IOExtTD *)CreateIORequest(port,sizeof(struct IOExtTD)))
									{
										if (!OpenDevice("trackdisk.device",disk,(struct IORequest *)io,0))
										{
											Inhibit(name,TRUE);

											PutStr("Writing SuperKickstart disk in ");
											PutStr(name);
											PutStr("\n");

											io->iotd_Req.io_Length=1;
											io->iotd_Req.io_Command=TD_MOTOR;
											DoIO((struct IORequest *)io);

											io->iotd_Req.io_Length=2*11*80*512;
											io->iotd_Req.io_Data=buffer;
											io->iotd_Req.io_Command=TD_FORMAT;
											io->iotd_Req.io_Offset=0;
											DoIO((struct IORequest *)io);

											io->iotd_Req.io_Command=CMD_UPDATE;
											DoIO((struct IORequest *)io);

											io->iotd_Req.io_Command=CMD_CLEAR;
											DoIO((struct IORequest *)io);

											io->iotd_Req.io_Length=0;
											io->iotd_Req.io_Command=TD_MOTOR;
											DoIO((struct IORequest *)io);

											CloseDevice((struct IORequest *)io);

											PutStr("Done.\n");

											Inhibit(name,FALSE);
										}
										else PutStr("Could not open disk device.\n");
										DeleteIORequest((struct IORequest *)io);
									}
									else PrintFault(IoErr(),NULL);
									DeleteMsgPort(port);
								}
								else PrintFault(IoErr(),NULL);
							}
							else PrintFault(IoErr(),NULL);
						}
						else PrintFault(IoErr(),NULL);
						Close(file);
					}
					else PrintFault(IoErr(),NULL);

					FreeVec(buffer);
				}
				else PrintFault(IoErr(),NULL);
			}
			else if (buffer=AllocVec(8+KS_SIZE,MEMF_PUBLIC))
			{
			BPTR	file;

				buffer[0]=0;
				buffer[1]=KS_SIZE;
				if (file=Open((char *)opts[OPT_FILE],MODE_OLDFILE))
				{
					if (KS_SIZE==Read(file,&(buffer[(opts[OPT_STRIP] ? 0 : 2)]),KS_SIZE))
					{
					BPTR	newfile;

						if (newfile=Open(name,MODE_NEWFILE))
						{
						ULONG	size=8+KS_SIZE;

							if (opts[OPT_STRIP]) size=KS_SIZE;

							if (size==Write(newfile,buffer,size))
							{
								rc=RETURN_OK;
							}
							else PrintFault(IoErr(),NULL);
							Close(newfile);
						}
						else PrintFault(IoErr(),NULL);
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
