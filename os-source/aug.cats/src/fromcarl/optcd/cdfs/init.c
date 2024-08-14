/***********************************************************************
****************                                        ****************
****************        -=  CDTV FILE SYSTEM  =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"

IMPORT	char ModIdent[];
IMPORT	char DevName[];
IMPORT	POOL *MakePool();
IMPORT	struct Process *FindTask();

#define	BOOT_PRI -3
#define	PROC_STACK_SIZE	4096


/***********************************************************************
***
***  DosEnvec
***
***	KMY (2415)
***	Borrowed from dos/filehandler.h (V2.0)
***
***********************************************************************/


/* The disk "environment" is a longword array that describes the
 * disk geometry.  It is variable sized, with the length at the beginning.
 * Here are the constants for a standard geometry.
 */

struct DosEnvec {
    ULONG de_TableSize;	     /* Size of Environment vector */
    ULONG de_SizeBlock;	     /* in longwords: standard value is 128 */
    ULONG de_SecOrg;	     /* not used; must be 0 */
    ULONG de_Surfaces;	     /* # of heads (surfaces). drive specific */
    ULONG de_SectorPerBlock; /* not used; must be 1 */
    ULONG de_BlocksPerTrack; /* blocks per track. drive specific */
    ULONG de_Reserved;	     /* DOS reserved blocks at start of partition. */
    ULONG de_PreAlloc;	     /* DOS reserved blocks at end of partition */
    ULONG de_Interleave;     /* usually 0 */
    ULONG de_LowCyl;	     /* starting cylinder. typically 0 */
    ULONG de_HighCyl;	     /* max cylinder. drive specific */
    ULONG de_NumBuffers;     /* Initial # DOS of buffers.  */
    ULONG de_BufMemType;     /* type of mem to allocate for buffers */
    ULONG de_MaxTransfer;    /* Max number of bytes to transfer at a time */
    ULONG de_Mask;	     /* Address Mask to block out certain memory */
    LONG  de_BootPri;	     /* Boot priority for autoboot */
    ULONG de_DosType;	     /* ASCII (HEX) string showing filesystem type;
			      * 0X444F5300 is old filesystem,
			      * 0X444F5301 is fast file system */
    ULONG de_Baud;	     /* Baud rate for serial handler */
    ULONG de_Control;	     /* Control word for handler/filesystem */
    ULONG de_BootBlocks;     /* Number of blocks containing boot code */

};


/***********************************************************************
***
***  InitGlobals
***
***	This is where we init important global variables.
***	
***
***********************************************************************/
InitGlobals()	
{
	Debug1(ModIdent);

	/* Set default values (all others should be zero): */
	BlockSize    = SECT_SIZE;
	BlockShift   = SECT_SHIFT;
	CacheSize    = 8;
	DirCacheSize = 16;
	Retry        = 32;
	LockPoolSize = 40;
	FilePoolSize = 16;
	CharSet      = UpperChars;
	CopyStr(DeviceName,DevName,31);
	ScratchBlock = MakeVector(SECT_SIZE);

	/* ewhac/KMY (2707/2710) */
	SetupFSE();

	/* Check for Debugging bookmark...  V23.4 added */
	if (!OpenDevice("bookmark.device",0x1fffe,&DevIOReq,0))
	{
		DoDevIO(CD_READ,0,4,&DebugLevel); /* nop if no bookmark */
		CloseDevice(&DevIOReq);
	}
}

/***********************************************************************
***
***  ValidDisk
***
***	This code is executed from the STRAP module via a lib call.
***
***	Check that a disk has been inserted, and that it is bootable.
***	If so, get its options and load lowlevel bootstrap.
***	Setup Boot node for later boostrap.
***
***	Return TRUE if a bootable disk has been found.
***
***********************************************************************/
ValidDisk()	
{
	Debug1("\tV\n");

	if (BootPVDLSN) return TRUE;	/* Already found */

	MUST(InitBootIO());		/* Special IO	*/

	if (!FindPVD()) return FALSE;	/* Find the PVD	*/

	if (BootPVDLSN)			/* Check options */
	{
		SetOptions(ScratchBlock);
#ifdef NDEF
		BootOptions();
		if (BootFile[0]) StopBootProcess();  /* will stop boot after DOS boots */
		/* Put on mountlist unless already there. */
		if (CDFSBoot.bn_Node.ln_Succ == NULL)
			Enqueue(&ExpBase->MountList,&CDFSBoot);
#endif
	}

	QuitBootIO();			/* Cleanup IO	*/

	return BootPVDLSN;
}

/***********************************************************************
***
***  Initialize
***
***********************************************************************/
Init()
{
	IMPORT struct Process *FindTask();
	REG struct DosPacket *pkt;
	struct	RootNode *root;

	Debug2("\tB\n");

	/* Get file system process, port, DOSRoot, DOSInfo */
	FSProc = FindTask(NULL);
	FSPort = &(FSProc->pr_MsgPort);
	root = (struct RootNode *)DOSBase->dl_Root;
	DOSInfo = (struct DosInfo *)BTOA(root->rn_Info);

	/* Wait for startup packet, and init system */
 	WaitPacket();
	Debug2("\tS\n");

	InitFileSys();

	if (Mount(TRUE))
	{
		if (BootFile[0]) BootBoot();	
		PktRes1 = ID_DOS_DISK;
		PktRes2 = ATOB(ThisVol);
	}
	else
 	{
		PktRes1 = ID_NO_DISK_PRESENT;
		PktRes2 = 0;
	}

	ReplyPacket();
}

/***********************************************************************
***
***  Initialize File System
***
***********************************************************************/
InitFileSys()
{
	REG DOSPKT *pkt = Packet;
	REG struct DosEnvec *env;
	DEVLST *devList;
	struct FileSysStartupMsg *fsStart;

	/* Get startup packet args: */
	HandlerName = (BSTR)(pkt->dp_Arg1);
	fsStart = (struct FileSysStartupMsg *)BTOA(pkt->dp_Arg2);
	devList = (struct DeviceList *)BTOA(pkt->dp_Arg3);

	/* Get other startup information: */
	UnitNumber = fsStart->fssm_Unit;
	OpenFlags  = fsStart->fssm_Flags;
	EnvrVec = BTOA(fsStart->fssm_Environ);

	/* ewhac/KMY (2414) (set ZeroPoint on drive) */
	env = (struct DosEnvec *) EnvrVec;
	if ( env->de_TableSize >= DE_UPPERCYL )
		{
		REG LONG blockbytes;

		blockbytes = ( env->de_SizeBlock << 2 );
		ZeroPoint = ( blockbytes *
			      env->de_Surfaces *
			      env->de_BlocksPerTrack *
			      env->de_LowCyl ) +
			    ( blockbytes * env->de_Reserved );
		}

	CopyStr(DeviceName,(char*)BTOA(fsStart->fssm_Device)+1,31);

	/* ewhac/KMY (2414) (See if we're using cdtv.device)	*/
	/*	 KMY (2416) (ZeroPoint should be zero, driver	*/
	/*		     already has info)			*/
	/*	 KMY (2417) (UnitNumber should be zero)		*/
	/* this method is too simplistic */
	if ( MatchStr( DeviceName, DevName, 11 ) )
		{
		UsingCDTV  = TRUE;
		UnitNumber = 0;
		ZeroPoint  = 0;
		}

	if (!InitCDIO()) return DOS_FALSE;

	LockPool = MakePool(LockPoolSize,sizeof(CDLOCK));
	FilePool = MakePool(FilePoolSize,sizeof(FCNTRL));

	if (!MakeCache()) return DOS_FALSE;

	if (devList) devList->dl_Task = FSPort;
	return TRUE;
}

/***********************************************************************
***
***  BootBoot
***
***	Create a new process which will load a file and execute it
***	as a bootstrap.
***
***********************************************************************/
BootBoot()
{
	extern ULONG BootFileSeg;
	extern struct Process *CreateProc();

	MUST(BootProc=CreateProc(BootFile,0,
			(ULONG)(&BootFileSeg)>>2,
			PROC_STACK_SIZE));
}

/***********************************************************************
***
***  BootMain
***
***	Main code to be executed by new boot process.
***
***********************************************************************/
BootMain()
{
	REG char *e;
	struct Process *p = FindTask(NULL);	
	p->pr_ConsoleTask = NULL;	

	Wait(1);	/* Wait for the signal to go ahead */

	e = (char*)(LoadSeg(BootFile)<<2);

/*	Debug1("LoadSeg %s: %lx\n",BootFile,e);*/

	CallBoot(&e[4],0);
}
