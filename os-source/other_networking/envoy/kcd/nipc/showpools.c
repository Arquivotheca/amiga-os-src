#include <exec/types.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include "nipc_pragmas.h"

struct PoolAlloc
{
    struct MinNode  PA_Node;
    ULONG	    PA_Size;
};

struct PrivatePool
{
    struct MinList	PP_List;
    ULONG		PP_Flags;
    ULONG		PP_Size;
    ULONG		PP_Thresh;
    struct MinList	PP_Track;
};

extern struct PrivatePool *LockPool(void);
extern VOID UnlockPool(void);

ULONG ShowPools(void)
{
    struct Library *SysBase;
    struct Library *DOSBase;
    struct Library *NIPCBase;
    struct PoolAlloc *pa;
    struct PrivatePool *nipcPool;

    SysBase = *(struct Library **)4L;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
    	if(NIPCBase = OpenLibrary("nipc.library",37L))
    	{
    	    nipcPool = LockPool();
            pa = (struct PoolAlloc *)nipcPool->PP_Track.mlh_Head;

            while(pa->PA_Node.mln_Succ)
            {
                Printf("A: %08lx  S: %ld\n",sizeof(struct PoolAlloc) + (ULONG)pa,pa->PA_Size-sizeof(struct PoolAlloc));
                pa = (struct PoolAlloc *)pa->PA_Node.mln_Succ;
            }
            UnlockPool();
            CloseLibrary(NIPCBase);
        }
        CloseLibrary(DOSBase);
    }
    return(0);
}

