
#include <stdio.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>

extern struct Library *DOSBase;
main()
{
    struct DateStamp *ds;
    ds = (struct DateStamp *) AllocMem(sizeof(struct DateStamp),0);
    if (ds)
    {
        DateStamp(ds);
        printf("%ld",ds->ds_Minute);
        FreeMem(ds,sizeof(struct DateStamp));
    }
}


