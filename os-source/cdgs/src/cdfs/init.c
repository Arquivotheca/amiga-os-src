/***********************************************************************
****************                                        ****************
****************        -=   CD FILE SYSTEM   =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"

IMPORT  char ModIdent[];
IMPORT  char DevName[];
IMPORT  POOL *MakePool();
IMPORT  struct Process *FindTask();
IMPORT  void    Quick_SigTask();
IMPORT  PVD  *FindPVD();

#define BOOT_PRI -3
#define PROC_STACK_SIZE 4096

#define D(x)    ;
#define DD(x)   ;

/***********************************************************************
***
***  InitGlobals
***
***     This is where we init important global variables.
***     
***
***********************************************************************/
InitGlobals(LONG is_mount)   
{
	/* is_mount is true if we're being mounted as opposed to being booted */
        Debug1(ModIdent);

        /* Set default values (all others should be zero): */
        BlockSize    = SECT_SIZE;
        BlockShift   = SECT_SHIFT;
	IsMount	     = is_mount;
        CacheSize    = 2;
        DirCacheSize = 8;
        Retry        = 32;
        LockPoolSize = 40;
        FilePoolSize = 16;
        CharSet      = UpperChars;
        CopyStr(DeviceName,DevName,31);

// FIX! remove
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
***     This code is executed from the STRAP module via a lib call.
***
***     Check that a disk has been inserted, and that it is bootable.
***     If so, get its options and load lowlevel bootstrap.
***     Setup Boot node for later boostrap.
***
***     Return TRUE if a bootable disk has been found.
***
***********************************************************************/
ValidDisk()     
{
        D(Debug0("ValidDisk\n"));
        Debug1("\tV\n");

        if (BootPVDLSN) return TRUE;    /* Already found */

        MUST(InitBootIO());             /* Special IO   */

        if (PVDBuffer)                  /* if lying around from prev call */
                FreeVector(PVDBuffer);

        if (!(PVDBuffer = FindPVD())) return FALSE;     /* Find the PVD */

        if (BootPVDLSN)                 /* Check options */
        {
                SetOptions(PVDBuffer);
        }

        QuitBootIO();                   /* Cleanup IO   */
        /* leave PVDBuffer around for Mount to free */

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
        struct  RootNode *root;

        Debug2("\tB\n");

        /* Get file system process, port, DOSRoot, DOSInfo */
        FSProc = FindTask(NULL);
        FSPort = &(FSProc->pr_MsgPort);
        root = (struct RootNode *)DOSBase->dl_Root;
        DOSInfo = (struct DosInfo *)BTOA(root->rn_Info);

        /* Wait for startup packet, and init system */
        WaitPacket();
        Debug2("\tS\n");

	/* InitFileSys sets dol_Task if it succeeds */
        if (!InitFileSys())	/* this can fail if there's no device */
	{
		PktRes1 = DOS_FALSE;
		PktRes2 = 0;
		ReplyPacket();	/* return startup packet with failure */

		return FALSE;
	}

        if (Mount(TRUE))
        {
                if (!IsMount && BootFile[0]) BootBoot();    
                PktRes1 = ID_DOS_DISK;
                PktRes2 = ATOB(ThisVol);
        }
        else
        {
                PktRes1 = ID_NO_DISK_PRESENT;
                PktRes2 = 0;
        }

        ReplyPacket();
 
        /* Modify process message port to use the "secret" mode=3
        ** port action -- direct jump
        */
        FSProc->pr_Task.tc_UserData  =(void *)FetchA4();
        FSProc->pr_MsgPort.mp_SigTask=(struct Task *)Quick_SigTask;
        FSProc->pr_MsgPort.mp_Flags |=PF_ACTION;

        D(Debug0("Have startup packet, will travel. SigTask=%lx/A4=%lx\n",FSProc->pr_MsgPort.mp_SigTask,FetchA4()));

	return TRUE;
}

/***********************************************************************
***
***  Initialize File System
***
***********************************************************************/
InitFileSys()
{
        REG DOSPKT *pkt = Packet;
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
        CopyStr(DeviceName,(char*)BTOA(fsStart->fssm_Device)+1,31);

        if (!InitCDIO()) return DOS_FALSE;

        LockPool = MakePool(LockPoolSize,sizeof(CDLOCK));
        FilePool = MakePool(FilePoolSize,sizeof(FCNTRL));

/* fix! free stuff on fail! */
        if (!MakeCache()) return DOS_FALSE;

        if (devList) devList->dl_Task = FSPort;
        return TRUE;
}

/***********************************************************************
***
***  BootBoot
***
***     Create a new process which will load a file and execute it
***     as a bootstrap.
***
***********************************************************************/
BootBoot()
{
        extern ULONG BootFileSeg;
        extern struct Process *CreateProc();

/* FIX!!!!!!! should we do this if we're not booting from this disk????? */

        MUST(BootProc=CreateProc(BootFile,0,
                        (ULONG)(&BootFileSeg)>>2,
                        PROC_STACK_SIZE));
}

/***********************************************************************
***
***  BootMain
***
***     Main code to be executed by new boot process.
***
***********************************************************************/
BootMain()
{
        REG char *e;
        struct Process *p = FindTask(NULL);     
        p->pr_ConsoleTask = NULL;       

/* FIX!!!!! EVIL!!! */
        Wait(1);        /* Wait for the signal to go ahead */

        e = (char*)(LoadSeg(BootFile)<<2);

/*      Debug1("LoadSeg %s: %lx\n",BootFile,e);*/

        CallBoot(&e[4],0);
}
