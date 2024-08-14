
#include <stdio.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/ports.h>
#include "pragmas/nipc_pragmas.h"
#include <appn/nipc.h>

struct Library *NIPCBase;

main(argc,argv)
int argc;
char **argv;
{
 void *fe, *source;
 struct Transaction *trans;
 ULONG tags[4]={TRN_AllocRespBuffer,10240,TAG_DONE,0};
 ULONG sigbit, ecode;
 ULONG srctags[6]={ENT_AllocSignal,0L,ENT_Name,0L,TAG_DONE,0};

 if (argc < 2)
   printf("Syntax:\ncmd_client machine commandstring\n");

 srctags[1] = (ULONG) &sigbit;
 srctags[3] = (ULONG) "client entity";

 NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);

 source = (void *) CreateEntityA(&srctags);
 fe = (void *) FindEntity(argv[1],"remote command server",source,&ecode);
 if (!fe)
   {
   printf("The named host isn't running a server.\n");
   printf("Error #%ld\n",ecode);
   DeleteEntity(source);
   exit(0);
   }

 trans = (struct Transaction *) AllocTransactionA(&tags);

 trans->trans_RequestData = argv[2];
 trans->trans_ReqDataLength = strlen(argv[2])+1;
 trans->trans_ReqDataActual = trans->trans_ReqDataLength;
 trans->trans_Timeout = 3;

 DoTransaction(fe,source,trans);
 if (!trans->trans_Error)
   printf("%s",trans->trans_ResponseData);
 else
   printf("Error returned: %d\n",trans->trans_Error);

 FreeTransaction(trans);
 LoseEntity(fe);
 DeleteEntity(source);
 CloseLibrary(NIPCBase);

}


