/*** DPswitch.c ***************************************************************
** Copyright 1991 CONSULTRON
**
**  File:       DPswitch.c
**  Program:    DOS Packet Switch
**  Author:     Len Poma
*/

#include "FS:FS.h"

/*---------------------------------------------------------------------------
** References to system
*/

extern struct DCIReq    *dci;

extern struct DOSBase   *DOSBase;
extern struct IOStdReq  *diskreq;
extern struct Process   *StatusProc;
extern struct ExecBase  *SysBase;

int                     DOSerror;

extern struct FS           *fsys;
UCHAR   *Pbclevtug   = "Pbafhygeba";


extern struct Process      *Proc;
extern struct MsgPort      *DOSPort;

extern struct Library *JCCBase ;			/*  wc */

extern VOID __stdargs kprintf();	/* wc.  debug */

/**********************************************************************
*   FileSystem() --- performs initialization, replies to startup packet and
*       dispatches incoming request packets to the apropriate functions.
*       Main IO processing packet switcher.
**********************************************************************/
void  FileSystem()
{
    struct FileHandle   *fh;
    struct FileSysStartupMsg *Fssm;
    struct DosPacket    *pkt;
    register int        DOSreturn=DOSFALSE;
    struct MLock        *lock, *l; /* wc */
    ULONG               signals;
    UBYTE               init=0;
    UBYTE               die=0;
    UBYTE               noreply=0;
    UBYTE               event=FALSE;
    register struct Message  *msg;

    fsys->f_FSflags |= MF_NODISK;

#define ARG1    pkt->dp_Arg1
#define ARG2    pkt->dp_Arg2
#define ARG3    pkt->dp_Arg3
#define ARG4    pkt->dp_Arg4
#define ARG5    pkt->dp_Arg5
#define ARG6    pkt->dp_Arg6
#define ARG7    pkt->dp_Arg7


    WaitPort(DOSPort);
    msg = (struct Message *) GetMsg(DOSPort);
    fsys->f_pkt = pkt = (struct DosPacket *) msg->mn_Node.ln_Name;

/*** Initialization code for first packet received ****/
    Fssm = (struct FileSysStartupMsg *) BADDR(ARG2);
        /* passed FileSysStartupMsg */

/*** For reasons that may seem strange, This code has been added to support a
    very peculiar feature of the mfm.device driver.  As of 38.6 of mfm.device,
    it installs the appropriate TDPatch needed under KS 33 or 34.  To do so
    correctly, we can not assume that the patch will be installed.  If the user
    uses CrossDOS from the product disk only, then we must execute the TDPatch
    from the current directory.  If KS 2.0 or higher is used this code is run
    but is useless.  Backwards compatibility.  Isn't it GRAND! */

    Proc->pr_CurrentDir = ((struct Process *)pkt->dp_Port->mp_SigTask)->pr_CurrentDir;
DPRTF(KPrintF("\nCurrentDir = %lx  Task = %lx",Proc->pr_CurrentDir,((struct Process *)pkt->dp_Port->mp_SigTask)));

    if(InitDev(Fssm))
    {
        DOSreturn = ((struct DosEnvec *)BADDR(Fssm->fssm_Environ))->de_DosType;
        event = TRUE;
        init = 1;
    }
    else
    {
        DOSreturn = DOSFALSE;
        die = 1;
    }
    ReplyPkt(pkt,DOSreturn, DOSerror);

    /* Perform DiskChange() by setting the task's SIGF_DCI interrupt bit */
#define SIGF_DCI    fsys->f_dci_sig


    SetSignal(SIGF_DCI, SIGF_DCI);

/****
    HUGH Switch statement for ACTION_XXX packets
*****/
    while(!die || fsys->f_IOR_pend)
    {
      signals = Wait(SIGF_DOS | SIGF_DCI);

/**********************************************************************
*   Check if diskchange occurred since last ACTION
**********************************************************************/
      if(signals & SIGF_DCI)
      {
        signals &= ~(SIGF_DCI);
        fsys->f_pkt = 0;
        DiskChange();
      }

/**********************************************************************
*   Check for DOS Packet
**********************************************************************/
      if(signals & SIGF_DOS)
      {
        while(msg = (struct Message *) GetMsg(DOSPort))
        {
    /**********************************************************************
    *   Check if diskchange occurred since last ACTION
    **********************************************************************/
            if(signals & SIGF_DCI)
            {
                signals &= ~(SIGF_DCI);
                fsys->f_pkt = 0;
                DiskChange();
            }

/**********************/
/* Process Packet types. */

          if(pkt = (struct DosPacket *) msg->mn_Node.ln_Name)
          {
            fsys->f_pkt = pkt;
                /* Make sure the first packet type == ACTION_STARTUP */
            if( init == 0 ) pkt->dp_Type = ACTION_STARTUP;
            DOSreturn= DOSFALSE;
            DOSerror= ERROR_ACTION_NOT_KNOWN;

    /*** The 'Big Switch' ***/
            switch(pkt->dp_Type)
            {
            case ACTION_LOCATE_OBJECT:

DPRTF(KPrintF("\nACTION_LOCATE_OBJECT"));

                DOSreturn = MKBPTR( GetLock(BADDR(ARG1), BADDR(ARG2), ARG3));

DPRTF(KPrintF("\nARG2=%lx  ARG2[0]=%lx  ARG2[1]=%lx  ARG2[2]=%lx",
    ARG2,((UBYTE *)BADDR(ARG2))[0],((UBYTE *)BADDR(ARG2))[1],((UBYTE *)BADDR(ARG2))[2]));
DPRTF(KPrintF("\n\tLock = %lx  Name = %s  Mode = %ld  Result(Lock) = %lxH  Result2 = %ld ",
    BADDR(ARG1), &(((UBYTE *)BADDR(ARG2))[1]), (ARG3), BADDR(DOSreturn), DOSerror));

                break;

            case ACTION_EXAMINE_OBJECT:

DPRTF(KPrintF("\nACTION_EXAMINE_OBJECT"));

                DOSreturn = ExamineObject( BADDR(ARG1),BADDR(ARG2));

DPRTF(KPrintF("\n\tLock = %lxH  FIB = %lxH   Result = %ld",
    BADDR(ARG1), BADDR(ARG2), DOSreturn));

                break;

            case ACTION_EXAMINE_FH:

DPRTF(KPrintF("\nACTION_EXAMINE_FH"));

                DOSreturn = ExamineObject( (struct MLock *)ARG1,BADDR(ARG2));

DPRTF(KPrintF("\n\tLock = %lxH  FIB = %lxH   Result = %ld",
    ARG1, BADDR(ARG2), DOSreturn));

                break;

            case ACTION_EXAMINE_NEXT:

DPRTF(KPrintF("\nACTION_EXAMINE_NEXT"));

                DOSreturn = ExamineNext( BADDR(ARG1),BADDR(ARG2));

DPRTF(KPrintF("\n\tLock = %lxH  FIB = %lxH   Result = %ld Result2 = %ld",
    BADDR(ARG1), BADDR(ARG2), DOSreturn, DOSerror));
DPRTF(KPrintF("\n\tFileName= %s",&((struct MFileInfoBlock *)BADDR(ARG2))->mfib_FileName[1]));

                break;

            case ACTION_PARENT:

DPRTF(KPrintF("\nACTION_PARENT"));

                DOSreturn = MKBPTR( Parent( BADDR(ARG1)));

DPRTF(KPrintF("\n\tLock = %lxH  Result(ParentLock) = %lxH  Result2 = %ld",
    BADDR(ARG1), BADDR( DOSreturn), DOSerror));

                break;

            case ACTION_PARENT_FH:

DPRTF(KPrintF("\nACTION_PARENT_FH"));

                DOSreturn = MKBPTR( Parent( (struct MLock *)ARG1));

DPRTF(KPrintF("\n\tLock = %lxH  Result(ParentLock) = %lxH  Result2 = %ld",
    ARG1, BADDR( DOSreturn), DOSerror));

                break;

            case ACTION_FREE_LOCK:

DPRTF(KPrintF("\nACTION_FREE_LOCK"));

                DOSreturn= FreeLock( BADDR(ARG1));

DPRTF(KPrintF("\n\tLock = %lxH  Result = %ld", BADDR(ARG1), DOSreturn));

                break;


            case ACTION_SAME_LOCK:

DPRTF(KPrintF("\nACTION_SAME_LOCK"));

                if(((struct FileLock *)BADDR(ARG1))->fl_Volume != ((struct FileLock *)BADDR(ARG2))->fl_Volume)
                {   /* Locks not the same volume */
                    DOSerror = ERROR_INVALID_LOCK;
                }
                else if(((struct FileLock *)BADDR(ARG1))->fl_Key == ((struct FileLock *)BADDR(ARG2))->fl_Key)
                {   /* Locks the same */
                    DOSreturn = DOSTRUE;   /* Locks the same */
                }

DPRTF(KPrintF("\n\tLock1 = %lxH  Lock2 = %lxH  Result = %ld", BADDR(ARG1), BADDR(ARG2), DOSreturn));

DOSerror = 0;
                break;


            case ACTION_READ:

DPRTF(KPrintF("\nACTION_READ"));
                DOSreturn = ReadData((struct MLock *)ARG1, (UBYTE *)ARG2, ARG3 );

DPRTF(KPrintF("\n\tFileLock = %lxH  Buffer = %lxH  length = %ld  Result = %ld",
    ARG1, ARG2, ARG3, DOSreturn));

                break;

            case ACTION_WRITE:

DPRTF(KPrintF("\nACTION_WRITE"));

                DOSreturn = WriteData(lock = (struct MLock *)ARG1, (UBYTE *)ARG2, ARG3 );

DPRTF(KPrintF("\n\tFileLock = %lxH  Buffer = %lxH  length = %ld  Result1 = %ld  Result2 = %ld",
    ARG1, ARG2, ARG3, DOSreturn, DOSerror));

                break;

            case ACTION_FINDINPUT:

DPRTF(KPrintF("\nACTION_FINDINPUT"));

                goto OpenFile;

            case ACTION_FINDUPDATE:

DPRTF(KPrintF("\nACTION_FINDUPDATE"));

                goto OpenFile;

            case ACTION_FINDOUTPUT:

DPRTF(KPrintF("\nACTION_FINDOUTPUT"));

OpenFile:
                DOSreturn = OpenFile((fh = (struct FileHandle *)BADDR(ARG1)),
                    BADDR(ARG2), BADDR(ARG3), pkt->dp_Type);

DPRTF(KPrintF("\n\tFileHandle = %lxH  Lock = %lxH  Name = %s  Result = %ld  Result2 = %ld  Newlock = %lx",
    fh, BADDR(ARG2), &(((UBYTE *)BADDR(ARG3))[1]), DOSreturn, DOSerror, fh->fh_Arg1));

                break;

            case ACTION_END:

DPRTF(KPrintF("\nACTION_END"));

                DOSreturn= CloseFile((struct MLock *)ARG1);

DPRTF(KPrintF("\n\tFileLock = %lxH  Result = %ld",
    ARG1, DOSreturn));

                break;

            case ACTION_SEEK:

DPRTF(KPrintF("\nACTION_SEEK"));
                DOSreturn = SeekFilePos((struct MLock *)ARG1, ARG2, ARG3);

DPRTF(KPrintF("\n\tFileLock = %lxH  Offset = %ld  Mode = %ld  Result( Old position ) = %ld",
    ARG1, ARG2, ARG3, DOSreturn));

                break;

            case ACTION_COPY_DIR:

DPRTF(KPrintF("\nACTION_COPY_DIR"));

//                DOSreturn= MKBPTR( CopyLock( BADDR(ARG1)));
                DOSreturn= MKBPTR( l = CopyLock( BADDR(ARG1)));

DPRTF(KPrintF("\n\tLock = %lxH  Result(NewLock) = %lxH",
    BADDR(ARG1), BADDR(DOSreturn)));
		l->ml_lock_ext.le_ReadHalf = FALSE;	/* wc */
		l->ml_lock_ext.le_WrtHalf = FALSE;	/* wc */
//		l->ml_lock_ext.le_HalfJIS2 = FALSE;	/* wc */
		l->ml_lock_ext.le_jcc_handle1 = AllocConversionHandle(TAG_DONE);  /* wc */
		l->ml_lock_ext.le_jcc_handle2 = AllocConversionHandle(TAG_DONE);  /* wc */
//kprintf ("alloc conversion handle %lx\n", l->ml_lock_ext.le_jcc_handle1);
//kprintf ("alloc conversion handle %lx\n", l->ml_lock_ext.le_jcc_handle2);
//kprintf ("dpswitch copy lock\n");

                break;

            case ACTION_DELETE_OBJECT:

DPRTF(KPrintF("\nACTION_DELETE_OBJECT"));

                DOSreturn= DeleteObject( BADDR(ARG1),BADDR(ARG2));

DPRTF(KPrintF("\n\tLock = %lxH  Name = %b  Result1 = %ld  Result2 = %ld",
    BADDR(ARG1), ARG2, (DOSreturn), DOSerror));

                break;

            case ACTION_CREATE_DIR:

DPRTF(KPrintF("\nACTION_CREATE_DIR"));

                DOSreturn= MKBPTR(CreateDirectory(BADDR(ARG1), BADDR(ARG2)));

DPRTF(KPrintF("\n\tLock = %lxH  Name = %b  Result1(Lock) = %lxH  Result2 = %ld",
    BADDR(ARG1), ARG2, BADDR(DOSreturn), DOSerror));

                break;

            case ACTION_RENAME_OBJECT:

DPRTF(KPrintF("\nACTION_RENAME_OBJECT"));

                DOSreturn= RenameObject(BADDR(ARG1), BADDR(ARG2),BADDR(ARG3),BADDR(ARG4));

DPRTF(KPrintF("\n\tFromLock = %lxH  FromName = %b  ToLock = %lxH  ToName = %b   Result1 = %ld  Result2 = %ld",
    BADDR(ARG1), ARG2, BADDR(ARG3), ARG4, DOSreturn, DOSerror));

                break;

            case ACTION_RENAME_DISK:

DPRTF(KPrintF("\nACTION_RENAME_DISK"));

                DOSreturn= RenameDisk(BADDR(ARG1),DOSFALSE);

DPRTF(KPrintF("\n\tNewName = %b  Result1 = %ld  Result2 = %ld",
    ARG1, DOSreturn, DOSerror));

                break;

            case ACTION_SET_DATE:

DPRTF(KPrintF("\nACTION_SET_DATE"));

                DOSreturn= SetDate((struct MLock *)BADDR(ARG2),(UCHAR *)BADDR(ARG3),(struct DateStamp *)ARG4);

DPRTF(KPrintF("\n\tDirLock = %lxH  Name = %b  DateStamp = %lxH  Result = %ld",
    BADDR(ARG2), ARG3, ARG4, DOSreturn));

                break;

            case ACTION_SET_PROTECT:

DPRTF(KPrintF("\nACTION_SET_PROTECT"));

                DOSreturn= SetProtect(BADDR(ARG2), BADDR(ARG3), ARG4);

DPRTF(KPrintF("\n\tDirLock = %lxH  Name = %b  Attributes = %lxH  Result = %ld  Result2 = %ld",
    BADDR(ARG2), ARG3, ARG4, DOSreturn, DOSerror));

                break;

            case ACTION_SET_FILE_SIZE:

DPRTF(KPrintF("\nACTION_SET_FILE_SIZE"));
    /* Fixed problem with ARG1 actually being fh->dp_Arg1 instead of fh.
    The Amiga Mail Volume II Page II-9 needs to be corrected to reflect fh->dp_Arg1 */
                DOSreturn = Set_File_Size((struct MLock *)ARG1,ARG2,ARG3);

DPRTF(KPrintF("\n\tLock = %lxH  Offset = %ld  Mode = %ld  Result = %ld",
    ((struct FileHandle *)BADDR(ARG1))->fh_Arg1,ARG2,ARG3, DOSreturn));

                break;


            case ACTION_MORE_CACHE:

DPRTF(KPrintF("\nACTION_MORE_CACHE"));

                DOSreturn = More_Cache(ARG1, fsys->f_blocks_clust<<fsys->f_bytes_block_sh);

DPRTF(KPrintF("\n\t# Added Buffers = %ld  Result(Total Buffers) = %ld",
    ARG1, DOSreturn));
                break;

            case ACTION_FLUSH:

DPRTF(KPrintF("\nACTION_FLUSH"));

                Flush_Buffers();
                DOSreturn= DOSTRUE;
                break;

            case ACTION_FH_FROM_LOCK:

DPRTF(KPrintF("\nACTION_FH_FROM_LOCK"));

                DOSreturn = OpenFile((fh = (struct FileHandle *)BADDR(ARG1)),
                    BADDR(ARG2), BADDR(ARG3), 0);

DPRTF(KPrintF("\n\tFileHandle = %lxH  Lock = %lxH  Name = %s  Result = %ld  Result2 = %ld  Newlock = %lx",
    fh, BADDR(ARG2), &(((UBYTE *)BADDR(ARG3))[1]), DOSreturn, DOSerror, fh->fh_Arg1));

                break;

            case ACTION_CHANGE_MODE:

DPRTF(KPrintF("\nACTION_CHANGE_MODE"));

                DOSreturn = Change_Mode(ARG1, (LONG)BADDR(ARG2), ARG3);

DPRTF(KPrintF("\n\tType = %lxH  Object = %lxH  Mode = %ld  Result = %ld  Result2 = %ld",
    ARG1, BADDR(ARG2), ARG3, DOSreturn, DOSerror));

                break;

            case ACTION_COPY_DIR_FH:

DPRTF(KPrintF("\nACTION_COPY_DIR_FH"));

                DOSreturn= MKBPTR( l = CopyLock( (struct MLock *)ARG1));

DPRTF(KPrintF("\n\tLock = %lxH  Result(NewLock) = %lxH",
    ARG1, BADDR(DOSreturn)));
		l->ml_lock_ext.le_ReadHalf = FALSE;	/* wc */
		l->ml_lock_ext.le_WrtHalf = FALSE;	/* wc */
//		l->ml_lock_ext.le_HalfJIS2 = FALSE;	/* wc */
		l->ml_lock_ext.le_jcc_handle1 = AllocConversionHandle(TAG_DONE);  /* wc */
		l->ml_lock_ext.le_jcc_handle2 = AllocConversionHandle(TAG_DONE);  /* wc */
//kprintf ("alloc conversion handle %lx\n", l->ml_lock_ext.le_jcc_handle1);
//kprintf ("alloc conversion handle %lx\n", l->ml_lock_ext.le_jcc_handle2);
//kprintf ("dpswitch copy lock\n");

                break;

            case ACTION_FORMAT:

DPRTF(KPrintF("\nACTION_FORMAT"));

                if((Find_File_Mod(fsys->f_LockList) == DOSFALSE))   /* check if any files open for writing */
                {
                    DOSreturn = FormatDisk(BADDR(ARG1),ARG2);
                }

DPRTF(KPrintF("\n\tDiskName = %b  FormatType = %.4s  Result = %ld",
    ARG1, &ARG2, DOSreturn));

                break;

            case ACTION_INHIBIT:

DPRTF(KPrintF("\nACTION_INHIBIT"));

                DOSerror = ERROR_OBJECT_IN_USE;

                if(ARG1 == DOSTRUE)         /* assert INHIBIT */
                {
                    if((fsys->f_Diskflags & WRITE_PROT_F)  /* if write protected */
                        || (Find_File_Mod(fsys->f_LockList) == DOSFALSE))   /* or any file writes pending */
                    {
                        (fsys->f_inhibit_cnt)++;  /* increment INHIBIT nesting count */
                        RemDisk();
                        DOSreturn= DOSTRUE;
                    }
                }
                 else
                {                            /* release INHIBIT */
                    (fsys->f_inhibit_cnt)--;  /* decrement INHIBIT nesting count */
                    if(fsys->f_inhibit_cnt < 0)
                    {
                        fsys->f_inhibit_cnt = 0;
                    }
                    else if(fsys->f_inhibit_cnt == 0)
                    {
                        DiskChange();
                    }
                    DOSreturn= DOSTRUE;
               }

DPRTF(KPrintF("\n\tState = %ld  INHIBIT = %ld",
    ARG1,fsys->f_inhibit_cnt));

                break;

            case ACTION_DIE:
                DOSreturn= DOSTRUE;
                die = 1;

DPRTF(KPrintF("\nACTION_DIE"));
                break;

#ifdef MFS
            case ACTION_EVENT:
                DOSreturn = DOSTRUE;
                event = TRUE;
                break;
#endif

            case ACTION_TIMER:
                (fsys->f_IOR_pend)--;  /* decrement IORequest pending */

                if(fsys->f_timer_start)
                {
                    fsys->f_timer_cnt = fsys->f_timer_start;
                    fsys->f_timer_start = 0;   /* set timer to start */
                }

                if( --(fsys->f_timer_cnt) == 0 )
                {
                    Flush_Buffers();
                    Motor_Off();
                }
                  else if( (fsys->f_timer_cnt) < 0 )
                {
                    fsys->f_timer_cnt = 0;    /* reset timer count */
                }
                  else if( !die )
                {
                /* Start timer again */
                      fsys->f_timer->tmr_req.tr_time.tv_secs = 1;
                      fsys->f_timer->tmr_req.tr_time.tv_micro = 0;
                    SendIO((struct IORequest *)fsys->f_timer);
                    (fsys->f_IOR_pend)++;  /* increment IORequest pending */
                }

                if(fsys->f_FS_special_flags & MF_ADD_VOL)
                {       /* Attempt to add the volume node by delayed addition */
DPRTF(KPrintF("\n\tAttempt to AllocVolNode"));
                    AllocVolNode(fsys->f_sys_id,&(fsys->f_vol_date));
                }

                if(fsys->f_FS_special_flags & MF_REM_VOL)
                {       /* Attempt to delete the volume node by delayed expunge */
DPRTF(KPrintF("\n\tAttempt to DeleteVolNode"));
                    DeleteVolNode(0);
                }

                noreply = 1;      /* do not reply packet */
                break;

            case ACTION_SERIALIZE_DISK:

DPRTF(KPrintF("\nACTION_SERIALIZE_DISK"));
        /**** Set Volume Date to current date ONLY ****/
                if(fsys->f_inhibit_cnt)
                {   /* Device inhibited, partially revalidated disk */
                    Is_Disk_Out();
                    if(0 == InitDisk())
                    {
                        fsys->f_FSflags |= MF_FSDISK;         /* set flag -- FS disk */
                    }
                }
                DOSreturn = RenameDisk(0,DOSTRUE);
                Flush_Buffers();
                break;

            case ACTION_WRITE_PROTECT:
DPRTF(KPrintF("\nACTION_WRITE_PROTECT"));
                DOSerror = ERROR_DISK_WRITE_PROTECTED;
                if( (ARG1 == DOSTRUE) && !(fsys->f_Diskflags & WRITE_PROT_F))
                {       /* Write protect to be set */
                    fsys->f_wp_passkey = ARG2;
                    fsys->f_FSflags |= MF_WP_DEVICE;
                    fsys->f_Diskflags |= WRITE_PROT_F;
                    DOSreturn = DOSTRUE;
                    DOSerror = 0;
                }
                if( (ARG1 == DOSFALSE)
                    && ((fsys->f_wp_passkey == 0)       /* check if pass key not set */
                    || (fsys->f_wp_passkey == ARG2)) )  /* or check against pass key */
                {       /* Write protect to be cleared */
                    fsys->f_wp_passkey = 0;
                    fsys->f_FSflags &= ~(MF_WP_DEVICE);
                    if(!DevCmd(TD_PROTSTATUS)
                        && ((fsys->f_FSflags & MF_NODISK)    /* check if no disk */
                        || !(diskreq->io_Actual)))   /* check if disk actually write-protected */
                    {
                        fsys->f_Diskflags &= ~(WRITE_PROT_F);
                        DOSreturn = DOSTRUE;
                        DOSerror = 0;
                    }
                }

DPRTF(KPrintF("\n\tState = %ld  passkey = %lx", ARG1,ARG2));
                break;

            default:

DPRTF(KPrintF("\nACTION = %ld",pkt->dp_Type));
DPRTF(KPrintF("\n%lxH",ARG1));
DPRTF(KPrintF("  %lxH",ARG2));
DPRTF(KPrintF("  %lxH",ARG3));
DPRTF(KPrintF("  %lxH",ARG4));
DPRTF(KPrintF("\n%lxH",DOSreturn));
DPRTF(KPrintF("  %ld\n",DOSerror));

                break;
            }

            if( !noreply )
            {
                ReplyPkt(pkt,DOSreturn, DOSerror);
            }
              else noreply = 0;    /* clear flag */
          }
          else
          {        /* no message recvd but not a DOS packet.  ReplyMsg if not this task */
            if(msg->mn_ReplyPort != DOSPort) ReplyMsg(msg);
          }
        }
      }
#ifdef MFS
/**********************************************************************
*   Check the semaphore
**********************************************************************/
      if(event)
      {
          event = FALSE;
          Check_Sema4();        /* Check the sema4 for changes */
      }
#endif
    /* Start Flush Timer if FS modified and timer not already active */
      if( (fsys->f_timer_start > 0 ) && (fsys->f_timer_cnt == 0 ) && !die )
      {
        /* Start Timer */
          fsys->f_timer_cnt = fsys->f_timer_start;
          fsys->f_timer_start = 0;   /* set timer to start */
            fsys->f_timer->tmr_req.tr_time.tv_secs = 1;
            fsys->f_timer->tmr_req.tr_time.tv_micro = 0;
          SendIO((struct IORequest *)fsys->f_timer);
          (fsys->f_IOR_pend)++;  /* increment IORequest pending */
      }
    }

/**********************************************************************
*   QUIT the File System!
**********************************************************************/
QUIT:
    if(fsys)
    {
        if(!((fsys->f_FSflags) & MF_NODISK))
        {
         Motor_Off();
        }

            /*  Return packets not used */
        while(msg = (struct Message *) GetMsg(DOSPort))
        {
            if(pkt = (struct DosPacket *) msg->mn_Node.ln_Name)
            {
                switch(pkt->dp_Type)
                {
                case ACTION_TIMER:
                    break;
                default:
                    ReplyPkt(pkt,DOSFALSE,ERROR_ACTION_NOT_KNOWN);
                    break;
                }
            }
        }
        RemDisk();
        RemDev();
        Signal((struct Task *)StatusProc, SIGF_KILLPARENT);  /* signal parent to die */
    }
}
