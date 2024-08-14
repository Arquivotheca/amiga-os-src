#include <exec/types.h>
#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>

void __saveds _main(void)
{
    struct Library *SysBase;
    struct NBase *NIPCBase;

    SysBase = (*(struct Library **)4L);

    if(NIPCBase=(struct NBase *)OpenLibrary("nipc.library",0L))
    {
    	CloseLibrary((struct Library *)NIPCBase);
    }
}
