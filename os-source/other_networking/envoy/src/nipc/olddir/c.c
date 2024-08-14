
#include <stdio.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/ports.h>
#include "nipc_pragma.h"
#include "amp.h"

struct Library *NIPCBase;

main()
{
 void *fe, *source;
 struct Transaction *trans;
 ULONG tags[6]={TRN_ALLOCREQBUFFER,80,TRN_ALLOCRESPBUFFER,80,TAG_DONE,0};
 ULONG sigbit;
 ULONG srctags[4]={ENT_AllocSignal,0L,TAG_DONE,0};

 srctags[1] = (ULONG) &sigbit;

 printf("Trying to open the library\n");
 NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
 printf("It's open\n");

 printf("Allocating a source entity !!!\n");

 source = (void *) CreateEntity(&srctags);
 printf("CreateEntity returned %lx\n",source);

 printf("Finding the destination ... \n");
 fe = (void *) FindEntity("scratchy","test port",source);
 printf("fe = %lx\n",fe);
 if (!fe)
   {
   printf("Couldn't get the remote entity!\n");
   DeleteEntity(source);
   exit(0);
   }

 printf("losing remote entity\n");
 LoseEntity(fe);

 printf("Dumping local entity\n");
 DeleteEntity(source);

 printf("Closing library\n");
 CloseLibrary(NIPCBase);

}


