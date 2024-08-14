/*
 * $Id: makeromimage.c,v 40.2 93/06/03 16:30:49 mks Exp $
 *
 * $Log:	makeromimage.c,v $
 * Revision 40.2  93/06/03  16:30:49  mks
 * Added status message so you can see that it did something
 * 
 * Revision 40.1  93/06/03  15:41:16  mks
 * *** empty log message ***
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
#include <dos/stdio.h>
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

#include	"makeromimage_rev.h"

/******************************************************************************/

/* This is the command template. */
#define TEMPLATE  "FILES/M/A,TO/A" VERSTAG

#define	OPT_FILES	0
#define	OPT_TO		1
#define	OPT_COUNT	2

LONG cmd(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
	LONG		rc=RETURN_FAIL;

	if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
	{
	struct	RDArgs	*rdargs;
		LONG	opts[OPT_COUNT];

		memset((char *)opts, 0, sizeof(opts));

		if (rdargs=ReadArgs(TEMPLATE, opts, NULL))
		{
		BPTR	ofile;

			if (ofile=Open((char *)opts[OPT_TO],MODE_NEWFILE))
			{
			char	**files;
			char	*file;
			ULONG	header[4]={0x000003E7,
					   0x00000000,
					   0x000003E9,
					   0x00000001};

				rc=RETURN_OK;

				SetVBuf(ofile,NULL,BUF_FULL,8*1024);

				FWrite(ofile,header,4,4);

				files=(char **)opts[OPT_FILES];
				while ((file=*files++) && (RETURN_OK==rc))
				{
				BPTR	ifile;

					if (ifile=Open(file,MODE_OLDFILE))
					{
					ULONG	size;
					char	*local;

						Seek(ifile,0,OFFSET_END);
						size=Seek(ifile,0,OFFSET_BEGINNING);
						if (local=AllocVec(size+4+32+8+3,MEMF_CLEAR|MEMF_PUBLIC))
						{
							strcpy(&local[4],FilePart(file));
							*(ULONG *)(&local[4+32])=NULL;		/* *next */
							*(ULONG *)(&local[4+32+4])=size+8;	/* size */
							Read(ifile,&local[4+32+8],size);	/* data */
							size=(size+4+32+8+3)/4;
							*(ULONG *)local=size;

							if (size!=FWrite(ofile,local,4,size))
							{
								PrintFault(IoErr(),NULL);
								rc=RETURN_FAIL;
							}
							header[3]+=size;

							FreeVec(local);
						}

						Close(ifile);
					}
					else PrintFault(IoErr(),file);
				}

				if (RETURN_OK==rc)
				{
				ULONG	external[8]={0x00000000,
						     0x000003EF,
						     0x01000002,
						     0x5F524F4D,
						     0x44617461,
						     0x00000000,
						     0x00000000,
						     0x000003F2};


					FWrite(ofile,external,4,8);
					Seek(ofile,0,OFFSET_BEGINNING);
					FWrite(ofile,header,4,4);

					header[2]=opts[OPT_TO];
					header[3]*=4;
					VPrintf("ROM Disk image built to %s, %ld bytes\n",&header[2]);
				}

				Close(ofile);
			}
			else PrintFault(IoErr(),(char *)opts[OPT_TO]);

			if (rc!=RETURN_OK) DeleteFile((char *)opts[OPT_TO]);

			FreeArgs(rdargs);
		}
		else PrintFault(IoErr(),NULL);

		CloseLibrary((struct Library *)DOSBase);
	}
	else
	{
		OPENFAIL;
	}
	return(rc);
}
