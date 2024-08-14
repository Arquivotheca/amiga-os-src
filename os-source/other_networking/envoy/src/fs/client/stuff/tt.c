


#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>


main()
{

    struct DevProc *d;
    STRPTR target="work:FinalCopy/ReadMe";

    d = (struct DevProc *) GetDeviceProc(target,NULL);
    if (d)
    {
        struct FileInfoBlock *fib;

        while (d->dvp_Flags & DVPF_ASSIGN)
        {
            printf("Assign bit set -- calling GetDeviceProc() again.\n");
            while (GetDeviceProc(target,d));
        }

        printf("Lock is %lx\n",d->dvp_Lock);
        printf("MsgPort is %lx\n",d->dvp_Port);

        fib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB);
        if (fib)
        {
            Examine(d->dvp_Lock,fib);
            printf("name is %s\n",&fib->fib_FileName);
            FreeDosObject(DOS_FIB,fib);
        }
        FreeDeviceProc(d);
    }


}

