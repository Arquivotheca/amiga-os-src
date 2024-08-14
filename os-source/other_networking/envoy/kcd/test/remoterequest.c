;/* NIPC Example Program #1
sc nostkchk link lib lib:envoy.lib RemoteRequest.c
quit
**
** This program is the client side of a pair of programs that allow
** you to bring up a simple requester on a remote machine.  This example shows
** the use of some of the more commonly used functions in NIPC.
**
** You may compile this example with:
**
** lc -v -O -L+lib:nipc.lib RemoteRequest.c
**
*/

#include <exec/types.h>
#include <envoy/nipc.h>
#include <envoy/errors.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>

#include <stdio.h>

struct Library *NIPCBase;

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
#endif

UBYTE myReqBuffer[10];
UBYTE myRespBuffer[10];

int main(int argc, char **argv)
{
    struct Entity *myentity,*serverentity;

    struct Transaction *trans;
    ULONG bitnumber, signals, sigmask;
    LONG queue = 0;
    BOOL done=FALSE;
    if(argc == 3)
    {
    	if(NIPCBase = OpenLibrary("nipc.library",37L))
    	{
            /* Let's create an Entity for our side.  I will ask NIPC to allocate
               a signal bit for this Entity and tell me it's bit number.  This
               is so that I can use Wait() instead of WaitEntity() below. */

            if(myentity = CreateEntity(ENT_AllocSignal, &bitnumber, TAG_DONE))
            {

                /* Try to find the server on the other side. */

                if(serverentity = FindEntity(argv[1],"RequestServer",myentity,NULL))
                {

                    /* Allocate a Transaction for use to use.  Allocate one big
                       enough to hold a copy of the string to be put into the
                       requester on the remote machine. */

                        while(queue < 2)
                        {
                            if(trans = AllocTransaction(TRN_AllocReqBuffer,65536,
                                            TAG_DONE))
                            {
                                /* Set everything up in the Transaction */

                                trans->trans_Command = 0;   /* We only know how to do one thing */
                                trans->trans_ReqDataActual = 65536;

                                BeginTransaction(serverentity,myentity,trans);
                                queue++;
                            }
                        }



                        /* Now wait for a CTRL-C or for the transaction to return */

                        sigmask = SIGBREAKF_CTRL_C | (1 << bitnumber);
                        while(!done)
                        {
                            while(trans = GetTransaction(myentity))
                            {
#define REUSE_TRANSACTIONS yeppers
#ifndef REUSE_TRANSACTIONS
                                FreeTransaction(trans);
                                queue--;

                                if(trans = AllocTransaction(TRN_AllocReqBuffer,65536,
                                                TAG_DONE))
                                {
                                    /* Set everything up in the Transaction */
#endif
                                    trans->trans_Command = 0;   /* We only know how to do one thing */
                                    trans->trans_ReqDataActual = 65536;

                                    BeginTransaction(serverentity,myentity,trans);
#ifndef REUSE_TRANSACTIONS

                                    queue++;
                                }
#endif

                                if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
                                {
                                    done = TRUE;
                                    break;
                                }

                            }
                            if(!done)
                            {
				signals = Wait(sigmask);

                                if(signals & SIGBREAKF_CTRL_C)
                                    break;
			    }
    			}

    			while(queue > 0)
    			{
    			    WaitEntity(myentity);

    			    if(trans = GetTransaction(myentity))
                                FreeTransaction(trans);

                            queue--;
                        }
                    LoseEntity(serverentity);
                }
                else
                    printf("Couldn't connect to server!\n");
            }
            DeleteEntity(myentity);
        }
        CloseLibrary(NIPCBase);
    }
    else
    	printf("Usage: RemoteRequest hostname message\n");

    return(0);
}
