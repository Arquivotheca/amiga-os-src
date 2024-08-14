
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <internal/commands.h>
#include <string.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/expansion_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/expansion_pragmas.h>

#include "guessbootdev_rev.h"


/****************************************************************************/


#define TEMPLATE        "BOOTDISKNAME" VERSTAG
#define OPT_BOOTDISK    0
#define OPT_COUNT	1


/****************************************************************************/


LONG main(VOID)
{
struct Library           *DOSBase;
struct Library           *SysBase = (*((struct Library **) 4));
struct Library           *UtilityBase;
struct ExpansionBase     *ExpansionBase;
LONG                      failureLevel = RETURN_FAIL;
struct BootNode          *bn;
struct DeviceNode        *dn;
struct FileSysStartupMsg *fssm;
struct DosEnvec          *env;
struct Process           *process;
APTR                      oldWP;
BPTR                      lock1,lock2;
BYTE                      maxPri;
char                      name[300];
BOOL                      ok;
struct RDArgs            *rdargs;
LONG                      opts[OPT_COUNT];

    DOSBase       = OpenLibrary("dos.library",37);
    UtilityBase   = OpenLibrary("utility.library",37);
    ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library",37);

    if (DOSBase && UtilityBase && ExpansionBase)
    {
        memset(opts,0,sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE, opts, NULL))
        {
            name[0] = 0;
            maxPri  = -128;
            process = (struct Process *)FindTask(NULL);
            oldWP   = process->pr_WindowPtr;
            process->pr_WindowPtr = (APTR)-1;

            lock1 = Lock("SYS:",ACCESS_READ);
            if (opts[OPT_BOOTDISK])
                lock2 = Lock((STRPTR)opts[OPT_BOOTDISK],ACCESS_READ);
            else
                lock2 = NULL;

            if (lock1 && lock2 && (SameLock(lock1,lock2) == LOCK_SAME))
            {
                bn = (struct BootNode *)ExpansionBase->MountList.lh_Head;
                while (bn->bn_Node.ln_Succ)
                {
                    if (dn = (struct DeviceNode *)bn->bn_DeviceNode)
                    {
                        if (bn->bn_Node.ln_Type == NT_BOOTNODE)
                        {
                            if (!(dn->dn_Handler & 0x80000000))
                            {
                                if (bn->bn_Node.ln_Pri > maxPri)
                                {
                                    ok = TRUE;
                                    if (fssm = (struct FileSysStartupMsg *)((ULONG)dn->dn_Startup * 4))
                                    {
                                        if (env = (struct DosEnvec *) ((ULONG)fssm->fssm_Environ * 4))
                                        {
                                            /* we don't want bootblock devices, only BootPoint ones */
                                            if ((env->de_TableSize >= DE_BOOTBLOCKS)
                                            &&  env->de_BootBlocks)
                                                ok = FALSE;
                                        }
                                    }

                                    if (ok)
                                    {
                                        sprintf(name,"%b",dn->dn_Name);
                                        maxPri = bn->bn_Node.ln_Pri;
                                    }
                                }
                            }
                        }
                    }
                    bn = (struct BootNode *)bn->bn_Node.ln_Succ;
                }
                PutStr(name);
                PutStr(":");
            }
            else
            {
                NameFromLock(lock1,name,sizeof(name));
                PutStr(name);
            }

            UnLock(lock1);
            UnLock(lock2);

            PutStr("\n");

            process->pr_WindowPtr = oldWP;

            FreeArgs(rdargs);
        }
        else
        {
            PrintFault(IoErr(),NULL);
        }
    }

    if (DOSBase)
    {
        CloseLibrary(UtilityBase);
        CloseLibrary(ExpansionBase);
        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}
