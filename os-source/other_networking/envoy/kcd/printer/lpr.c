/*
** Example Envoy Service client.
**
** This program is the companion to the example Envoy
** Service.
**
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <envoy/nipc.h>
#include <envoy/services.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/services_pragmas.h>

#include <utility/tagitem.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <clib/services_protos.h>

#include <string.h>

#define geta4 __builtin_geta4
void geta4(void);

#define LPCMD_START_PRT 1
#define LPCMD_START_PAR 2
#define LPCMD_START_SER 3
#define LPCMD_DATA      4
#define LPCMD_END       5

struct TagItem cetags[2] =
{
    ENT_AllocSignal, NULL,
    TAG_END, 0
};

struct TagItem ttags[3] =
{
    TRN_AllocReqBuffer,16384,
    TRN_AllocRespBuffer,4,
    TAG_END, 0
};

/* I don't need to use the SAS startup code...*/

int _main(void)
{
    struct Library *SysBase;
    struct Library *DOSBase;
    struct Library *NIPCBase;
    struct Library *ServicesBase;
    struct RDArgs *lpargs;
    void *entity, *dest;
    struct Transaction *trans;
    ULONG sigbit;
    ULONG inc_job;
    ULONG args[3];
    UBYTE **files;
    UBYTE *hostname;
    LONG varlen;
    UWORD index=0;
    BPTR file;
    BPTR stdin=NULL;
    ULONG count,returncode=0;

    /* Fix up our data pointer. */

    geta4();

    /* Load ExecBase */

    SysBase = *((struct Library **)4L);
    cetags[0].ti_Data = (ULONG) &sigbit;


    /* Open up dos.library, at least V37 */

    if (DOSBase = OpenLibrary("dos.library", 37L))
    {
    	if(hostname=AllocMem(256,MEMF_CLEAR|MEMF_PUBLIC))
    	{
            /* Attempt to set our hostname via the environment
               variable named "PrinterHost" */

            varlen = GetVar("PrinterHost",hostname,256,0);

            /* Clear our args array for ReadArgs */

            args[0]=args[1]=args[2]=0;

            /* Do command line parsing via ReadArgs */

            if(lpargs = ReadArgs("FileName/M,RAW/S,HOST/K",args,NULL))
            {
            	if(args[2])
            	    stccpy(hostname,(STRPTR)args[2],256);

            	files = (UBYTE **)args[0];

		if(!files)
		    stdin = Input();

                /* Open nipc.library */

                if (NIPCBase = OpenLibrary("nipc.library", 0L))
                {
                    /* Open services.library */

                    if (ServicesBase = OpenLibrary("services.library", 37L))
                    {
                        /*
                        ** Create the Entity for our side of the communications
                        ** path with the Service.
                        */
                        if (entity = CreateEntityA((struct TagItem *) cetags))
                        {
                            /*
                            ** Allocate the transaction that we will use
                            ** to send data back and forth with.
                            */
                            if (trans = AllocTransactionA((struct TagItem *) ttags))
                            {

                                /* Try to locate the service on the remote machine */

                                if (dest = FindServiceA(hostname, "Printer_Service", entity, NULL))
                                {
                                    while(TRUE)
                                    {
                                        /* Try to open the user's file */

					if(stdin)
					    file=stdin;
					else
					    file=Open((UBYTE *)files[index],MODE_OLDFILE);

                                        if(file)
                                        {
                                            /* Should this be sent through printer.device? */

                                            if(args[1])
                                            {
                                                trans->trans_Command = LPCMD_START_PAR;
                                            }
                                            else
                                            {
                                                trans->trans_Command = LPCMD_START_PRT;
                                            }

                                            /*
                                            ** Fire off the initial transaction that tells
                                            ** there server we would like to print a file.
                                            */

                                            DoTransaction(dest,entity,trans);

                                            /*
                                            ** Copy the spool ID from the Response data into the
                                            ** data area of the transaction so that the Service
                                            ** knows which job we are sending data for.
                                            */

                                            *(ULONG *)(trans->trans_RequestData) = *(ULONG *)(trans->trans_ResponseData);
                                            inc_job = *(ULONG *)(trans->trans_RequestData);

                                            /*
                                            ** Keep reading from the file and sending data to
                                            ** the Service.  This version sends the file in
                                            ** 16k pieces.  It could be sped up by using a much
                                            ** larger transaction size.
                                            */
                                            while(count = Read(file, ((UBYTE *)trans->trans_RequestData + 4), 16384))
                                            {
                                                trans->trans_Command = LPCMD_DATA;
                                                trans->trans_ReqDataActual = count + 4;
                                                DoTransaction(dest,entity,trans);
                                            }

                                            /*
                                            ** Tell the service that file is complete and
                                            ** that we will be disconnecting.
                                            */

                                            trans->trans_Command = LPCMD_END;
                                            trans->trans_ReqDataActual = 4;
                                            DoTransaction(dest,entity,trans);
                                            if(!stdin)
                                                Close(file);
                                            index++;
                                        }
                                        else
                                        {
                                            if(!stdin)
                                            {
                                            	PutStr("Error opening file: ");
                                            	PutStr(files[index]);
                                            	PutStr("\n");
                                            }
                                        }
                                	if(stdin)
                                	    break;
                                	if(!files[index])
                                	    break;
                                    }
                                    /* Disconnect */
                                    LoseService(dest);
                                }
                                /* Sorry, no Printer Service was found */
                                else
                                {
                                    PutStr("Unable to connect to server.\n");
                                    returncode = 20;
                                }
                                trans->trans_ReqDataLength = 16384;
                                FreeTransaction(trans);
                            }
                            DeleteEntity(entity);
                        }
                        CloseLibrary(ServicesBase);
                    }
                    CloseLibrary(NIPCBase);
                }
                FreeArgs(lpargs);
            }
            FreeMem(hostname,256);
        }
        CloseLibrary(DOSBase);
    }
    return(returncode);
}
