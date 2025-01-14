
/*
 * Kludge to get V38 printer.device working with V37
 *
 * $id$
 *
 */

#include <stdio.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/iffparse.h>
#include <utility/tagitem.h>
#include <intuition/preferences.h>

#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <prefs/prefhdr.h>
#include <prefs/printertxt.h>
#include "printv37.h"

 STRPTR revisionstring=VERSTAG;

ULONG start(void)
{
    struct Library *SysBase=(*(void **)4L);
    struct Library *DOSBase;
    struct Library *IntuitionBase;
    struct Process *pr;
    struct Preferences *p;
    struct Message *w;

    pr = (struct Process *) FindTask(0L);

    if (!pr->pr_CLI)
        w = GetMsg(&pr->pr_MsgPort);

    p = (struct Preferences *) AllocMem(sizeof(struct Preferences),0);
    if (p)
    {
        DOSBase = (struct Library *) OpenLibrary("dos.library",37L);
        if (DOSBase)
        {
            IntuitionBase = (struct Library *) OpenLibrary("intuition.library",37L);
            if (IntuitionBase)
            {
                GetPrefs(p,sizeof(struct Preferences));
                strcpy(p->PrtDevName,"envoyprint");
                SetPrefs(p,sizeof(struct Preferences),TRUE);
                CloseLibrary(IntuitionBase);
            }
            CloseLibrary(DOSBase);
        }
        FreeMem(p,sizeof(struct Preferences));
    }
    if (!pr->pr_CLI)
    {
        Forbid();
        ReplyMsg(w);
    }
    else
        return(0L);
}



