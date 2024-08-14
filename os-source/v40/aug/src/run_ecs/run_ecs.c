
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <workbench/startup.h>
#include <string.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "run_ecs_rev.h"


/*****************************************************************************/

char version[] = VERSTAG;

#define TEMPLATE      "COMMAND/F"
#define OPT_COMMAND   0
#define OPT_COUNT     1


/*****************************************************************************/


/*****************************************************************************/

struct EasyStruct  easy = {
	sizeof(struct EasyStruct),
	0,
	"Run_ECS",
	"%s",
	"OK",
};

LONG main(VOID)
{
struct Library    *SysBase = (*((struct Library **) 4));
struct Library    *DOSBase;
struct GfxBase    *GfxBase;
struct Library    *IntuitionBase;
LONG               opts[OPT_COUNT];
struct RDArgs     *rdargs;
LONG               failureCode;
LONG               failureLevel;
UBYTE		   save_memtype;
struct WBStartup  *WBenchMsg = NULL;
BPTR		   olddir = 0;
UBYTE		   buffer[90];

    failureCode  = ERROR_INVALID_RESIDENT_LIBRARY;
    failureLevel = RETURN_FAIL;

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        if (GfxBase = (void *) OpenLibrary("graphics.library",37))
        {
            if (IntuitionBase = OpenLibrary("intuition.library",37))
            {
		failureCode = 0;	// libs open
		memset(opts,0,sizeof(opts));

		if (Cli() != NULL)
		{
		    if ((rdargs = ReadArgs(TEMPLATE,opts,NULL)) == NULL)
	                failureCode = IoErr();

		} else {
		    // started from WB, find the tool and run it...
		    WaitPort(&(((struct Process *)
				FindTask(NULL))->pr_MsgPort));
		    WBenchMsg = (void *) GetMsg(&(((struct Process *)
						 FindTask(NULL))->pr_MsgPort));

		    // save currentdir...
		    olddir = CurrentDir(NULL);

		    if (WBenchMsg->sm_NumArgs <= 1)
			failureCode = ERROR_REQUIRED_ARG_MISSING;
		    else if (WBenchMsg->sm_NumArgs > 2)
			failureCode = ERROR_TOO_MANY_ARGS;
		    else {
			// If we wanted to REALLY do this right, we'd start
			// the program up as a process and send it a WBStartup
			// message.  For the time being we'll run it from
			// a System() call.
			CurrentDir(WBenchMsg->sm_ArgList[1].wa_Lock);
			opts[0] = (LONG) WBenchMsg->sm_ArgList[1].wa_Name;
		    }
		}

		if (!failureCode)
		{
			// fool graphics into thinking that we're 1x...
			// NOTE: this is NOT safe!  This WILL break in the
			// future!
			save_memtype = GfxBase->MemType;
			GfxBase->MemType = BANDWIDTH_1X;

			// force chips into old modes...
			LoadView(NULL);

			// make sure it took (paranoia?)
			WaitTOF();
			WaitTOF();

			// execute their command...
			failureLevel = SystemTagList((char *) opts[0],NULL);
			failureCode = IoErr();

			// make intuition rebuild it's world (overkill)
			GfxBase->MemType = save_memtype;
			RethinkDisplay();
	        }

		if (WBenchMsg)
			CurrentDir(olddir);

		// don't bother with printing error if we can't open a lib...
		if (failureLevel != RETURN_OK)
		{
			if (WBenchMsg)
			{
			    char *buf = buffer;  // varargs list for ER()

			    // can't call EasyRequest because it uses globals..
			    Fault(failureCode,"Error",buffer,sizeof(buffer));
			    EasyRequestArgs(NULL,&easy,NULL,&buf);
			} else
	        	    PrintFault(failureCode,NULL);
		}
	        SetIoErr(failureCode);

		CloseLibrary(IntuitionBase);
	    }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
	Forbid();
	ReplyMsg(&(WBenchMsg->sm_Message));
    }

    return(failureLevel);
}


