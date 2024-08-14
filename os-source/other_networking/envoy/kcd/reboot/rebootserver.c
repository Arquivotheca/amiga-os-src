/* NIPC Example Program #2
**
** This program is the server side of a pair of programs that allow
** you to bring up a simple requester on a remote machine.  This example shows
** the use of some of the more commonly used functions in NIPC.
**
** You may compile this example with:
**
** lc -v -O -L+lib:nipc.lib RequestServer.c
**
*/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <envoy/nipc.h>
#include <envoy/errors.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/nipc_protos.h>

#include <stdio.h>

struct Library *NIPCBase;
struct Library *IntuitionBase;
struct Library *DOSBase;

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
#endif

void main(void)
{
    struct Entity *myentity;

    struct Transaction *trans;
    ULONG bitnumber, signals, sigmask;
    struct EasyStruct es;
    BOOL reboot=FALSE;

    /* Open Intuition for our requesters. */

    if(IntuitionBase = OpenLibrary("intuition.library",37L))
    {
    	/* Open NIPC for the network stuff */

    	if(NIPCBase = OpenLibrary("nipc.library",37L))
    	{

	    if(DOSBase = OpenLibrary("dos.library",37L))
	    {
                /* Create our public entity */

                if(myentity = CreateEntity(ENT_AllocSignal, &bitnumber,
                                           ENT_Name, "RebootServer",
                                           ENT_Public, TRUE,
                                           TAG_DONE))
                {
                    sigmask = SIGBREAKF_CTRL_C | (1 << bitnumber);

                    while(TRUE)
                    {
                        signals = Wait(sigmask);

                        while(trans = GetTransaction(myentity))
                        {
                            reboot = TRUE;
                            ReplyTransaction(trans);
                        }
                        if(reboot)
                            break;
                        if(signals & SIGBREAKF_CTRL_C)
                            break;
                    }
                    DeleteEntity(myentity);
                    if(reboot)
                    {
                    	Delay(250);
                    	ColdReboot();
                    }

                }
                CloseLibrary(NIPCBase);
            }
	}
	CloseLibrary(IntuitionBase);
    }
}

