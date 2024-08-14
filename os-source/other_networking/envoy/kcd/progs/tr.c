#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/nipc_protos.h>
#include <string.h>
#include "/nipcinternal.h"
#include "/nipcbase.h"

#define geta4 __builtin_geta4
void geta4(void);

void _main(void)
{
    struct Library *DOSBase;
    struct Library *SysBase;
    struct NBase *NIPCBase;
    struct NIPCRouteInfo *RouteInfo;
    struct NIPCRoute *route;
    ULONG i;
    UBYTE linebuff[80];

    geta4();

    SysBase = (*(struct Library **)4L);

    if(DOSBase=OpenLibrary("dos.library",37L))
    {
        PutStr("DOS Opened.\n");

        if(NIPCBase=(struct NBase *)OpenLibrary("nipc.library",0L))
        {
            PutStr("NIPC Opened.\n");

            RouteInfo=&NIPCBase->RouteInfo;
            if(route=RouteInfo->nri_Default)
            {
                sprintf(linebuff,"default: %lx %lx %lx %ld %lx\n",route->nr_Network,route->nr_Mask,route->nr_Gateway,(ULONG)route->nr_Hops,route->nr_Device);
                PutStr(linebuff);
            }
            else
                PutStr("no default route.\n");

            for(i=0;i<MAX_ROUTES;i++)
            {
                route=(struct NIPCRoute *) RouteInfo->nri_Routes[i].mlh_Head;
                while(route->nr_Node.mln_Succ)
                {
                    sprintf(linebuff,"entry: %lx %lx %lx %ld %lx\n",route->nr_Network,route->nr_Mask,route->nr_Gateway,(ULONG)route->nr_Hops,route->nr_Device);
                    PutStr(linebuff);
                    route = (struct NIPCRoute *) route->nr_Node.mln_Succ;
                }
            }
            CloseLibrary((struct Library *)NIPCBase);
        }
        CloseLibrary(DOSBase);
    }
}



