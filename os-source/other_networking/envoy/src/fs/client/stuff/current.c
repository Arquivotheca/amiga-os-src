
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <stdio.h>

#define DLFLAGS (LDF_VOLUMES|LDF_WRITE)
#define ENVOYTYPE  (0x444F5380)
#define ACTION_EFS_INFO 64000

struct AlreadyMounted
{
    struct Node am_Node;
    struct MsgPort *am_Task;
    UBYTE       am_VolName[80];
    UBYTE       am_HostName[80];
    UBYTE       am_RemotePath[80];
};

main()
{

    struct DosList *n;
    struct List alist;
    struct AlreadyMounted *am;

    NewList(&alist);
    n = (struct DosList *) LockDosList(DLFLAGS);
    if (n)
    {
        while (n = (struct DosList *) NextDosEntry(n,LDF_VOLUMES))
        {
            if (n->dol_misc.dol_volume.dol_DiskType == ENVOYTYPE)
            {
                struct AlreadyMounted *a;
                STRPTR x=BADDR(n->dol_Name);
                a = (struct AlreadyMounted *) AllocMem(sizeof(struct AlreadyMounted),0);
                if (a)
                {
                    CopyMem(&x[1],&a->am_VolName[0],x[0]);
                    a->am_VolName[x[0]]=0;
                    a->am_Task = n->dol_Task;
                    AddTail(&alist,(struct Node *)a);
                }
            }
        }
        UnLockDosList(DLFLAGS);
    }

    am = (struct AlreadyMounted *) alist.lh_Head;
    while (am->am_Node.ln_Succ)
    {
        am->am_HostName[0]=am->am_RemotePath[0]=0;
        DoPkt(am->am_Task,ACTION_EFS_INFO,&am->am_HostName[0],&am->am_RemotePath[0],80,80,0);
        am = (struct AlreadyMounted *) am->am_Node.ln_Succ;
    }

    while(am = (struct AlreadyMounted *) RemHead(&alist))
    {
        printf("---\nMount: %s\n   Host %s\n   Remote %s\n",&am->am_VolName[0],
                &am->am_HostName[0],&am->am_RemotePath[0]);
        FreeMem(am,sizeof(struct AlreadyMounted));
    }

}
