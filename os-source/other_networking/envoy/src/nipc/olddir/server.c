#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <work:nipc/amp.h>
#include <pragmas/exec_pragmas.h>
#include <work:nipc/nipc_pragma.h>
#include <pragmas/dos_pragmas.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include "nipc_protos.h"
#include <string.h>

UBYTE cname[]="test_server";

struct TagItem cetags[] =
{
    ENT_Name, &cname,
    ENT_Public, 0,
    ENT_AllocSignal, 0,
    TAG_END, 0
};

struct TagItem ttags[] =
{
    TRN_AllocReqBuffer, 256,
    TAG_END, 0
};

void __saveds _main(void)
{
    struct Library *DOSBase;
    struct Library *NIPCBase;
    void *entity, *dest;
    struct Transaction *trans;

    if(DOSBase=OpenLibrary("dos.library",37L))
    {
        PutStr("DOS Opened.\n");

        if(NIPCBase=OpenLibrary("nipc.library",0L))
        {
            PutStr("NIPC Opened.\n");

            if(entity=CreateEntityA((struct TagItem *)&cetags))
            {
                PutStr("Created Entity.\n");

                WaitEntity(entity);

                trans=GetTransaction(entity);

                PutStr(trans->trans_RequestData);
                ReplyTransaction(trans);

                DeleteEntity(entity);
            }
            CloseLibrary(NIPCBase);
        }
        CloseLibrary(DOSBase);
    }
}



