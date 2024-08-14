/*
 * $Id: FixNames.c,v 1.3 90/09/17 12:26:43 mks Exp $
 *
 * $Log:	FixNames.c,v $
 * Revision 1.3  90/09/17  12:26:43  mks
 * Script now correctly CDs back...
 * 
 * Revision 1.2  90/09/17  12:24:02  mks
 * Now addes the line 'FailAt 100' to the script
 *
 * Revision 1.1  90/09/17  12:13:01  mks
 * Initial revision
 *
 */

/******************************************************************************
 *
 * FixNames - Takes a volume name and a script file name and produces
 *            Two scripts - A "'script'.makesafe" and "'script'.restore"
 *
 *            The scripts will rename the files on the system that contain
 *            European characters and rename them to "__SafeFile #__" name
 *            which, after the new OS is installed, will be renamed back
 *            to the original names.
 *
 *****************************************************************************/

/* Compile with (lattice 5.05)
 *
 * lc -L -cufist fixnames.c
 */

#include	<exec/types.h>
#include	<exec/memory.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>

#include	<libraries/dos.h>
#include	<libraries/dosextens.h>

#include	<clib/exec_protos.h>
#include	<clib/dos_protos.h>

#include	<pragmas/exec_pragmas.h>
#include	<pragmas/dos_pragmas.h>

#include	<string.h>
#include	<stdio.h>

extern	struct Library	*SysBase;
extern	struct Library	*DOSBase;

int CXBRK(void)		{ return(0); }
int chkabort(void)	{ return(0); }

long mywrite(BPTR file,char *string)
{
	return(Write(file,string,strlen(string)));
}

short checkname(char *name)
{
short result;

	for (result=FALSE;((!result) && (*name));name++) if (*name>191) result=TRUE;
	return(result);
}

void bumpname(char *name)
{
short x;

	x=strlen(name);
	while (x-->0)
	{
		if ((name[x]+=1)>'9') name[x]='0';
		else x=0;
	}
}

void writesafe(BPTR file,char *safe)
{
	mywrite(file,"__Safename ");
	mywrite(file,safe);
	mywrite(file,"__");
}

/*
 * DoScan - Scan the lock's directory and write any script information
 * into the files...
 */
short DoScan(BPTR lock, BPTR file1, BPTR file2,char *safename)
{
	short		result=TRUE;
struct	FileInfoBlock	*fib;

	if (fib=AllocMem(sizeof(struct FileInfoBlock),MEMF_PUBLIC|MEMF_CLEAR))
	{
		if (Examine(lock,fib))
		{
			if (fib->fib_DirEntryType>0)
			{
				while (ExNext(lock,fib) && result)
				{
				short dirflag;

					if (dirflag=checkname(fib->fib_FileName))
					{
						bumpname(safename);

						mywrite(file1,"Rename \"");
						mywrite(file1,fib->fib_FileName);
						mywrite(file1,"\" \"");
						writesafe(file1,safename);
						mywrite(file1,"\"\n");

						mywrite(file2,"Rename \"");
						writesafe(file2,safename);
						mywrite(file2,"\" \"");
						mywrite(file2,fib->fib_FileName);
						mywrite(file2,"\"\n");
					}

					if (fib->fib_DirEntryType>0)
					{
					BPTR	newlock;

						lock=CurrentDir(lock);
						newlock=Lock(fib->fib_FileName,ACCESS_READ);
						lock=CurrentDir(lock);
						if (newlock)
						{
							mywrite(file1,"CD \"");
							if (dirflag) writesafe(file1,safename);
							else mywrite(file1,fib->fib_FileName);
							mywrite(file1,"\"\n");
							mywrite(file2,"CD \"");
							mywrite(file2,fib->fib_FileName);
							mywrite(file2,"\"\n");

							printf("%s/",fib->fib_FileName);
							if (result=DoScan(newlock,file1,file2,safename))
							{
							short x;

								x=strlen(fib->fib_FileName)+1;
								while (x--) printf("%c %c",8,8);
								mywrite(file1,"CD /\n");
								mywrite(file2,"CD /\n");
							}

							UnLock(newlock);
						}
					}
				}
			}
			else
			{
				printf("\nInvalid directory\n");
				result=FALSE;
			}
		}
		else
		{
			printf("\nError during Examine()\n");
			result=FALSE;
		}
	}
	else
	{
		printf("\nOut of memory\n");
		result=FALSE;
	}
	return(result);
}

void main(int argc, char *argv[])
{
BPTR	lock;
BPTR	file1;
BPTR	file2;
char	tmp[32];
char	safename[16];


	strcpy(safename,"000000000000000");
	if (argc)	/* Check for CLI start only... */
	{
		if (argc!=3) printf("Usage: %s <device> <scriptname>\n",argv[0]);
		else if (lock=Lock(argv[1],ACCESS_READ))
		{
			if (strlen(argv[2])>20) printf("Script file name too long.\n");
			else
			{
				strcpy(tmp,argv[2]);
				strcat(tmp,".makesafe");
				file1=Open(tmp,MODE_NEWFILE);
				strcpy(tmp,argv[2]);
				strcat(tmp,".restore");
				file2=Open(tmp,MODE_NEWFILE);
				if ((file1)&&(file2))
				{
					mywrite(file1,"; Rename files to safe names\n");
					mywrite(file1,"FailAt 100\n");
					mywrite(file1,"Assign __Junk__: \"\"\n");
					mywrite(file1,"CD \"");
					mywrite(file1,argv[1]);
					mywrite(file1,"\"\n");
					mywrite(file2,"; Rename files back to original names\n");
					mywrite(file2,"FailAt 100\n");
					mywrite(file2,"Assign __Junk__: \"\"\n");
					mywrite(file2,"CD \"");
					mywrite(file2,argv[1]);
					mywrite(file2,"\"\n");
					printf("%s",argv[1]);
					if (DoScan(lock,file1,file2,safename))
					{
						printf("   ...  Finished\n");
						mywrite(file1,"CD __Junk__:\n");
						mywrite(file1,"Assign __Junk__:\n");
						mywrite(file2,"CD __Junk__:\n");
						mywrite(file2,"Assign __Junk__:\n");
					}
					else
					{
						Seek(file1,0,OFFSET_BEGINNING);
						mywrite(file1,"QUIT ; Scan returned error\n");
						Seek(file2,0,OFFSET_BEGINNING);
						mywrite(file2,"QUIT ; Scan returned error\n");
					}
				}
				else printf("Could not open script file(s)\n");
				if (file1) Close(file1);
				if (file2) Close(file2);
			}
			UnLock(lock);
		}
		else printf("Could not lock %s\n",argv[1]);
	}
}
