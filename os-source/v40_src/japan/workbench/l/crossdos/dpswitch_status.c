/*** DPswitch_Status.c ***************************************************************
** Copyright 1991 CONSULTRON
****************************************************************************/

#include "FS:FS.h"
#include "dos.h"

/*---------------------------------------------------------------------------
** References to system
*/

extern char             DeviceName[];

struct DosLibrary       *DOSBase=0;

extern int              DOSerror;
extern APTR             NewProc_seglist;
extern APTR             Data_seg;
extern struct ExecBase  *SysBase;


extern struct MemEntry  *MemEntry;
extern struct Process   *StatusProc;
extern struct MsgPort   *StatusPort;
extern struct Process   *Proc;
extern struct MsgPort   *DOSPort;
extern struct FS        *fsys;

static int Disk_Info(register struct InfoData *infodata);
static int DInfo(struct MLock *filelock, struct InfoData *infodata);

/**********************************************************************
*   FileSystem_Status() --- performs the DOS packet status calls such as
*       ACTION_INFO, ACTION_DISK_INFO, ACTION_CURRENT_VOLUME and 
*       ACTION_IS_FILESYSTEM.
*       It forwards the other calls to the other FileSystem task (mostly
*       device IO related stuff).
**********************************************************************/
void FileSystem_Status()
{ 
    struct DosPacket    *pkt;
    register int        DOSreturn=DOSFALSE;
    struct MsgPort      *SPort = StatusPort;
    struct MsgPort      *replyport;
    UBYTE               die=0;
    UBYTE               noreply=0;
    register struct Message  *msg;
    struct DeviceNode   *DevNode=0;
    struct FileSysStartupMsg *Fssm;
    UBYTE               *temp;
    int                 i;

#define ARG1    pkt->dp_Arg1
#define ARG2    pkt->dp_Arg2
#define ARG3    pkt->dp_Arg3
#define ARG4    pkt->dp_Arg4
#define ARG5    pkt->dp_Arg5
#define ARG6    pkt->dp_Arg6
#define ARG7    pkt->dp_Arg7


    if( !(DOSBase = (struct DosLibrary *)OpenLibrary(DOSNAME, LIB_VERS)) ) goto QUIT;

/*** Initialization code for first packet received ****/
                /* Create new process to pass DOS pkts to perform actual disk IO */
    pkt = WaitPkt();

    Forbid();
#define NEWPROCPRI      10
#define MIN_STACK_SZ    4000
    if((DOSPort = CreateProc(StatusProc->pr_Task.tc_Node.ln_Name, 
        NEWPROCPRI, MKBPTR(NewProc_seglist), MIN_STACK_SZ)) == 0)
    {
        goto QUIT;   /* return; proceed no further */
    }
/* Pass the data seg pointer which is in register A4 to the child process. */
    ((struct Task *)DOSPort->mp_SigTask)->tc_UserData = (APTR)&Data_seg;
    Permit();

    if(fsys->f_DevNode = DevNode = (struct DeviceNode *) BADDR(ARG3))    /* passed DeviceNode from ARG3 if available*/
    {
        DevNode->dn_Task= SPort;   /* make sure DOS sends DOS pkts to this task */
        DevNode->dn_GlobalVec= (BPTR)fsys;  /* Debug info; temp use */
        Install_on_FileSysRes(DevNode->dn_SegList);
    }
    Fssm = (struct FileSysStartupMsg *) BADDR(ARG2);    /* passed FileSysStartupMsg from ARG2 */
    fsys->f_fssm = Fssm;

    temp = ((UBYTE *)BADDR(ARG1));   /* get device name from ARG1 */
    for(i=temp[0]-1; i>0; i--)            /* Copy the device name */
    {
        /* for some reason, DeviceName has the ':' tacked on the end.  Get rid of it */
        if(temp[i] == ':') DeviceName[i-1] = '\0';
        else DeviceName[i-1] = toupper(temp[i]); /* convert to uppercase */
    } 
    DeviceName[temp[0]]  = '\0';    /* Add null at the end of the name */

    AllocSignal(SIGB_KILLPARENT);

    replyport = pkt->dp_Port;   /* save the replyport of the originator */

/*** For reasons that may seem strange, This code has been added to support a
    very peculiar feature of the mfm.device driver.  As of 38.6 of mfm.device,
    it installs the appropriate TDPatch needed under KS 33 or 34.  To do so
    correctly, we can not assume that the patch will be installed.  If the user
    uses CrossDOS from the product disk only, then we must execute the TDPatch
    from the current directory.  If KS 2.0 or higher is used this code is run
    but is useless.  Backwards compatibility.  Isn't it GRAND!
**********/
    StatusProc->pr_CurrentDir = ((struct Process *)replyport->mp_SigTask)->pr_CurrentDir;

    SendPkt(pkt, DOSPort, SPort);   /* pass initial pkt off to filesystem process */

    if(pkt = WaitPkt())    /* Wait for startup pkt to come back */
    {
        DOSreturn = pkt->dp_Res1;
        SendPkt(pkt, replyport, SPort);   /* Send startup pkt back to DOS */
    }
    if(DOSreturn == DOSFALSE) goto QUIT; /* check startup packet for successful child startup */

/**********************************************************************
*   Check for DOS Packet 
**********************************************************************/
    while(!die && (pkt = WaitPkt()))
    {

/**********************/
/* Process Packet types. */
        DOSreturn= DOSFALSE;
        DOSerror= ERROR_ACTION_NOT_KNOWN;

        switch(pkt->dp_Type)
        {
        case ACTION_INFO:

DPRTF(KPrintF("\naction_INFO"));

            DOSreturn = MKBPTR( DInfo( BADDR(ARG1),BADDR(ARG2)));
DPRTF(KPrintF("\n\tLock = %lxH  InfoData = %lxH  Result1 = %ld  Result2 = %ld",
    BADDR(ARG1), BADDR(ARG2), DOSreturn, DOSerror));
            break;

        case ACTION_CURRENT_VOLUME:

DPRTF(KPrintF("\naction_CURRENT_VOLUME"));

/* I found this out as of 8/2/90!  NOT DOCUMENTED anywhere in specs available at this time! */
/* if dp_Arg1 != 0 then = fh_Arg1 which in my case is a lock.  Get current volume node from lock */
            if(ARG1)
            {
                DOSreturn = ((struct MLock *)ARG1)->ml_lock.fl_Volume;
            }

             else
            {
                DOSreturn = MKBPTR(fsys->f_VolumeNode);
            }
            DOSerror = fsys->f_fssm->fssm_Unit;

DPRTF(KPrintF("\n\tLock = %lxH  VolNode = %lxH  unit = %ld", ARG1, BADDR(DOSreturn), DOSerror));

            break;

        case ACTION_DISK_INFO:

DPRTF(KPrintF("\naction_DISK_INFO  "));

            DOSreturn = MKBPTR( Disk_Info( BADDR(ARG1)));

DPRTF(KPrintF("\n\tInfoData = %lxH  Result1 = %ld  Result2 = %ld",
    BADDR(ARG1), DOSreturn, DOSerror));

            break;


        case ACTION_IS_FILESYSTEM:

DPRTF(KPrintF("\naction_IS_FILESYSTEM"));

            DOSreturn = DOSTRUE;
            break;

        case ACTION_DIE:
DPRTF(KPrintF("\naction_DIE"));
            DOSreturn= DOSTRUE;
            die = 1;

        case ACTION_STARTUP:
DPRTF(KPrintF("\naction_STARTUP"));
        default:
DPRTF(KPrintF("\naction = %ld",pkt->dp_Type));
DPRTF(KPrintF("\n%lxH",ARG1));
DPRTF(KPrintF("  %lxH",ARG2));
DPRTF(KPrintF("  %lxH",ARG3));
DPRTF(KPrintF("  %lxH",ARG4));

PassOnDOSPkt:
            SendPkt(pkt, DOSPort, pkt->dp_Port);
/* if die = 1, after this point, do not use anything from _fsys or the data seg */
/* This is because you cannot trust that it will still be there */
            noreply = 1;

            break;
        }

        if( !noreply )
        {
            ReplyPkt(pkt,DOSreturn, DOSerror);
        }
          else noreply = 0;    /* clear flag */
    }


/**********************************************************************
*   QUIT the File System!
**********************************************************************/
QUIT:
    if(DevNode)
    {
        DevNode->dn_Task= NULL;     /* bad if someone in process of accessing us . . . */
        DevNode->dn_GlobalVec= -1;
    }
       /*  Return packets not used */
    while(msg = (struct Message *) GetMsg(SPort))
    {
        if(pkt = (struct DosPacket *) msg->mn_Node.ln_Name)
        {
            ReplyPkt(pkt,DOSFALSE,ERROR_ACTION_NOT_KNOWN);
        }
    }
    Wait(SIGF_KILLPARENT);  /* Wait for child process to signal that he is dead */
    if(DOSBase) CloseLibrary((struct Library *)DOSBase);
}


/*************
* Disk_Info()  --- Get disk info and fill supplied InfoData struct
*************/
static int Disk_Info(register struct InfoData *infodata)
{

    if(infodata)
    {
        infodata->id_UnitNumber   = fsys->f_fssm->fssm_Unit;
        infodata->id_NumSoftErrors= fsys->f_NumSoftErrors;
        infodata->id_NumBlocksUsed= fsys->f_used_blocks;
        infodata->id_VolumeNode   = 0;
        infodata->id_InUse        = 0;
        infodata->id_NumBlocks    = fsys->f_blocks_part;
        infodata->id_BytesPerBlock= fsys->f_bytes_block;
        if(fsys->f_Diskflags & WRITE_PROT_F) infodata->id_DiskState = ID_WRITE_PROTECTED;
          else  if(fsys->f_FSflags & MF_VALIDATED) infodata->id_DiskState = ID_VALIDATED;
          else  infodata->id_DiskState = ID_VALIDATING;
    }
      else return(DOSFALSE);    /* no valid infodata struct passed */

      if(fsys->f_FSflags & MF_NODISK)
    {
        infodata->id_DiskType             = ID_NO_DISK_PRESENT;
    }
      else if( fsys->f_inhibit_cnt )
    {
        infodata->id_DiskType             = ID_BUSY_DISK;
    }
      else if( (fsys->f_FSflags & MF_NOTREADABLE) )
    {
        infodata->id_DiskType             = ID_UNREADABLE_DISK;
    }
      else if( (fsys->f_FSflags & MF_FSDISK) && (fsys->f_FSflags & MF_VALIDATED))
    {
      /* for reasons unknown, only this DiskType is accepted by AmigaDOS V1.2 and V1.3 */
      /* It should be the DosType specified in the struct DosEnvec ->de_DosType */
        infodata->id_DiskType             = ID_DOS_DISK;
        infodata->id_VolumeNode   = MKBPTR(fsys->f_VolumeNode);
        infodata->id_InUse        = fsys->f_LockList;
    }
      else
    {
        infodata->id_DiskType             = ID_NOT_REALLY_DOS;
    }

    DOSerror = 0;

/*
DPRTF(KPrintF("\n id_UnitNumber     = %ld ",infodata->id_UnitNumber   ));
DPRTF(KPrintF("\n id_NumSoftErrors  = %ld ",infodata->id_NumSoftErrors));
DPRTF(KPrintF("\n id_NumBlocksUsed  = %ld ",infodata->id_NumBlocksUsed));
DPRTF(KPrintF("\n id_VolumeNode     = %lxH",infodata->id_VolumeNode   ));
DPRTF(KPrintF("\n id_InUse          = %lxH",infodata->id_InUse        ));
DPRTF(KPrintF("\n id_NumBlocks      = %ld ",infodata->id_NumBlocks    ));
DPRTF(KPrintF("\n id_BytesPerBlock  = %ld ",infodata->id_BytesPerBlock));
DPRTF(KPrintF("\n id_DiskState      = %lxH",infodata->id_DiskState    ));
DPRTF(KPrintF("\n id_DiskType       = %lxH",infodata->id_DiskType     ));
*/

    return(DOSTRUE);
}


/*************
* DInfo()  --- Get disk info based on lock passed and fill supplied InfoData struct
*************/
static int DInfo(struct MLock *filelock, struct InfoData *infodata)
{
    if((fsys->f_VolumeNode == (struct DosList *)BADDR(filelock->ml_lock.fl_Volume)))
    {       /* Volume node in the lock is the same as the current volume mounted */
        return(Disk_Info(infodata));
    }
    return(DOSFALSE);
}
