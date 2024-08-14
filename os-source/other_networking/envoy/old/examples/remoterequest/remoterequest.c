/* NIPC Example Program #1
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

int main(int argc, char **argv)
{
    struct Entity *myentity,*serverentity;

    struct Transaction *trans;
    ULONG bitnumber, signals, sigmask;
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

                    if(trans = AllocTransaction(TRN_AllocReqBuffer,strlen(argv[2]) + 1,
                                                TAG_DONE))
                    {

                        /* Set everything up in the Transaction */

                        trans->trans_Command = 0;   /* We only know how to do one thing */

                        strcpy((char *)trans->trans_RequestData,argv[2]);
                        trans->trans_ReqDataActual = strlen(argv[2]) + 1;

                        /* Send it on it's way */

                        BeginTransaction(serverentity,myentity,trans);

                        /* Now wait for a CTRL-C or for the transaction to return */

                        sigmask = SIGBREAKF_CTRL_C | (1 << bitnumber);
                        while(TRUE)
                        {
                            signals = Wait(sigmask);

                            if(GetTransaction(myentity))
                            {
                            	if(trans->trans_Error)
                            	    printf("Error occured: %ld\n",(ULONG)trans->trans_Error);
                                break;
                            }
                            if(signals & SIGBREAKF_CTRL_C)
                            {
                            	printf("Aborting...\n");
                                AbortTransaction(trans);
                                WaitTransaction(trans);
                                break;
                            }
    			}
                        FreeTransaction(trans);
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
