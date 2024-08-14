/*
 * $Id: SaveVF.c,v 40.3 93/03/03 14:06:19 mks Exp $
 *
 * $Log:	SaveVF.c,v $
 * Revision 40.3  93/03/03  14:06:19  mks
 * Added APPEND keyword to let multipart builds work...
 * 
 * Revision 40.2  93/02/25  15:25:53  mks
 * No longer uses the internal (private) include
 *
 * Revision 40.1  93/02/18  15:39:18  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.12  92/06/06  19:39:26  mks
 * Added message to end of file...
 *
 * Revision 1.11  92/06/06  16:04:08  mks
 * Changed the header for the ReKick files...
 * They now have an ASCII Copyright in them
 *
 * Revision 1.10  92/06/06  11:56:44  mks
 * Added REKICK and FKICK keywords and removed the KICKIT keyword...
 *
 * Revision 1.9  91/10/18  18:41:42  mks
 * New KickIt support fixed
 *
 * Revision 1.8  91/10/18  18:36:48  mks
 * Added KICKIT option to make KickIt files
 *
 * Revision 1.7  91/10/18  15:31:14  mks
 * Very minor code cleanup
 *
 * Revision 1.6  91/10/17  10:26:57  mks
 * Cleaned up formatting so it will print nice
 *
 * Revision 1.5  91/10/15  10:19:39  mks
 * Added standard version string to the commands
 *
 * Revision 1.4  91/10/14  14:11:59  mks
 * Now closes the SuperKick bonus file too... ;^)
 *
 * Revision 1.3  91/10/14  13:54:14  mks
 * Added the SUPERKICK option for making superkickstart files
 *
 * Revision 1.2  91/10/11  20:47:31  mks
 * Added symbol file completion
 *
 * Revision 1.1  91/10/11  08:28:27  mks
 * Moved FreeArgs() call to before the CloseLibrary(DOSBase)
 *
 * Revision 1.0  91/09/16  14:54:32  mks
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

#include	"savevf_rev.h"

/******************************************************************************/

#include	"vf.h"

/*
 * This command saves the VF into a file
 */

/* This is the command template. */
#define TEMPLATE  "FILE/A,ALL/S,SYMBOL/K,SUPERKICK/K,REKICK/S,FKICK/S,APPEND/S" VERSTAG

#define	OPT_FILE	0
#define	OPT_ALL		1
#define	OPT_SYMBOL	2
#define	OPT_SUPERKICK	3
#define	OPT_REKICK	4
#define	OPT_FKICK	5
#define	OPT_APPEND	6
#define	OPT_COUNT	7

LONG cmd_savevf(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
struct	RDArgs		*rdargs;
struct	VF_Sem		*vf;
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
			BPTR	fh;
			ULONG	size;

				if (fh=Open((char *)opts[OPT_FILE],(opts[OPT_APPEND] ? MODE_OLDFILE : MODE_NEWFILE)))
				{
					if (opts[OPT_APPEND]) Seek(fh,0,OFFSET_END);

					size=(vf->LastPos)*4;
					if (opts[OPT_ALL]) size=FULL_SIZE*4;

					/*
					 * If a FKICK file, we do this
					 */
					if (opts[OPT_FKICK])
					{
					ULONG	tmp[2];

						tmp[0]=0;
						tmp[1]=size;
						Write(fh,tmp,8);
					}
					/*
					 * If a REKICK file, we do this...
					 */
					if (opts[OPT_REKICK])
					{
					ULONG	code;
					ULONG	*current;
					ULONG	bytes;

						Write(fh,"AMIGA ROM Operating System and Libraries: Copyright © 1985-1992 Commodore-Amiga, Inc.  All Rights Reserved.",108);

						/*
						 * Now, encode the file
						 */
						code=0xDEADFEED;
						current=vf->Memory;
						bytes=size;

						while (bytes)
						{
							code^=*current;
							*current=code;
							current++;
							bytes-=4;
						}
					}

					if (size!=Write(fh,vf->Memory,size))
					{
						PrintFault(IoErr(),NULL);
					}
					else if (opts[OPT_REKICK])
					{
						Write(fh,"AMIGA ROM Operating System and Libraries: Copyright © 1985-1992 Commodore-Amiga, Inc.  All Rights Reserved.",108);
						Write(fh,"\0Beta Release!  Do not distribute!\0",36);
						rc=RETURN_OK;
					}
					else if (opts[OPT_SUPERKICK])
					{
					BPTR	sfh;

						if (sfh=Open((char *)opts[OPT_SUPERKICK],MODE_OLDFILE))
						{
						ULONG	*buffer;

							Seek(sfh,0,OFFSET_END);
							size=Seek(sfh,0,OFFSET_BEGINNING);
							if (buffer=AllocVec(size,MEMF_PUBLIC))
							{
								if (size=Read(sfh,buffer,size))
								{
									if (size==Write(fh,buffer,size)) rc=RETURN_OK;
									else PrintFault(IoErr(),NULL);
								}
								else PrintFault(IoErr(),NULL);

								FreeVec(buffer);
							}
							else PrintFault(IoErr(),NULL);

							Close(sfh);
						}
						else PrintFault(IoErr(),NULL);
					}
					else rc=RETURN_OK;

					Close(fh);
					/*
					 * Now, if the user wanted to fix the
					 * symbol file...
					 */
					if (rc==RETURN_OK) if (opts[OPT_SYMBOL])
					{
						if (fh=Open((char *)opts[OPT_SYMBOL],MODE_OLDFILE))
						{
						ULONG	*buffer;

							Seek(fh,0,OFFSET_END);
							size=Seek(fh,0,OFFSET_BEGINNING);
							if (buffer=AllocVec(size,MEMF_PUBLIC))
							{
								if (size=Read(fh,buffer,size))
								{
								ULONG	junk[5];
								ULONG	loop;

									Seek(fh,0,OFFSET_BEGINNING);
									junk[0]=HUNK_HEADER;
									junk[1]=0;
									junk[2]=vf->LastHunk;
									junk[3]=0;
									junk[4]=vf->LastHunk-1;
									FWrite(fh,junk,5*4,1);
									junk[0]=0;
									loop=vf->LastHunk;
									while (loop--) FWrite(fh,junk,4,1);
									Flush(fh);
									Write(fh,buffer,size);
								}
								else PrintFault(IoErr(),"Symbol file read error");
								FreeVec(buffer);
							}
							else PrintFault(IoErr(),NULL);
							Close(fh);
						}
						else PrintFault(IoErr(),"Could not open symbol file");
					}
				}
				else PrintFault(IoErr(),NULL);

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
