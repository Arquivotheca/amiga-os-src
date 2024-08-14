/*
**  serc.c  -- Simple Envoy Remote Command example program, client.  
**	      Requires NIPC.
**
**  Copyright 1992, Commodore-Amiga, Inc.
**  Permission to use granted this notice remains intact.
**
**	To-do:  "Can't find server program" needs detailed error text.
*/
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <envoy/nipc.h>
#include <clib/nipc_protos.h>
#include <pragmas/nipc_pragmas.h>
#include <string.h>

extern struct Library *SysBase;
struct Library *NIPCBase = NULL;

int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */


main(int argc, char *argv[])
{
struct Entity *myentity = NULL,
              *hisentity = NULL;
struct Transaction *trans = NULL;
LONG rc = RETURN_FAIL;			/* return code */
ULONG entsig, mask, error;

	if(argc != 3)
	{
		printf("Usage: %s host command\n", argv[0]);
		exit(rc);
	}
	if( !(NIPCBase = OpenLibrary("nipc.library", 0)) )
	{
		printf("Can't open NIPC.\n");
		exit(rc);
	}
	if(myentity = CreateEntity(ENT_AllocSignal, (LONG)&entsig, TAG_DONE))
	{
		if(hisentity = FindEntity(argv[1], "serc_server", myentity, &error))
		{
			if(trans = AllocTransaction(TRN_AllocRespBuffer, 5000, TAG_DONE))
			{
				trans->trans_RequestData = argv[2];
				trans->trans_ReqDataActual = strlen(argv[2])+1;
				trans->trans_Timeout = 10;  /*  10 Second time-out  */
				BeginTransaction(hisentity, myentity, trans);
				mask = Wait(1L << entsig | SIGBREAKF_CTRL_C);
				if(mask & 1L << entsig)
				{
					if( GetTransaction(myentity) )  /*  pull the reply off. */
					{
						printf(trans->trans_ResponseData);
						rc=trans->trans_Error;
					}
				}
				if(mask & SIGBREAKF_CTRL_C)
				{
					AbortTransaction(trans);  /* Sure it's OK to abort even if we got it above. */
					printf("Please wait, aborting transaction with server.\n");
					WaitTransaction(trans);  /* always wait on aborted transactions */
				}
			FreeTransaction(trans);
			}else
			{
				printf("Can't alloc transaction.\n");
			}
		LoseEntity(hisentity);
		}else
		{
			printf("Can't find server program, error code %ld.\n", error);
		}
		DeleteEntity(myentity);
	}else
	{
		printf("Can't create entity.\n");
	}
	CloseLibrary(NIPCBase);
	exit(rc);
}
