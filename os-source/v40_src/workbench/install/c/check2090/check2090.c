
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

#include "check2090_rev.h"


/****************************************************************************/


LONG main(VOID)
{
struct Library           *DOSBase;
struct Library           *SysBase = (*((struct Library **) 4));
struct ExpansionBase     *ExpansionBase;
LONG                      failureLevel = RETURN_FAIL;
struct ConfigDev         *cd;

    DOSBase       = OpenLibrary("dos.library" VERSTAG,37);
    ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library",37);

    if (DOSBase && ExpansionBase)
    {
        failureLevel = 0;   /* no 2090XXX */
        if (cd = FindConfigDev(NULL,514,1))
        {
            failureLevel = 1;  /* A2090-A */
            if (!(cd->cd_Rom.er_Type & ERTF_DIAGVALID))
            {
                failureLevel = 2;   /* A2090-SCSI/ST506 */
            }
        }
    }

    if (DOSBase)
    {
        CloseLibrary(ExpansionBase);
        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}
