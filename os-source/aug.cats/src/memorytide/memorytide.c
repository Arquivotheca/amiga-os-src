;/* memorytide.c - Execute me to compile me with SAS C 5.10
LC -b0 -cfistq -v -j73 memorytide.c
Blink FROM LIB:rxstartup.obj,memorytide.o TO memorytide ND LIBRARY LIB:Amiga.lib,lib:debug.lib,lib:LC.lib
quit
*/

#include <exec/types.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <stdlib.h>

extern struct ExecBase *SysBase;
extern struct Library  *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/**********    debug macros     ***********/
#define MYDEBUG  1
void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);
#define DEBTIME 0
#define bug kprintf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

#define LOWLIMIT 25
#define MINARGS 1

UBYTE *vers =  "$VER: memorytide 37.1";

/* Note - if you change usage, change length for Write */
UBYTE *usage = "memorytide [ticks]  (default/lowest is 25 ticks)\n";

void main(int argc, char **argv)
{
char *cmd;
long interval;
long largestchip, largestfast;
struct MemChunk *first;
register struct MemHeader *memHdr;
unsigned long total,totalchip,totalfast,oldtotal;


    interval = 0;
    if (argc > 1)
        {
	if(argv[1][0] == '?')
	    {
	    Write(Output(),usage,49);
	    exit(0);
	    }

	cmd = argv[1];

        while ((*cmd >= '0') && (*cmd <= '9'))
            /* Multiply it by 10 without using a subroutine */
            interval = (((interval << 2) + interval) << 1) + *cmd++ - '0';
	}

    if (interval < LOWLIMIT) interval = LOWLIMIT;

    oldtotal=100000000L;

    while(!(SetSignal(0,0) & SIGBREAKF_CTRL_C))
        {
        totalchip=0L,totalfast=0L,largestchip=0L,largestfast=0L;

        Forbid();

        /* point to the first MemHeader in the list */
	memHdr=(struct MemHeader *)&SysBase->MemList.lh_Head->ln_Succ;


        /* Walk through the memory pool, looking for ram. */
        while (memHdr->mh_Node.ln_Succ)
            {
            for (first = memHdr->mh_First; first; first = first->mc_Next)
                {
                if (memHdr->mh_Attributes & MEMF_CHIP)
                    {
                    totalchip +=first->mc_Bytes;
                    if (first->mc_Bytes >largestchip)
                            largestchip=first->mc_Bytes;
                    }
                else
		    {
                    totalfast +=first->mc_Bytes;
                    if (first->mc_Bytes >largestfast)
                            largestfast=first->mc_Bytes;
		    }
                }
            memHdr = (struct MemHeader *)memHdr->mh_Node.ln_Succ;
            }

        Permit();

        total = totalchip+totalfast;
        if (total==oldtotal)
            {
            D(bug("."));
            }
        else
            {
            D(bug("\nChip: %8ld (L %8ld)  Fast: %8ld (L %8ld)  Total: %8ld", 
			totalchip,largestchip,totalfast,largestfast,total));
            oldtotal=total;
            }
        Delay(interval);
        }
    D(bug("\n"));
    return;
}


