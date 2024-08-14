/************************************************************************
 *
 * DJMount.c - mount command for jdisk.device
 *
 * Copyright (c) 1988, Commodore Amiga Inc., All rights reserved.
 *
 * 10-04-88 - BK - Converted to C lanquage
 ************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <janus/janus.h>

#include <stdio.h>

#ifdef LATTICE
int CXBRK(void) {return 0;}
#endif

#define PARMPKTSIZE  (25*4)

#define D(x) ;

char *name_array[8] = {
	"JH0",
	"JH1",
	"JH2",
	"JH3",
	"JH4",
	"JH5",
	"JH6",
	"JH7" };

char *name_array1[8] = {
	"JH0:",
	"JH1:",
	"JH2:",
	"JH3:",
	"JH4:",
	"JH5:",
	"JH6:",
	"JH7:" };


struct JanusBase *JanusBase = 0;
struct ExpansionBase *ExpansionBase = 0;

extern APTR DOSBase;

void main(argc, argv)
int argc;
char *argv[];
{
int	ReturnCode = 0;
int	Signal = -1;
struct	SetupSig *SS = 0;
struct	HardDskReq *ReqPtr = 0;
int	Parts = 0;
struct	HDskPartition *PartTable = 0;
struct	HDskPartition *PartPtr = 0;
int	CPart = 0;
ULONG	*ParmPkt = 0;
struct	DeviceNode *DN = 0;
UBYTE	*StupidAssBCPLHName = 0;
UBYTE	FFSFlag	= 0;
int	x, waitsecs;
struct	FileLock *lock;
char *av[4];
char ibuf[80];
char *token;

#define BRKCHR	" \t"

	waitsecs = 0;
	FFSFlag = 0;

	/* wb? */
	if (argc == 0)
		exit(0);

	for (x = 0; x < argc; x++)
		av[x] = argv[x];

again:
	if (argc > 4) {
		printf("DJMount: too many arguments\n");
		exit(118);
	}

	if (argc == 2 && !strcmp(av[1], "?")) {
		printf("FFS/S,DELAY/N/K: ");
		fgets(ibuf, sizeof(ibuf) - 1, stdin);
		x = strlen(ibuf);
		if (x) {
			if (ibuf[x-1] == '\n')
				ibuf[x-1] = 0;
		}
		argc = 1;
		token = strtok(ibuf, BRKCHR);
		while (token && token[0]) {
			av[argc++] = token;
			if (argc == 4)
				break;
			token = strtok(0, BRKCHR);
		}

		goto again;
	}

/*
	for (x = 0; x < argc; x++)
		printf("av[%d] = '%s'\n", x, av[x]);
*/
	for (x = 1; x < argc; x++) {
		if (stricmp(av[x], "FFS") == 0) {
			FFSFlag = 1;
		} else if (stricmp(av[x], "DELAY") == 0) {
			if (++x < argc) {
				waitsecs = atoi(av[x]);
				if (waitsecs <= 0 || waitsecs > 120) {
					printf("DJMount: DELAY value must be between 1 and 120 seconds.\n");
					exit(100);
				}
			} else {
				printf("DJMount: illegal command line option\n");
				exit(100);
			}
		} else {
			printf("DJMount: illegal command line option\n");
			exit(100);
		}
	}

	/* if no wait specified, use 60 */
	if (waitsecs == 0)
		waitsecs = 60;

	if ((JanusBase = (struct JanusBase *)OpenLibrary("janus.library", 0)) == 0) {
		printf("DJMount: could not open Janus.library.\n");
		ReturnCode = 100;
		goto cleanup;
	}

	if ((ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0)) == 0) {
		printf("DJMount: could not open Expansion.library.\n");
		ReturnCode = 100;
		goto cleanup;
	}

	if ((Signal = AllocSignal(-1)) == -1) {
		printf("DJMount: could not allocate a signal.\n");
		ReturnCode = 100;
		goto cleanup;
	}

	/*** This SHOULD fail if jdisk.device is up and running!! ***/
	/*** djmount used to look in the DOS device lists to see  ***/
	/*** if jh0: was already mounted. I found that jdisk uses ***/
	/*** setup sig if it is running so we assume this call	  ***/
	/*** will fail if any unit is mounted!			  ***/

	if ((SS = SetupJanusSig(JSERV_HARDDISK, Signal,
			     sizeof(struct HardDskReq),
			     MEMF_PARAMETER | MEM_WORDACCESS)) == 0) {
		printf("DJMount: devices already mounted.\n");
		ReturnCode = 10;
		goto cleanup;
	}

	/* Check if PC running */
	if (((struct JanusAmiga *)MakeWordPtr(JanusBase->jb_ParamMem))->ja_HandlerLoaded == 0) {

		/* it's not running - inform user and wait for it */
		printf("DJMount: waiting %d seconds for the PC to initialize - CTRL-C to abort...", waitsecs);

		for (x = 0; x < waitsecs; x++) {
			if (((struct JanusAmiga *)MakeWordPtr(JanusBase->jb_ParamMem))->ja_HandlerLoaded != 0)
				break;

D(printf("Waiting for PC handler\n"))

			Delay(50);	/* wait a second */

			/* check for ctrl-c from user */
			if (SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
				printf("\nDJMount: user aborted.\n");
				ReturnCode = 20;
				goto cleanup;
			}
		}

		printf("\n");

		if (x == waitsecs) {
			printf("DJMount: timed out waiting for JanusHandler.\n");
			ReturnCode = 20;
			goto cleanup;
		}

		/* 1 second of fudge after he says go */
		Delay(10 * 50);
	}

	ReqPtr = (struct HardDskReq *)SS->ss_ParamPtr;

D(printf("ReqPtr @ %lx\n", ReqPtr))

	ReqPtr->hdr_Fnctn = HDR_FNCTN_INIT;
	SendJanusInt(JSERV_HARDDISK);
	Wait(SS->ss_SigMask);

D(printf("hdr_Err = %ld\n", ReqPtr->hdr_Err))

D(printf("%ld Partitions Found.\n", (ULONG)ReqPtr->hdr_Part))


	Parts = ReqPtr->hdr_Part;
	if (Parts == 0) {
		printf("DJMount: no Amiga partitions found.\n");
		ReturnCode = 10;
		goto cleanup;
	}

	if ((PartTable = (struct HDskPartition *)
			AllocJanusMem(sizeof(struct HDskPartition)*8,
				      MEMF_BUFFER | MEM_WORDACCESS)) == 0) {
		printf("DJMount: could not allocate memory for PartTable.\n");
		ReturnCode = 100;
		goto cleanup;
	}

D(printf("PartTable @ %lx\n", PartTable))

	ReqPtr->hdr_BufferAddr = JanusMemToOffset(PartTable);

	for (CPart = 0; CPart < Parts; CPart++) {
		ReqPtr->hdr_Fnctn = HDR_FNCTN_INFO;
		ReqPtr->hdr_Count = sizeof(struct HDskPartition);
		ReqPtr->hdr_Part = CPart;

		SendJanusInt(JSERV_HARDDISK);
		Wait(SS->ss_SigMask);

		ReqPtr->hdr_BufferAddr += sizeof(struct HDskPartition);
	}

	if (SS) {
		CleanupJanusSig(SS);
		SS = 0;
	}

	if ((ParmPkt = (ULONG *)AllocMem(PARMPKTSIZE, MEMF_CLEAR)) == 0) {
		printf("DJMount: could not allocate memory for ParmPkt.\n");
		ReturnCode = 100;
		goto cleanup;
	}

	if (FFSFlag) {
		if ((StupidAssBCPLHName = (UBYTE *)AllocMem(20, MEMF_CLEAR)) == 0) {
			printf("DJMount: could not allocate memory for handler name.\n");
			ReturnCode = 100;
			goto cleanup;
		}

		memcpy(StupidAssBCPLHName, "\020L:FastFileSystem", 17);
	}

	PartPtr = PartTable;
	for (CPart = 0; CPart < Parts; CPart++) {

D(printf("PartPtr = %lx\n", PartPtr))
D(printf("Partition %ld\n", CPart))
D(printf("BaseCyl = %ld\n", PartPtr->hdp_BaseCyl))
D(printf("EndCyl  = %ld\n", PartPtr->hdp_EndCyl))
D(printf("Drive	= %lx\n", PartPtr->hdp_DrvNum))
D(printf("Heads	= %ld\n", PartPtr->hdp_NumHeads))
D(printf("Secs	 = %ld\n", PartPtr->hdp_NumSecs))

		ParmPkt[0] = (ULONG)name_array[CPart];
		ParmPkt[1] = (ULONG)"jdisk.device";	/* Device name */
		ParmPkt[2] = CPart;			/* Device unit # */
		ParmPkt[3] = 0;				/* Device open flags */
		ParmPkt[4] = 16;			/* Environment length */
		ParmPkt[5] = 512 >> 2;			/* Long words is block */
		ParmPkt[6] = 0;				/* Sector origin */
		ParmPkt[7] = PartPtr->hdp_NumHeads;	/* # of surfaces */
		ParmPkt[8] = 1;				/* Secs/logical block */
		ParmPkt[9] = PartPtr->hdp_NumSecs;	/* Secs per track */
		ParmPkt[10] = 2;			/* Reserved blocks */
		ParmPkt[11] = 0;			/* ?? unused */
		ParmPkt[12] = 0;			/* Interleave */
		ParmPkt[13] = 0;			/* Lower Cylinder */
		ParmPkt[14] = PartPtr->hdp_EndCyl - PartPtr->hdp_BaseCyl - 2; /* Upper Cylinder */
		ParmPkt[15] = 30;			/* # of buffers	*/
		ParmPkt[16] = 0;			/* buff mem type */
		ParmPkt[17] = 0x7FFFFFFF;		/* Max Transfer */
		ParmPkt[18] = 0;			/* Mask */
		ParmPkt[19] = 0;			/* Boot Pri */

		if (FFSFlag)
			ParmPkt[20] = 0x444F5301;	/* Dos Type FFS */
		else
			ParmPkt[20] = 0x444F5300;	/* Dos Type OldFileSys */

		if (ParmPkt[14] < 3) {
			printf("DJMount: partition %d must be at least 3 cylinders.\n", CPart);
			ReturnCode = 10;
			goto nextpart;
		}

		if ((DN = (struct DeviceNode *)MakeDosNode(ParmPkt)) == 0) {
			printf("DJMount: MakeDosNode for partition %d failed.\n", CPart);
			ReturnCode = 100;
			goto cleanup1;
		}

		if (FFSFlag) {
			DN->dn_Handler = (((ULONG)StupidAssBCPLHName) >> 2L);
			DN->dn_GlobalVec = 0xffffffff;
			DN->dn_StackSize = 4000L;
		}

		if (AddDosNode(-128, 1L, DN) == 0) {
			printf("DJMount: AddDosNode for partition %d failed.\n", CPart);
			ReturnCode = 100;
			goto cleanup1;
		}

		/* fire up the icon! */
		if (lock = (struct FileLock *)Lock(name_array1[CPart],
						   ACCESS_READ))
			UnLock(lock);

nextpart:
		PartPtr++;
	}

cleanup1:
	printf("DJMount: mounted %d partition%s.\n",
		CPart, ((CPart == 1) ? "" : "s"));

cleanup:

/*** Can't free this up cause device is not accessed until this program ***/
/*** is history ***/
/*
	if (StupidAssBCPLHName)
		FreeMem(StupidAssBCPLHName, 20);
*/
	if (ParmPkt)
		FreeMem(ParmPkt, PARMPKTSIZE);

	if (PartTable)
		FreeJanusMem(PartTable, sizeof(struct HDskPartition)*8);

	if (SS)
		CleanupJanusSig(SS);

	if (Signal != -1)
		FreeSignal(Signal);

	if (ExpansionBase)
		CloseLibrary(ExpansionBase);

	if (JanusBase)
		CloseLibrary(JanusBase);

	exit(ReturnCode);
}
