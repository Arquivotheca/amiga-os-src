
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <exec/io.h>
#include <devices/printer.h>
#include <devices/timer.h>
#include <devices/parallel.h>
#include <devices/serial.h>
#include <envoy/nipc.h>
#include <envoy/services.h>
#include "lpdbase.h"
#include <libraries/iffparse.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/accounts_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include <utility/tagitem.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/accounts_protos.h>
#include <clib/iffparse_protos.h>

#include "string.h"

#define LPCMD_START_PRT 1
#define LPCMD_START_PAR 2
#define LPCMD_START_SER 3
#define LPCMD_DATA      4
#define LPCMD_END       5

#define LP_IDLE         1
#define LP_PRINTING     2
#define LP_PAUSED       3
#define LP_SLEEPING     4
#define LP_ERROR        255

struct PrinterJob
{
    struct MinNode pj_Node;
    BPTR           pj_File;
    ULONG          pj_Type;
    ULONG          pj_Status;
    ULONG          pj_JobNum;
    ULONG          pj_Size;
    UBYTE          pj_JobName[32];
    UBYTE          pj_FileName[256];
};

extern STRPTR       LPDUser;
extern STRPTR       LPDPassword;
extern STRPTR       LPDEntityName;
extern ULONG        LPDSignalMask;
extern ULONG        LPDError;
extern struct Task *LPDSMProc;

extern __far LPDServer(void);

struct ConfigUser
{
    UWORD   cu_ID;
    UBYTE   cu_Flags;
    UBYTE   cu_Filler;
};

struct UserData
{
    struct Node ud_Node;
    UWORD       ud_ID;
    UBYTE       ud_Flags;
    UBYTE       ud_Filler;
};

#define UDF_GROUP   1

#define ID_EPRT     MAKE_ID('E','P','R','T')
#define ID_USER     MAKE_ID('U','S','E','R')

#define IFFPrefChunkCnt     1

static LONG far IffPrefChunks[]={ID_EPRT,ID_USER};

BOOL LoadConfig(void)
{
    struct ConfigUser *cu;
    struct Library *IB;

    cu = (struct ConfigUser *) AllocMem(sizeof(struct ConfigUser),MEMF_PUBLIC);
    if (cu)
    {
        IB = (struct Library *) OpenLibrary("iffparse.library",37L);
        IFFParseBase = IB;
        if (IFFParseBase)
        {
            struct IFFHandle *iff;
            iff = (struct IFFHandle *) AllocIFF();
            if (iff)
            {
                iff->iff_Stream = (ULONG) Open("env:envoy/printerexport.config",MODE_OLDFILE);
                if (!iff->iff_Stream)
                    iff->iff_Stream = (ULONG) Open("envarc:envoy/printerexport.config",MODE_OLDFILE);
                if (iff->iff_Stream)
                {
                    InitIFFasDOS(iff);
                    if (!OpenIFF(iff,IFFF_READ))
                    {
                        if (!StopChunk(iff,ID_EPRT,ID_USER))
                        {
                            if (!ParseIFF(iff,IFFPARSE_SCAN))
                            {
                                while (TRUE)
                                {
                                    struct ContextNode *cn;
                                    cn = (struct ContextNode *) CurrentChunk(iff);
                                    if (cn)
                                    {
                                        if (ReadChunkBytes(iff,cu,sizeof(struct ConfigUser)) == sizeof(struct ConfigUser))
                                        {
                                            struct UserData *ud;
                                            ud = (struct UserData *) AllocMem(sizeof(struct UserData),MEMF_PUBLIC);
                                            if (ud)
                                            {
                                                struct List *l=&LPDBase->LPD_Users;
                                                ud->ud_ID = cu->cu_ID;
                                                ud->ud_Flags = cu->cu_Flags;
                                                AddTail(l,&ud->ud_Node);
                                            }
                                        }
                                        if (ParseIFF(iff,IFFPARSE_SCAN))
                                            break;
                                    }
                                }
                            }
                        }
                        CloseIFF(iff);
                    }
                    Close(iff->iff_Stream);
                }
                FreeIFF(iff);
            }
            CloseLibrary(IB);
        }
        FreeMem(cu,sizeof(struct ConfigUser));
    }
    return(TRUE);
}




ULONG ASM StartService(REG(a0) struct TagItem *taglist)
{
    STRPTR userName;
    STRPTR password;
    STRPTR entityName;
    ULONG cptags[5];
    struct Process *lpdproc;
    BOOL status = FALSE;
    UBYTE  sigbit;
    struct TagItem *tstate;
    struct TagItem *tag;
    struct UserData *ud;
    UWORD uid, gid;
    struct UserInfo *ui;
    struct GroupInfo *gi;
    BOOL gooduser = FALSE;

    tstate = taglist;
    while (tag = NextTagItem(&tstate))
    {
        switch(tag->ti_Tag)
        {
            case SSVC_EntityName:
                entityName = (STRPTR) tag->ti_Data;
                break;
            case SSVC_UserName:
                userName = (STRPTR) tag->ti_Data;
                break;
            case SSVC_Password:
                password = (STRPTR) tag->ti_Data;
                break;
        }
    }

    ui = AllocUserInfo();
    if (ui)
    {
        if (!VerifyUser(userName,password,ui))
        {
            uid = ui->ui_UserID;
            gid = ui->ui_PrimaryGroupID;
            ud = (struct UserData *) LPDBase->LPD_Users.lh_Head;
            while (ud->ud_Node.ln_Succ)
            {
                if (ud->ud_Flags)   /* Groups */
                {
                    gi = AllocGroupInfo();
                    if (gi)
                    {
                        if (!IDToGroup(ud->ud_ID,gi))
                        {
                            if (!MemberOf(gi,ui))
                            {
                                gooduser = TRUE;
                                break;
                            }
                        }
                        FreeGroupInfo(gi);
                    }
                }
                else                /* Users */
                {
                    if (ud->ud_ID == uid)
                    {
                        gooduser = TRUE;
                        break;
                    }

                }
                ud = (struct UserData *) ud->ud_Node.ln_Succ;
            }
        }
        FreeUserInfo(ui);
    }
    if (!gooduser)
        return(ACCERROR_NOPRIVS);

    if(!LPDBase->LPD_Entity)
    {
        if(sigbit = AllocSignal(-1L))
        {
            LPDSMProc = FindTask(0L);
            LPDSignalMask = (1L<<sigbit);

            cptags[0] = NP_Entry;
            cptags[1] = (ULONG) LPDServer;
            cptags[2] = NP_Name;
            cptags[3] = (ULONG) "Envoy Print Spooler";
            cptags[4] = TAG_DONE;

            LPDUser = userName;
            LPDPassword = password;
            LPDEntityName = entityName;

            if(lpdproc = CreateNewProc((struct TagItem *)cptags))
            {
                Wait(1L<<sigbit);
                status = TRUE;
            }
            FreeSignal(sigbit);
        }
    }
    else
    {
        LPDError = 0;
        strcpy(entityName,"Print Spooler");
    }
    return LPDError;
}

VOID ASM GetServiceAttrsA(REG(a0) struct TagItem *tagList)
{
    struct TagItem *ti;

    if(ti=FindTagItem( SVCAttrs_Name, tagList))
    {
        strcpy((STRPTR)ti->ti_Data,"Print Spooler");
    }
}

VOID ASM Server(REG(a0) STRPTR userName,
                REG(a1) STRPTR password,
                REG(a2) STRPTR EntityName)
{
    struct Library *SvcBase;
    struct LPDSvc *lb = LPDBase;
    ULONG nextfilenumber,signals,die,dead;
    ULONG status,loop,waitmask;
    ULONG StartupError = 0;
    ULONG ent_sigbit;
    struct MsgPort *prt_port,*timer_port;
    BOOL printerdevopen=FALSE;

    struct PrinterJob *inc_job,*prt_job = NULL;

    UBYTE *prt_buffer;

    struct IOStdReq *prt_io;
    struct timerequest *timer_io;

    void *entity;
    struct Transaction *trans;

    ULONG cetags[8]={ENT_Name, 0, ENT_Public, TRUE, ENT_AllocSignal, 0, TAG_END, 0};

    geta4();
    nextfilenumber = 1;
    die = FALSE;
    dead = FALSE;
    ent_sigbit = 0;
    cetags[1] = (ULONG) "Print Spooler";
    cetags[5] = (ULONG) &ent_sigbit;

    StartupError = -1;
    if (SvcBase = OpenLibrary("printspool.service", 0L))
    {
        if (entity = CreateEntityA((struct TagItem *) cetags))
        {
            if(prt_buffer = AllocMem(1024,MEMF_CLEAR));
            {
                if(prt_port = CreateMsgPort())
                {
                    if(timer_port = CreateMsgPort())
                    {
                        if(timer_io = (struct timerequest *) CreateIORequest(timer_port,sizeof(struct timerequest)))
                        {
                            if(prt_io = (struct IOStdReq *) CreateIORequest(prt_port,sizeof(struct IOExtPar)))
                            {
                                LPDBase->LPD_Entity = entity;

                                strcpy(EntityName,"Print Spooler");

                                LPDError = StartupError = 0;

                                Signal(LPDSMProc, LPDSignalMask);

                                waitmask = (1<<prt_port->mp_SigBit) | (1<<ent_sigbit) | (1<<timer_port->mp_SigBit) | SIGBREAKF_CTRL_F;

                                status = LP_IDLE;

                                while(TRUE)
                                {
                                    signals = Wait(waitmask);

                                  /*  if(signals & SIGBREAKF_CTRL_F)
                                        die = TRUE;
                                    */
                                    loop = TRUE;

                                    while(loop)
                                    {
                                        loop = FALSE;
                                        if(trans = GetTransaction(entity))
                                        {
                                            loop = TRUE;
                                            switch(trans->trans_Command)
                                            {
                                                case LPCMD_START_PRT:
                                                case LPCMD_START_SER:
                                                case LPCMD_START_PAR:   if(inc_job = AllocMem(sizeof(struct PrinterJob),MEMF_CLEAR))
                                                                        {
                                                                            sprintf(inc_job->pj_JobName,"t:job%ld",nextfilenumber);
                                                                            nextfilenumber++;
                                                                            if(inc_job->pj_File = Open(inc_job->pj_JobName,MODE_NEWFILE))
                                                                            {
                                                                                inc_job->pj_Type = trans->trans_Command;
                                                                                inc_job->pj_File = inc_job->pj_File;
                                                                               /* strcpy(inc_job->pj_FileName,trans->trans_RequestData); */
                                                                                *(ULONG *)(trans->trans_ResponseData) = (ULONG) inc_job;
                                                                                trans->trans_RespDataActual = 4;
                                                                                trans->trans_Error = 0;
                                                                                AddTail((struct List *)&lb->LPD_Incoming,(struct Node *)inc_job);
                                                                            }
                                                                            else
                                                                            {
                                                                                trans->trans_Error = 2;
                                                                                trans->trans_RespDataActual = 0;
                                                                                FreeMem(inc_job,sizeof(struct PrinterJob));
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            trans->trans_Error = 1;
                                                                            trans->trans_RespDataActual = 0;
                                                                        }
                                                                        ReplyTransaction(trans);
                                                                        break;

                                                case LPCMD_DATA:        inc_job = (struct PrinterJob *) *(ULONG *)(trans->trans_RequestData);
                                                                        FWrite(inc_job->pj_File,(UBYTE *)((UBYTE *)trans->trans_RequestData + 4),1,(trans->trans_ReqDataActual - 4));
                                                                        ReplyTransaction(trans);
                                                                        break;

                                                case LPCMD_END:         inc_job = (struct PrinterJob *) *(ULONG *)(trans->trans_RequestData);
                                                                        Close(inc_job->pj_File);
                                                                        ReplyTransaction(trans);
                                                                        Remove((struct Node *)inc_job);
                                                                        AddTail((struct List *)&lb->LPD_Spool,(struct Node *)inc_job);
                                                                        break;
                                            }
                                        }
                                        if(GetMsg(prt_port))
                                        {
                                            loop = TRUE;
                                            if(prt_io->io_Length = FRead(prt_job->pj_File,prt_buffer,1,1024))
                                            {
                                                SendIO((struct IORequest *)prt_io);
                                            }
                                            else
                                            {
                                                Close(prt_job->pj_File);
                                                DeleteFile((STRPTR)prt_job->pj_JobName);
                                                FreeMem(prt_job,sizeof(struct PrinterJob));
                                                if (printerdevopen)
                                                {
                                                    CloseDevice((struct IORequest *)prt_io);
                                                    printerdevopen = FALSE;
                                                }
                                                status = LP_IDLE;
                                                prt_job = NULL;
                                            }
                                        }
                                        if(GetMsg(timer_port))
                                        {
                                            CloseDevice((struct IORequest *)timer_io);
                                            timer_io->tr_node.io_Device = NULL;
                                            status = LP_IDLE;
                                        }
                                        if(status == LP_IDLE)
                                        {
                                            if(prt_job = (struct PrinterJob *) RemHead((struct List *)&lb->LPD_Spool))
                                            {
                                                loop = TRUE;
                                                status = LP_PRINTING;
                                                if(prt_job->pj_Type == LPCMD_START_PRT)
                                                {
                                                    if(!OpenDevice("printer.device",0L,(struct IORequest *)prt_io,0))
                                                    {
                                                        prt_io->io_Command = PRD_RAWWRITE;
                                                        printerdevopen = TRUE;
                                                    }
                                                    else
                                                    {
                                                        status = LP_SLEEPING;
                                                    }
                                                }
                                                else if(prt_job->pj_Type == LPCMD_START_SER)
                                                {
                                                    status = LP_IDLE;
                                                    prt_job = NULL;
                                                    DeleteFile((STRPTR)prt_job->pj_JobName);
                                                    FreeMem(prt_job,sizeof(struct PrinterJob));
                                                }
                                                else if(prt_job->pj_Type == LPCMD_START_PAR)
                                                {
                                                    if(!OpenDevice("parallel.device",0L,(struct IORequest *)prt_io,0))
                                                    {
                                                        prt_io->io_Command = CMD_WRITE;
                                                        printerdevopen = TRUE;
                                                    }
                                                    else
                                                    {
                                                        status = LP_SLEEPING;
                                                    }
                                                }
                                                if (status == LP_PRINTING)
                                                {
                                                    prt_io->io_Data = prt_buffer;

                                                    if(prt_job->pj_File = Open(prt_job->pj_JobName,MODE_OLDFILE))
                                                    {
                                                        if(prt_io->io_Length = FRead(prt_job->pj_File,prt_buffer,1,1024))
                                                        {
                                                            SendIO((struct IORequest *)prt_io);
                                                        }
                                                        else
                                                        {
                                                            loop = TRUE;
                                                            Close(prt_job->pj_File);
                                                            DeleteFile((STRPTR)prt_job->pj_JobName);
                                                            FreeMem(prt_job,sizeof(struct PrinterJob));
                                                            if (printerdevopen)
                                                            {
                                                                CloseDevice((struct IORequest *)prt_io);
                                                                printerdevopen = FALSE;
                                                            }
                                                            status = LP_IDLE;
                                                            prt_job = NULL;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        loop = TRUE;
                                                        DeleteFile((STRPTR)prt_job->pj_JobName);
                                                        FreeMem(prt_job,sizeof(struct PrinterJob));
                                                        if (printerdevopen)
                                                        {
                                                            CloseDevice((struct IORequest *)prt_io);
                                                            printerdevopen = FALSE;
                                                        }
                                                        status = LP_IDLE;
                                                        prt_job = NULL;
                                                    }
                                                }
                                                else if(status == LP_SLEEPING)
                                                {
                                                    AddHead((struct List *)&lb->LPD_Spool,(struct Node *)prt_job);
                                                    status = LP_SLEEPING;
                                                    OpenDevice("timer.device",UNIT_VBLANK,(struct IORequest *)timer_io,0);
                                                    timer_io->tr_node.io_Command = TR_ADDREQUEST;
                                                    timer_io->tr_time.tv_secs = 30;
                                                    timer_io->tr_time.tv_micro = 0;
                                                    SendIO((struct IORequest *)timer_io);
                                                }
                                            }
                                        }
                                        if((status == LP_IDLE) && IsListEmpty((struct List *)&lb->LPD_Incoming))
                                        {
                                            LPDBase->LPD_Entity = NULL;
                                            dead = TRUE;
                                            break;
                                        }
                                    }
                                    if(dead)
                                        break;
                                }
                                DeleteIORequest(prt_io);
                            }
                            DeleteIORequest(timer_io);
                        }
                        DeleteMsgPort(timer_port);
                    }
                    DeleteMsgPort(prt_port);
                }
                FreeMem(prt_buffer,1024);
            }
            DeleteEntity(entity);
        }
        if(StartupError)                       /* If we failed for some reason, we still have to let the Services Manager
                                               know about it. */
        {
            LPDError = StartupError;
            Signal(LPDSMProc, LPDSignalMask);
        }
        Forbid();                               /* Make sure we exit before someone could call expunge! */
        CloseLibrary(SvcBase);
    }
}
