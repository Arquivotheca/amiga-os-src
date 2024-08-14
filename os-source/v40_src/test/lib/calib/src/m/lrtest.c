#include <exec/types.h>
#include <exec/memory.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include "protos.h"

#define DIST    (57 * 4)
#define NUMREGS 57
#define TAP     3
#define SEED    2001

int main()
{
    struct LREnv *lr;
    ULONG *dist;
    ULONG i, cnt = 0;

    if (dist = AllocVec(DIST * sizeof(ULONG), MEMF_CLEAR | MEMF_ANY))
    {
        if (lr = LROpen(SEED, NUMREGS, TAP))
        {
            while (!(SetSignal(NULL, NULL) & SIGBREAKF_CTRL_C))
            {
                ULONG ra;

                ra = LRand(lr);
                //Printf("--> %08lx\t%8ld\t%8ld\t%ld\n", ra, ra % DIST, ra / DIST, ra);
                ++dist[ra % DIST];
                ++cnt;
            }
        }

        for (i = 0; i < DIST; ++i)
            Printf("dist %4ld = %ld\n", i, dist[i]);

        FreeVec(dist);
    }

    return cnt;
}
