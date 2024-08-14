

#include <stdio.h>
#include <exec/types.h>
#include <dos/dos.h>

main()
{

    APTR x;
    x = Open("SoftServe:greg/afile",MODE_OLDFILE);
    if (x)
    {
        int q;
        q = Read(x,0L,0L);
        printf("Read = %ld\n",q);
        Close(x);
    }
}
