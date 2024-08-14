#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <work:nipc/amp.h>
#include <pragmas/exec_pragmas.h>
#include "nipc_pragma.h"
#include <pragmas/dos_pragmas.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include "nipc_protos.h"
#include <string.h>

#define geta4 __builtin_geta4
void geta4(void);

UBYTE cname[]="TEST Client";
ULONG sigbit;

struct TagItem cetags[] =
{
    ENT_Name, &cname,
    ENT_Public, 0,
    ENT_AllocSignal, &sigbit,
    TAG_END, 0
};

struct TagItem ttags[] =
{
    TRN_AllocRespBuffer, 10256,
    TAG_END, 0
};

void _main(void)
{
    struct Library *DOSBase;
    struct Library *NIPCBase;
    void *entity, *dest;
    struct Transaction *trans;
    ULONG sigbit;

    geta4();

    if(DOSBase=OpenLibrary("dos.library",37L))
    {
        PutStr("DOS Opened.\n");

        if(NIPCBase=OpenLibrary("nipc.library",0L))
        {
            PutStr("NIPC Opened.\n");

            if(entity=CreateEntityA((struct TagItem *)&cetags))
            {
                PutStr("Created Entity.\n");

                if(trans=AllocTransactionA((struct TagItem *)&ttags))
                {
                    PutStr("Transaction Allocated.\n");

                    trans->trans_RequestData="Hi there!\n";
                    trans->trans_ReqDataLength=strlen(trans->trans_RequestData);

                    if(dest=FindEntity("scratchy","test_server",entity,0L))
                    {
                        PutStr("Found Server Entity!\n");
                        PutStr("oog\n");
                        DoTransaction(dest,entity,trans);
                        PutStr("Transaction Done.\n");
                        LoseEntity(dest);
                        PutStr("LoseEntity Done.\n");
                    }
                    else
                    {
                        PutStr("Server Not Found!\n");
                    }
                    FreeTransaction(trans);
                }
                DeleteEntity(entity);
            }
            CloseLibrary(NIPCBase);
        }
        CloseLibrary(DOSBase);
    }
}



