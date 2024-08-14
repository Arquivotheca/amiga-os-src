#include <clib/exec_protos.h>
#include <stdio.h>

void main()
{
    struct Library *Svc;
    if(Svc=OpenLibrary("lpd.service",0L))
    {
        printf("Okay!\n");
        CloseLibrary(Svc);
    }
    else
    {
        printf("Shit!\n");
    }
}

