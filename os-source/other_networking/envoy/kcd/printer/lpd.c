/*
** An example Envoy Service.
**
** This service implements a very simple print spooler.
**
*/

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

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <utility/tagitem.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>

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

/*
** This is the LVO that is called by the Envoy Services Manager when a client
** is requesting to use your service.  The first two parameters may be NULL
** if the client did not specify a user name or password.
*/
ULONG __saveds ASM StartService(REG(a0) STRPTR userName,
                       REG(a1) STRPTR password,
                       REG(a2) STRPTR entityName)
{
    ULONG cptags[5];
    struct Process *lpdproc;
    BOOL status = FALSE;
    UBYTE  sigbit;

    /*
    ** Check to see if the Entity for our spooler process exists, and if
    ** it doesn't, start it up.
    */
    Forbid();

    if(!LPDBase->LPD_Entity)
    {
    	LPDBase->LPD_Clients++;
    	Permit();
    	/* Allocate a signal bit for the spooler process to use. */

        if(sigbit = AllocSignal(-1L))
        {
            /*
            ** Set up the external variables to be used by the
            ** spooler process at startup time.
            */

            LPDSMProc = FindTask(0L);
            LPDSignalMask = (1L<<sigbit);
            LPDUser = userName;
            LPDPassword = password;
            LPDEntityName = entityName;

            /*
            ** Set up the Tags for the new process
            */

            cptags[0] = NP_Entry;
            cptags[1] = (ULONG) LPDServer;
            cptags[2] = NP_Name;
            cptags[3] = (ULONG) "LPD Daemon";
            cptags[4] = TAG_DONE;

            if(lpdproc = CreateNewProc((struct TagItem *)cptags))
            {
                Wait(1L<<sigbit);
                status = TRUE;
            }
            FreeSignal(sigbit);
        }
    }

    /*
    ** Okay, the server is still there, so just pass back the name
    ** of it's public entity.
    */

    else
    {
    	LPDBase->LPD_Clients++;
    	Permit();
        LPDError = 0;
        strcpy(entityName,"LPD_Service");
    }
    return LPDError;
}

/*
** This function is used by the Services Manager configuration tool to
** determine what the default name of your service is.  Other Tags will
** be defined in the future.  You may also define tags of your own.
*/

VOID ASM GetServiceAttrsA(REG(a0) struct TagItem *tagList)
{
    struct TagItem *ti;

    if(ti=FindTagItem( SVCAttrs_Name, tagList))
    {
        strcpy((STRPTR)ti->ti_Data,"Printer_Service");
    }
}

/*
** This is the C entry point of the print spooler process.
** The only parameter that I really pay attention to in this example
** is the EntityName parameter.
*/

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
    struct PrinterJob *inc_job,*prt_job = NULL;
    UBYTE *prt_buffer;
    struct IOStdReq *prt_io;
    struct timerequest *timer_io;
    void *entity;
    struct Transaction *trans;
    ULONG cetags[8]={ENT_Name, 0, ENT_Public, TRUE, ENT_AllocSignal, 0, TAG_END, 0};

    /* Set up our data pointer */

    geta4();

    nextfilenumber = 1;
    die = FALSE;
    dead = FALSE;
    ent_sigbit = 0;
    cetags[1] = (ULONG) "LPD_Service";
    cetags[5] = (ULONG) &ent_sigbit;

    StartupError = -1;

    /*
    ** Open ourselves, thus insuring that we won't get flushed.
    ** This is also useful in case we were to have our own functions
    ** in the service besides those required by Envoy Services.
    */
    if (SvcBase = OpenLibrary("lpd.service", 0L))
    {
    	/*
    	** Create our public Entity.
    	*/
        if (entity = CreateEntityA((struct TagItem *) cetags))
        {
            /*
            ** Allocate our printer I/O buffer.
            */
            if(prt_buffer = AllocMem(1024,MEMF_CLEAR));
            {

            	/*
            	** Create our replyport for printer device I/O
            	*/
                if(prt_port = CreateMsgPort())
                {

                    /*
                    ** Create our reply port for timer device I/O.
                    */
                    if(timer_port = CreateMsgPort())
                    {

                    	/*
                    	** Create our timer IORequest
                    	*/
                        if(timer_io = (struct timerequest *) CreateIORequest(timer_port,sizeof(struct timerequest)))
                        {

                            /*
                            ** Create our printer IORequest
                            */
                            if(prt_io = (struct IOStdReq *) CreateIORequest(prt_port,sizeof(struct IOExtPar)))
                            {
                            	/*
                            	** Everything is okay, so set the pointer to our
                            	** entity in our library base, and then signal
                            	** Services Manager.
                            	*/
                                LPDBase->LPD_Entity = entity;

                                strcpy(EntityName,"LPD_Service");

                                LPDError = StartupError = 0;

                                Signal(LPDSMProc, LPDSignalMask);

                                waitmask = (1<<prt_port->mp_SigBit) | (1<<ent_sigbit) | (1<<timer_port->mp_SigBit) | SIGBREAKF_CTRL_F;

                                status = LP_IDLE;

/****************************************************************************
**
** Main server loop stars here
**
****************************************************************************/

                                while(TRUE)
                                {
                                    signals = Wait(waitmask);

                                    do
                                    {
                                        loop = FALSE;

                                        /* See if we've received a new transaction */

                                        if(trans = GetTransaction(entity))
                                        {
                                            loop = TRUE;
                                            switch(trans->trans_Command)
                                            {
                                            	/* We have a request to spool a new file */

                                                case LPCMD_START_PRT:
                                                case LPCMD_START_PAR:   /* Allocate memory for a new print job status structure */
                                                			if(inc_job = AllocMem(sizeof(struct PrinterJob),MEMF_CLEAR))
                                                                        {
                                                                            /* Generate the job's filename */

                                                                            sprintf(inc_job->pj_JobName,"spool:job%ld",nextfilenumber);
                                                                            nextfilenumber++;

                                                                            /* Try to open the job file */

                                                                            if(inc_job->pj_File = Open(inc_job->pj_JobName,MODE_NEWFILE))
                                                                            {
                                                                            	/* Store the job type (printer/raw) */

                                                                                inc_job->pj_Type = trans->trans_Command;
                                                                                inc_job->pj_File = inc_job->pj_File;

                                                                                /* Store the job ID in the reply transaction data */
                                                                                *(ULONG *)(trans->trans_ResponseData) = (ULONG) inc_job;

                                                                                trans->trans_RespDataActual = 4;
                                                                                trans->trans_Error = 0;

                                                                                /* Add the job status struct to our queue */
                                                                                AddTail((struct List *)&lb->LPD_Incoming,(struct Node *)inc_job);
                                                                            }
                                                                            else
                                                                            {
                                                                            	/* Something went wrong. Reply with an error condition */

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

                                                /*
						** We have some data for a particular print job.  Write it to the
						** spool file.
						*/
                                                case LPCMD_DATA:        inc_job = (struct PrinterJob *) *(ULONG *)(trans->trans_RequestData);
                                                                        FWrite(inc_job->pj_File,(UBYTE *)((UBYTE *)trans->trans_RequestData + 4),1,(trans->trans_ReqDataActual - 4));
                                                                        ReplyTransaction(trans);
                                                                        break;

						/*
						** We now have all of the data for the spool file.  Unlink the
						** job from the incoming queue and link it in with the spool
						** queue.
						*/
                                                case LPCMD_END:         inc_job = (struct PrinterJob *) *(ULONG *)(trans->trans_RequestData);
                                                                        Close(inc_job->pj_File);
                                                                        ReplyTransaction(trans);
                                                                        Remove((struct Node *)inc_job);
                                                                        AddTail((struct List *)&lb->LPD_Spool,(struct Node *)inc_job);
                                                                        LPDBase->LPD_Clients--;
                                                                        break;
                                            }
                                        }

                                        /* See if our printer I/O request is complete. */

                                        if(GetMsg(prt_port))
                                        {
                                            loop = TRUE;

                                            /* Try to send more data to the printer device. */
                                            if(prt_io->io_Length = FRead(prt_job->pj_File,prt_buffer,1,1024))
                                            {
                                                SendIO((struct IORequest *)prt_io);
                                            }

                                            /*
                                            ** We're done with the file, so close the printer device
                                            ** and set our status to IDLE.  This will be noticed down
                                            ** below.
                                            */
                                            else
                                            {
                                                Close(prt_job->pj_File);
                                                DeleteFile((STRPTR)prt_job->pj_JobName);
                                                FreeMem(prt_job,sizeof(struct PrinterJob));
                                                CloseDevice((struct IORequest *)prt_io);
                                                status = LP_IDLE;
                                                prt_job = NULL;
                                            }
                                        }

                                        /*
                                        ** See if our timer request has returned.  If it has,
                                        ** set our status back to IDLE.
                                        */
                                        if(GetMsg(timer_port))
                                        {
                                            CloseDevice((struct IORequest *)timer_io);
                                            timer_io->tr_node.io_Device = NULL;
                                            status = LP_IDLE;
                                        }

                                        /*
                                        ** Check to see if we are IDLE.  If we are, see if we
                                        ** have any job's awaiting attention.
                                        */
                                        if(status == LP_IDLE)
                                        {

                                            if(prt_job = (struct PrinterJob *) RemHead((struct List *)&lb->LPD_Spool))
                                            {
                                                /*
                                            	** Okay, we have a job to send to the printer,
                                            	** mark our status as PRINTING.
                                            	*/
                                                loop = TRUE;
                                                status = LP_PRINTING;

                                                /* Check to see what kind of file we have */

                                                if(prt_job->pj_Type == LPCMD_START_PRT)
                                                {
                                                    /* The file is intended for the printer.device */
                                                    if(!OpenDevice("printer.device",0L,(struct IORequest *)prt_io,0))
                                                    {
                                                        prt_io->io_Command = CMD_WRITE;
                                                    }

                                                    /* Oops. Someone else is using the printer. Go to sleep. */
                                                    else
                                                    {
                                                        status = LP_SLEEPING;
                                                    }
                                                }
                                                else if(prt_job->pj_Type == LPCMD_START_PAR)
                                                {
                                                    /* The file is intended for the parallel.device */
                                                    if(!OpenDevice("parallel.device",0L,(struct IORequest *)prt_io,0))
                                                    {
                                                        prt_io->io_Command = CMD_WRITE;
                                                    }

                                                    /* Oops. Someone else is using the printer. Go to sleep. */
                                                    else
                                                    {
                                                        status = LP_SLEEPING;
                                                    }
                                                }

                                                /*
                                                ** If we are PRINTING, open up the next spool file and
                                                ** and send the initial IO request off to the appropriate
                                                ** device.
                                                */
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
                                                            status = LP_ERROR;
                                                    }
                                                }

                                                /*
                                                ** See if we are SLEEPING.  If so, put the current job back
                                                ** onto the queue and send a request to the timer device so
                                                ** we sleep for 30 seconds.
                                                */
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

					/*
					** Before we can quit, we need to make sure that:
					**
					** 1) We are IDLE
					** 2) Our incoming queue is empty
					** 3) There aren't any more clients connected to us.
					**
					** Here I Forbid() to make sure someone doesn't try connecting to me
					** while I'm trying to shut down.
					*/
                                        Forbid();
                                        if((status == LP_IDLE) && IsListEmpty((struct List *)&lb->LPD_Incoming) && !LPDBase->LPD_Clients)
                                        {
                                            LPDBase->LPD_Entity = NULL;	/* No more entity */
                                            dead = TRUE;		/* To get our of outer loop */
                                            Permit();
                                            break;
                                        }
                                        Permit();
                                    }
                                    while(loop);

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
            LPDBase->LPD_Clients--;
            LPDError = StartupError;
            Signal(LPDSMProc, LPDSignalMask);
        }
        Forbid();                               /* Make sure we exit before someone could call expunge! */
        CloseLibrary(SvcBase);
    }
}
