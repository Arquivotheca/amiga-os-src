
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>


main(int argc,char **argv)
{

    if (argc)
    {
        struct DevProc *d;

        d = (struct DevProc *) GetDeviceProc(argv[1],0);
        if (d)
        {
            ULONG owner, group, res1, l;
            STRPTR b;

            l = strlen(argv[1]);
            if (b = (APTR) AllocMem(l+1,0))
            {
                b[0] = l;
                CopyMem(argv[1],&b[1],l);
                if (d->dvp_Flags & DVPF_ASSIGN)
                    while (GetDeviceProc(argv[1],d));

                sscanf(argv[2],"%ld",&owner);
                sscanf(argv[3],"%ld",&group);
                res1 = DoPkt(d->dvp_Port,ACTION_SET_OWNER,0,d->dvp_Lock,MKBADDR(b),((owner<<16)|group));
                if (!res1)
                    PrintFault(IoErr(),"SetOwner failed");
                FreeMem(b,l+1);
            }
            FreeDeviceProc(d);
        }
    }
}

