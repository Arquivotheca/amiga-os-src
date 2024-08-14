
#include <stdio.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include "nipc_pragma.h"

struct Library *NIPCBase;

main()
{
 ULONG fe;

 printf("Trying to open the library\n");
 NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
 printf("It's open\n");

 fe = FindEntity("scratchy","test port");
 printf("fe = %lx\n",fe);

 CloseLibrary(NIPCBase);

}


