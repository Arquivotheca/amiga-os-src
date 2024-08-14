
#include <stdio.h>
#include <dos/dos.h>

main()
{
    APTR x;
    x = Open("PRT:",MODE_NEWFILE);
    if (x)
    {
        Close(x);
    }
}

