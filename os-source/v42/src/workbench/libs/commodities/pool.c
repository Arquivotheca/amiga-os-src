
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/lists.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "commoditiesbase.h"
#include "pool.h"


/*****************************************************************************/


/* In this module, all instances of the local variable "pools" are
 * there strictly to avoid a compiler bug in SAS/C 5.10, where the value
 * of CxBase was being lost. When compiling under latter versions of SAS/C,
 * simply replace all references to pools by CxBase->cx_Pools
 */


/*****************************************************************************/


/* This function must be called under Forbid(), and must not break Forbid(),
 * since it is called from the library's expunge vector
 */
VOID ASM FlushPools(REG(a6) struct CxLib *CxBase)
{
UWORD  i;
ULONG  size;
APTR   memory;
APTR   next;
APTR  *pools;

    pools = CxBase->cx_Pools;

    for (i=0; i<NUMPOOLS; i++)
    {
        size   = i*8+8;
        memory = pools[i];
        while (memory)
        {
            next = *(APTR *)memory;
            FreeMem(memory,size);
            memory = next;
        }
        pools[i] = NULL;
    }
}


/*****************************************************************************/


APTR AllocPoolMem(struct CxLib *CxBase, ULONG size)
{
ULONG  i;
APTR   memory;
ULONG *ptr;
APTR  *pools;

    i      = ((size+4)+7) / 8 - 1;   /* get a pool number    */
    size   = i*8+8;                  /* size things up...    */
    memory = NULL;
    pools  = CxBase->cx_Pools;

    if (i < NUMPOOLS)
    {
        Forbid();
        if (memory = pools[i])
            pools[i] = *(APTR *)memory;  /* address of next node in list */
        Permit();
    }

    if (memory)
    {
        i   = size / 4;
        ptr = memory;
        while (i)
            ptr[--i] = 0;
    }
    else
    {
        memory = AllocMem(size,MEMF_CLEAR);
    }

    if (memory)
    {
        *(ULONG *)memory = size;
        memory = (APTR)((ULONG)memory + 4);
    }

    return(memory);
}


/*****************************************************************************/


VOID FreePoolMem(struct CxLib *CxBase, APTR memory)
{
ULONG  size;
ULONG  i;
APTR  *pools;

    if (memory)
    {
        memory = (APTR)((ULONG)memory - 4);  /* get real memory pointer  */
        size   = *(ULONG *)memory;           /* get size of memory block */
        i      = (size / 8) - 1;             /* in which pool does the block go? */
        pools  = CxBase->cx_Pools;

        if (i < NUMPOOLS)
        {
            Forbid();
            *(APTR *)memory = pools[i];
            pools[i] = memory;
            Permit();
        }
        else
        {
            FreeMem(memory,size);
        }
    }
}
