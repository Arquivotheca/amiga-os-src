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
    TRN_AllocReqBuffer,8196,
    TRN_AllocRespBuffer,4,
    TAG_END, 0
};

void _main(void)
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
    ULONG args[4];
    BPTR file;
    ULONG count;

    geta4();
//    kprintf("Data base set up.\n");
    SysBase = *((struct Library **)4L);
    cetags[0].ti_Data = (ULONG) &sigbit;

    if (DOSBase = OpenLibrary("dos.library", 37L))
    {
//        kprintf("Dos open.\n");
        args[0]=args[1]=args[2]=args[3]=0;

        if(lpargs = ReadArgs("Server/A,FileName/A,PAR/S,SER/S",args,NULL))
        {
            if (NIPCBase = OpenLibrary("nipc.library", 0L))
            {
//                kprintf("nipc open.\n");
                if (ServicesBase = OpenLibrary("services.library", 37L))
                {
//                    kprintf("services open.\n");
                    if (entity = CreateEntityA((struct TagItem *) cetags))
                    {
//                        kprintf("Created my entity.\n");
                        if (trans = AllocTransactionA((struct TagItem *) ttags))
                        {
//                            kprintf("Transaction allocated.\n");

                            if (dest = FindServiceA((UBYTE *)args[0], "Printer_Service", entity, NULL))
                            {
//                                kprintf("found lpd daemon.\n");

                                if(file=Open((UBYTE *)args[1],MODE_OLDFILE))
                                {
                                    if(args[2])
                                    {
                                        trans->trans_Command = LPCMD_START_PAR;
                                    }
                                    else if(args[3])
                                    {
                                        trans->trans_Command = LPCMD_START_SER;
                                    }
                                    else
                                    {
                                        trans->trans_Command = LPCMD_START_PRT;
                                    }
                                    DoTransaction(dest,entity,trans);

                                    *(ULONG *)(trans->trans_RequestData) = *(ULONG *)(trans->trans_ResponseData);
                                    inc_job = *(ULONG *)(trans->trans_RequestData);

                                    while(count = FRead(file, ((UBYTE *)trans->trans_RequestData + 4), 1, 8192))
                                    {
                                        trans->trans_Command = LPCMD_DATA;
                                        trans->trans_ReqDataActual = count + 4;
                                        DoTransaction(dest,entity,trans);
                                    }
                                    trans->trans_Command = LPCMD_END;
                                    trans->trans_ReqDataActual = 4;
                                    DoTransaction(dest,entity,trans);
                                    Close(file);
                                }
                                LoseService(dest);
                            }
                            else
                                PutStr("Unable to connect to server.\n");
                            trans->trans_ReqDataLength = 8196;
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
        CloseLibrary(DOSBase);
    }
}
