
#include <stdio.h>
#include "//fs.h"
#include <clib/nipc_protos.h>
#include <clib/services_protos.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/services_pragmas.h>

struct Library *NIPCBase;
struct Library *ServicesBase;

main(int argc, char **argv)
{

    NIPCBase = (struct Library *) OpenLibrary("nipc.library",37L);
    if (NIPCBase)
    {
        ServicesBase = (struct Library *) OpenLibrary("services.library",37L);
        if (ServicesBase)
        {
            struct Entity *e;
            e = CreateEntity(ENT_Name,"Shower",ENT_Public,TRUE,ENT_AllocSignal,NULL,TAG_DONE);
            if (e)
            {
                struct Entity *re;
                re = (struct Entity *) FindService(argv[1],"Filesystem",e,TAG_DONE);
                if (re)
                {
                    struct Transaction *t;
                    t = AllocTransaction(TRN_AllocReqBuffer,128,TRN_AllocRespBuffer,1024,TAG_DONE);
                    if (t)
                    {
                        struct RequestMounts *rm;
                        rm = (struct RequestMounts *) t->trans_RequestData;
                        strcpy(&rm->User,"Greg Miller");
                        strcpy(&rm->Password,"Password");
                        t->trans_Command = FSCMD_SHOWMOUNTS;
                        DoTransaction(re,e,t);
                        printf("T has returned!\n");
                        if (t->trans_Error)
                            printf("Error was %lx\n",t->trans_Error);
                        else
                        {
                            STRPTR x;
                            ULONG f=0;
                            x = (STRPTR) t->trans_ResponseData;
                            while (f < t->trans_RespDataActual)
                            {
                                printf("Mount: %s\n",x);
                                x = &x[64];
                                f += 64;
                            }

                        }
                        FreeTransaction(t);
                    }
                    else
                        printf("Couldn't allocate a transaction\n");
                    LoseService(re);
                }
                else
                    printf("Couldn't locate service\n");
                DeleteEntity(e);
            }
            else
                printf("Couldn't create local entity\n");
            CloseLibrary(ServicesBase);
        }
        else
            printf("Couldn't open services\n");
        CloseLibrary(NIPCBase);
    }
    else
        printf("Couldn't open nipc\n");
}


