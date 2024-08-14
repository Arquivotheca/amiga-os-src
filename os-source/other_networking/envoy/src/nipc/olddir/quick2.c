
#include <stdio.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include "nipc_pragma.h"
#include "rdp.h"
#include "amp.h"

struct Library *NIPCBase;

main()
{
 void *ret;
 ULONG sigbit=0;
 ULONG mytags[8]={ENT_Name,((ULONG) "test port"),ENT_Public,0L,ENT_AllocSignal,0,TAG_DONE,0L};

 mytags[5]=(ULONG) &sigbit;
 printf("Trying to open the library\n");
 NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
 printf("It's open\n");

 printf("Tags = %lx\n",&mytags);
 ret = (void *) CreateEntity(&mytags);
 printf("CreateEntity returns %lx\n",ret);

 CloseLibrary(NIPCBase);

}


