/*
** $Source: HOG:Other/cd32mpeg/RCS/dev.c,v $
** $State: Exp $
** $Revision: 40.1 $
** $Date: 94/01/26 11:55:14 $
** $Author: kcd $
**
** Amiga MPEG device driver.
**
** Basic Device Functions
**
** (C) Copyright 1993 Commodore-Amiga, Inc.
**
*/

#include "mpeg_device.h"

#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dostags.h>
#include <devices/timer.h>
#include <libraries/configvars.h>
#include <devices/cd.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/timer_protos.h>
#include <clib/expansion_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/timer_pragmas.h>
#include <pragmas/expansion_pragmas.h>

#include <string.h>
#ifndef _GENPROTO
#include "dev_protos.h"
#include "cl450_protos.h"
#include "l64111_protos.h"
#include "demux_protos.h"
#include "datapump_protos.h"
#include "task_protos.h"
#endif
/*
** External variables and functions
**
*/

#if 0
#define D(x)	x
#else
#define D(x)
#endif

#if 0
#define DS(x)	x
#else
#define DS(x)
#endif

// #define DUMP_CL450 yeppers

#define NEW_UCODE yeppers

// #define NO_AUDIO	yeppers

#define CL450INTERRUPTS	(CL450INTF_PIC|CL450INTF_SCN|CL450INTF_BUF|CL450INTF_RDY)

extern VOID MPEGBoardInterrupt();
VOID DevTaskCEntry(VOID);

/*
** Device Init routine called from the Init
** vector in mpeg_device.asm.  Looks for known
** MPEG boards.
**
** a4 - Pointer to our device base
** a6 - Pointer to ExecBase
**
*/

void ASM DevInit(REG(a5) struct MPEGDevice *mpegDevice,
		 REG(a6) struct ExecBase *SBase)
{
    struct Library *ExpansionBase;
    struct ConfigDev *cdev=NULL;
    ULONG boardNum=0,boardType=MPEG_ZORRO2_ID;

    mpegDevice->md_TimerIO.tr_node.io_Message.mn_Length = sizeof(struct timerequest);

    if(!OpenDevice("timer.device",UNIT_ECLOCK,(struct IORequest *)&mpegDevice->md_TimerIO,0))
    {
        if(ExpansionBase = OpenLibrary("expansion.library",37L))
        {
            D(kprintf("DevInit: Looking for ZorroII boards.\n"));

            while(cdev = FindConfigDev(cdev,MPEG_MANUFACTURER,boardType))
            {
                D(kprintf("DevInit: Found ZorroII board @ $%lx\n",cdev->cd_BoardAddr));

                mpegDevice->md_Boards[boardNum] = cdev;
                mpegDevice->md_BoardType[boardNum] = boardType;
                boardNum++;
            }
            cdev = NULL;
            boardType = MPEG_CDGS_ID;

            D(kprintf("DevInit: Looking for CD32 boards.\n"));

            while(cdev = FindConfigDev(cdev,MPEG_MANUFACTURER,boardType))
            {
                D(kprintf("DevInit: Found CD32 board @ $%lx\n",cdev->cd_BoardAddr));

                mpegDevice->md_Boards[boardNum] = cdev;
                mpegDevice->md_BoardType[boardNum] = boardType;
                boardNum++;
            }
            CloseLibrary(ExpansionBase);
       }
   }
   D(kprintf("DevInit: Done.\n"));
}

/*
** Device Open vector
**
** a1 -	MPEG IO Request
** a6 -	Pointer	to our device base
** d0 -	Unit number
** d1 -	Flags
**
*/

ULONG ASM DevOpen(REG(a1) struct IOMPEGReq *iomr,
		  REG(d0) ULONG	mpg_unit,
		  REG(d1) ULONG	mpg_flags,
		  REG(a6) struct MPEGDevice *mpegDevice)
{
    struct MPEGUnit *mpegUnit;
    ULONG returncode;
    BOOL status	= FALSE;

    /* Note, we increase the Open count *BEFORE* Obtaining the semaphore.
       This is because another task could own the sempaphore, fail the
       open and then expunge the device before we can wake up. */

    mpegDevice->md_Device.lib_OpenCnt++;	/* So we won't expunge ourselves... */

    /* Make sure our open remains single-threaded. */

    ObtainSemaphore(&mpegDevice->md_Lock);	/* Enforce single threading since we need to
						   Wait() when starting	up the Unit process */

    if(mpg_unit < MD_MAX_UNIT)	/* Legal Unit? */
    {
	/* Initialize the Unit.  This will fail if the
	   unit is already in use or something went
	   wrong. */

	D(kprintf("DevOpen: Init Unit %ld\n",mpg_unit));

	if(mpegUnit = InitMPEGUnit(mpg_unit,mpegDevice))
	{

	    D(kprintf("DevOpen: Init okay\n"));

            /* Everything went okay. */
            status = TRUE;
            returncode = 0;

            mpegDevice->md_Device.lib_OpenCnt++;
            mpegDevice->md_Device.lib_Flags &=~LIBF_DELEXP;

            mpegUnit->mu_Unit.unit_OpenCnt++;

            /* Fix up the initial io request */
            iomr->iomr_Req.io_Error = 0;
            iomr->iomr_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
            iomr->iomr_Req.io_Unit = (struct Unit *)mpegUnit;
            iomr->iomr_Req.io_Device = (struct Device *)mpegDevice;
        }
    }

    /* See if something	went wrong. */
    if(!status)
    {
    	D(kprintf("DevOpen: Init failed.\n"));

	iomr->iomr_Req.io_Error	= IOERR_OPENFAIL;
	iomr->iomr_Req.io_Unit = (struct Unit *) -1;
	iomr->iomr_Req.io_Device = (struct Device *) -1;
	returncode = IOERR_OPENFAIL;
    }
    ReleaseSemaphore(&mpegDevice->md_Lock);
    mpegDevice->md_Device.lib_OpenCnt--;

    return(returncode);
}

/*
** Device Close	vector.
**
** a1 -	IOReq
** a6 -	Device Pointer
**
*/

BPTR ASM DevClose(REG(a1) struct IOMPEGReq *iomr,
		  REG(a6) struct MPEGDevice *mpegDevice)
{
    struct MPEGUnit *mpegUnit;
    BPTR seglist = 0L;

    ObtainSemaphore(&mpegDevice->md_Lock);

    mpegUnit = (struct MPEGUnit *)iomr->iomr_Req.io_Unit;

    /* Trash the io_Device and io_Unit fields so that any attempt to use this
       request will die	immediatly. */

    iomr->iomr_Req.io_Device = (struct Device *) -1;
    iomr->iomr_Req.io_Unit = (struct Unit *) -1;

    /* I always	shut the unit process down if the open count drops to zero.
       That way, if I need to expunge, I never have to call Wait(). */

    mpegUnit->mu_Unit.unit_OpenCnt--;
    if(!mpegUnit->mu_Unit.unit_OpenCnt)
    {
	ExpungeUnit(mpegUnit,mpegDevice);
    }

    mpegDevice->md_Device.lib_OpenCnt--;

    /* Check to	see if we've been asked to expunge. */
    if(mpegDevice->md_Device.lib_Flags & LIBF_DELEXP)
	seglist	= DevExpunge(mpegDevice);

    ReleaseSemaphore(&mpegDevice->md_Lock);

    return(seglist);
}

/*
** Device Expunge vector
**
** a6 -	Device base
**
** Note: You may NEVER EVER Wait() in expunge.
**	 Don't even *think* about it.
*/

BPTR ASM DevExpunge(REG(a6) struct MPEGDevice *mpegDevice)
{
    BPTR seglist;
    ULONG devbase;
    LONG devbasesize;

    D(kprintf("Expunging Device\n"));

    if(mpegDevice->md_Device.lib_OpenCnt)
    {
	/* Sorry, we're busy.  We'll expunge later on
	   if we can. */
	mpegDevice->md_Device.lib_Flags |= LIBF_DELEXP;
	seglist	= (BPTR)0L;
    }
    else
    {
        CloseDevice((struct IORequest *)&mpegDevice->md_TimerIO);

	/* Free	up our library base and	function table after
	   removing ourselves from the library list. */
	Remove((struct Node *)mpegDevice);
	seglist	= mpegDevice->md_SegList;

	devbase	= (ULONG) mpegDevice;

	devbasesize = (ULONG)mpegDevice->md_Device.lib_NegSize;
	devbase	= devbase - devbasesize;

	devbasesize += (ULONG)mpegDevice->md_Device.lib_PosSize;

	FreeMem((APTR)devbase,devbasesize);
    }
    return(seglist);
}

/*
**
** BeginIO
**
** This	is the dispatch	point for the driver's incoming IORequests.
**
** Registers at entry:
**
** a1 = IORequest
** a6 = Device Base
**
*/

#define	MPEG_IMMEDIATES	0L

VOID ASM DevBeginIO(REG(a1) struct IOMPEGReq *iomr,
		    REG(a6) struct MPEGDevice *mpegDevice)
{
    struct MPEGUnit *mpegUnit;
    iomr->iomr_Req.io_Error = 0;
    iomr->iomr_Req.io_Message.mn_Node.ln_Type =	NT_MESSAGE;

    if(iomr->iomr_Req.io_Command < MPEGCMD_END)
    {
	if((1L << iomr->iomr_Req.io_Command) & MPEG_IMMEDIATES)
	{
#ifndef _GENPROTO
	    PerformIO(iomr);
#else
		;
#endif
	}
	else
	{
    	    mpegUnit = (struct MPEGUnit *)iomr->iomr_Req.io_Unit;

	    iomr->iomr_Req.io_Flags &= ~IOF_QUICK;

            ObtainSemaphore(&mpegUnit->mu_Lock);

	    PutMsg((struct MsgPort *)iomr->iomr_Req.io_Unit,(struct Message *)iomr);

	    ReleaseSemaphore(&mpegUnit->mu_Lock);

	}
    }
    else
    {
	iomr->iomr_Req.io_Error	= IOERR_NOCMD;
	TermIO(iomr,mpegDevice);
    }
}

/* CreateTask code stolen from amiga.lib */

/*
 *  Create a task with given name, priority, and stack size.
 *  It will use the default exception and trap handlers for now.
 */
/* the template for the mementries.  Unfortunately, this is hard to
 * do from C: mementries have unions, and they cannot be statically
 * initialized...
 *
 * In the interest of simplicity I recreate the mem entry structures
 * here with appropriate sizes.  We will copy this to a local
 * variable and set the stack size to what the user specified,
 * then attempt to actually allocate the memory.
 */

#define ME_TASK 	0
#define ME_STACK	1
#define NUMENTRIES	2

struct FakeMemEntry
{
    ULONG fme_Reqs;
    ULONG fme_Length;
};

struct FakeMemList
{
    struct Node fml_Node;
    UWORD	fml_NumEntries;
    struct FakeMemEntry fml_ME[NUMENTRIES];
} TaskMemTemplate = {
    { 0 },						/* Node */
    NUMENTRIES, 					/* num entries */
    {							/* actual entries: */
	{ MEMF_PUBLIC | MEMF_CLEAR, sizeof( struct Task ) },    /* task */
	{ MEMF_CLEAR,	0 }					/* stack */
    }
};

struct Task *IntCreateTask( name, pri, initPC, stackSize, mpegDevice )
    char *name;
    UBYTE pri;
    APTR  initPC;
    ULONG stackSize;
    struct MPEGDevice *mpegDevice;
{
    struct Task *newTask;
    struct FakeMemList fakememlist;
    struct MemList *ml;
ULONG result;

    /* round the stack up to longwords... */
    stackSize = (stackSize +3) & ~3;

    /*
     * This will allocate two chunks of memory: task of PUBLIC
     * and stack of PRIVATE
     */
    fakememlist = TaskMemTemplate;
    fakememlist.fml_ME[ME_STACK].fme_Length = stackSize;

    ml = (struct MemList *) AllocEntry( (struct MemList *)&fakememlist );

    if( ! ml ) {
	return( NULL );
    }

    /* set the stack accounting stuff */
    newTask = (struct Task *) ml->ml_ME[ME_TASK].me_Addr;

    newTask->tc_SPLower = ml->ml_ME[ME_STACK].me_Addr;
    newTask->tc_SPUpper = (APTR)((ULONG)(newTask->tc_SPLower) + stackSize);
    newTask->tc_SPReg = newTask->tc_SPUpper;

    /* misc task data structures */
    newTask->tc_Node.ln_Type = NT_TASK;
    newTask->tc_Node.ln_Pri = pri;
    newTask->tc_Node.ln_Name = name;

    /* add it to the tasks memory list */
    NewList( &newTask->tc_MemEntry );
    AddHead( &newTask->tc_MemEntry, (struct Node *)ml );

    /* add the task to the system -- use the default final PC */

    result = (ULONG)AddTask( newTask, initPC, 0 );
    if ((((struct ExecBase *)SysBase)->LibNode.lib_Version >= 37) && (result == 0))
    {
        FreeEntry(ml);
        return(NULL);
    }
    return( newTask );
}

/*
** InitMPEGUnit
**
** Initialize (if needed) a new	MPEG device Unit and process.
**
*/

struct MPEGUnit *InitMPEGUnit(ULONG mpg_unit, struct MPEGDevice *mpegDevice)
{
    struct MPEGUnit *mpegUnit;
    struct ConfigDev *cfg;
    ULONG boardAddr;

    /* Check to	see if the Unit	is already up and running.  If
       it is, fail as we only allow exclusive access. */

    if(!mpegDevice->md_Units[mpg_unit])
    {
        /* Make sure a board exists for the Unit they'd like to
           use. */

        D(kprintf("InitUnit: Unit %ld not running, starting...\n",mpg_unit));

        if(cfg = mpegDevice->md_Boards[mpg_unit])
        {
            boardAddr = (ULONG)cfg->cd_BoardAddr;

            D(kprintf("InitUnit: Board located at $%lx\n",boardAddr));

            /* A board exists, so let's allocate a new unit structure and
               set it up. */

            /* Allocate a new Unit structure */
            if(mpegUnit = AllocVec(sizeof(struct MPEGUnit), MEMF_CLEAR|MEMF_PUBLIC))
            {
                D(kprintf("InitUnit: New Unit Struct Allocated at $%lx\n",mpegUnit));

                /* Do some initialization on the Unit structure */

                NewList(&mpegUnit->mu_Unit.unit_MsgPort.mp_MsgList);
                NewList((struct List *)&mpegUnit->mu_VideoStream);
                NewList((struct List *)&mpegUnit->mu_AudioStream);
                NewList((struct List *)&mpegUnit->mu_CMDQueue);
                NewList((struct List *)&mpegUnit->mu_PlayQueue);
                NewList((struct List *)&mpegUnit->mu_CDIOList);
                NewList((struct List *)&mpegUnit->mu_SeqHeadQueue);
                InitSemaphore(&mpegUnit->mu_Lock);

                mpegUnit->mu_Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
                mpegUnit->mu_Unit.unit_MsgPort.mp_Flags = PA_IGNORE;
                mpegUnit->mu_Unit.unit_MsgPort.mp_Node.ln_Name = "cd32mpeg.device";

                mpegUnit->mu_UnitNum = mpg_unit;
                mpegUnit->mu_Device = mpegDevice;
                mpegUnit->mu_ProductID = mpegDevice->md_BoardType[mpg_unit];
                mpegUnit->mu_VEnable = FALSE;
                mpegUnit->mu_AEnable = FALSE;
		mpegUnit->mu_AudioID = 0;

                /* Set up appropriate board pointers depending on the
                   board type. */

                switch(mpegDevice->md_BoardType[mpg_unit])
                {

                    case MPEG_ZORRO2_ID:    mpegUnit->mu_CL450Fifo = (UWORD *)((ULONG)boardAddr + 0x380000);
                                            mpegUnit->mu_CL450 = (CL450Regs *)((ULONG)boardAddr + 0x300000);
                                            mpegUnit->mu_L64111 = (L64111Regs *)((ULONG)boardAddr + 0x100000);
                                            mpegUnit->mu_Control = (UWORD *)((ULONG)boardAddr);
                                            mpegUnit->mu_CL450Dram = (UWORD *)((ULONG)boardAddr + 0x200000);
                                            mpegUnit->mu_CL450IntMask = 0x80;
                                            mpegUnit->mu_L64111IntMask = 0x40;
                                            mpegUnit->mu_CFLevelBit = 0x0008;
                                            mpegUnit->mu_ControlVal = Z2_DACINIT | Z2_LSTARTI;
                                            break;

#ifndef OLD_HARDWARE
                    case MPEG_CDGS_ID:      mpegUnit->mu_CL450Fifo = (UWORD *)((ULONG)boardAddr   + 0x60000);
                                            mpegUnit->mu_CL450 = (CL450Regs *)((ULONG)boardAddr   + 0x70000);
                                            mpegUnit->mu_L64111 = (L64111Regs *)((ULONG)boardAddr + 0x50000);
                                            mpegUnit->mu_Control = (UWORD *)((ULONG)boardAddr     + 0x40000);
                                            mpegUnit->mu_CL450Dram = (UWORD *)((ULONG)boardAddr   + 0x80000);
                                            mpegUnit->mu_CL450IntMask =  0x8000;
                                            mpegUnit->mu_L64111IntMask = 0x4000;
                                            mpegUnit->mu_CFLevelBit = 0x0800;
                                            mpegUnit->mu_ControlVal = CD32_CL450_RESET | CD32_MAP_UMPEG;
                                            break;
#else
                    case MPEG_CDGS_ID:      mpegUnit->mu_CL450Fifo = (UWORD *)((ULONG)boardAddr   + 0x50000);
                                            mpegUnit->mu_CL450 = (CL450Regs *)((ULONG)boardAddr   + 0x60000);
                                            mpegUnit->mu_L64111 = (L64111Regs *)((ULONG)boardAddr + 0x20000);
                                            mpegUnit->mu_Control = (UWORD *)((ULONG)boardAddr     + 0x40000);
                                            mpegUnit->mu_CL450Dram = (UWORD *)((ULONG)boardAddr   + 0x80000);
                                            mpegUnit->mu_CL450IntMask =  0x8000;
                                            mpegUnit->mu_L64111IntMask = 0x4000;
                                            mpegUnit->mu_CFLevelBit = 0x0800;
                                            mpegUnit->mu_ControlVal = CD32_CL450_RESET;
                                            break;
#endif
                }

                /* Initialize Control Register */

		SetBlankMode(mpegUnit);

                /* Let CL450 Come out of reset */
                {
                     int i;
                     volatile UBYTE delay;

                     for(i = 0; i < 1000; i++)
                     	delay = *(volatile UBYTE *) 0xbfe001;
                }

                /* Initialize the CL450 (Load Microcode and start the CPU) */

                mpegUnit->mu_Interrupt.is_Data = mpegUnit;
                mpegUnit->mu_Interrupt.is_Code = (VOID (*)())MPEGBoardInterrupt;
                mpegUnit->mu_SysBase = SysBase;

                AddIntServer(3L,&mpegUnit->mu_Interrupt);

                D(kprintf("InitUnit: Initializing CL450 Microcode\n"));

                if(!InitCL450(mpegUnit))
                {
                    struct Task *task;

                    D(kprintf("InitUnit: CL450 Microcode loaded okay\n"));

                    InitL64111(mpegUnit);

                    D(kprintf("InitUnit: Starting Unit Task\n"));

                    if(task = IntCreateTask("cd32mpeg.device",MPEG_PRI,DevTaskCEntry,4096,mpegDevice))
                    {
                        D(kprintf("InitUnit: Unit task at $%lx\n",task));
                        task->tc_UserData = mpegUnit;
                        mpegUnit->mu_Task = FindTask(0L);
                        SetSignal(0L,SIGF_SINGLE);
                        Signal(task,SIGF_SINGLE);
                        Wait(SIGF_SINGLE);
                    }
                }
                if(!mpegUnit->mu_Task)
                {
                    D(kprintf("InitUnit: Unit process couldn't start.\n"));

                    /* The Unit process couldn't start for some reason, so free the Unit structure. */

                    /* Reset both decoders so they will keep quiet. */

                    ResetCL450(mpegUnit);
                    ResetL64111(mpegUnit);

                    RemIntServer(3L,&mpegUnit->mu_Interrupt);

                    FreeVec(mpegUnit);
                }
                else
                {
                    D(kprintf("InitUnit: Unit startup okay.\n"));

                    /* Set up the Unit structure pointer in the device base */
                    mpegDevice->md_Units[mpg_unit] = (struct Unit *)mpegUnit;
                }
            }
        }
        return((struct MPEGUnit *)mpegDevice->md_Units[mpg_unit]);
    }
    else
        return((struct MPEGUnit *)NULL);
}

/*
** The device AbortIO()	entry point.
**
** A1 -	The IO request to be aborted.
** a6 -	The device base.
*/
ULONG ASM DevAbortIO(REG(a1) struct IOMPEGReq *iomr,
		     REG(a6) struct MPEGDevice *mpegDevice)
{
    ULONG result = 0L;
    struct IOMPEGReq *req;
    struct MPEGUnit *mpegUnit = (struct MPEGUnit *)iomr->iomr_Req.io_Unit;

    ObtainSemaphore(&mpegUnit->mu_Lock);
    if(iomr->iomr_Req.io_Message.mn_Node.ln_Type != NT_REPLYMSG)
    {
    	/* Look in any of four places for the request */
	if(iomr == mpegUnit->mu_ScanIO)
	{
	    iomr->iomr_Req.io_Error = IOERR_ABORTED;
	    TermIO(iomr,mpegDevice);
	    mpegUnit->mu_ScanIO = NULL;

            /* Check for another pending MPEGCMD_SCAN request and kick it off. */
            if(req = (struct IOMPEGReq *)RemHead((struct List *)&mpegUnit->mu_CMDQueue))
                DevBeginIO(req,mpegDevice);

	}
	else if(iomr == mpegUnit->mu_CurrentPlayCmd)
	{
	    DS(kprintf("[AbortPlay]"));
	    Signal(mpegUnit->mu_Task,mpegUnit->mu_AbortSigMask);
	}
    	else if(!(result = AbortReq((struct MinList *)&mpegUnit->mu_Unit.unit_MsgPort.mp_MsgList,iomr,mpegDevice)))
	{
	    ;
	}
    	else if(!(result = AbortReq(&mpegUnit->mu_VideoStream,iomr,mpegDevice)))
    	{
    	    mpegUnit->mu_VPackets--;
    	}
    	else if(!(result = AbortReq(&mpegUnit->mu_AudioStream,iomr,mpegDevice)))
    	{
    	    mpegUnit->mu_APackets--;
    	}
    	else if(!(result = AbortReq(&mpegUnit->mu_PlayQueue,iomr,mpegDevice)))
    	{
    	    ;
    	}
    	else if(!(result = AbortReq(&mpegUnit->mu_SeqHeadQueue,iomr,mpegDevice)))
    	{
    	    ;
    	}
    	else result = AbortReq(&mpegUnit->mu_SeqHeadQueue,iomr,mpegDevice);

    }

    ReleaseSemaphore(&mpegUnit->mu_Lock);
    return(result);
}

/*
** This	funcion	is used	to locate an IO	request	in a linked
** list	and abort it if	found.
*/
ULONG ASM AbortReq(REG(a2) struct MinList *minlist,
	       	   REG(a3) struct IOMPEGReq *iomr,
	           REG(a6) struct MPEGDevice *mpegDevice)
{
    struct Node	*node, *next;

    node = (struct Node	*)minlist->mlh_Head;

    while(node->ln_Succ)
    {
	next = node->ln_Succ;

	if(node	== (struct Node	*)iomr)
	{
	    Remove((struct Node	*)iomr);
	    iomr->iomr_Req.io_Error = IOERR_ABORTED;
	    TermIO(iomr,mpegDevice);
	    return(0);
	}
	node = next;
    }
    return(IOERR_NOCMD);
}

/*
** This	function is used to return an IO request
** back	to the sender.
*/
VOID ASM TermIO(REG(a1) struct IOMPEGReq *iomr,
	        REG(a6) struct MPEGDevice *mpegDevice)
{
    if(!(iomr->iomr_Req.io_Flags & IOF_QUICK))
	ReplyMsg((struct Message *)iomr);
}

/****** cd32mpeg.device/SetSCR ***********************************************
*
*   NAME
*       SetSCR -- Set the current time
*
*   SYNOPSIS
*       success = SetSCR(unitPtr, clockValue)
*       D0               D0
*
*       BOOL SetSCR(struct Unit *unit, ULONG clockValue);
*
*   FUNCTION
*       This call is used to set the MPEG System Clock Reference for a MPEG
*       device driver.  This is usually used for synchronizing one device
*       driver to another.
*
*   INPUTS
*	unit	     - The value in the io_Unit field of the IO request
*		       returned by OpenDevice().
*       clockValue   - The lower 32 bits of the SCR.  The MPEG clock is
*                      specified to be 33 bits, but we only use 32 bits of
*                      that clock.
*
*   OUTPUTS
*	success	     - Whether or not the clock was set.
*
*   BUGS
*	The current hardware has a limitation in that you can only use
*	this function to synchronize a raw video stream to another source.
*
*****************************************************************************/
BOOL ASM DevSetSCR(REG(a0) struct MPEGUnit *mpegUnit,
		   REG(d0) ULONG clockVal,
		   REG(a6) struct MPEGDevice *mpegDevice)
{
    if(SetSCR((clockVal >> 30) & 0x3,
    	   (clockVal >> 15) & 0x7fff,
    	   (clockVal & 0x7fff),mpegUnit))
    {
        Disable();
        mpegUnit->mu_PlayFlags |= MPEGPF_SYNC;
//        mpegUnit->mu_PlayFlags &= ~MPEGPF_INTMUTE;
        Enable();
        return(1);
    }
    else
    	return(0);
}

/****** cd32mpeg.device/GetSCR ***********************************************
*
*   NAME
*       GetSCR -- Get the current System Clock Reference value.
*
*   SYNOPSIS
*       clockValue = GetSCR()
*       D0
*
*       ULONG GetSCR(struct Unit *unit);
*
*   FUNCTION
*       This call is used to get the MPEG System Clock Reference from a MPEG
*       device driver.  This is usually used for synchronizing one device
*       driver to another.
*
*   INPUTS
*	unit	     - The value in the io_Unit field of the IO request
*		       returned by OpenDevice.
*
*   OUTPUTS
*       clockValue   - The lower 32 bits of the SCR.  The MPEG clock is
*                      specified to be 33 bits, but we only use 32 bits of
*                      that clock.
*
*****************************************************************************/
ULONG ASM DevGetSCR(REG(a0) struct MPEGUnit *mpegUnit)
{
    UWORD hi, mid, low;
    ULONG pts;
    hi = mid = low = 0;

    if(mpegUnit->mu_PlayFlags & MPEGPF_SYNC)
    {
        GetSCR(&hi,&mid,&low,mpegUnit);

        pts = (hi<<30) | (mid<<15) | low;
    }
    else pts = 0xffffffff;

    return(pts);
}

/*
** Ugh.  This function is a nasty kludge.
**
*/
ULONG ASM DevGetSector(REG(a0) struct MPEGUnit *mpegUnit)
{
    UWORD hi, mid, low;
    ULONG pts, begin;
    hi = mid = low = 0;

    if(mpegUnit->mu_PlayFlags & MPEGPF_SYNC)
    {
        GetSCR(&hi,&mid,&low,mpegUnit);

        pts = (hi<<30) | (mid<<15) | low;

        /* Okay.  Now divide the SCR by 1200 */

        pts = pts / 1200;	/* Sectors into the stream */

        /* Calculate beginning sector number */

        begin = mpegUnit->mu_StreamBegin / mpegUnit->mu_SectorSize;

        /* Return current sector number */

        return(begin + pts);
    }
    else
    {
        /* Ugh.  Give back something reasonable */
        begin = (mpegUnit->mu_CDOffset / mpegUnit->mu_SectorSize) - 10;
        return(begin);
    }
}

/*
** ExpungeUnit
**
** Tells a unit	process	to go away...
**
** This	function is called from	the DevClose routine when the open count for a
** unit	reaches	zero.  This routine signals the	unit process to	exit and then
** waits for the unit process to acknowledge.  The unit	structure is then
** freed.
*/

VOID ExpungeUnit(struct MPEGUnit *mpegUnit, struct MPEGDevice *mpegDevice)
{
    struct Task	*unittask;

    D(kprintf("Expunging Unit %ld\n",mpegUnit->mu_UnitNum));

    unittask = mpegUnit->mu_Task;

    mpegUnit->mu_Task = FindTask(0L);

    SetSignal(0L,SIGF_SINGLE);

    D(kprintf("Signalling unit task to exit.\n"));
    Signal(unittask,SIGBREAKF_CTRL_F);

    D(kprintf("Waiting for unit task to exit.\n"));
    Wait(SIGF_SINGLE);

    D(kprintf("Unit task dead.\n"));

    mpegDevice->md_Units[mpegUnit->mu_UnitNum] = NULL;

    RemIntServer(3L,&mpegUnit->mu_Interrupt);

    FreeVec(mpegUnit);
}


