#include <exec/types.h>
#include <exec/semaphores.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <clib/exec_protos.h>
#include "/nipcinternal.h"
#include "/nipcbase.h"


void __saveds _main(void)
{
    struct Library *SysBase;
    struct NBase *NIPCBase;

    SysBase = (*(struct Library **)4L);

    if(NIPCBase=(struct NBase *)OpenLibrary("nipc.library",0L))
    {
    	NIPCBase->RIPTimer = 0;
    	CloseLibrary((struct Library *)NIPCBase);
    }
}
