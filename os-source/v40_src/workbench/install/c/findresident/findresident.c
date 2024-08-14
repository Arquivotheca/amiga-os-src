
#include <exec/types.h>
#include <dos/rdargs.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "findresident_rev.h"


/****************************************************************************/


#define TEMPLATE   "MODULE/A" VERSTAG
#define OPT_MODULE 0
#define OPT_COUNT  1


/****************************************************************************/


LONG main(VOID)
{
struct Library  *DOSBase;
struct Library  *SysBase = (*((struct Library **) 4));
LONG             failureLevel = RETURN_FAIL;
struct RDArgs   *rdargs;
LONG             opts[OPT_COUNT];

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        memset(opts,0,sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE, opts, NULL))
        {
            failureLevel = 1;      /* resident module not found */
            if (FindResident((STRPTR)opts[OPT_MODULE]))
                failureLevel = 0;

            FreeArgs(rdargs);
        }
        else
        {
            PrintFault(IoErr(),NULL);
        }

        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}
