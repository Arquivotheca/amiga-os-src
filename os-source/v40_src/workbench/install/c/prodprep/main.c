/* main.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <devices/trackdisk.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/expansionbase.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <resources/filesysres.h>

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>

#include <clib/alib_protos.h>

#include "global.h"
#include "refresh.h"
#include "protos.h"

#include "scsi.h"
#include "scsidisk.h"

#include "dummy.h"
#include "prod_prep_rev.h"

#define SAME 0

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))

void FindForiegn(struct RigidDiskBlock *);

extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

extern struct IntuitionBase *IntuitionBase;

extern struct WBStartup *WBenchMsg;

int slowdown = FALSE;

char *device  = "scsi.device";
char *badname = NULL;
int unit = 0;

enum { FP_NOFORIEGN, FP_FORIEGN, FP_FORIEGNPRESERVED };

long ForiegnPartition = FP_NOFORIEGN;

int interactive = FALSE;

int makeflags = 1;              /* to change or not to change drive flags */

char *Vers = VERSTAG;

void
main (int,char **);

void
main (argc,argv)
	int argc;
	char **argv;
{

	int ret,i=1,format_only=FALSE,verify=TRUE,verify_only=FALSE;
	char *name = "rdsk";
	int layout = FALSE;

	IntuitionBase = (struct IntuitionBase *)
			OpenLibrary("intuition.library",33L);
	if (!IntuitionBase)
		exit(20);

	while (argc > i) {
		if (stricmp(argv[i],"badfile") == SAME)
			badname = argv[++i];
		else if (stricmp(argv[i],"formatonly") == SAME)
			format_only = TRUE;
		else if (stricmp(argv[i],"noverify") == SAME)
			verify = FALSE;
		else if (stricmp(argv[i],"verifyonly") == SAME)
			verify_only = TRUE;
		else if (stricmp(argv[i],"slowdown") == SAME)
			slowdown = TRUE;
		else if (stricmp(argv[i],"device") == SAME)
			device = argv[++i];
		else if (stricmp(argv[i],"unit") == SAME)
			unit = atoi(argv[++i]);
		else if (stricmp(argv[i],"layout") == SAME)
			layout = TRUE;
		else if (stricmp(argv[i],"?") == SAME)
		{
			printf("%s\n", VERS);
                        printf(
"usage: Prod_Prep [<filename>|-] [device <name>] [unit <number>]\n"
"\t[layout] [badfile <file>] [formatonly] [noverify] [verifyonly] [slowdown]\n"
			);
			exit(0);
		} else
			name = argv[i];
		i++;
	}

	ret = HandlePrep (name,format_only,verify,verify_only,layout);

	CloseLibrary((struct Library *) IntuitionBase);

	exit(ret);
}

void help(int *);
void quit(int *);
void addpart(int *);
void deletepart(int *);
void writerdb(int *);
void format(int *);
void verify(int *);
void readfs(int *);
void synch(int *);
void reselect(int *);
void readrdb(int *);
void warnerror(int *);

struct commands {
	char *name;
	char *helpstr;
	void (*rtn)(int *);
} comm[] = {
	"?"," - list all commands",help,
	"quit"," - exit this program",quit,
	"addpart",
	  "<name> <size in meg>M|<in k>K|<in Cyls>C|<in percent>%|rest\n\
\t\t[bootable <priority>] [dostype 0x<hex number>]\n\
\t\t[buffers <number>] [mask 0x<hex number>]\n\
\t\t[maxtransfer 0x<hex number>] [customboot <size>]\n\
\t\t[nomount]",addpart,
	"deletepart","<name> [noerror]- delete a partition",deletepart,
	"writerdb","[force] - write out rigid disk block",writerdb,
	"format","[force] - format drive",format,
	"verify","- check for bad sectors and map out",verify,
	"readfs","[<filename>] [version <vers>] [dostype 0x<hex>] - \n"
		 "\t\tread FFS from file (default l:fastfilesystem)",
		readfs,
	"synch","[on|off] - turn synchronous mode on or off",synch,
	"reselect","[on|off] - turn reselect mode on or off",reselect,
	"readrdb","- reads rigid disk block from drive",readrdb,
};

void
help (done)
	int *done;
{
	int i;

	for (i = 0; i < sizeof(comm)/sizeof(comm[0]); i++)
	{
		printf(" %s %s\n",comm[i].name,comm[i].helpstr);
	}
}

void
quit (done)
	int *done;
{
	*done = -1;
}

char *lastarg;		/* for getnum failures */

int
getnum (num)
	long *num;
{
	lastarg = strtok(NULL," ");
	if (!lastarg)
		return FALSE;

	if (strlen(lastarg) != stcd_l(lastarg,num))
	{
		printf("error: bad number (%s)\n",lastarg);
		return FALSE;
	}

	return TRUE;
}

int
gethexnum (num)
	long *num;
{
	lastarg = strtok(NULL," ");
	if (!lastarg)
		return FALSE;

	if (*lastarg != '0' || (*(lastarg+1) != 'x' && *(lastarg+1) != 'X'))
	{
badhex:		printf("error: bad hex number (%s)\n",lastarg);
		return FALSE;
	}
	if (strlen(lastarg+2) != stch_l(lastarg+2,num))
		goto badhex;

	return TRUE;
}

int
get2nums (num1,num2)
	long *num1, *num2;
{
	if (!getnum(num1))
		return FALSE;

	return getnum(num2);
}

int
get3nums (num1,num2,num3)
	long *num1, *num2, *num3;
{
	if (!getnum(num1) || !getnum(num2))
		return FALSE;

	return getnum(num3);
}

int
freespace (void)
{
	register struct PartitionBlock *p;
	LONG numcyl;

	p = SelectedDrive->rdb->rdb_PartitionList;

	/* there's at least one partition */
	numcyl = p->pb_Environment[DE_LOWCYL] -
		 SelectedDrive->rdb->rdb_LoCylinder;

	for (;
	     p->pb_Next;
	     p = p->pb_Next)
	{
		numcyl += p->pb_Next->pb_Environment[DE_LOWCYL] -
			  p->pb_Environment[DE_UPPERCYL] - 1;
	}
	/* add final area */
	numcyl += totalcyls - currcyl;

	return numcyl;
}

void
addpart (done)
	int *done;
{
	char *name;
	int percent = FALSE;
	int cylsize = FALSE;
	long size;
	int bootable = FALSE;
	long bootpri = 0;
	long dostype = 0x444f5301;
	long buffers = 30;
	long mask = 0xfffffffe; /* ? */
	long xfer = 0x00ffffff; /* ? */
	int customboot = FALSE;
	long customblocks = 0;
	int mount = TRUE;
	long startcyl = currcyl;
	long endcyl,numcyl;
	long big_gap;
	char type;
	long left;

	if (!(name = strtok(NULL," ")))
	{
		printf("error: no name specified\n");
		goto err;
	}

	if (!(lastarg = strtok(NULL," ")))
	{
		printf("partition %s: error - no size specified\n",name);
		goto err;
	}

	/* find largest block of empty space */
	big_gap = 0;
	for (CurrentPart = SelectedDrive->rdb->rdb_PartitionList;
	     CurrentPart && CurrentPart->pb_Next;
	     CurrentPart = CurrentPart->pb_Next)
	{
		if ((((LONG)
		      CurrentPart->pb_Next->pb_Environment[DE_LOWCYL]) -
		     CurrentPart->pb_Environment[DE_UPPERCYL]) - 1 >
		    big_gap)
		{
		  big_gap = (((LONG)
			     CurrentPart->pb_Next->pb_Environment[DE_LOWCYL]) -
			     CurrentPart->pb_Environment[DE_UPPERCYL]) - 1;
		}
/*
printf("parts %s and %s: %ld to %ld, big_gap = %ld\n",
	CurrentPart->pb_DriveName,CurrentPart->pb_Next->pb_DriveName,
	CurrentPart->pb_Environment[DE_UPPERCYL],
	CurrentPart->pb_Next->pb_Environment[DE_LOWCYL],
	big_gap);
*/
	}
	/* left pointing at last partition or NULL*/
	if (!CurrentPart)
	{
		/* no partitions */
		big_gap = (SelectedDrive->rdb->rdb_HiCylinder -
			   SelectedDrive->rdb->rdb_LoCylinder) + 1;
	}
	else if (((LONG) SelectedDrive->rdb->rdb_HiCylinder) -
		 CurrentPart->pb_Environment[DE_UPPERCYL] >
		   big_gap)
	{
	   big_gap = ((LONG) SelectedDrive->rdb->rdb_HiCylinder) -
		     CurrentPart->pb_Environment[DE_UPPERCYL];
	}
	/* big_gap is now the largest gap on the disk */

	if (stricmp(lastarg,"rest") == SAME)
	{
		endcyl = startcyl + big_gap - 1;
	} else {
		type = lastarg[strlen(lastarg)-1];
		lastarg[strlen(lastarg)-1] = '\0';
		if (strlen(lastarg) != stcd_l(lastarg,&size) || size <= 0)
		{
			printf("partition %s: error - bad size (%s)\n",
				name,lastarg);
			goto err;
		}

		switch (type) {
		case '%':
			percent = TRUE;
			break;
		case 'M':
		case 'm':
			size = size * 1024;	/* 1M = 1024K */
			break;
		case 'K':
		case 'k':
			break;
		case 'C':
		case 'c':
			endcyl = startcyl + (size-1);
			cylsize = TRUE;
			break;
		default:
		     printf("partition %s: error - unknown size type '%c'\n",
				name,type);
			goto err;
		}

		if (percent)
		{
			endcyl = (((totalcyls -
				    SelectedDrive->rdb->rdb_LoCylinder)*size) /
				  100) + startcyl - 1;
		} else if (!cylsize) {
			/* size is in K */
			/* make sure it rounds up!!! */
			endcyl = startcyl - 1 +
			  ((((size*1024)/SelectedDrive->rdb->rdb_BlockBytes) +
			    SelectedDrive->rdb->rdb_CylBlocks-1) /
			   SelectedDrive->rdb->rdb_CylBlocks);
		}
	}

/*
printf("big_gap = %ld, start = %ld, endcyl = %ld\n",big_gap,startcyl,endcyl);
*/
	if (endcyl < startcyl)
	{
		printf("partition %s: error - partition too small\n",name);
		goto warn;
	}

	if (endcyl >= totalcyls)
	{
		/* search for a hole big enough */
		/* leave CurrentPart pointing at partition before it. */

		/* number of cylinders, inclusive */
		numcyl = (endcyl - startcyl) + 1;

		if (!SelectedDrive->rdb->rdb_PartitionList ||
		    (SelectedDrive->rdb->rdb_PartitionList->
			pb_Environment[DE_LOWCYL] -
		     SelectedDrive->rdb->rdb_LoCylinder >= numcyl))
		{
			/* no other partitions, or enough space up front */
			/* make it the first partition */
			CurrentPart = NULL;
			startcyl = SelectedDrive->rdb->rdb_LoCylinder;
			endcyl = startcyl + numcyl - 1;

		} else {
		  for (CurrentPart = SelectedDrive->rdb->rdb_PartitionList;
		       CurrentPart->pb_Next;
		       CurrentPart = CurrentPart->pb_Next)
		  {
			if ((((LONG)
			     CurrentPart->pb_Next->pb_Environment[DE_LOWCYL]) -
			     CurrentPart->pb_Environment[DE_UPPERCYL]) - 1 >=
			    numcyl)
			{
				/* enough space! */
				break;
			}
		  }
		  if (!CurrentPart->pb_Next) {
		    printf("partition %s: error - not enough space on disk!\n",
			   name);
		    goto warn;
		  }

		  startcyl = CurrentPart->pb_Environment[DE_UPPERCYL] + 1;
		  endcyl = startcyl + numcyl - 1;

		}

	} else {
		/* find last partition */
		for (CurrentPart = SelectedDrive->rdb->rdb_PartitionList;
		     CurrentPart && CurrentPart->pb_Next;
		     CurrentPart = CurrentPart->pb_Next)
		{
			;
		}
		/* startcyl and endcyl set already */
	}

/*
printf("big_gap = %ld, start = %ld, endcyl = %ld\n",big_gap,startcyl,endcyl);
*/

	while (lastarg = strtok(NULL," ")) {
		if (stricmp("bootable",lastarg) == SAME)
		{
			bootable = TRUE;
			if (!getnum(&bootpri))
				goto err;
		} else if (stricmp("dostype",lastarg) == SAME)
		{
			if (!gethexnum(&dostype))
				goto err;
		} else if (stricmp("buffers",lastarg) == SAME)
		{
			if (!getnum(&buffers))
				goto err;
		} else if (stricmp("mask",lastarg) == SAME)
		{
			if (!gethexnum(&mask))
				goto err;
		} else if (stricmp("maxtransfer",lastarg) == SAME)
		{
			if (!gethexnum(&xfer))
				goto err;
		} else if (stricmp("customboot",lastarg) == SAME)
		{
			customboot = TRUE;
			if (!getnum(&customblocks))
				goto err;
		} else if (stricmp("nomount",lastarg) == SAME)
		{
			mount = FALSE;
		} else {
			printf("partition %s: error - unknown option %s\n",
				name,lastarg);
			goto err;
		}
	}

	if (!MakeNewPart(startcyl,endcyl))
	{
		printf("partition %s: error - can't make partitions!\n",name);
		*done = 20;
		return;
	}

	if (endcyl >= currcyl)
		currcyl = endcyl + 1;  /* no more errors possible */

	CurrentPart->pb_Environment[DE_NUMBUFFERS] = buffers;
	CurrentPart->pb_Environment[DE_MASK]       = mask;
	CurrentPart->pb_Environment[DE_MAXTRANSFER] = xfer;
	CurrentPart->pb_Environment[DE_DOSTYPE]    = dostype;

	if (bootable)
		CurrentPart->pb_Flags |= PBFF_BOOTABLE;
	else
		CurrentPart->pb_Flags &= ~PBFF_BOOTABLE;
	CurrentPart->pb_Environment[DE_BOOTPRI] = min(127,max(-128,bootpri));

	if (mount)
		CurrentPart->pb_Flags &= ~PBFF_NOMOUNT;
	else
		CurrentPart->pb_Flags |= PBFF_NOMOUNT;

	if (customboot)
	{
		CurrentPart->pb_Environment[DE_TABLESIZE]  = DE_BOOTBLOCKS;
		CurrentPart->pb_Environment[DE_BOOTBLOCKS] = customblocks;
	} else {
		CurrentPart->pb_Environment[DE_TABLESIZE]  = 16;
		CurrentPart->pb_Environment[DE_BOOTBLOCKS] = 0;
	}

	if (name[strlen(name)-1] == ':')
		name[strlen(name)-1] = '\0';
	strcpy(&CurrentPart->pb_DriveName[0],name);

	size = (endcyl+1-startcyl) * SelectedDrive->rdb->rdb_CylBlocks *
		SelectedDrive->rdb->rdb_BlockBytes;

	left = freespace() * SelectedDrive->rdb->rdb_CylBlocks *
		SelectedDrive->rdb->rdb_BlockBytes;

	if (left/(1024*1024) > 0)
		printf("partition %s: %ldMB (%ldK), %ldMB left\n",name,
			size/(1024*1024),size/1024,left/(1024*1024));
	else
		printf("partition %s: %ldMB (%ldK), %ldK left\n",name,
			size/(1024*1024),size/1024,left/1024);

	/* success */
	return;

err:
	*done = 10;
	return;

warn:	*done = 5;
	return;
}

void
deletepart (int *done)
{
	register struct PartitionBlock *p,*lastp = NULL;
	char *name;
	ULONG left;
	int noerr = FALSE;

	if (!(name = strtok(NULL," ")))
	{
		printf("error: no name specified\n");
		goto err;
	}

	if (name[strlen(name)-1] == ':')
		name[strlen(name)-1] = '\0';

	if (lastarg = strtok(NULL," ")) {
		if (stricmp("noerror",lastarg) == SAME)
			noerr = TRUE;
		else {
			printf("deletepart %s: unknown option %s\n",
				name,lastarg);
			goto err;
		}
	}

	/* find the right partition and delete it */
	for (p = SelectedDrive->rdb->rdb_PartitionList;
	     p && stricmp(name,p->pb_DriveName) != SAME;
	     lastp = p, p = p->pb_Next)
	{
		;
	}
	if (!p)
	{
		if (!noerr)
			printf("deletepart error: can't find partition %s\n",
				name);
		goto err;
	}

	/* if last partition, bump top pointer down */
	if (currcyl == p->pb_Environment[DE_UPPERCYL] + 1)
	{
		if (lastp)
			currcyl = lastp->pb_Environment[DE_UPPERCYL] + 1;
		else
			currcyl = p->pb_Environment[DE_LOWCYL];

		printf("Moved last cyl used down to %ld\n",currcyl-1);
	}

	/* kill old one */
	if (lastp)
		lastp->pb_Next = p->pb_Next;
	else
		SelectedDrive->rdb->rdb_PartitionList = p->pb_Next;

	FreeMem((char *) p, sizeof(*p));

	CurrentPart = SelectedDrive->rdb->rdb_PartitionList;

	left = freespace() * SelectedDrive->rdb->rdb_CylBlocks *
		SelectedDrive->rdb->rdb_BlockBytes;

	if (left/(1024*1024) > 0)
		printf("partition %s deleted, %ldMB left\n",name,
			left/(1024*1024));
	else
		printf("partition %s deleted: %ldK left\n",name,
			left/1024);

	return;

err:
	if (!noerr)
		*done = 10;
}

void
writerdb (done)
	int *done;
{

char *force;
int errwarn = TRUE;

        if (force = strtok(NULL," "))
          if (stricmp(force,"force") == SAME) errwarn = FALSE;

        if (errwarn && ForiegnPartition == FP_FORIEGN) {
          printf("error: tried to overwrite unknown partition type\n");
          *done = 5;
          return;
          }

	/* find last partition */
	for (CurrentPart = SelectedDrive->rdb->rdb_PartitionList;
	     CurrentPart && CurrentPart->pb_Next;
	     CurrentPart = CurrentPart->pb_Next)
	{
		;
	}

	/* don't check if he did a readrdb */
	if (CurrentPart) {
		if (CurrentPart->pb_Environment[DE_UPPERCYL] != totalcyls-1)
		{
			printf("Warning: %ld cylinders not used\n",
			       (totalcyls-1) -
			       CurrentPart->pb_Environment[DE_UPPERCYL]);
		}
	}

	if (!CommitChanges(NULL,FALSE))
	{
		printf("error: write of rdb failed\n");
		*done = 20;
		return;
	}
	// mount new partitions, disable old ones
	if (SelectedDrive->oldrdb)
	{
		// turn off requesters.
		((struct Process *) FindTask(NULL))->pr_WindowPtr = (APTR) -1;

		/* ok, it had partitions on it */
		for (CurrentPart = SelectedDrive->oldrdb->rdb_PartitionList;
		     CurrentPart;
		     CurrentPart = CurrentPart->pb_Next)
		{
			char name[50];
			sprintf(name,"%s:",CurrentPart->pb_DriveName);
/*
kprintf("inhibiting %s\n",name);
{int rc; rc =
*/
			Inhibit(name,TRUE);
/*
kprintf("result %ld\n",rc);
}*/
		}
		((struct Process *) FindTask(NULL))->pr_WindowPtr = NULL;
	}
	// mount the new partitions
	for (CurrentPart = SelectedDrive->rdb->rdb_PartitionList;
	     CurrentPart;
	     CurrentPart = CurrentPart->pb_Next)
	{
		struct DosList *dol;
		struct FileSysStartupMsg *startup;
		LONG *vector;
		char *devicename;
		struct FileSysResource *fsr;
		struct FileSysEntry *fse;
		struct DevProc *dp;
		struct BootNode *bootnode;
		struct ExpansionBase *ExpansionBase;

		ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library",0);

		dol = MakeDosEntry(CurrentPart->pb_DriveName,DLT_DEVICE);
		if (dol)
		{
	            startup = AllocVec(sizeof(struct FileSysStartupMsg),
					MEMF_CLEAR|MEMF_PUBLIC);
	            vector = AllocVec((CurrentPart->pb_Environment[0]+1)*4,
					MEMF_CLEAR|MEMF_PUBLIC);
		    devicename = AllocVec(strlen(device)+2,0);

		    bootnode = AllocVec(sizeof(struct BootNode) + strlen(device) + 1,MEMF_PUBLIC | MEMF_CLEAR);

	            if (!startup || !vector || !devicename || !bootnode)
	            {
			FreeVec(startup);
			FreeVec(vector);
			FreeVec(devicename);
			FreeVec(bootnode);
			FreeDosEntry(dol);
			printf("can't allocate memory!\n");
			break;
		    }
		    strcpy(devicename+1,device);
		    *devicename = strlen(device);
		    startup->fssm_Environ = MKBADDR(vector);
		    startup->fssm_Device  = MKBADDR(devicename);
		    startup->fssm_Unit    = unit;
		    startup->fssm_Flags   = 0;		// FIX?
		    CopyMem(CurrentPart->pb_Environment,vector,
			    (CurrentPart->pb_Environment[0]+1)*4);

                    bootnode->bn_Node.ln_Pri  = vector[DE_BOOTPRI];
                    bootnode->bn_Node.ln_Type = NT_BOOTNODE;
                    bootnode->bn_Node.ln_Name = (STRPTR)((ULONG)bootnode + sizeof(struct BootNode));
                    bootnode->bn_Flags        = 0;
                    strcpy(bootnode->bn_Node.ln_Name,device);
                    bootnode->bn_DeviceNode   = dol;

		    dol->dol_misc.dol_handler.dol_Startup = MKBADDR(startup);
		    dol->dol_misc.dol_handler.dol_StackSize = 600; // LW's
		    dol->dol_misc.dol_handler.dol_Priority  = 10;
		    dol->dol_misc.dol_handler.dol_GlobVec = 0; // default
		    dol->dol_misc.dol_handler.dol_Handler = NULL;
		    dol->dol_misc.dol_handler.dol_SegList = DOSBase->dl_Root->rn_FileHandlerSegment;

		  // stolen from c:mount
                  if (fsr = OpenResource("FileSystem.resource"))
                  {
                    Forbid();

                    fse = (struct FileSysEntry *)fsr->fsr_FileSysEntries.lh_Head;
                    while (fse->fse_Node.ln_Succ)
                    {
                        if (fse->fse_DosType ==
			    CurrentPart->pb_Environment[DE_DOSTYPE])
                        {
                            if (fse->fse_PatchFlags & 0x0001)
                                dol->dol_Type = fse->fse_Type;
                            if (fse->fse_PatchFlags & 0x0002)
                                dol->dol_Task = (struct MsgPort *)fse->fse_Task;
                            if (fse->fse_PatchFlags & 0x0004)
                                dol->dol_Lock = fse->fse_Lock;
                            if (fse->fse_PatchFlags & 0x0008)
                                dol->dol_misc.dol_handler.dol_Handler = fse->fse_Handler;
                            if (fse->fse_PatchFlags & 0x0010)
                                dol->dol_misc.dol_handler.dol_StackSize = fse->fse_StackSize;
                            if (fse->fse_PatchFlags & 0x0020)
                                dol->dol_misc.dol_handler.dol_Priority = fse->fse_Priority;
                            if (fse->fse_PatchFlags & 0x0040)
                                dol->dol_misc.dol_handler.dol_Startup = fse->fse_Startup;
                            if (fse->fse_PatchFlags & 0x0080)
                                dol->dol_misc.dol_handler.dol_SegList = fse->fse_SegList;
                            if (fse->fse_PatchFlags & 0x0100)
                                dol->dol_misc.dol_handler.dol_GlobVec = fse->fse_GlobalVec;
                            break;
                        }
                        fse = (struct FileSysEntry *)fse->fse_Node.ln_Succ;
                    }

                    Permit();
                  } // if fsr...

	  	    if (!AddDosEntry(dol))
		    {
			FreeDosEntry(dol);
			FreeVec(startup);
			FreeVec(vector);
			FreeVec(devicename);
			FreeVec(bootnode);
			break;
		    }

                    // if its bootable, add it to the list...
                    if (bootnode->bn_Node.ln_Pri > -128)
                        Enqueue(&ExpansionBase->MountList,&bootnode->bn_Node);

		    // added, let's touch it
		    // we know that it's null-terminated
		    dp = GetDeviceProc(((char *)BADDR(dol->dol_Name))+1,NULL);
		    if (dp)
			FreeDeviceProc(dp);
		} else {
		    printf("can't make device %s!\n",CurrentPart->pb_DriveName);
		} // if dol


                CloseLibrary(ExpansionBase);
	} // for currentpart

	printf("RigidDiskBlock written\n");
}

void
format (done)
	int *done;
{
char *force;
int errwarn = TRUE;

        if (force = strtok(NULL," "))
          if (stricmp(force,"force") == SAME) errwarn = FALSE;

        if (errwarn && ForiegnPartition) {
          printf("error: tried to format over unknown partition type\n");
          *done = 5;
          return;
          }

	DoLowLevel();
}

void
verify (done)
	int *done;
{
        printf("Verifying...\n");
	VerifyRDSK();
}

struct Resident *FindRomTag(BPTR segList)
{
struct Resident *tag;
UWORD           *seg;
ULONG            i;
ULONG           *ptr;

    while (segList)
    {
        ptr     = (ULONG *)((ULONG)segList << 2);
        seg     = (UWORD *)((ULONG)ptr);
        segList = *ptr;

        for (i=*(ptr-1)>>1; (i>0) ; i--)
        {
            if (*(seg++) == RTC_MATCHWORD)
            {
                tag = (struct Resident *)(--seg);
                if (tag->rt_MatchTag == tag)
                {
                    return(tag);
                }
            }
        }
    }

    return(NULL);
}


void
readfs (done)
	int *done;
{
	char *name;
	LONG version = 0;
	LONG revision = 0;
	long dostype = 0x444f5301;
	struct FileSysHeaderBlock *oldfs;

	if (!(name = strtok(NULL," ")))
		goto argsdone;

	while (lastarg = strtok(NULL," ")) {
		if (stricmp("version",lastarg) == SAME)
		{
			if (!getnum(&version))
				goto err;
		} else if (stricmp("revision",lastarg) == SAME)
		{
			if (!getnum(&revision))
				goto err;
		} else if (stricmp("dostype",lastarg) == SAME)
		{
			if (!gethexnum(&dostype))
				goto err;
		} else {
			printf("unknown option %s\n",lastarg);
err:			printf("error: ReadFS [<filename>] "
		      "[version <vers>] [revision <rev>] [dostype 0x<hex>]\n");
			return;
		}
	}

argsdone:
	/* save old fs ptr */
	oldfs = SelectedDrive->rdb->rdb_FileSysHeaderList;

	if (version == 0 && revision == 0)
	{
		// look for romtag in file
		BPTR seg;
		struct Resident *res;
		char *str;

		seg = LoadSeg(name);
		if (!seg)
			goto no_file;
		res = FindRomTag(seg);
		if (res)
		{
			version = res->rt_Version;

			// we must search for the '.' and get the rev number
			for (str = (char *) res->rt_IdString; *str; str++)
				if (*str == '.')
				{
					revision = atol(str+1);
					break;
				}

		}
		UnLoadSeg(seg);
	}

	SelectedDrive->rdb->rdb_FileSysHeaderList =
	      MakeFileSys(NULL,SelectedDrive->rdb,
			  name,dostype,(version<<16)|revision);

	if (!SelectedDrive->rdb->rdb_FileSysHeaderList)
	{
		SelectedDrive->rdb->rdb_FileSysHeaderList = oldfs; /* restore */
no_file:	printf("ERROR: Can't read %s!\n",name);
		*done = 10;
		return;
	}

	printf("loaded filesystem from %s, version = %ld, type = 0x%lx\n",
		name,version,dostype);

	FreeFileSys(oldfs);
}

void
synch (done)
	int *done;
{
	char *onoff;
	int on;

	if (!(onoff = strtok(NULL," ")))
	{
err:		printf("usage: synch [on|off]\n");
		return;
	}

	if (stricmp(onoff,"on") == SAME)
		on = TRUE;
	else if (stricmp(onoff,"off") == SAME)
		on = FALSE;
	else if (SelectedDrive->rdb->rdb_Flags & RDBFF_SYNCH)
	{
		printf("Synchronous mode is set\n");
		return;
	} else {
		printf("Synchronous mode is not set\n");
		return;
	}

	if (on)
		SelectedDrive->rdb->rdb_Flags |= RDBFF_SYNCH;
	else
		SelectedDrive->rdb->rdb_Flags &= ~RDBFF_SYNCH;
}

void
reselect (done)
	int *done;
{
	char *onoff;
	int on;

	if (!(onoff = strtok(NULL," ")))
	{
err:		printf("usage: reselect [on|off]\n");
		return;
	}

	if (stricmp(onoff,"on") == SAME)
		on = TRUE;
	else if (stricmp(onoff,"off") == SAME)
		on = FALSE;
	else if (!(SelectedDrive->rdb->rdb_Flags & RDBFF_NORESELECT))
	{
		printf("Reselection mode is set\n");
		return;
	} else {
		printf("Reselection mode is not set\n");
		return;
	}

	if (on)
		SelectedDrive->rdb->rdb_Flags &= ~RDBFF_NORESELECT;
	else
		SelectedDrive->rdb->rdb_Flags |= RDBFF_NORESELECT;
}

void
readrdb (int *done)
{
	struct IOStdReq *ior = NULL;
	struct MsgPort *port;
	int    opened = FALSE,i;
	struct RigidDiskBlock *rdb,*oldrdb;

        ForiegnPartition = FP_FORIEGNPRESERVED;
	 /* since we are reading the RDB, and you can't remove a partiton,
            if we do a write RDB we won't destroy anything */
	 /* well, you can, but they have to try. */
        makeflags = 1;

	if (!(port = CreatePort(0L,0L)))
		goto cleanup;
	if (!(ior = CreateStdIO(port)))
		goto cleanup;

	if (i = OpenDevice(device,unit,
			   (struct IORequest *) ior,0L))
	{
		printf("Error %d on device open!\n",i);
		goto cleanup;
	}
	opened = TRUE;

	/* save for freeing */
	rdb	= SelectedDrive->rdb;
	oldrdb	= SelectedDrive->oldrdb;

	if (!GetRDB(ior,SelectedDrive))
		printf("No RDB on drive!\n");	/* didn't touch rdb, oldrdb */
	else {
                makeflags = 0;

		/* defaults - override GetDef */
		currcyl = SelectedDrive->rdb->rdb_LoCylinder;
		totalcyls = SelectedDrive->rdb->rdb_HiCylinder + 1;

		/* make it look like we created all these partitions */
		/* find last partition */
		for (CurrentPart = SelectedDrive->rdb->rdb_PartitionList;
		     CurrentPart && CurrentPart->pb_Next;
		     CurrentPart = CurrentPart->pb_Next)
		{
			printf("\tFound partition %s\n",
				CurrentPart->pb_DriveName);
		}
		/* left pointing at last partition or NULL */
		if (CurrentPart)
			printf("\tFound partition %s\n",
				CurrentPart->pb_DriveName);

		LastPart = CurrentPart;

		/* correct uppercyl for adding partitions */
		if (CurrentPart)
			currcyl = CurrentPart->pb_Environment[DE_UPPERCYL] + 1;

		printf("Found RDB at block %ld\n",SelectedDrive->Block);
		FreeRDB(rdb);
		FreeRDB(oldrdb);

		FindForiegn(SelectedDrive->rdb);
	}

cleanup:
	if (opened)
		CloseDevice((struct IORequest *) ior);
	if (ior)
		DeleteStdIO(ior);
	if (port)
		DeletePort(port);
}

int
HandlePrep (name,format_only,verify,verify_only,layout)
	char *name;
	int format_only,verify,verify_only,layout;
{
	CPTR rdsk;
	LONG size,done = 20;
	FILE *fp;

	if ((verify_only && (layout || format_only || name)) ||
	    (format_only && (layout || name)))
	{
		printf("incompatible options!\n");
		return done;
	}

	if (!verify_only && !format_only)
	{
		if (!layout)
		{
			rdsk = GetRDSK(name,&size);
			if (size == -1)
			{
				printf("Error: can't open %s!\n",name);
				return done;
			}
		} else {
			/* open layout file to read command lines from */
			if (strcmp(name,"-") == SAME)
			{
				fp = stdin;
				interactive = TRUE;
			} else {
				printf("reading from file %s\n",name);
				fp = fopen(name,"r");
				if (!fp)
				{
					printf("Error: can't open %s!\n",name);
					return done;
				}
			}
		}
	}

	if (layout)
	{
		char buff[256];
		char *command;
		struct DriveDef *def;
		struct Drive *d;
		int i,len,eof = TRUE;

                d = SelectedDrive = AllocNew(Drive);

		printf("%s\n", VERS);

		/* read default configuration from drive */
		d->Flags = SCSI;
		strcpy(&d->DeviceName[0],device);
		d->Addr = unit % 10;
		d->Lun  = unit / 10;

		if (!(def = DoDefine(NULL)))
			goto layout_err;

		d->rdb = CopyRDB(def->Initial_RDB);
		if (!d->rdb)
		{
			FreeDef(def);
			printf("Out of mem!\n");
			goto layout_err;
		}

		currcyl = SelectedDrive->rdb->rdb_LoCylinder;
		totalcyls = SelectedDrive->rdb->rdb_HiCylinder + 1;

		/* tell what drive it is */
		memcpy(&buff[0],&(d->rdb->rdb_DiskVendor[0]),8);
		buff[8] = ' ';
		memcpy(&buff[9],&(d->rdb->rdb_DiskProduct[0]),16);
		buff[8+1+16] = ' ';
		memcpy(&buff[9+16+1],&(d->rdb->rdb_DiskProduct[0]),4);
		buff[8+1+16+1+4] = '\0';

		printf("%s\n\tsize = %ldMB (%ld cylinders)\n",buff,
			((d->rdb->rdb_HiCylinder + 1 - d->rdb->rdb_LoCylinder) *
			 d->rdb->rdb_CylBlocks * d->rdb->rdb_BlockBytes) /
			(1024*1024),d->rdb->rdb_Cylinders);


		/* read lines to describe partitions */
		if (!interactive)
			printf("Reading partition/setup information\n");

		eof = FALSE;

		/* eof == returncode on error! */
		/* don't exit if interactive */
		while (eof != -1 && (interactive || eof == 0))
		{
			if (fp == stdin)
			{
				fputs("Production Prep> ",stdout);
				fflush(stdout);
			}
			if (fgets(buff,sizeof(buff),fp) == NULL)
				break;			/* eof */

			len = strlen(buff);		/* kill \n */
			if (len && buff[len-1] == '\n')
				buff[len-1] = '\0';

			command = strtok(buff," ");
			if (!command || !*command ||	/* no command */
			    *command == ';')
				continue;

			for (i = 0; i < sizeof(comm)/sizeof(comm[0]); i++)
			{
				if (stricmp(command,comm[i].name) == SAME)
				{
					/* found it - execute */

					(*(comm[i].rtn))(&eof);
					break;
				}
			}
			if (i >= sizeof(comm)/sizeof(comm[0]))	/* failure */
				printf("Error: command %s not known!\n",command);
		}
layout_err:
		if (eof == -1)	/* exit via quit */
			eof = 0;
		done = eof;

		if (fp != stdin)
			fclose(fp);

		if (d) {
			FreeRDB(d->rdb);
			FreeRDB(d->oldrdb);

			FreeBad(d->bad);
			FreeMem((char *) d, sizeof(*d));
		}
	} else if (!verify_only)
	{
/* old-style prodprep code */
	    if (WriteRDSK(rdsk,size,0)) /* because of stupid WD drives */
	    {
	      ReopenDrives();     /* make driver re-read it */
	      Delay(200L); /* give it time */

	      if (badname)	/* file of bad blocks */
		  ReadBad((struct RigidDiskBlock *) rdsk);

	      printf("Formatting...\n");
	      if (!DoLowLevel())
	      {
		if (!format_only)
		{
			if (WriteRDSK(rdsk,size,0))
							  /* format whomps it */
			{
				if (verify)
				{
				        printf("Verifying...\n");
					if (!VerifyRDSK())
					{
						ReopenDrives();
						Delay(200L); /* give it time */
						done = 0;
					}
				} else
					done = 0;
			}
		} else
			done = 0;
	      }
	    }
	} else {
		printf("Verifying...\n");
		if (!VerifyRDSK())
			done = 0;
	}
	return done;
}

CPTR
GetRDSK (name,size)
	char *name;
	LONG *size;
{
	BPTR fh;
	CPTR ptr = NULL;

	*size = -1;

	if (fh = Open(name,MODE_OLDFILE))
	{
		Seek(fh,0L,OFFSET_END);
		*size = Seek(fh,0L,OFFSET_CURRENT);
		Seek(fh,0L,OFFSET_BEGINNING);

		ptr = (CPTR) AllocMem(*size,MEMF_CLEAR);
		if (ptr)
		{
			if (Read(fh,(char *) ptr, *size) != *size)
			{
				FreeMem((char *) ptr,*size);
				*size = -1;
			}
		} else
			*size = -1;

		Close(fh);
	}

	return ptr;
}

int
ReadBad (rdb)
	struct RigidDiskBlock *rdb;
{
	struct BadBlockBlock *bad;
	BPTR fh;
	char buf[80],ch;
	int eof = FALSE,i,j;
	LONG block,LastBlock,len;

	bad = (struct BadBlockBlock *) AllocMem(512,MEMF_CLEAR);
	if (!bad)
		return FALSE;

	bad->bbb_ID          = IDNAME_BADBLOCK;
	bad->bbb_SummedLongs = 6;
	bad->bbb_HostID	     = 7; /* FIX! */
	bad->bbb_Next	     = (struct BadBlockBlock *) -1;

	LastBlock = rdb->rdb_RDBBlocksHi + 1;

	fh = Open(badname,MODE_OLDFILE);
	if (!fh)
	{
		printf("Can't open %s!\n",badname);
		FreeMem((char *) bad,512);
		return FALSE;
	}

	while (!eof)
	{
next_block:
		ch = '\0';
		for (i = 0; ch != '\n'; i++)
		{
			len = Read(fh,&ch,1);
			if (len <= 0)
			{
				eof = TRUE;
				break; /* from for */
			}
			buf[i] = ch;
		}
		buf[i] = '\0';

		/* parse the line */
		len = sscanf(buf,"bad block: %ld",&block);
		if (len)
		{
			/* use driver mapping */
			i = ((bad->bbb_SummedLongs - 6) * 4) /
			    sizeof(struct BadBlockEntry);
			if (i >= 61)
			{
			  printf("Too many bad blocks (can only handle 61)!\n");
			  printf("Use HDToolbox\n",0);
			  Close(fh);
			  FreeMem((char *) bad,512);
			  return FALSE;
			}

			/* check for dups */
			for (j = 0; j < i; j++)
			{
			  if (bad->bbb_BlockPairs[j].bbe_BadBlock == block)
			  {
				printf("Block already mapped to block %ld\n",
				       bad->bbb_BlockPairs[i].bbe_GoodBlock);
				goto next_block;
			  }
			}

			bad->bbb_BlockPairs[i].bbe_BadBlock  = block;
			bad->bbb_BlockPairs[i].bbe_GoodBlock = --LastBlock;
			bad->bbb_SummedLongs += (sizeof(struct BadBlockEntry)
						 >> 2);
			printf("Replaced with block %ld\n",LastBlock);
		}
	}
	Close(fh);

	/* handles MAX 1 block of bad blocks!!!! */
	if (bad->bbb_SummedLongs != 6)
	{
		if ((LONG) rdb->rdb_BadBlockList == -1)
			rdb->rdb_BadBlockList = (struct BadBlockBlock *)
						++(rdb->rdb_HighRDSKBlock);
		printf("Writing bad block table at block %ld\n",
		       (LONG) rdb->rdb_BadBlockList);

		CheckSumBlock(rdb);
		WriteRDSK((CPTR) rdb,512,0);
		CheckSumBlock((struct RigidDiskBlock *) bad);
		WriteRDSK((CPTR) bad,512,((LONG)rdb->rdb_BadBlockList) * 512);
		FreeMem((char *) rdb,512);
	}
	FreeMem((char *) bad, 512);

	return TRUE;
}

void
FreeBad (b)
	register struct Bad *b;
{
	register struct Bad *nextb;

	for(; b; b = nextb)
	{
		nextb = b->Next;
		FreeMem((char *) b, sizeof(*b));
	}
}

void
FindForiegn(struct RigidDiskBlock *rdb) {
  struct PartitionBlock *Next;

  Next = rdb->rdb_PartitionList;

  while (Next) {
    if ((Next->pb_Environment[DE_DOSTYPE] & 0xFFFFFF00L) != 'DOS\0')
      ForiegnPartition = FP_FORIEGN;
    Next = Next->pb_Next;
    }

}
