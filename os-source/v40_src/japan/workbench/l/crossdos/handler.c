/*  handler.c  **********************************************************
** Copyright 1991 CONSULTRON
*
*   handler related functions
*
************************************************************************/

#include "FS:FS.h"
#include "intuition/intuition.h"
#include "resources/filesysres.h"

/******
* External data references
*******/
extern struct ExecBase  *SysBase;
extern struct DosLibrary    *DOSBase;
extern struct IntuitionBase *IntuitionBase;
extern struct FS            *fsys;
extern struct Process       *StatusProc;
extern struct MsgPort       *StatusPort;

static struct DosList * FindVolNode(register char *volstr,register struct DateStamp *voldate);
static UBYTE DVND_Req(void);

/**********************************************************************
*   Install_on_FileSysRes() -- Install this file system on the
*       file system resource list if not already there.
**********************************************************************/
void Install_on_FileSysRes(seglist)
BPTR seglist;
{
    struct FileSysResource *fsr;
    struct List *fsr_list;
    struct FileSysEntry *fsr_node;
    struct FileSysEntry *fse0;
    struct FileSysEntry *fse1;

    if( 0 == (fsr = (APTR)OpenResource(FSRNAME)))
    {
        return;     /* couldn't open resource (?) */
    }    

    fsr_list=&(fsr->fsr_FileSysEntries);

    Forbid();
    for(fsr_node=(struct FileSysEntry *)(fsr_list->lh_Head);
        fsr_node->fse_Node.ln_Succ
        &&  (fsr_node->fse_DosType != ID_MSDOS_DISK)
        &&  (fsr_node->fse_DosType != ID_MSDOS_DISK_HD);
        fsr_node=(struct FileSysEntry *)fsr_node->fse_Node.ln_Succ);
    Permit();

    if(fsr_node->fse_Node.ln_Succ == 0)   /* our file system not found on the file system resource list */
    {
        if(0 == (fse0 = (struct FileSysEntry *)AllocMem(2*sizeof(struct FileSysEntry), MEMF_PUBLIC|MEMF_CLEAR)))
        {
            return; /* could not allocate memory needed (?).  Forget installing */
        }    
        fse1 = (struct FileSysEntry *)((UBYTE *)fse0+sizeof(struct FileSysEntry));
        fse0->fse_DosType = ID_MSDOS_DISK;
        fse1->fse_DosType = ID_MSDOS_DISK_HD;
        fse0->fse_Version = fse1->fse_Version = ((VERSION<<16) + REVISION);
        fse0->fse_PatchFlags = fse1->fse_PatchFlags = 0x180;
        fse0->fse_SegList = fse1->fse_SegList = seglist;
        fse0->fse_GlobalVec = fse1->fse_GlobalVec = -1;
        Forbid();   /* Altering public list requires Forbid()ing first */
        AddHead(fsr_list,(struct Node *)fse0);
        AddHead(fsr_list,(struct Node *)fse1);
        Permit();
    }
}


/**********************************************************************
*   FindVolNode()  --- Find the specified Volume Node in the DOS list
*       Perform an AttemptLockDosList() to prevent a dead-lock condition.
*       Return:
*       0       if no volume node found that matches
*       1       if AttemptLockDosList() failed.  Try again later.
*       volnode found that matches      
**********************************************************************/
static struct DosList * FindVolNode(register char *volstr,register struct DateStamp *voldate)
{

    register struct DosList *dl;
    ULONG LDflags=LDF_VOLUMES|LDF_READ;

    if((ULONG)(dl = AttemptLockDosList(LDflags)) <= 1 )
        return((struct DosList *)1L);
    while( (dl = NextDosEntry(dl,LDflags))
            && (dl = FindDosEntry(dl,(UBYTE *)&(volstr[1]),LDflags)))
    {
        if(0 == CompareDates(voldate,&(dl->dol_misc.dol_volume.dol_VolumeDate)))
        {
        /* Fill in handler process port in found Volnode if no one else claimed it */
            if(dl->dol_Task == 0)
            {
                dl->dol_Task = StatusPort;
            }
            UnLockDosList(LDflags);
            return(dl);
        }
    }
    UnLockDosList(LDflags);

    return(0);
}


/**********************************************************************
*   DeleteVolNode()  --- Attempts to delete the specified Volume Node
*   from the DOS list ONLY if there no locks on the Volume.
**********************************************************************/
BYTE DeleteVolNode(volnode)
register struct DosList *volnode;
{
    register struct DosList *dl;
    BPTR  next_dl;
    ULONG LDflags=LDF_VOLUMES|LDF_WRITE;
    BYTE  volnode_rem=FALSE;

    if(volnode && (volnode->dol_Type == DLT_VOLUME))
    {   
        if((fsys->f_VolumeNode != (struct DosList *)volnode)
            && (volnode->dol_misc.dol_volume.dol_LockList) );   /* do nothing to the volnode */
         else if( (fsys->f_VolumeNode == (struct DosList *)volnode)
            && (fsys->f_LockList) )
        {
            volnode->dol_misc.dol_volume.dol_LockList = fsys->f_LockList;  /* put Volume LockList in volnode */
            volnode->dol_Task = 0;   /* remove task port from volnode */
        }
        else
        {   /* Remove the volume node ONLY if the LockList of the node is ZERO and not currently mounted */
            volnode_rem=TRUE;
            if(fsys->f_FS_special_flags & (MF_ADD_VOL))
            {   /* volume node to be deleted but never added to DosList. Remove NOW */
                FreeMem(volnode, sizeof(struct VolNode) );
            }
            else
            {
            ((struct DeviceList *)volnode)->dl_unused =  /* schedule for removal */
                fsys->f_FS_special_flags |= MF_REM_VOL;  /* set flag for delayed removal */
            }
        }
    }
        /* The version 39 autodoc specs on the AttemptLockDosList() function
            do not exactly match its operation.  The valid returns if the locking
            of the DosList fails is either 0 or 1.  This has been validated
            in  the dos.library 37.45 source code by Martin Taillefer.  It
            has been submitted as a bug report 10/10/92. */
    if( (fsys->f_FS_special_flags & MF_REM_VOL)   /* no locks held;  completely remove VolNode */
        && ((ULONG)(dl = AttemptLockDosList(LDflags)) > 1))
    {
        while( dl = NextDosEntry(dl,LDflags) )
        {   /* Go remove all volume nodes scheduled for removal */
            if((((struct DeviceList *)dl)->dl_unused & MF_REM_VOL)  /* check if the node is to be removed */
                && CHECK_DOSTYPE(dl->dol_misc.dol_volume.dol_DiskType ) )
            {
                next_dl = dl->dol_Next;
                if(RemDosEntry(dl)) FreeMem((UBYTE *)dl, sizeof(struct VolNode) );
                dl = (struct DosList *)&next_dl;
            }
        }
        fsys->f_FS_special_flags &= ~(MF_REM_VOL);  /* clear flag */
        UnLockDosList(LDflags);
    }

    if(fsys->f_FS_special_flags & (MF_REM_VOL))
    {   /* Set timer for delayed removal */
        fsys->f_timer_start++;   /* set timer count to disk time-out */
    }
    return(volnode_rem);
}


static const struct EasyStruct DVND_ERS =
{
sizeof (struct EasyStruct),
0,
"Disk Warning!",
"A volume with the same name\n and date is already mounted.\nIt is ADVISED that a new volume\n date be written to the disk in\n%s",
"SetDate|Ignore",
};


extern char DeviceName[];

/******************
* DVND_Req() -- Make a simple requester for  "Duplicate volume name and date".
*
******************/
static UBYTE DVND_Req(void)
{
    UBYTE bool=FALSE;

    if((fsys->f_FSflags & MF_FSDISK) && !(fsys->f_Diskflags & WRITE_PROT_F))
    {
        Motor_Off();

        bool = EasyRequest(PKT_WINDOW, &DVND_ERS, 0, DeviceName);
    }
    return(bool);
}



/**********************************************************************
*   AllocVolNode()  --- Allocate a new Volume Node in the Dos list
**********************************************************************/
struct DosList *AllocVolNode(volstr, voldate)
char *volstr;
struct DateStamp *voldate;
{
    register struct FS          *fs = fsys;
    register struct DosList     *volnode=0, *newvolnode=0;
    register struct DosList *dl;
    ULONG LDflags=LDF_VOLUMES|LDF_WRITE;
    BYTE  volnode_add=FALSE;

    if( (ULONG)(volnode = FindVolNode(volstr, voldate)) == 1 ) /* Could not lock DosList */
        volnode = 0;
    else if(volnode == 0)
    {
            /* Fill volume Dos Entry */
        if(volnode = newvolnode = (struct DosList *) AllocMem(sizeof(struct VolNode), MEMF_PUBLIC|MEMF_CLEAR))
        {
            CopyMem(volstr, ((struct VolNode *)newvolnode)->vn_VolName, volstr[0]+1);

            newvolnode->dol_Type      = DLT_VOLUME;
            newvolnode->dol_misc.dol_volume.dol_DiskType
                                  = ((struct DosEnvec *)BADDR(fs->f_fssm->fssm_Environ))->de_DosType;
            newvolnode->dol_Task      = StatusPort;
            newvolnode->dol_Name      = (BSTR) MKBPTR(((struct VolNode *)newvolnode)->vn_VolName);
            newvolnode->dol_misc.dol_volume.dol_VolumeDate    = *voldate;

               /* Insert volume node in DOS lists */   
            /* The version 39 autodoc specs on the AttemptLockDosList() function
                do not exactly match its operation.  The valid returns if the locking
                of the DosList fails is either 0 or 1.  This has been validated
                in  the dos.library 37.45 source code by Martin Taillefer.  It
                has been submitted as a bug report 10/10/92. */
            if( (ULONG)(dl = AttemptLockDosList(LDflags)) > 1 )
            {
                if(AddDosEntry((struct DosList *)volnode))
                {
                    fs->f_FS_special_flags &= ~(MF_ADD_VOL);  /* clear flag */
                    volnode_add=TRUE;   /* set flag that this volume node was added */
                }
                UnLockDosList(LDflags);
            }
        }    
    }    
    else
    {
        fs->f_FS_special_flags &= ~(MF_ADD_VOL);  /* clear flag */
    }

    /* fix prob w/ multiple disks with same volume name and date. */
    if(volnode)
    {
        if(volnode->dol_Task != StatusPort)
        {
            if( DVND_Req() )
            {
                fs->f_FS_special_flags |= MF_ADD_VOL;  /* set flag for delayed addition */
                    /* Keep same volume name but change date to current date. */
                if(RenameDisk(0,DOSTRUE))   /* Go serialize disk */
                {
                    volnode = AllocVolNode(volstr, voldate);
                }
            }
        }
        else
        {
            fs->f_VolumeNode = volnode;
            fs->f_LockList = volnode->dol_misc.dol_volume.dol_LockList;
            volnode->dol_misc.dol_volume.dol_LockList=0;
        }
    }

    if(fs->f_FS_special_flags & (MF_ADD_VOL))
    {   /* Set timer for delayed removal */
        fs->f_timer_start++;   /* set timer count to disk time-out */
    }
        /* Could not add volume node but volnode memory allocated. Free memory */
    if(newvolnode && (volnode_add==FALSE))
    {
        FreeMem((UBYTE *)newvolnode, sizeof(struct VolNode) );
    }
    return(volnode);
}


/**********************************************************************
*   MakeVolNode()  --- Make the specified Volume Node in the Dos list
**********************************************************************/
struct DosList *MakeVolNode()
{
    register struct FS_dir_ent  *dirent;

    /* Temporarily assign the device node as the volume node in the FS struct only */
    fsys->f_VolumeNode = (struct DosList *)fsys->f_DevNode;

    if(dirent = (struct FS_dir_ent *)FindVolEntry())
    {
                /* Get the Volume ID (make into BSTR) */
        ConvertVolumeName(dirent->fde_filename, fsys->f_sys_id);

                /* Get Volume Date. */
        ConvertFileDate( dirent->fde_time, &(fsys->f_vol_date));
    }

    fsys->f_FS_special_flags |= MF_ADD_VOL;  /* set flag for delayed addition */
    return(AllocVolNode(fsys->f_sys_id,&(fsys->f_vol_date)));
}
