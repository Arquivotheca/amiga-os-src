
/***************************************************************************
*
*  $Id: vgaonly.c,v 39.2 92/10/08 10:34:07 spence Exp $
*
****************************************************************************/

#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dosextens.h>
#include <graphics/gfxbase.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "v39:src/kickstart/graphics/vp_internal.h"
#include "VGAOnly_rev.h"


/*****************************************************************************/


LONG main(VOID)
{
APTR            vstring = VERSTAG;
struct GfxBase *GfxBase;
struct Library *SysBase;
struct Process *process;
APTR            WBenchMsg = NULL;

    SysBase = (*((struct Library **) 4));

    process = (struct Process *)FindTask(NULL);
    if (!(process->pr_CLI))
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = GetMsg(&process->pr_MsgPort);
    }

    if (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39))
    {
        GfxBase->Bugs &= ~(BUG_ALICE_LHS_PROG);
        CloseLibrary(GfxBase);
    }

    if (WBenchMsg)
    {
    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(0);
}
