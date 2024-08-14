
#include <stdio.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <exec/tasks.h>

struct Task *orig;

void newtask()
{

    APTR x;
    Wait(SIGBREAKF_CTRL_F);
    x = (APTR) AllocMem(64,0);
    if (x)
        FreeMem(x);

    Forbid();
    Signal(orig,SIGBREAKF_CTRL_F);
}


main()
{

    UBYTE *y;
    struct Task *t;

    orig = (struct Task *) FindTask(0L);
    t = (struct Task *) CreateTask("MemoryTest",0,&newtask,4100);
    if (t)
    {
        Wait(SIGBREAKF_CTRL_C);
        Signal(t,SIGBREAKF_CTRL_F);
        Wait(SIGBREAKF_CTRL_F);
    }
}

