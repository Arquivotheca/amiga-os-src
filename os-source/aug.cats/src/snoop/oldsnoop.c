
#include "exec/types.h"
#include "exec/exec.h" 
#include "exec/execbase.h" 

extern struct ExecBase *SysBase;
extern LVOAllocMem;
extern LVOFreeMem;
extern APTR SetFunction();
extern SnoopAllocMem();
extern SnoopFreeMem();

char *program;
APTR  OldAllocMem, OldFreeMem;

main(argc,argv)
    int  argc;
    char *argv[];
{
    short   b;
    long    size;
    struct ExecBase *execlib = SysBase;

    program = *argv++;

    printf ("Alloc/Free Memory Snooper #2\n");
    printf ("(does not use caller's stack)\n");
    printf ("Type 'q' <RETURN> to quit...\n");

    OldAllocMem = SetFunction (execlib, &LVOAllocMem, SnoopAllocMem);
    OldFreeMem = SetFunction (execlib, &LVOFreeMem, SnoopFreeMem);

    while (getchar () != 'q');

    SetFunction (execlib, &LVOAllocMem, OldAllocMem);
    SetFunction (execlib, &LVOFreeMem, OldFreeMem);

    printf ("%s: terminated\n", program);
}
