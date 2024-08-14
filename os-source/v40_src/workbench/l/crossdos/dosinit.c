/*** DOSinit.c  ************************************************************
** Copyright 1991 CONSULTRON
*
*       DOS initialization subroutines.
*
****************************************************************************/

#include "FS:FS.h"
#include "intuition/intuition.h"

/******
* STATIC data references
*******/
struct IOStdReq *diskreq;
struct MsgPort *diskport;
struct IntuitionBase *IntuitionBase;
struct Library          *UtilityBase;

/******
* External data references
*******/
extern struct ExecBase  *SysBase;
extern struct DosLibrary *DOSBase;
extern void diskchange_int();
extern struct FS *fsys;
extern struct MsgPort      *DOSPort;
extern struct Process      *Proc;

#define NUM_SP_DEVICE  2
#define NUM_MFM_DEVICE 0
static const char *devices[] = 
{
    "mfm.device",
    "filedisk.device",
    "carddisk.device",
    "trackdisk.device",
};

static void RemBuffers(void);


/*******
* InitDev() -- Initialize disk device.   Assemble necessary parameters.
*   Return  if error occurred.
*******/
int InitDev(fssm)
register struct FSSM *fssm;
{
F();
    int error=TRUE, i, SizeBlock;
    STRPTR devdrvr;
    register struct FS   *fs = fsys;
    register struct DCIReq  *dci;
    register struct TMRReq  *timer;
    register struct DosEnvec *fssm_Env = (struct DosEnvec *)BADDR(fssm->fssm_Environ);
    register UBYTE mfm_flag=FALSE;
#ifdef V33
    char cstr[32];
#endif


    fs->f_bytes_block = SizeBlock = (fssm_Env->de_SizeBlock)<<2; /* set default block size */
    for(i=1,SizeBlock=fssm_Env->de_SizeBlock; SizeBlock!=0; i++, SizeBlock>>=1);
    fs->f_bytes_block_sh = i; /* set block size shift value */

/* Calculate the maximum blocks per transfer value for those 'lame' device drivers
that do not know their own limitations. */
    fs->f_maxblocks_transfer = fssm_Env->de_MaxTransfer>>fs->f_bytes_block_sh;
    fs->f_mem_mask = fssm_Env->de_Mask;

    fs->f_beg_disk = fssm_Env->de_DiskLowBlock = fssm_Env->de_LowCyl * fssm_Env->de_BlocksPerTrack * fssm_Env->de_Surfaces;

/****
    Allocate 'scratch' memory 
****/
    if( !(fs->f_scratch = (UBYTE *)(AllocMem(
        (fs->f_scratch_sz = fs->f_bytes_block), fssm_Env->de_BufMemType) )) )
    {
        fs->f_scratch_sz = 0;
        return(FALSE);
    }

/****
 Open Libraries needed for later use
****/
    if( !(IntuitionBase = (struct IntuitionBase *)
        OpenLibrary("intuition.library", LIB_VERS)) ) return(FALSE);

#ifndef V33
    if( !(UtilityBase = (struct Library *)
        OpenLibrary("utility.library", LIB_VERS)) ) return(FALSE);
#endif

/****
 Allocate resources for DiskIO
****/
    if( !(diskport = CreateMsgPort()))  return(FALSE);   /* error in createport */

    if( !(diskreq = (struct IOStdReq *)CreateIORequest(diskport,sizeof(struct IOStdReq )))) return(FALSE); 

        /* open the device for access */
    devdrvr = &(((STRPTR)BADDR(fssm->fssm_Device))[1]);

DPRTF(KPrintF("\nInitDev: devdrvr=%s",devdrvr));

/* Using the utility.library equivalent to stricmp() because with localization
it should handle case for international characters properly */
    for(i=(sizeof(devices)/sizeof(*devices))-1;
        (i >= 0);
        i--)
    {
        if(0 == stricmp(devdrvr,devices[i]))
        {
            fs->f_FS_special_flags |= MF_GG_DEVICE;
                /* inserted for special product disk kludge */
            if((i < NUM_SP_DEVICE)
                && ((fssm_Env->de_DosType == ID_MSDOS_DISK)
                ||  (fssm_Env->de_DosType == ID_MSDOS_DISK_DS)))
            {
                if(i == NUM_MFM_DEVICE) mfm_flag = TRUE;
                fs->f_FS_special_flags |= MF_SP_DEVICE;
            }
            break;
        }
    }

#ifdef V33
    /**** This code has been added to find the device driver on the product disk if
    ****   not found in the resident device drivers list */
    if(mfm_flag==TRUE)
    {
        sprintf(cstr,":devs/%s",devdrvr);
DPRTF(KPrintF("\nInitDev: 1  cstr=%s",cstr));
        error = OpenDevice( cstr,fssm->fssm_Unit,(struct IORequest *)diskreq, fssm->fssm_Flags);
    }
#endif
DPRTF(KPrintF("\nInitDev: 1  devdrvr=%s",devdrvr));
    if(error && OpenDevice( devdrvr,fssm->fssm_Unit,(struct IORequest *)diskreq, fssm->fssm_Flags))
    {
DPRTF(KPrintF("\nInitDev: Cannot open device"));
        diskreq->io_Device = 0;
        return(FALSE);
    }

    /* calculate the last block # of the MS-DOS "disk" and store it in de_DiskHighBlock. */
    fs->f_end_disk = fssm_Env->de_DiskHighBlock = ((fssm_Env->de_HighCyl+1)
                        * fssm_Env->de_Surfaces
                        * fssm_Env->de_BlocksPerTrack)-1;

/* Set the physical parameters of the disk as originally specified */
/* This 'kludge' is here to fix a problem with de_DosType = ID_MSDOS_DISK_DS */
    if(fs->f_FS_special_flags & MF_SP_DEVICE)
    {
          diskreq->io_Data = (APTR) fssm_Env;
          diskreq->io_Command = MDCMD_SETPARMS;
        DoIO((struct IORequest *)diskreq);
    }

/****
 Allocate resources for DiskChange interrupt
****/
    if( !( fs->f_dci = (dci = 
        (struct DCIReq *)CreateIORequest(DOSPort,
        sizeof(struct DCIReq))))) return(FALSE); 
       /* Initialize device and unit ptrs */
      dci->dci_req.io_Device = diskreq->io_Device;
      dci->dci_req.io_Unit  = diskreq->io_Unit;

        /* Initialize Interrupt server */
      dci->dci_int.is_Node.ln_Type = NT_INTERRUPT;
      dci->dci_int.is_Node.ln_Pri = 32;
      dci->dci_int.is_Node.ln_Name = "diskchange_int";

      fs->f_dci_sig = 1<<(AllocSignal(-1));
      fs->f_Task           = (struct Task *)Proc;
      dci->dci_int.is_Data = (APTR)fs;

      dci->dci_int.is_Code = diskchange_int;

      dci->dci_req.io_Data = (APTR)(&dci->dci_int);
      dci->dci_req.io_Length = sizeof(struct Interrupt);
      dci->dci_req.io_Command = TD_ADDCHANGEINT;

    SendIO((struct IORequest *)dci);

/****
 Allocate resources for Timer
****/
    if( !( fs->f_timer = (timer = 
        (struct TMRReq *)CreateIORequest(DOSPort,
        sizeof(struct TMRReq))))) return(FALSE); 

      fs->f_timer_cnt = 0;   /* set timer count to 0 (Reset) */

      timer->tmr_req.tr_node.io_Command = TR_ADDREQUEST;

        /* Initialize DosPacket params */
      timer->tmr_req.tr_node.io_Message.mn_Node.ln_Name = (char *)&(timer->tmr_pkt);

      timer->tmr_pkt.dp_Link   = (struct Message *) &(timer->tmr_req);
      timer->tmr_pkt.dp_Port   = (struct MsgPort *) DOSPort;
      timer->tmr_pkt.dp_Type   = ACTION_TIMER;

        /* open the device for access */
    if( error = OpenDevice( TIMERNAME, UNIT_VBLANK,(struct IORequest *)timer, 0)) 
        return(FALSE);

    return(TRUE);
}


/**********************************************************************
*   RemBuffers() --  Remove FS Disk buffers. Do this if no valid FS disk.
**********************************************************************/
static void RemBuffers(void)
{
F();
/* Free FAT memory */
    if( fsys->f_fat_copy )
    {
        FreeMem(fsys->f_fat_copy, fsys->f_fat_copy_sz);
        fsys->f_fat_copy = 0;
    }

/* Free Root Directory cache memory */
    if( fsys->f_rootdir_cache)
    {
        FreeMem(fsys->f_rootdir_cache, fsys->f_rootdir_cache_sz);
        fsys->f_rootdir_cache = 0;
    }

/* Free cache buffers */
    if( fsys->f_cache )
    {
        FreeMem(fsys->f_cache,fsys->f_cache_sz);
        fsys->f_cache = 0;
        fsys->f_num_buffs = 0;
    }
    DevCmd(CMD_CLEAR);
}


/**********************************************************************
*   RemDisk() --  Remove FS Disk resources.  Do this if no disk in drive.
**********************************************************************/
void RemDisk()
{
F();
    if(!((fsys->f_FSflags) & MF_NODISK))
    {
        Flush_Buffers();
    }
    if(fsys->f_VolumeNode)
    {
        DeleteVolNode(fsys->f_VolumeNode);
        fsys->f_LockList=0;    /* clear current Volume LockList */
        fsys->f_VolumeNode=0;  /* clear current Volume Node */
    }
    fsys->f_Diskflags &= ~FAT16_F;      /* clear flag -- 16 bit FAT */
    fsys->f_FSflags &= ~MF_FSDISK;      /* clear flag -- FS disk */

    RemBuffers();       /* Free buffer memory */
}


/**********************************************************************
*   RemDev() --  Remove FS Device resources.  Do this only after ACTION_DIE pkt.
**********************************************************************/
void RemDev()
{
F();

    if( fsys->f_timer ) 
    {
        if( fsys->f_timer->tmr_req.tr_node.io_Device )
        {
            RemDevice(fsys->f_timer->tmr_req.tr_node.io_Device);
            CloseDevice((struct IORequest *)fsys->f_timer);
        }
        DeleteIORequest((struct IORequest *)fsys->f_timer);
    }

    if( fsys->f_dci ) 
    {
#ifdef V33
            /* Alas, TD_REMCHANGEINT has got some problems.  We need to remove
            the interrupt this way.  THIS MAY CHANGE IN THE FUTURE! */
            /* Do not expect the TD_ADDCHANGEINT request replied back */ 
        Forbid();
        Remove((struct Node *)fsys->f_dci);
        Permit();
#else
DPRTF(KPrintF("\nRemDev: 1"));
          fsys->f_dci->dci_req.io_Command = TD_REMCHANGEINT;
        DoIO((struct IORequest *)fsys->f_dci);
#endif

        DeleteIORequest((struct IORequest *)fsys->f_dci);
    }

    if( diskreq )
    {
        if( diskreq->io_Device )
        {
            RemDevice(diskreq->io_Device);
            CloseDevice((struct IORequest *)diskreq);
        }
        DeleteIORequest(diskreq);
    }
    DeleteMsgPort(diskport);
    if(UtilityBase)
    {
        CloseLibrary((struct Library *)UtilityBase);
    }
    if(IntuitionBase)
    {
        CloseLibrary((struct Library *)IntuitionBase);
    }
    if( fsys->f_scratch )  FreeMem(fsys->f_scratch, fsys->f_scratch_sz);

}


/*************
* ValidateDisk()  --- Disk Validation.
*************/
int ValidateDisk()
{
F();
    int error;

/****
    Initialize disk parameters
****/

    if(error = InitDisk()) return(error);
    fsys->f_used_blocks = usedblocks();   /* calc used blocks in FAT */
/****
    Make Volume Node
****/
    fsys->f_FSflags |= MF_FSDISK;         /* set flag -- FS disk */
    MakeVolNode();
    fsys->f_FSflags |= MF_VALIDATED;      /* set flag -- disk validated */

    return(0);    
}
