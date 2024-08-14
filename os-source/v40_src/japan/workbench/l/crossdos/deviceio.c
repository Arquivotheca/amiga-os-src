/* DeviceIO.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      Device IO related functions.
*
*************************************************************************/

#include "FS:FS.h"
#include "intuition/intuition.h"
#include "FS:Fault_txt.h"

extern struct ExecBase  *SysBase;
extern struct FS     *fsys;
extern struct IOStdReq  *diskreq;
extern int  DOSerror;
extern char DeviceName[];
extern struct IntuitionBase *IntuitionBase;
extern struct DosLibrary *DOSBase;

static void Send_DiskChange_Event(BOOL diskstate);
static UBYTE DETS_Req(void);
static UBYTE WP_Req(void);
static UBYTE RWE_Req(LONG RWE, LONG error, char *errortype, ULONG block, LONG length);


/********
    DevCmd() -- Issue a simple command to the device.
        return = 0 or error from IOReq
********/
BYTE DevCmd(command)
UWORD command;
{
    if(diskreq && diskreq->io_Device)  /* check if successful OpenDevice() */
    {
        diskreq->io_Command = command;
        return(DoIO((struct IORequest *)diskreq));
    }
}


/********
    Motor_Off()
********/
void Motor_Off()
{
    ULONG length;

    if(diskreq->io_Device)  /* check if successful OpenDevice() */
    {
        length = diskreq->io_Length;        /* preserve io_Length */
          diskreq->io_Length = MOTOR_OFF;
        DevCmd(TD_MOTOR);
          diskreq->io_Length = length;      /* restore io_Length */
    }
}


/******************
* EasyRequest() -- Calls exec.library EasyRequestArgs().  Also checks for
*                   which window to put it on.
******************/
LONG EasyRequest( window, es, ip, arg1 )
struct Window   *window;
struct EasyStruct *es;
ULONG  *ip;
APTR  arg1;
{
    if((struct Window *)(-1L) == window)
    {       /* no requesters allowed */
        return(FALSE);
    }
    return ( EasyRequestArgs( window, es, ip, &arg1 ) );
}

static const char *RWE_header_crc = "Header CRC error";
static const char *RWE_data_crc = "Data CRC error";
static const char *RWE_unspecified = "Device Error";
static const char *RWE_block_n_found = "Block not found";
static const char *RWE_index_miss = "Index signal missing";
static const char *RWE_track_n_mfm = "Track not MFM formatted";
static const char *disk_error= "Disk Error!!!";

extern struct EasyStruct ERS;
extern char *disk_warn;


/**********************************************************************
*   FixStringBuff()   -- Fix the string buffer to change all ':' to '\n'
**********************************************************************/
char *FixStringBuff(buffer)
char *buffer;
{
    ULONG i;

    for(i=0; buffer[i]!='\0'; i++)
    {
        if(buffer[i] == ':') buffer[i]='\n';    /* if ':' change to '\n' */
    }    
    return(buffer);
}

/**********************************************************************
*   Get_Default_Drive_Geometry() -- if supported
**********************************************************************/
ULONG Get_Default_Drive_Geometry()
{
    register ULONG blocks_part=0;
    register struct DosEnvec *fssm_env = (struct DosEnvec *)BADDR(fsys->f_fssm->fssm_Environ);
    struct DriveGeometry dg;

/* Use Drive Geometry if:
  1) de_DosType = ID_MSDOS_DISK or ID_MSDOS_DISK_DS    and
  2) the boot block is to be located at sector 0    and
  3) TD_GETGEOMETRY command supported.
*/
    if( ((fssm_env->de_DosType == ID_MSDOS_DISK) || (fssm_env->de_DosType == ID_MSDOS_DISK_DS))
        && (fssm_env->de_LowCyl == 0))
    {
    /* Reset mfm.device physical drive parameters to default */
        if(fsys->f_FS_special_flags & MF_SP_DEVICE)
        {
              diskreq->io_Data = (APTR) 0;
              diskreq->io_Command = MDCMD_SETPARMS;
            DoIO((struct IORequest *)diskreq);
        }
        if(fsys->f_FS_special_flags & MF_GG_DEVICE)
        {
              diskreq->io_Data = (APTR)&dg;
            if(!DevCmd(TD_GETGEOMETRY))
            {
                fssm_env->de_HighCyl        = (dg.dg_Cylinders)-1;
                fssm_env->de_BlocksPerTrack = dg.dg_TrackSectors;
                fssm_env->de_Surfaces       = dg.dg_Heads;
                fssm_env->de_BufMemType     = dg.dg_BufMemType;
                blocks_part                 = dg.dg_TotalSectors;
            }
else DPRTF(KPrintF("\nGet_Default_Drive_Geometry: 5  TD_GETGEOMETRY no got it"));
        }
    }
    return(blocks_part);
}


/***************************************************************************
*   Flush_Buffers() -- Flush buffers, caches and update FAT
***************************************************************************/
void Flush_Buffers()
{
    register UWORD i, numbuffs;
    register UBYTE fsysdisk;

    fsysdisk = fsys->f_FSflags & MF_FSDISK;

    if(fsys->f_Diskflags & CACHE_MOD_F)    /* if cache modified flag set */
    {
        if(fsysdisk) FlushCache();         /* if an FS disk -- Flush cache */
          else                      /* if not an FS disk -- clear cache MOD flags*/
        {
    /* Load cache buffer block # */
            numbuffs=fsys->f_num_buffs;
            for(i=0; i<numbuffs; i++)
            {
                fsys->f_cache[i].cache_block &= ~C_BLOCK_MOD;  /* clear block mod flag */
            }
        }
    }
    if(fsys->f_Diskflags & ROOTDIR_MOD_F)      /* if Root Directory modified flag set */
    {
        if(fsysdisk)
        {
            PutBlockMem((APTR)fsys->f_rootdir_cache,fsys->f_beg_root_dir,fsys->f_root_blocks);
        }
    }
    if(fsys->f_Diskflags & FAT_MOD_F)      /* if FAT modified flag set */
    {
        if(fsysdisk)
        {
            PutBlockMem((APTR)fsys->f_fat_copy,fsys->f_beg_fat1,fsys->f_blocks_fat);
            PutBlockMem((APTR)fsys->f_fat_copy,fsys->f_beg_fat2,fsys->f_blocks_fat);
        }
    }
    if(fsys->f_Diskflags & DISK_MOD_F)     /* if disk modified flag set */
    {
        if(fsysdisk) DevCmd(CMD_UPDATE);
          else DevCmd(CMD_CLEAR);
        Motor_Off();
    }

    fsys->f_Diskflags &= ~(CACHE_MOD_F|ROOTDIR_MOD_F|FAT_MOD_F|DISK_MOD_F); /* clear ALL flags */
}


#include "devices/input.h"
#include "devices/inputevent.h"
/**********************************************************************
*   Send_DiskChange_Event() -- Open input.device and send IECLASS_DISKINSERTED
*       or IECLASS_DISKREMOVED input event to those applications waiting for it.
**********************************************************************/
static void Send_DiskChange_Event(BOOL diskstate)
/* if = 0 = disk in drive;  != 0 = disk out of drive */
{
    struct MsgPort *iport;
    struct IOStdReq *ireq;
    struct InputEvent ievent;

    if( iport = CreateMsgPort())
    {
        if(ireq = (struct IOStdReq *)CreateIORequest(iport,sizeof(struct IOStdReq )))
        {
            if(!OpenDevice("input.device",0,(struct IORequest *)ireq,0)) 
            {
                  ireq->io_Command = IND_WRITEEVENT;
                  ireq->io_Data = (APTR)&ievent;
                if(diskstate == 0) ievent.ie_Class = IECLASS_DISKINSERTED;
                 else  ievent.ie_Class = IECLASS_DISKREMOVED;
                DoIO((struct IORequest *)ireq);
                CloseDevice((struct IORequest *)ireq);
            }
            DeleteIORequest(ireq);
        }
        DeleteMsgPort(iport);
    }
}


/**********************************************************************
*   Is_Disk_Out() -- Is disk out of drive?
*
*   This routine also does a TD_GETGEOMETRY on floppies and sets the DosEnvec.
*   It also does a check for write protect of the disk.
*
*   This routine is created to fix a deficiency of the scsi.device (37.19)
*   when using removable media.  Return = 0 for no; !=0 for disk out.
**********************************************************************/
UBYTE Is_Disk_Out()
{
    UBYTE disk_out;


/* Some SCSI device drivers do not update the Change state on 
removable media until a command is issued that actually accesses the media.
Somtimes the Change state is updated based on only the previous command
results.  This appears to be the case with CBM's scsi.device (37.19).
The "fix" is to do a TD_SEEK first then the TD_CHANGESTATE cmd.
In my opinion this is a kludge.*/
     diskreq->io_Offset = 0;    /* Seek to block 0 */
    DevCmd(TD_SEEK);
      fsys->f_timer_start = DISK_TIME_OUT;   /* set timer count to disk time-out */
    DevCmd(TD_CHANGESTATE);
    if(disk_out = diskreq->io_Actual)
    {       /* no disk in drive */
DPRTF(KPrintF("\nIs_Disk_Out: No Disk"));
        fsys->f_FSflags |= MF_NODISK;      /* set flag */
        DOSerror = ERROR_NO_DISK;
        fsys->f_NumSoftErrors = 0;
        fsys->f_Diskflags &= ~(WRITE_PROT_F);  /* clear flag */
    }
    else
    {
        fsys->f_FSflags &= ~MF_NODISK;  /* clear flag */

        Get_Default_Drive_Geometry();

        DevCmd(TD_PROTSTATUS);
        if(diskreq->io_Actual)
        {
            fsys->f_Diskflags |= WRITE_PROT_F;  /* set flag */
            DOSerror = ERROR_DISK_WRITE_PROTECTED;
        }
        else fsys->f_Diskflags &= ~WRITE_PROT_F;  /* clear flag */

    }

/* This code is here to support the ACTION_WRITE_PROTECT function */
    if(fsys->f_FSflags & MF_WP_DEVICE)
    {
        fsys->f_Diskflags |= WRITE_PROT_F;  /* set flag */
        DOSerror = ERROR_DISK_WRITE_PROTECTED;
    }

    return(disk_out);
}


/******************
* DETS_Req() -- Make a simple requester for "Disk Ejected Too Soon!"
******************/
static UBYTE DETS_Req(void)
{
    UBYTE bool;
    char tempbuff[TSTRG_BUFF_SZ];
    char tempbuff1[TSTRG1_BUFF_SZ];

    ERS.es_TextFormat   = &tempbuff[1];
    ERS.es_Title        = disk_warn;

    Motor_Off();
    Fault(DISK_CORRUPT,"",tempbuff,TSTRG_BUFF_SZ);          /* Fault() = Disk corrupt - task stopped */
    Fault(YOU_MUST_REPLACE,tempbuff,tempbuff,TSTRG_BUFF_SZ);/* Fault() = You MUST replace volume */
    strcat(tempbuff,"\n %b");                               /* "insert volume name" */
    Fault(IN_DEVICE,tempbuff,tempbuff,TSTRG_BUFF_SZ);       /* Fault() = in device %s%s */
    FixStringBuff(&tempbuff[1]);
    Fault(RETRY_CANCEL, 0, tempbuff1,TSTRG1_BUFF_SZ);       /* Fault() = Retry/Cancel */
    bool = EasyRequest(PKT_WINDOW, &ERS, 0,
        (APTR)fsys->f_VolumeNode->dol_Name, DeviceName,"",tempbuff1);
    return(bool);
}


/******************
* WP_Req() -- Make a simple requester for "Write Protected"
******************/
static UBYTE WP_Req(void)
{
    UBYTE bool;
    char tempbuff[TSTRG_BUFF_SZ];
    char tempbuff1[TSTRG1_BUFF_SZ];

    ERS.es_TextFormat   = &tempbuff[1];
    ERS.es_Title        = disk_warn;

    Motor_Off();
    Fault(DISK_CORRUPT,"",tempbuff,TSTRG_BUFF_SZ);          /* Fault() = Disk corrupt - task stopped */
    Fault(VOLUME,tempbuff,tempbuff,TSTRG_BUFF_SZ);          /* Fault() = Volume */
    Fault(IS_PROTECTED,tempbuff,tempbuff,TSTRG_BUFF_SZ);    /* Fault() = is write protected */
    FixStringBuff(&tempbuff[1]);
    Fault(RETRY_CANCEL, 0, tempbuff1,TSTRG1_BUFF_SZ);       /* Fault() = Retry/Cancel */
    bool = EasyRequest(PKT_WINDOW, &ERS, 0, tempbuff1);
    return(bool);
}


/**********************************************************************
*   DiskChange() -- Diskchange; Remove disk then re-validate disk
*                   Return !=0 if NOT_FS_DISK or =0 if FS_DISK
**********************************************************************/
UBYTE DiskChange()
{
    register UBYTE disk_out;
    register UBYTE bool = FALSE;

DPRTF(KPrintF("\nDiskChange:"));

/****
    Check if disk installed and if write protected
****/
    disk_out = Is_Disk_Out();
    if( !(fsys->f_inhibit_cnt) )     /* only if FS not INHIBITED */
    {
        fsys->f_FSflags &= ~MF_VALIDATED;      /* clear flag -- disk not validated */

    /* if something modified on old disk and no disk in drive
      -- put up "Disk Ejected Too Soon" requestor */
        if(fsys->f_Diskflags & (CACHE_MOD_F|ROOTDIR_MOD_F|FAT_MOD_F|DISK_MOD_F))
        {   /* buffers or disk modified */
            do
            {
                if(disk_out)
                {       /* no disk in drive */
                    if((bool = DETS_Req()) == TRUE)
                    {   /* "Retry" */
                        if((disk_out = Is_Disk_Out()) == FALSE)
                        {   /* disk back in drive */
                            bool = FALSE;
                        }
                    }
                     else
                    {   /* "Cancel".  Clear buffers and do not write to the disk */
                        fsys->f_FSflags &= ~MF_FSDISK;  /* make NOT an FS DISK */
                    }
                }
                if(fsys->f_Diskflags & WRITE_PROT_F)
                {   /* check if write protected */
                    if((bool = WP_Req()) == TRUE)
                    {   /* "Retry" */
                        if((disk_out = Is_Disk_Out()) == TRUE)
                        {   /* disk out of drive */
                            bool = TRUE;
                        }
                         else if(!(fsys->f_Diskflags & WRITE_PROT_F))
                        {   /* disk not write-protected any more */
                            bool = FALSE;
                        }
                    }
                     else
                    {   /* "Cancel".  Clear buffers and do not write to the disk */
                        fsys->f_FSflags &= ~MF_FSDISK;  /* make NOT an FS DISK */
                    }
                }
            } while(bool == TRUE);  /* Retry requested */
        }

        RemDisk();      /* Remove disk logically and Flush_Buffers() if necessary */
        if(!disk_out) bool = ValidateDisk();   /* Try to validate disk */

#ifdef DEBUG
if(disk_out)
{
  DPRTF(KPrintF("\n\tNO Disk in drive  DOS = %lx",disk_out));
}
   else
{
  DPRTF(KPrintF("\n\tDisk in drive  DOS = %lx",disk_out));
}
#endif
    }
    Send_DiskChange_Event(disk_out);

    return(bool);
}


/******************
* RWE_Req() -- Make a simple requester for "Read Error!" or "Write Error!"
******************/
static UBYTE RWE_Req(LONG RWE, LONG error, char *errortype, ULONG block, LONG length)
{

    UBYTE bool=FALSE;
    ULONG phys_block;
    char tempbuff[TSTRG_BUFF_SZ];
    char tempbuff1[TSTRG1_BUFF_SZ];
    UBYTE *volname;

    fsys->f_NumSoftErrors++;

    if(fsys->f_FSflags & MF_FSDISK)
    {
        if(fsys->f_VolumeNode == 0) volname = DeviceName;
        else  volname = &(((UBYTE *)BADDR(fsys->f_VolumeNode->dol_Name))[1]);

        if(RWE == READ_ERROR)       /* == Read Error! */
        {
            DevCmd(CMD_UPDATE);     /* Update the current track */
            DevCmd(CMD_CLEAR);     /* Clear the current track */
        }

        ERS.es_TextFormat   = tempbuff;
        ERS.es_Title        = disk_error;

        Motor_Off();
        phys_block = block+(length>>fsys->f_bytes_block_sh);    /* calc block where error occurred */

        Fault(RWE," %s",tempbuff,TSTRG_BUFF_SZ);                /* Fault() = has a read error */
        Fault(ON_DISK_BLOCK,tempbuff,tempbuff,TSTRG_BUFF_SZ);   /* Fault() = on disk block %ld */
        Fault(ERROR,tempbuff,tempbuff,TSTRG_BUFF_SZ);           /* Fault() = Error */
        strcat(tempbuff,"= %ld");                               /* = %ld */
        strcat(tempbuff,"\n %s");                               /* "ErrorType" */
        FixStringBuff(tempbuff);
        Fault(RETRY_CANCEL, 0, tempbuff1,TSTRG1_BUFF_SZ);       /* Fault() = Retry/Cancel */
        bool = EasyRequest(PKT_WINDOW, &ERS, 0,
            (APTR)volname, phys_block, error, errortype,tempbuff1);
    }
    return(bool);
}



/********
*   GetBlockMem() -- Get blocks from disk and insert into memory specified
*       (usually cache).
********/
BYTE GetBlockMem(memory, block, num_blocks)
UBYTE *memory;
ULONG num_blocks;
ULONG block;
{
    BYTE error;
    UBYTE bool;
    char tempbuff1[TSTRG1_BUFF_SZ];
    register struct FS *fs = fsys;
    register bytes_block_sh = fs->f_bytes_block_sh;
    ULONG minnumblocks;

    fs->f_timer_start = DISK_TIME_OUT;   /* set timer count to disk time-out */

      diskreq->io_Length = 0;
      diskreq->io_Offset = block<<bytes_block_sh;
      diskreq->io_Data = memory;
    do
    {
          diskreq->io_Data   = (APTR)(memory += diskreq->io_Length);
          diskreq->io_Offset += diskreq->io_Length;
          minnumblocks = min(fs->f_maxblocks_transfer, num_blocks);
          diskreq->io_Length = minnumblocks<<bytes_block_sh;
        do
        {
            bool = FALSE;
              diskreq->io_Command = CMD_READ;
            if( (error = DoIO((struct IORequest *)diskreq)))
            {
                switch(error)
                {   /* handle error and put up requester if necessary */
                case TDERR_DiskChanged:
                    bool = DiskChange();
                    break;
                case TDERR_BadHdrSum:
                    if((bool = RWE_Req(READ_ERROR, error, RWE_header_crc,
                        block, diskreq->io_Actual)) == FALSE)
                    {
                        diskreq->io_Error = error;
                        error = 0;
                    }
                    break;
                case TDERR_BadSecSum:
                    if((bool = RWE_Req(READ_ERROR, error, RWE_data_crc,
                        block, diskreq->io_Actual)) == FALSE)
                    {
                        diskreq->io_Error = error;
                        error = 0;
                    }
                    break;
                case TDERR_BadSecID:
                    bool = RWE_Req(READ_ERROR, error, RWE_block_n_found,
                        block, diskreq->io_Actual);
                    break;
                case MDERR_IndexNotSync:
                    bool = RWE_Req(READ_ERROR, error, RWE_index_miss,
                        block, diskreq->io_Actual);
                    break;
                case TDERR_NoSecHdr:
                    bool = RWE_Req(READ_ERROR, error, RWE_track_n_mfm,
                        block, diskreq->io_Actual);
                    break;
                case TDERR_NoMem:
                    if((bool = RWE_Req(READ_ERROR, error,
                            /* Fault() = not enough memory available */
                        (Fault(ERROR_NO_FREE_STORE, 0, tempbuff1,TSTRG1_BUFF_SZ),tempbuff1),
                        block, diskreq->io_Actual)) == FALSE)
                    {
                        diskreq->io_Error = error;
                        error = 0;
                    }
                    break;
                default:
                    bool = RWE_Req(READ_ERROR, error, RWE_unspecified,
                        block, diskreq->io_Actual);
                    break;
                }
            }
        } while(bool == TRUE);  /* If TRUE, try again */
    } while((num_blocks -= minnumblocks) != 0);  /* If more blocks to be transfered, do more */

    return(error);
}




/********
*   PutBlockMem() -- Put specified memory (usually cache) into blocks of disk.
********/
BYTE PutBlockMem(memory, block, num_blocks)
UBYTE *memory;
ULONG num_blocks;
ULONG block;
{
    BYTE error;
    UBYTE bool;
    char tempbuff1[TSTRG1_BUFF_SZ];
    register struct FS *fs = fsys;
    register bytes_block_sh = fs->f_bytes_block_sh;
    ULONG minnumblocks;

      fsys->f_timer_start = DISK_TIME_OUT;   /* set timer count to disk time-out */

      diskreq->io_Command = CMD_WRITE;
      diskreq->io_Length = 0;
      diskreq->io_Offset = block<<bytes_block_sh;
      diskreq->io_Data = memory;
    do
    {
          diskreq->io_Data   = (APTR)(memory += diskreq->io_Length);
          diskreq->io_Offset += diskreq->io_Length;
          minnumblocks = min(fs->f_maxblocks_transfer, num_blocks);
          diskreq->io_Length = minnumblocks<<bytes_block_sh;
        do
        {
            bool = FALSE;
            if( (error = DoIO((struct IORequest *)diskreq)))
            {
                switch(error)
                {   /* handle error and put up requester if necessary */
                case TDERR_DiskChanged:
                    bool = DiskChange();
                    break;
                case TDERR_BadHdrSum:
                    bool = RWE_Req(WRITE_ERROR, error, RWE_header_crc,
                        block, diskreq->io_Actual);
                    break;
                case TDERR_BadSecID:
                    bool = RWE_Req(WRITE_ERROR, error, RWE_block_n_found,
                        block, diskreq->io_Actual);
                    break;
                case MDERR_IndexNotSync:
                    bool = RWE_Req(WRITE_ERROR, error, RWE_index_miss,
                        block, diskreq->io_Actual);
                    break;
                case TDERR_NoSecHdr:
                    bool = RWE_Req(WRITE_ERROR, error, RWE_track_n_mfm,
                        block, diskreq->io_Actual);
                    break;
                case TDERR_WriteProt:
/* Some SCSI device drivers do not update the Write Protect state on 
removable media until a command is issued that actually accesses the media.
This appears to be the case with CBM's scsi.device (37.19). */
                    if((bool = RWE_Req(WRITE_ERROR, error,
                            /* Fault() = disk is write protected */
                        (Fault(ERROR_DISK_WRITE_PROTECTED, 0, tempbuff1,TSTRG1_BUFF_SZ),tempbuff1),
                        block, diskreq->io_Actual)) == FALSE)
                    {
                        fsys->f_Diskflags |= WRITE_PROT_F;  /* set flag */
                        fsys->f_Diskflags &= ~(CACHE_MOD_F|ROOTDIR_MOD_F|FAT_MOD_F|DISK_MOD_F); /* clear ALL flags */
                    }
                    break;
                default:
                    bool = RWE_Req(WRITE_ERROR, error, RWE_unspecified,
                        block, diskreq->io_Actual);
                    break;
                }
            }
        } while(bool == TRUE);  /* If TRUE, try again */
    } while((num_blocks -= minnumblocks) != 0);  /* If more blocks to be transfered, do more */

    if(error == 0) fsys->f_Diskflags |= DISK_MOD_F;   /* disk modified flag set */

    return(error);
}
